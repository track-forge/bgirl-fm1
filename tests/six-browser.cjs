// SPDX-License-Identifier: GPL-3.0-only
// Actual Chromium AudioWorklet, normal panel editing and origin persistence.
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict'),fs=require('node:fs');
(async()=>{
 const browser=await chromium.launch({headless:true,args:['--no-sandbox'],...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{})});
 const artifacts=process.env.BGIRL_ARTIFACTS||'tmp/six-browser';fs.mkdirSync(artifacts,{recursive:true});
 try {
  const page=await browser.newPage({viewport:{width:1300,height:1100}}),errors=[];page.on('pageerror',e=>errors.push(e.message));
  await page.addInitScript(()=>{
   const AC=window.AudioContext;window.AudioContext=class extends AC{constructor(o){super(o);window.testAC=this;}};
   const AW=window.AudioWorkletNode;window.AudioWorkletNode=class extends AW{constructor(...a){super(...a);window.testNode=this;}};
  });
  await page.goto(process.env.BGIRL_URL||'http://127.0.0.1:8792');
  const on=async()=>{await page.locator('#go').click();await page.waitForFunction(()=>document.querySelector('#start').hidden);await page.waitForTimeout(1200);};
  const info=()=>page.evaluate(()=>new Promise(resolve=>{const fn=e=>{if(e.data.type==='six-info'){testNode.port.removeEventListener('message',fn);resolve(e.data.values);}};testNode.port.addEventListener('message',fn);testNode.port.postMessage({type:'six-info'});}));
  const tap=async name=>{await page.getByRole('button',{name,exact:true}).click({delay:90});await page.waitForTimeout(350);};
  const turn=async(name,up=true)=>{await page.getByRole('slider',{name,exact:true}).press(up?'ArrowUp':'ArrowDown');await page.waitForTimeout(150);};
  const fixture=async id=>{page.once('dialog',d=>d.accept());await page.locator(id).click();await page.waitForTimeout(500);};
  await on();await fixture('#six-demo');let q=await info();assert.equal(q[0][0],6);assert.equal(q[1][0],8);assert(q[11].every(x=>x===64));assert(q[3][0]<3840);
  const screens=[];
  for(let t=0;t<6;t++){q=await info();assert.equal(q[2][0],t);screens.push(await page.locator('#screen').evaluate(c=>c.toDataURL()));await turn('ALGORITHM');}
  assert.equal(new Set(screens).size,6);assert.equal((await info())[2][0],0);await turn('ALGORITHM',false);assert.equal((await info())[2][0],5);
  await turn('ALGORITHM',false);assert.equal((await info())[8][0],4);
  // GLO held: four knobs map to T5/T6/--/--. No UI-only parameter workaround.
  const glo=page.getByRole('button',{name:'GLO',exact:true});await glo.hover();await page.mouse.down();await page.waitForTimeout(800);
  const before=(await info())[10];await turn('KNOB 3');await turn('KNOB 4');assert.deepEqual((await info())[10],before);
  await turn('KNOB 1');q=await info();assert.notEqual(q[10][4],before[4]);assert.equal(q[10][0],before[0]);
  await page.screenshot({path:`${artifacts}/bank-t5-t6.png`,fullPage:true});await page.mouse.up();await page.waitForTimeout(600);
  await page.evaluate(()=>{window.analyser=testAC.createAnalyser();analyser.fftSize=4096;testNode.connect(analyser);});
  const peak=()=>page.evaluate(()=>{const a=new Float32Array(4096);analyser.getFloatTimeDomainData(a);return Math.max(...a.map(Math.abs));});
  await tap('PLAY / STOP');await page.waitForTimeout(1700);const auditionPeak=await peak();assert(auditionPeak>.0001&&auditionPeak<=1);
  await page.screenshot({path:`${artifacts}/six-overview.png`,fullPage:true});await tap('PLAY / STOP');await page.waitForTimeout(15000);
  q=await info();assert(q[4][0]>0);const savedLevels=q[10];
  await page.reload();await on();q=await info();assert(q[11].every(x=>x===64));assert.deepEqual(q[10],savedLevels);assert.equal(q[7][0],0);
  // Project slot write via inherited SAVE > PROJECT controls.
  for(let i=0;i<4;i++)await tap('SAVE');await turn('KNOB 4');await tap('OCT+');await tap('OCT+');
  assert.equal((await info())[6][0],1);
  const flash=await page.evaluate(()=>localStorage.getItem('bgirl-felucca-v1-flash'));
  await fixture('#six-dense');assert((await info())[3][0]>3840);
  for(let i=0;i<4;i++)await tap('SAVE');await turn('KNOB 4');await tap('OCT+');await tap('OCT+');
  await page.waitForTimeout(12500);q=await info();assert.equal(q[5][0],1);assert.equal(q[6][0],1);
  assert.equal(await page.evaluate(()=>localStorage.getItem('bgirl-felucca-v1-flash')),flash);
  await tap("OCT-");
  await page.screenshot({path:`${artifacts}/project-full.png`,fullPage:true});
  await fixture('#six-demo');await tap('GLO');await page.screenshot({path:`${artifacts}/six-song.png`,fullPage:true});await tap('HOME');await page.setViewportSize({width:390,height:844});await page.screenshot({path:`${artifacts}/mobile.png`,fullPage:true});
  assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),false);assert.deepEqual(errors,[]);
  const result={tracks:6,voiceBudget:8,distinctScreens:new Set(screens).size,auditionPeak,autosaveReload:true,manualSlotSaved:true,fullPreservesFlash:true,errors,humanListening:'not performed'};
  fs.writeFileSync(`${artifacts}/results.json`,JSON.stringify(result,null,2)+'\n');console.log(result);
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
