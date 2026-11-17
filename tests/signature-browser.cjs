// SPDX-License-Identifier: GPL-3.0-only
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict');
const fs=require('node:fs');
(async()=>{
 const browser=await chromium.launch({headless:true,args:['--no-sandbox'],...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{})});
 try {
  const page=await browser.newPage({viewport:{width:1300,height:1000}}),errors=[];page.on('pageerror',e=>errors.push(e.message));
  await page.addInitScript(()=>{
   const AC=AudioContext;window.AudioContext=class extends AC{constructor(o){super(o);window.testAC=this;}};
   const AW=AudioWorkletNode;window.AudioWorkletNode=class extends AW{constructor(...a){super(...a);window.testNode=this;}};
  });
  const url=process.env.BGIRL_URL||'http://127.0.0.1:8786/';
  await page.goto(url);await page.locator('#go').click();await page.waitForTimeout(1500);
  const state=()=>page.evaluate(()=>new Promise(resolve=>{
   const f=e=>{if(e.data.type==='signature-info'){testNode.port.removeEventListener('message',f);resolve(e.data.tracks);}};
   testNode.port.addEventListener('message',f);testNode.port.postMessage({type:'signature-info'});
  }));
  const initial=await state();assert.equal(initial[0][1],2);assert.equal(initial[3][0],13);
  const project=async(id,accept=true)=>{page.once('dialog',d=>accept?d.accept():d.dismiss());await page.locator(id).click();await page.waitForTimeout(400);};
  await project('#signature-a',false);assert.deepEqual(await state(),initial);
  await project('#signature-blank');let a=await state();assert(a.every(t=>t[5]===0 && t[6]===0));
  await page.evaluate(()=>{window.meter=testAC.createAnalyser();meter.fftSize=4096;testNode.connect(meter);});
  const peak=()=>page.evaluate(()=>{const a=new Float32Array(4096);meter.getFloatTimeDomainData(a);return Math.max(...a.map(Math.abs));});
  const engines=[12,12,10,10,10,10,10,12,12,12],presets=[8,9,4,5,6,7,8,10,11,12];
  for(let i=0;i<10;i++) {
   await page.locator('#signature-sound').selectOption({index:i});await page.locator('#signature-load').click();await page.waitForTimeout(250);
   a=await state();assert.equal(a[0][0],engines[i]);assert.equal(a[0][1],presets[i]);
   await page.evaluate(()=>testNode.port.postMessage({type:'midi',data:[0x90,60,100]}));await page.waitForTimeout(35);
   assert((await peak())>.00001,`audible selected sound ${i}`);
   await page.evaluate(()=>testNode.port.postMessage({type:'midi',data:[0x80,60,0]}));await page.waitForTimeout(1200);
  }
  // The normal FM6 group starts with BG originals; labels do not change IDs.
  await page.locator('#signature-sound').selectOption({index:0});await page.locator('#signature-load').click();await page.waitForTimeout(250);
  for(const preset of [9,10,11,12,0,1,2,3,4,5,6,7]) {
   await page.getByRole('slider',{name:'PRESETS',exact:true}).press('ArrowUp');await page.waitForTimeout(180);
   a=await state();assert.equal(a[0][0],12);assert.equal(a[0][1],preset);
  }
  const demoPeaks=[];
  for(const id of ['#signature-a','#signature-b','#signature-choke']) {
   await project(id);a=await state();assert(a.every(t=>t[6]===0));assert(a.some(t=>t[5]>0));
   await page.getByRole('button',{name:'PLAY / STOP',exact:true}).click({delay:90});await page.waitForTimeout(1500);
   const p=await peak();assert(p>.001 && p<.99);demoPeaks.push(p);
   await page.getByRole('button',{name:'PLAY / STOP',exact:true}).click({delay:90});await page.waitForTimeout(2500);assert((await peak())<.0001);
  }
  // Normal panel PRESETS browsing still loads bank members; no special user slot.
  await project('#signature-blank');await page.getByRole('slider',{name:'PRESETS',exact:true}).press('ArrowUp');await page.waitForTimeout(200);
  a=await state();assert.equal(a[0][0],10);assert.equal(a[0][1],5);
  // Normal save and reload, then signature project operations preserve the saved flash.
  const tap=async name=>{await page.getByRole('button',{name,exact:true}).click({delay:90});await page.waitForTimeout(180);};
  await tap('SAVE');await tap('OCT+');await tap('OCT+');
  await page.waitForFunction(()=>Object.keys(JSON.parse(localStorage.getItem('bgirl-felucca-v1-flash')||'{}')).length>0);
  const saved=await page.evaluate(()=>localStorage.getItem('bgirl-felucca-v1-flash'));
  await project('#signature-a');assert.equal(await page.evaluate(()=>localStorage.getItem('bgirl-felucca-v1-flash')),saved);
  await page.reload();await page.locator('#go').click();await page.waitForTimeout(1500);
  assert(await page.evaluate(()=>!!localStorage.getItem('bgirl-felucca-v1-flash')));
  await project('#demo');a=await state();assert.equal(a[3][0],13);assert.equal(a[0][1],2);
  const dir=process.env.BGIRL_ARTIFACTS||'local/signature/browser';fs.mkdirSync(dir,{recursive:true});
  await page.screenshot({path:dir+'/desktop.png',fullPage:true});
  await page.setViewportSize({width:390,height:844});await page.waitForTimeout(300);
  assert(await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth+1));
  await project('#signature-b');await page.screenshot({path:dir+'/mobile.png',fullPage:true});
  assert.deepEqual(errors,[]);console.log({sounds:10,fm6BGFirst:true,confirmCancel:true,templatesStopped:true,demoPeaks,panelSelection:true,saveReload:true,vocalDemoPreserved:true,mobile:true,pageErrors:errors});
 } finally {await browser.close();}
})().catch(e=>{console.error(e);process.exit(1);});
