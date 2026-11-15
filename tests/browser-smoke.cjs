// SPDX-License-Identifier: GPL-3.0-only
// Real Chromium AudioWorklet + inherited panel. No connected MIDI device needed.
const { chromium } = require(process.env.PLAYWRIGHT_MODULE || 'playwright');
const assert = require('node:assert/strict');
const fs = require('node:fs');
(async () => {
  const browser = await chromium.launch({ headless: true, args: ['--no-sandbox'],
    ...(process.env.CHROMIUM_PATH ? { executablePath: process.env.CHROMIUM_PATH } : {}) });
  try {
    const page = await browser.newPage({ viewport: { width: 1300, height: 1000 } });
    const errors = []; page.on('pageerror', e => errors.push(e.message));
    await page.addInitScript(() => {
      const AC = window.AudioContext;
      window.AudioContext = class extends AC { constructor(o) { super(o); window.testAC = this; } };
      const AW = window.AudioWorkletNode;
      window.AudioWorkletNode = class extends AW { constructor(...a) { super(...a); window.testNode = this; } };
    });
    await page.goto(process.env.BGIRL_URL || 'http://127.0.0.1:8767');
    assert.match(await page.title(), /BGIRL/);
    assert.match(await page.locator('main').innerText(), /vocoder electro demo/);
    const on = async () => { await page.locator('#go').click(); await page.waitForFunction(() => document.querySelector('#start').hidden); await page.waitForTimeout(1100); };
    await on();
    const listen = () => page.evaluate(() => { window.analyser = testAC.createAnalyser(); analyser.fftSize = 4096; testNode.connect(analyser); });
    await listen();
    const peak = () => page.evaluate(() => { const a = new Float32Array(4096); analyser.getFloatTimeDomainData(a); return Math.max(...a.map(Math.abs)); });
    const tap = async name => { await page.getByRole('button', { name, exact: true }).click({ delay: 90 }); await page.waitForTimeout(180); };
    assert.equal(await peak(), 0);
    await tap('PLAY / STOP'); await page.waitForTimeout(700); const demoPeak = await peak(); assert(demoPeak > .001 && demoPeak <= 1);
    const artifacts = process.env.BGIRL_ARTIFACTS || 'tmp/browser'; fs.mkdirSync(artifacts, { recursive: true });
    await page.screenshot({ path: `${artifacts}/desktop.png`, fullPage: true });
    // Fresh startup already features the vocoder, without speech-lab loading.
    for(let i=0;i<3;i++) await page.getByRole('slider',{name:'ALGORITHM',exact:true}).press('ArrowUp');
    await page.waitForTimeout(200);
    await tap('FX'); await page.waitForTimeout(650);
    await tap('FX'); await page.waitForTimeout(650);
    await page.screenshot({ path: `${artifacts}/default-vocoder.png`, fullPage: true });
    await tap('HOME');
    for(let i=0;i<3;i++) await page.getByRole('slider',{name:'ALGORITHM',exact:true}).press('ArrowDown');
    await page.waitForTimeout(200);
    await tap('PLAY / STOP'); await page.waitForTimeout(4500); assert((await peak()) < .0001);
    const screen = () => page.locator('#screen').evaluate(c => c.toDataURL());
    const screens = [await screen()];
    for (let i = 0; i < 3; i++) {
      await page.getByRole('slider', { name: 'ALGORITHM', exact: true }).press('ArrowUp');
      await page.waitForTimeout(100); screens.push(await screen());
      await page.locator('h1').click(); await page.keyboard.down('KeyZ'); await page.waitForTimeout(150); if(i===0) assert((await peak()) > .00001); // bass is directly monitored; pad/speech need each other
      await page.keyboard.up('KeyZ'); await page.waitForTimeout(3500);
    }
    assert.equal(new Set(screens).size, 4);
    // Save an edited speech sound using the inherited SAVE panel confirmation.
    await page.getByRole('slider', { name: 'KNOB 1', exact: true }).press('ArrowUp');
    await tap('SAVE'); await tap('OCT+'); await tap('OCT+');
    await page.waitForFunction(() => Object.keys(JSON.parse(localStorage.getItem('bgirl-felucca-v1-flash') || '{}')).length > 0);
    const stored = await page.evaluate(() => localStorage.getItem('bgirl-felucca-v1-flash'));
    await page.reload(); await on(); await listen();
    assert(await page.evaluate(() => !!localStorage.getItem('bgirl-felucca-v1-flash')));
    // User confirmation: cancel does nothing, accept loads stopped blank/demo without erasing slots.
    page.once('dialog', d => d.dismiss()); await page.locator('#blank').click();
    assert.equal(await page.locator('#project-status').textContent(), '');
    page.once('dialog', d => d.accept()); await page.locator('#blank').click();
    await page.waitForFunction(() => document.querySelector('#project-status').textContent.startsWith('Loaded'));
    await page.waitForTimeout(3500); assert((await peak()) < .0001);
    assert.equal(await page.evaluate(() => localStorage.getItem('bgirl-felucca-v1-flash')), stored);
    page.once('dialog', d => d.accept()); await page.locator('#demo').click(); await page.waitForTimeout(300);
    assert((await peak()) < .0001); await tap('PLAY / STOP'); await page.waitForTimeout(700); assert((await peak()) > .001);
    await page.setViewportSize({ width: 390, height: 844 }); await page.waitForTimeout(300);
    await page.screenshot({ path: `${artifacts}/mobile.png`, fullPage: true });
    assert.equal(await page.evaluate(() => document.documentElement.scrollWidth > innerWidth), false);
    await tap('PLAY / STOP');
    assert.deepEqual(errors, []);
    const result = {demoPeak, distinctTrackScreens: new Set(screens).size, savedSectors: Object.keys(JSON.parse(stored)).length, errors,
      humanListening: 'not performed', physicalMidiClock: 'not tested'};
    fs.writeFileSync(`${artifacts}/results.json`, JSON.stringify(result, null, 2)+'\n'); console.log(result);
  } finally { await browser.close(); }
})().catch(e => { console.error(e); process.exitCode = 1; });
