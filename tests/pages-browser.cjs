const { chromium } = require(process.env.PLAYWRIGHT_MODULE || "playwright");
const assert = require("node:assert/strict");
const url = process.env.BGIRL_PAGES_URL || "http://127.0.0.1:8772/bgirl-fm1/";
(async () => {
  const { semantic } = await import('../felucca/web/emu/song-snapshot.mjs');
  const ticker = require('../felucca/web/emu/demos/ticker.bgirl.json');
  const b = await chromium.launch({
    executablePath: process.env.CHROMIUM_PATH || undefined,
    args: ["--no-sandbox"],
  });
  try {
    for (const mobile of [false, true]) {
      const p = await b.newPage({
        viewport: mobile
          ? { width: 390, height: 844 }
          : { width: 1366, height: 900 },
        hasTouch: mobile,
        isMobile: mobile,
      });
      const errors = [];
      p.on("pageerror", (e) => errors.push(e.message));
      p.on("response", (r) => {
        if (r.url().startsWith(url) && r.status() >= 400)
          errors.push(r.url() + ": " + r.status());
      });
      await p.addInitScript(() => {
        const AW = window.AudioWorkletNode;
        window.AudioWorkletNode = class extends AW {
          constructor(ctx, ...args) {
            super(ctx, ...args);
            window.instrumentPort = this.port;
            window.meter = ctx.createAnalyser();
            this.connect(meter);
          }
        };
      });
      await p.goto(url, { waitUntil: "domcontentloaded" });
      assert.match(
        await p.locator(".hero-copy").textContent(),
        /cybernetic b-boys and b-girls/,
      );
      assert.equal(
        await p
          .locator(".foot-credits a")
          .filter({ hasText: "Hortator’s browser simulator" })
          .count(),
        1,
      );
      await p.evaluate(async () => {
        for (const spec of [
          "16px Inter Tight",
          "500 12px Chakra Petch",
          "32px Michroma",
        ])
          if (!(await document.fonts.load(spec)).length) throw Error(spec);
      });
      await p.waitForFunction(() => document.querySelector("#device canvas"));
      const transform = () =>
        p.locator("#tilt").evaluate((e) => getComputedStyle(e).transform);
      const before = await transform();
      await p.locator('a.cta[href="#play"]').first().click();
      await p.waitForFunction(() => location.hash === "#play");
      await p.waitForTimeout(750);
      assert.notEqual(await transform(), before);
      await p.locator("#power-on").click();
      await p
        .waitForFunction(() => window.meter, null, { timeout: 3000 })
        .catch(async (e) => {
          console.log(await p.locator("#power-note").textContent(), errors);
          throw e;
        });
      await p.waitForFunction(
        () => document.querySelector("#power-note").hidden,
      );
      await p.waitForTimeout(650);
      const loaded = await p.evaluate(() => new Promise((resolve, reject) => {
        const port = window.instrumentPort;
        const listener = ({data}) => {
          if (data.type !== "song-result" || data.id !== "featured-test") return;
          clearTimeout(timer);
          port.removeEventListener("message", listener);
          data.error ? reject(Error(data.error)) : resolve(data.doc);
        };
        const timer = setTimeout(() => reject(Error("Featured song capture timed out")), 5000);
        port.addEventListener("message", listener);
        port.postMessage({type:"song-capture", id:"featured-test", deadline:Date.now()+5000});
      }));
      assert.equal(loaded.title, "Ticker");
      assert.deepEqual(semantic(loaded), semantic(ticker));
      if (mobile) await p.locator('[data-ctl="btn:10"]').tap();
      else await p.keyboard.press("Space", { delay: 100 });
      await p.waitForFunction(() => {
        const a = new Float32Array(meter.fftSize);
        meter.getFloatTimeDomainData(a);
        return a.some((x) => Math.abs(x) > 0.001);
      });
      await p.locator('#manual li[data-ctl="MASTER"]').focus();
      assert.match(await p.locator("#map-caption").innerText(), /MASTER/);
      assert(await p.locator("#manual-mini .on").count());
      assert.equal(await p.locator("#features,#install").count(), 0);
      assert((await p.request.get(new URL("play/", url).href)).ok());
      assert.deepEqual(errors, []);
      await p.close();
    }
  } finally {
    await b.close();
  }
  console.log(
    "Desktop and phone viewport: reveal, audio, manual highlight, studio route; no page errors or failed local resources.",
  );
})().catch((e) => {
  console.error(e);
  process.exitCode = 1;
});
