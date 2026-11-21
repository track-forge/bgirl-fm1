// SPDX-License-Identifier: GPL-3.0-only
// The factory demo must keep its four-bar arrangement without a private bank.
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict');
(async()=>{
 const browser=await chromium.launch({headless:true,...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{}),args:['--no-sandbox']});
 try {
  const page=await browser.newPage(),errors=[];page.on('pageerror',e=>errors.push(e.message));
  await page.addInitScript(()=>{
   const AC=AudioContext;window.AudioContext=class extends AC{constructor(o){super(o);window.testAC=this;}};
   const AW=AudioWorkletNode;window.AudioWorkletNode=class extends AW{constructor(...a){super(...a);window.testNode=this;}};
  });
  await page.goto(process.env.BGIRL_URL||'http://127.0.0.1:8774/');
  assert(await page.locator('#vocal-group').isDisabled());
  assert(await page.locator('#vocal-whole-dry').isDisabled());
  await page.getByRole('button',{name:'Switch on',exact:true}).click();
  await page.waitForFunction(()=>document.querySelector('#vocal-status').textContent.startsWith('Installed bank'));
  assert(await page.locator('#vocal-group').isEnabled());
  assert.equal(await page.locator('#vocal-group option:checked').innerText(),'SHOTS · 13 entries');
  assert(await page.locator('#vocal-whole-dry').isEnabled());
  await page.waitForTimeout(1300);
  const state=()=>page.evaluate(()=>new Promise(resolve=>{
   const f=e=>{if(e.data.type==='vocal-info'){testNode.port.removeEventListener('message',f);resolve(e.data);}};
   testNode.port.addEventListener('message',f);testNode.port.postMessage({type:'vocal-info'});
  }));
  const check=async()=>{
   const info=await state();
   assert.equal(info.name,'Miami Bass');
   assert.deepEqual(info.views,[{name:'SHOTS',count:13}]);
   assert.equal(info.state,4096+6,'VOCALS / SHOTS / START0 / matching bank');
   assert.match(await page.locator('#vocal-group option').first().innerText(),/^SHOTS/);
  };
  await check();
  page.once('dialog',d=>d.accept());await page.locator('#demo').click();await page.waitForTimeout(400);await check();
  await page.evaluate(()=>{window.meter=testAC.createAnalyser();meter.fftSize=4096;testNode.connect(meter);});
  await page.getByRole('button',{name:'PLAY / STOP',exact:true}).click({delay:90});
  let peak=0;for(let k=0;k<45;k++){
   await page.waitForTimeout(200);
   const p=await page.evaluate(()=>{const a=new Float32Array(4096);meter.getFloatTimeDomainData(a);return Math.max(...a.map(Math.abs));});
   assert(p<=1);peak=Math.max(peak,p);
  }
  assert(peak>.001);await check();
  await page.getByRole('button',{name:'PLAY / STOP',exact:true}).click({delay:90});await page.waitForTimeout(3000);
  const stopped=await page.evaluate(()=>{const a=new Float32Array(4096);meter.getFloatTimeDomainData(a);return Math.max(...a.map(Math.abs));});assert(stopped<.0001);
  assert.deepEqual(errors,[]);console.log({startupAndExplicitDemo:'Miami Bass matching bank',peak,stopped,seconds:9,pageErrors:errors});
 } finally {await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
