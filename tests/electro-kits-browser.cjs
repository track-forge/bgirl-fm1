// SPDX-License-Identifier: GPL-3.0-only
const assert = require('node:assert/strict');
const { chromium } = require(process.env.PLAYWRIGHT_MODULE || 'playwright');

(async () => {
  const browser = await chromium.launch({ args: ['--no-sandbox'], executablePath: process.env.CHROMIUM_PATH });
  try {
    const page = await browser.newPage();
    const errors = [];
    page.on('pageerror', error => errors.push(error.message));
    await page.addInitScript(() => {
      const AC = window.AudioContext;
      const AW = window.AudioWorkletNode;
      window.AudioContext = class extends AC {
        constructor(o) { super(o); window.testAC = this; }
      };
      window.AudioWorkletNode = class extends AW {
        constructor(...args) {
          super(...args);
          window.testNode = this;
          window.analyser = testAC.createAnalyser();
          analyser.fftSize = 4096;
          this.connect(analyser);
        }
      };
    });
    await page.goto(process.env.BGIRL_URL || 'http://127.0.0.1:8837');
    const boot = async () => {
      await page.locator('#go').click();
      await page.waitForFunction(() => document.querySelector('#start').hidden);
      await page.waitForTimeout(1300);
    };
    await boot();
    const capture = () => page.evaluate(() => new Promise((resolve, reject) => {
      const id = Math.random();
      const listener = ({ data }) => {
        if (data.type !== 'song-result' || data.id !== id) return;
        clearTimeout(timer);
        testNode.port.removeEventListener('message', listener);
        if (data.error) reject(Error(data.error));
        else resolve(data.doc);
      };
      const timer = setTimeout(() => reject(Error('capture timeout')), 4000);
      testNode.port.addEventListener('message', listener);
      testNode.port.postMessage({ type: 'song-capture', deadline: Date.now() + 4000, id });
    }));
    const midi = (...data) => page.evaluate(data => testNode.port.postMessage({ type: 'midi', data }), data);
    const peak = () => page.evaluate(() => {
      const a = new Float32Array(analyser.fftSize);
      analyser.getFloatTimeDomainData(a);
      return Math.max(...a.map(Math.abs));
    });
    const play = () => page.getByRole('button', { name: 'PLAY / STOP', exact: true }).click({ delay: 90 });
    const readKit = async () => (await capture()).song.tracks[0].parameters.P_E0;
    assert.equal(await readKit(), 19);
    const original = await capture();
    const peaks = [];
    for (let pc = 16; pc <= 18; pc++) {
      await midi(0xc0, pc, 0);
      await page.waitForTimeout(260);
      const current = await capture();
      assert.equal(current.song.tracks[0].parameters.P_E0, pc + 3);
      assert.deepEqual(current.song.tracks.map(t => t.steps), original.song.tracks.map(t => t.steps));
      await midi(0x90, 36, 105);
      await page.waitForTimeout(76);
      const kick = await peak();
      assert(kick > .005 && kick < .99, `kick peak ${kick}`);
      await midi(0x80, 36, 0);
      await play();
      await page.waitForTimeout(660);
      const loop = await peak();
      assert(loop > .005 && loop < .99, `sequence peak ${loop}`);
      peaks.push({ kit: pc + 3, kick, loop });
      await play();
      await page.waitForTimeout(350);
    }
    await page.waitForFunction(() => Object.keys(JSON.parse(localStorage.getItem("bgirl-felucca-v1-flash") || "{}")).length > 0, null, { timeout: 35000 });
    await page.reload();
    await boot();
    assert.equal(await readKit(), 21);
    assert.deepEqual((await capture()).song.tracks.map(t => t.steps), original.song.tracks.map(t => t.steps));
    // Real key press, not MIDI, on a quiet blank project.
    page.once('dialog', dialog => dialog.accept());
    await page.locator('#blank').click();
    await page.waitForFunction(() => document.querySelector('#project-status').textContent.startsWith('Loaded'));
    assert.equal(await readKit(), 19);
    await page.locator('h1').click();
    await page.keyboard.down('z');
    await page.waitForTimeout(65);
    assert((await peak()) > .001);
    await page.keyboard.up('z');
    await page.waitForTimeout(1700);
    assert((await peak()) < .0001);
    assert.deepEqual(errors, []);
    console.log('Chromium: MIDI kit switches preserve pattern, audio + normal start/stop, autosave/refresh, blank default and live key release PASS', peaks);
  } finally {
    await browser.close();
  }
})().catch(error => { console.error(error); process.exitCode = 1; });
