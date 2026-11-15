/* SPDX-License-Identifier: MPL-2.0
 * Copyright (C) 2026 Track Forge */
// Optional local integration check: serve app/build/emu first. Requires Playwright Chromium.
const { chromium } = require(process.env.PLAYWRIGHT_MODULE || 'playwright');
const assert = require('node:assert/strict');
const fs = require('node:fs');
(async () => {
  const browser = await chromium.launch({
    ...(process.env.CHROMIUM_PATH ? { executablePath: process.env.CHROMIUM_PATH } : {}),
    headless: true, args: ['--no-sandbox']
  });
  try {
    const page = await browser.newPage({ viewport: { width: 1300, height: 1150 } });
    const errors = []; page.on('pageerror', e => errors.push(e.message));
    // Observe actual AudioWorklet output without changing production DSP or controls.
    await page.addInitScript(() => {
      const AC = window.AudioContext;
      window.AudioContext = class extends AC { constructor(o) { super(o); window.testAC = this; } };
    });
    await page.goto(process.env.BGIRL_URL || 'http://127.0.0.1:8765');
    assert.match(await page.title(), /BGIRL/);
    await page.locator('#go').click();
    await page.waitForFunction(() => document.querySelector('#status').textContent.startsWith('On.'));
    await page.evaluate(() => {
      window.analyser = testAC.createAnalyser(); analyser.fftSize = 4096;
      node.connect(analyser);
    });
    const peak = () => page.evaluate(() => {
      const b = new Float32Array(4096); analyser.getFloatTimeDomainData(b);
      return Math.max(...b.map(Math.abs));
    });
    const control = name => page.getByRole('button', { name: new RegExp('^' + name + '(?:,|$)') });
    const tap = name => control(name).press('Space', { delay: 90 });
    const turn = (name, key) => control(name).press(key);
    const wait = ms => page.waitForTimeout(ms);
    await wait(400); assert.equal(await peak(), 0);
    const key = control('White key 5');
    await key.focus(); await page.keyboard.down('Space'); await wait(100);
    const bassPeak = await peak(); assert(bassPeak > .001);
    await page.keyboard.up('Space'); await wait(500); const releasedPeak = await peak();
    assert(releasedPeak < .0001);
    await tap('ARP'); await wait(400); const groovePeak = await peak(); assert(groovePeak > .005);
    await turn('KNOB 1', 'ArrowUp'); await wait(100);
    assert.match(await page.locator('#live-a').textContent(), /TEMPO.*121/);
    await tap('FX'); await wait(150);
    assert.match(await page.locator('#live-a').textContent(), /DRUMS.*MUTED/);
    await tap('FX'); await wait(150);
    assert.match(await page.locator('#live-a').textContent(), /DRUMS.*ON/);
    await turn('KNOB 2', 'ArrowUp'); await wait(100);
    assert.match(await page.locator('#live-a').textContent(), /TONE.*46/);
    await turn('PRESETS', 'ArrowUp'); await wait(100);
    assert.match(await page.locator('#live-a').textContent(), /PART.*VOICE/);
    // Stop groove before measuring isolated experimental voice.
    await tap('ARP'); await wait(300);
    await key.focus(); await page.keyboard.down('Space'); await wait(550);
    const voicePeak = await peak(); assert(voicePeak > .001);
    await page.keyboard.up('Space'); await wait(500);
    assert((await peak()) < .0001);
    // Panic while a physical keyboard key is held.
    await key.focus(); await page.keyboard.down('Space'); await wait(200);
    await control('HOME').click({ delay: 90 });
    await page.keyboard.up('Space'); await wait(300);
    const panicPeak = await peak(); assert(panicPeak < .0001);
    await turn('PRESETS', 'ArrowDown'); await tap('ARP'); await wait(800);
    const artifacts = process.env.BGIRL_ARTIFACTS || 'app/build/browser';
    fs.mkdirSync(artifacts, { recursive: true });
    await page.screenshot({ path: `${artifacts}/desktop.png`, fullPage: true });
    await page.setViewportSize({ width: 390, height: 844 });
    await page.screenshot({ path: `${artifacts}/mobile.png`, fullPage: true });
    assert.equal(await page.evaluate(() => document.documentElement.scrollWidth > innerWidth), false);
    assert.deepEqual(errors, []);
    const result = { status: await page.locator('#status').textContent(), bassPeak, releasedPeak,
      groovePeak, voicePeak, panicPeak, errors, listening: 'Human audition NOT performed' };
    fs.writeFileSync(`${artifacts}/results.json`, JSON.stringify(result, null, 2) + '\n');
    console.log(result);
  } finally { await browser.close(); }
})().catch(e => { console.error(e); process.exitCode = 1; });
