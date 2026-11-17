// SPDX-License-Identifier: GPL-3.0-only
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');const assert=require('node:assert/strict'),fs=require('node:fs');
(async()=>{const browser=await chromium.launch({headless:true,args:['--no-sandbox'],...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{})});try{
 const page=await browser.newPage({viewport:{width:1000,height:900}}),errors=[];page.on('pageerror',e=>errors.push(e.message));
 await page.goto(new URL('listening/clarity/',(process.env.BGIRL_URL||'http://127.0.0.1:8794')+'/').href);assert.equal(await page.locator('audio').count(),26);
 for(const name of ['dry','whole-12','whole-24','blend-24','grain-24']) {
  const a=page.locator(`audio[src="${name}-matched.wav"]`);await a.click({position:{x:20,y:25}});
  await page.waitForFunction(name=>{const a=document.querySelector(`audio[src="${name}-matched.wav"]`);return a.currentTime>0&&a.duration===12;},name);
  await a.evaluate(a=>{a.currentTime=6;});
  await page.waitForFunction(name=>{const a=document.querySelector(`audio[src="${name}-matched.wav"]`);return a.currentTime>=6&&!a.seeking;},name);
  await a.evaluate(a=>a.pause());
 }
 fs.mkdirSync('local/clarity/browser',{recursive:true});await page.screenshot({path:'local/clarity/browser/gallery.png',fullPage:true});
 await page.setViewportSize({width:390,height:844});assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),false);
 await page.screenshot({path:'local/clarity/browser/gallery-mobile.png',fullPage:true});assert.deepEqual(errors,[]);
 console.log('gallery:26 players,5 matched WAVs actually play/seekable12s, desktop/mobile no overflow/page errors PASS; human listening pending');
}finally{await browser.close();}})().catch(e=>{console.error(e);process.exitCode=1;});
