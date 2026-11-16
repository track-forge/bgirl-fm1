// SPDX-License-Identifier: GPL-3.0-only
// Real browser display, responsive target bounds and keyboard focus evidence.
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict'),fs=require('node:fs');
(async()=>{
 const browser=await chromium.launch({headless:true,args:['--no-sandbox'],...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{})});
 try {
  const page=await browser.newPage({viewport:{width:1300,height:1000}}),errors=[];
  page.on('pageerror',e=>errors.push(e.message));
  await page.goto(process.env.BGIRL_URL||'http://127.0.0.1:8781');
  await page.locator('#go').click();await page.waitForFunction(()=>document.querySelector('#start').hidden);await page.waitForTimeout(1200);await page.evaluate(()=>document.fonts.ready);
  const out=process.env.BGIRL_ARTIFACTS||'tmp/battle-browser';fs.mkdirSync(out,{recursive:true});
  const capture=async name=>{
   const canvas=await page.locator('#screen').evaluate(c=>c.toDataURL().split(',')[1]);
   fs.writeFileSync(`${out}/${name}-native.png`,Buffer.from(canvas,'base64'));
  };
  const layouts=[];
  for(const [name,width,height] of [['desktop',1300,1000],['portrait',390,844]]) {
   await page.setViewportSize({width,height});await page.waitForTimeout(350);
   assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),false);
   const targets=await page.locator('.hit').evaluateAll(elements=>elements.map(e=>{
    const r=e.getBoundingClientRect(),d=document.querySelector('#device').getBoundingClientRect();
    return {name:e.getAttribute('aria-label'),width:r.width,height:r.height,inside:r.left>=d.left&&r.right<=d.right&&r.top>=d.top&&r.bottom<=d.bottom};
   }));
   assert.equal(await page.locator('#circuit-mark').count(),0);
   assert(!/BG[–-]01\s*\/\s*BATTLE CIRCUIT/i.test(await page.locator('main').innerText()));
   assert.equal(targets.length,49);assert(targets.every(t=>t.inside&&t.width>14&&t.height>14));
   const knob=page.getByRole('slider',{name:'KNOB 1',exact:true});
   await knob.focus();await knob.press('ArrowUp');await page.waitForTimeout(150);
   assert(await knob.evaluate(e=>e===document.activeElement&&getComputedStyle(e).outlineStyle!=='none'));
   await page.screenshot({path:`${out}/${name}.png`,fullPage:true});await capture(name);
   layouts.push({name,targetCount:targets.length,minWidth:Math.min(...targets.map(t=>t.width)),minHeight:Math.min(...targets.map(t=>t.height))});
  }
  // A genuine synth envelope and sequencer view, not a static design mockup.
  const tap=async name=>{await page.getByRole('button',{name,exact:true}).click({delay:90});await page.waitForTimeout(400);};
  await page.getByRole('slider',{name:'ALGORITHM',exact:true}).press('ArrowUp');await page.waitForTimeout(150);
  await tap('ENV');await capture('synth');await tap('SEQ');await capture('sequencer');
  assert.deepEqual(errors,[]);const result={layouts,errors,physicalDisplay:'not tested'};
  fs.writeFileSync(`${out}/results.json`,JSON.stringify(result,null,2)+'\n');console.log(result);
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
