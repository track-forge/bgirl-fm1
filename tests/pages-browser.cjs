// SPDX-License-Identifier: GPL-3.0-only
const {chromium} = require(process.env.PLAYWRIGHT_MODULE || 'playwright');
const assert = require('node:assert/strict');

(async () => {
  const browser = await chromium.launch({args:['--no-sandbox'],
    executablePath:process.env.CHROMIUM_PATH || undefined});
  const url = process.env.BGIRL_PAGES_URL || 'http://127.0.0.1:8772/bgirl-fm1/';
  try {
    for (const touch of [false,true]) {
      const page = await browser.newPage({hasTouch:touch, isMobile:touch,
        viewport:touch ? {width:390,height:844} : {width:1200,height:900},
        reducedMotion:touch ? 'reduce' : 'no-preference'});
      const errors = [];
      page.on('pageerror',error => errors.push(error.message));
      await page.addInitScript(() => {
        const Node = window.AudioWorkletNode;
        window.AudioWorkletNode = class extends Node {
          constructor(context,...args) {
            super(context,...args);
            window.meter = context.createAnalyser();
            meter.fftSize = 4096;
            this.connect(meter);
          }
        };
      });
      await page.goto(url);
      await page.locator('#preview').evaluate(img => img.decode());
      assert.equal(await page.evaluate(() => !!window.meter),false);
      if (!touch && await page.locator('#skip').isVisible()) await page.locator('#skip').click();
      const storage = await page.evaluate(() => {
        localStorage.setItem('bgirl-felucca-v1-flash','{"sentinel":true}');
        return JSON.stringify(localStorage);
      });
      await page.locator('#power-on').click();
      await page.waitForFunction(() => !document.querySelector('#screen').hidden);
      const peak = () => page.evaluate(() => {
        const samples = new Float32Array(meter.fftSize);
        meter.getFloatTimeDomainData(samples);
        return Math.max(...samples.map(Math.abs));
      });
      const touchSession = touch ? await page.context().newCDPSession(page) : null;
      async function play(key, label) {
        if (touch) {
          const control = page.getByRole('button',{name:label,exact:true});
          await control.scrollIntoViewIfNeeded();
          const {x,y,width,height} = await control.boundingBox();
          await touchSession.send('Input.dispatchTouchEvent',{type:'touchStart',
            touchPoints:[{x:x+width/2,y:y+height-14}]});
        } else await page.keyboard.down(key);
        await page.waitForFunction(() => {
          const samples = new Float32Array(meter.fftSize);
          meter.getFloatTimeDomainData(samples);
          return samples.some(sample => Math.abs(sample) > .001);
        },null,{timeout:4000});
        assert(await peak() <= 1);
        if (touch) await touchSession.send('Input.dispatchTouchEvent',{type:'touchEnd',touchPoints:[]});
        else await page.keyboard.up(key);
        await page.waitForFunction(() => {
          const samples = new Float32Array(meter.fftSize);
          meter.getFloatTimeDomainData(samples);
          return samples.every(sample => Math.abs(sample) < .0001);
        },null,{timeout:4000});
      }
      await play('a','C · A');
      await play('s','D · S');
      await page.locator('#patch').selectOption('1');
      await page.locator('#patch').blur();
      await play('a','C · A');
      assert.equal(await page.evaluate(() => JSON.stringify(localStorage)),storage);
      await page.getByRole('link',{name:'Open full studio'}).click();
      assert.equal(page.url(),new URL('play/',url).href);
      assert.equal(await page.locator('#go').count(),1);
      assert.deepEqual(errors,[]);
      await page.close();
    }
    console.log('Pages: silent preview, keyboard and touch audio/release, patch change, isolated storage, studio link');
  } finally {
    await browser.close();
  }
})().catch(error => { console.error(error); process.exitCode=1; });
