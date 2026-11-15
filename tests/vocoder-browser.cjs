// SPDX-License-Identifier: GPL-3.0-only
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict'), fs=require('node:fs');
(async()=>{
 const browser=await chromium.launch({headless:true,args:['--no-sandbox'],...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{})});
 try{
  const page=await browser.newPage({viewport:{width:1300,height:1100}}),errors=[];
  page.on('pageerror',e=>errors.push(e.message));
  await page.addInitScript(()=>{
   const AC=window.AudioContext;window.AudioContext=class extends AC{constructor(o){super(o);window.testAC=this;}};
   const AW=window.AudioWorkletNode;window.AudioWorkletNode=class extends AW{constructor(...a){super(...a);window.testNode=this;}};
  });
  await page.goto(process.env.BGIRL_URL||'http://127.0.0.1:8771');
  await page.getByRole('button',{name:'Switch on',exact:true}).click();
  await page.waitForFunction(()=>window.testAC?.state==='running');await page.waitForTimeout(1300);
  await page.evaluate(()=>{window.meter=testAC.createAnalyser();meter.fftSize=4096;testNode.connect(meter);});
  const peak=()=>page.evaluate(()=>{let a=new Float32Array(4096);meter.getFloatTimeDomainData(a);return Math.max(...a.map(Math.abs));});
  const tap=async name=>{await page.getByRole('button',{name,exact:true}).click({delay:90});await page.waitForTimeout(name==='FX'?650:160);};
  const peaks={};
  for(const id of ['speech-dry','speech-8','speech-12']){
   page.once('dialog',d=>d.accept());await page.locator('#'+id).click();await page.waitForTimeout(200);
   await tap('PLAY / STOP');peaks[id]=0;for(let i=0;i<12;i++){await page.waitForTimeout(80);peaks[id]=Math.max(peaks[id],await peak());}assert(peaks[id]>.00001&&peaks[id]<=1,`${id}: peak ${peaks[id]}`);
   await tap('PLAY / STOP');await page.waitForTimeout(1000);
  }
  // Real FX family pages and four-knob control, not a JavaScript alternate synth.
  await tap('FX'); // FX
  await tap('FX'); // VOCODER, FX page2
  fs.mkdirSync('tmp/vocoder-browser',{recursive:true});
  await page.screenshot({path:'tmp/vocoder-browser/routing.png',fullPage:true});
  await tap('FX'); // VOC INPUT
  const before=await page.locator('#screen').evaluate(c=>c.toDataURL());
  await page.getByRole('slider',{name:'KNOB 1',exact:true}).press('ArrowUp');await page.waitForTimeout(180);
  const after=await page.locator('#screen').evaluate(c=>c.toDataURL());assert(before!==after,"panel knob changes VOC INPUT screen");
  await page.screenshot({path:'tmp/vocoder-browser/freeze.png',fullPage:true});
  for(const name of ['dry','8-band','12-band','12-consonants','12-chorus']){
   const r=await page.request.get(new URL('listening/'+name+'.flac',page.url()).href);assert.equal(r.status(),200);assert((await r.body()).length>1000);
  }
  await page.setViewportSize({width:390,height:844});await page.waitForTimeout(150);
  assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),false);
  await page.screenshot({path:'tmp/vocoder-browser/mobile.png',fullPage:true});
  assert.deepEqual(errors,[]);const result={peaks,normalPanelKnobChangedScreen:true,referenceFiles:5,pageErrors:errors,humanListening:'pending'};
  fs.writeFileSync('tmp/vocoder-browser/results.json',JSON.stringify(result,null,2)+'\n');console.log(result);
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
