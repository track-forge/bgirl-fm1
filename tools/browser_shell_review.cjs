// SPDX-License-Identifier: GPL-3.0-only
// Actual browser screenshot pairs. Start both review servers before running.
const { chromium } = require(process.env.PLAYWRIGHT_MODULE || 'playwright');
const fs = require('node:fs');
(async () => {
  const out = process.env.BGIRL_ARTIFACTS || 'docs/img/browser-shell';
  fs.mkdirSync(out, { recursive: true });
  const browser = await chromium.launch({ headless: true, args: ['--no-sandbox'],
    ...(process.env.CHROMIUM_PATH ? { executablePath: process.env.CHROMIUM_PATH } : {}) });
  try {
    for (const [label, url] of [
      ['before', process.env.BGIRL_BEFORE_URL || 'http://127.0.0.1:8812/'],
      ['after', process.env.BGIRL_URL || 'http://127.0.0.1:8814/bgirl-fm1/'],
    ]) {
      for (const [name, width, height] of [['desktop', 1300, 1000], ['mobile', 390, 844]]) {
        const page = await browser.newPage({ viewport: { width, height } });
        await page.goto(url);
        await page.evaluate(() => document.fonts.ready);
        await page.locator('#go').click();
        await page.waitForFunction(() => document.querySelector('#start').hidden);
        await page.waitForTimeout(1300);
        await page.evaluate(() => scrollTo(0, 0));
        await page.screenshot({ path: `${out}/${name}-${label}.png`, fullPage: true });
        await page.locator('#device').screenshot({ path: `${out}/${name}-panel-${label}.png` });
        await page.close();
      }
    }
  } finally { await browser.close(); }
})().catch(error => { console.error(error); process.exitCode = 1; });
