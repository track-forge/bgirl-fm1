// SPDX-License-Identifier: GPL-3.0-only
const {chromium} = require(process.env.PLAYWRIGHT_MODULE || 'playwright');
const assert = require('node:assert/strict');
const fs = require('node:fs');

(async () => {
  const browser = await chromium.launch({executablePath: process.env.CHROMIUM_PATH, args: ['--no-sandbox']});
  try {
    const page = await browser.newPage({viewport: {width: 1366, height: 950}});
    const errors = [];
    page.on('pageerror', error => errors.push(error.message));
    await page.addInitScript(() => {
      const AW = window.AudioWorkletNode;
      window.AudioWorkletNode = class extends AW {
        constructor(ctx, ...args) {
          super(ctx, ...args);
          window.probe = (type, params = {}) => new Promise(resolve => {
            const id = Math.random();
            const handle = e => {
              if (e.data.id !== id && e.data.type !== 'crew-info') return;
              this.port.removeEventListener('message', handle);
              resolve(e.data);
            };
            this.port.addEventListener('message', handle);
            this.port.start();
            this.port.postMessage({type, id, deadline: Date.now()+3000, ...params});
          });
          window.jamAudio = ctx.createMediaStreamDestination();
          this.connect(jamAudio);
        }
      };
    });
    await page.goto(process.env.BGIRL_PAGES_URL || 'http://127.0.0.1:8767/');
    assert.match(await page.locator('#m-choreo').innerText(), /drum fill/);
    await page.locator('a.cta[href="#play"]').first().click();
    await page.locator('#power-on').press('Enter');
    await page.waitForFunction(() => window.probe && document.querySelector('#power-note').hidden);
    const info = () => page.evaluate(async () => (await probe('crew-info')).values);
    const wait = async test => {
      const end = Date.now() + 8000;
      do { const q = await info(); if (test(q)) return q; } while (Date.now() < end);
      throw Error('Timed out waiting for panel state: '+JSON.stringify(await info()));
    };
    const snapshot = (type, doc) => page.evaluate(async ({type,doc}) => {
      const result = await probe(type,{doc});
      if (result.error) throw Error(result.error);
      return result.doc;
    }, {type,doc});
    const button = n => page.locator(`#device [data-ctl="btn:${n}"]`);
    const knob = n => page.getByRole("slider", {name:["SELECT","ALGORITHM","PRESETS","KNOB1","KNOB2","KNOB3","KNOB4"][n],exact:true});
    const tap = n => button(n).click({delay:85});
    const holdFX = () => button(0).click({button:'right'});
    const turn = async (n, direction = 1) => {
      await knob(n).hover();
      await page.keyboard.press(direction > 0 ? 'ArrowUp' : 'ArrowDown');
    };

    await wait(q => q[1] === 1);
    await holdFX();
    await wait(q => q[13] === 1 && q[1] === 1 && q[20] === 0);
    await tap(0);
    await wait(q => q[13] === 0 && q[1] === 1);
    await tap(0);
    await wait(q => q[1] === 0);
    await holdFX();
    await wait(q => q[20] === 1);
    await tap(0);
    await wait(q => q[20] === 0);
    await tap(6);
    await wait(q => q[1] === 1);

    const fixture = await snapshot('song-capture');
    const drum = fixture.song.tracks[0];
    assert.equal(drum.engine, 10);
    fixture.song.tracks.forEach((track,i) => { track.parameters.P_MUTE = Number(i !== 0); });
    drum.parameters.P_SLEN = 16;
    drum.steps[11] = {...drum.steps[0], hits: 3};
    drum.steps[14] = {...drum.steps[0], hits: 5};
    fixture.song.motion = {enabledTracks:1,events:[
      {track:0,step:0,parameter:'P_LEVEL',lock:false,value:92},
      {track:0,step:8,parameter:'P_LEVEL',lock:false,value:106},
    ]};
    await snapshot('song-import', fixture);
    await tap(10);
    await wait(q => q[6] && q[2] === 0);
    await holdFX();
    await wait(q => q[13] === 1);
    await turn(5);
    await wait(q => q[17] === 1);
    await turn(6,-1);
    await wait(q => q[18] < 8);
    await turn(1);
    await wait(q => q[2] === 1 && q[17] === 0 && q[18] === 8);
    await turn(1,-1);
    await wait(q => q[2] === 0 && q[17] === 1);
    await tap(0);
    await tap(11);
    await wait(q => q[19] === 1);
    await holdFX();
    await wait(q => q[13] === 1);

    // Capture the real LCD with the worklet's audio, not fabricated cue frames.
    await page.evaluate(() => {
      const stream = document.querySelector('#device canvas').captureStream(30);
      stream.addTrack(jamAudio.stream.getAudioTracks()[0]);
      const chunks = [];
      window.jamRecorder = new MediaRecorder(stream, {mimeType:'video/webm;codecs=vp8,opus'});
      jamRecorder.ondataavailable = e => chunks.push(e.data);
      window.jamVideo = new Promise(resolve => {
        jamRecorder.onstop = async () => resolve(Array.from(new Uint8Array(await new Blob(chunks).arrayBuffer())));
      });
      jamRecorder.start();
    });
    await knob(4).hover();
    await wait(q => q[14] === 11 && q[15] === 11);
    await page.keyboard.press('ArrowUp');
    await wait(q => q[16] === 2);
    await knob(3).hover();
    await wait(q => q[14] === 14 && q[15] === 14);
    await page.keyboard.press('ArrowUp');
    await wait(q => q[4] === 1);
    await tap(11);
    await wait(q => !q[19]);
    await tap(0);
    await wait(q => !q[13] && q[1]);
    const passes = [];
    for (let pass = 0; pass < 2; pass++) {
      const reset = await wait(q => q[14] === 0);
      const fill = await wait(q => q[14] === 11);
      const turnaround = await wait(q => q[14] === 14);
      assert.deepEqual([reset[4],reset[16],reset[21]], [0,1,92]);
      assert.deepEqual([fill[4],fill[16],fill[21]], [0,2,106]);
      assert.deepEqual([turnaround[4],turnaround[16],turnaround[21]], [1,2,106]);
      passes.push({fill:fill[14]+1,turnaround:turnaround[14]+1,levels:[reset[21],fill[21],turnaround[21]]});
    }
    const bytes = await page.evaluate(async () => { jamRecorder.stop(); return jamVideo; });
    if (process.env.BGIRL_JAM_CAPTURE) fs.writeFileSync(process.env.BGIRL_JAM_CAPTURE, Buffer.from(bytes));
    await tap(10);
    await wait(q => !q[6]);
    await page.waitForTimeout(150);
    const captured = await snapshot('song-capture');
    assert.deepEqual(captured.song.tracks, fixture.song.tracks);
    assert.deepEqual(captured.song.motion.events, [...fixture.song.motion.events,
      {track:0,step:11,parameter:'CREW_RATE',lock:false,value:2},
      {track:0,step:14,parameter:'CREW_MOVE',lock:false,value:1},
    ]);
    await snapshot('song-import', captured);
    assert.deepEqual((await snapshot('song-capture')).song.motion, captured.song.motion);
    const level = (await info())[21];
    await turn(3,-1);
    await wait(q => q[21] < level);
    await turn(4,-1);
    await page.waitForTimeout(150);
    const reshaped = await snapshot('song-capture');
    assert(reshaped.song.tracks[0].parameters.P_SGATE < drum.parameters.P_SGATE);

    // T4's VOC switch must not touch the other three vocoder settings.
    for (let i = 1; i <= 3; i++) {
      await turn(1);
      await wait(q => q[2] === i);
    }
    const voc = await info();
    assert(voc[23] > 0);
    await turn(4);
    const off = await wait(q => q[22] === 1);
    assert.deepEqual(off.slice(23,26), voc.slice(23,26));
    await turn(4,-1);
    await wait(q => q[22] === 0);
    assert.deepEqual(errors, []);
    console.log({passes, snapshotRoundTrip:true, audioEventsUnchanged:true, captureBytes:bytes.length});
  } finally { await browser.close(); }
})().catch(error => { console.error(error); process.exitCode = 1; });
