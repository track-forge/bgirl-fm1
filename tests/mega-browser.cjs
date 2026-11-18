// SPDX-License-Identifier: GPL-3.0-only
// Actual browser at tested preview origin; never clears existing user storage.
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict'),fs=require('node:fs');
(async()=>{
 const browser=await chromium.launch({headless:true,args:['--no-sandbox'],...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{})});
 const dir=process.env.BGIRL_ARTIFACTS||'tmp/mega-browser';fs.mkdirSync(dir,{recursive:true});
 try {
  const page=await browser.newPage({viewport:{width:1300,height:1100}}),errors=[];
  page.on('pageerror',e=>errors.push(e.message));
  await page.addInitScript(()=>{
   const AC=AudioContext;window.AudioContext=class extends AC{constructor(o){super(o);window.testAC=this;}};
   const AW=AudioWorkletNode;window.AudioWorkletNode=class extends AW{constructor(...a){super(...a);window.testNode=this;}};
  });
  await page.goto(process.env.BGIRL_URL||'http://127.0.0.1:8796');
  const on=async()=>{await page.locator('#go').click();await page.waitForTimeout(1500);};
  const info=()=>page.evaluate(()=>new Promise(resolve=>{
   const fn=e=>{if(e.data.type==='six-info'){testNode.port.removeEventListener('message',fn);resolve(e.data.values);}};
   testNode.port.addEventListener('message',fn);testNode.port.postMessage({type:'six-info'});
  }));
  const vocal=()=>page.evaluate(()=>new Promise(resolve=>{
   const fn=e=>{if(e.data.type==='vocal-info'){testNode.port.removeEventListener('message',fn);resolve(e.data);}};
   testNode.port.addEventListener('message',fn);testNode.port.postMessage({type:'vocal-info'});
  }));
  const tap=async name=>{await page.getByRole('button',{name,exact:true}).click({delay:90});await page.waitForTimeout(250);};
  const turn=async name=>{await page.getByRole('slider',{name,exact:true}).press('ArrowUp');await page.waitForTimeout(180);};
  await on();await page.waitForFunction(()=>document.querySelector('#vocal-status').textContent.startsWith('Installed bank'));
  let q=await info();assert(q[11].every(n=>n===64));assert.equal(q[9][4],8);assert.equal(q[7][0],0);
  assert.equal((await vocal()).state,4096+6,'whole G6 identity-bound source');
  page.once('dialog',d=>d.dismiss());await page.locator('#demo').click();assert.deepEqual(await info(),q);
  page.once('dialog',d=>d.accept());await page.locator('#demo').click();await page.waitForTimeout(400);
  assert.equal((await info())[7][0],0);
  await page.evaluate(()=>{window.meter=testAC.createAnalyser();meter.fftSize=4096;testNode.connect(meter);});
  const peak=()=>page.evaluate(()=>{const a=new Float32Array(4096);meter.getFloatTimeDomainData(a);return Math.max(...a.map(Math.abs));});
  await tap('PLAY / STOP');let max=0;
  for(let i=0;i<80;i++){await page.waitForTimeout(200);const p=await peak();assert(p<=1);max=Math.max(max,p);}assert(max>.001);
  await page.screenshot({path:`${dir}/playing-desktop.png`,fullPage:true});
  await tap('PLAY / STOP');await page.waitForTimeout(5000);assert((await peak())<.0001);
  await tap('PLAY / STOP');await page.waitForTimeout(700);assert((await peak())>.0001);await tap('PLAY / STOP');
  // Normal six-track selection and sound edit on the pad, preserved by autosave.
  const screens=[];
  for(let t=0;t<6;t++){assert.equal((await info())[2][0],t);screens.push(await page.locator('#screen').evaluate(c=>c.toDataURL()));await turn('ALGORITHM');}
  assert.equal(new Set(screens).size,6);
  for(let i=0;i<4;i++)await turn('ALGORITHM');
  await tap('EDIT');const before=await page.locator('#screen').evaluate(c=>c.toDataURL());await turn('KNOB 3');
  const edited=await page.locator('#screen').evaluate(c=>c.toDataURL());assert.notEqual(before,edited);
  await tap('HOME');
  const glo=page.getByRole('button',{name:'GLO',exact:true});await glo.hover();await page.mouse.down();await page.waitForTimeout(800);
  const oldLevel=(await info())[10][4];await turn('KNOB 1');await page.mouse.up();await page.waitForTimeout(400);
  const savedLevel=(await info())[10][4];assert.notEqual(savedLevel,oldLevel);q=await info();
  // Existing SAVE > PROJECT controls. Separate user-preset and project mechanisms.
  for(let i=0;i<4;i++)await tap('SAVE');await turn('KNOB 4');await tap('OCT+');await tap('OCT+');
  assert.equal((await info())[6][0],1);await tap('HOME');
  await page.waitForTimeout(15000);const flash=await page.evaluate(()=>localStorage.getItem('bgirl-felucca-v1-flash'));assert(flash);
  await page.reload();await on();let restored=await info();assert.equal(restored[7][0],0);assert.equal(restored[9][4],8);assert.equal(restored[2][0],4);assert(restored[11].every(n=>n===64));
  assert.equal(await page.evaluate(()=>localStorage.getItem('bgirl-felucca-v1-flash')),flash);
  assert.equal(restored[10][4],savedLevel,'normal pad mixer edit persists');await tap('EDIT');
  await page.setViewportSize({width:390,height:844});await page.screenshot({path:`${dir}/mobile.png`,fullPage:true});
  assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),false);
  assert.deepEqual(errors,[]);const result={peak:max,stop:true,restart:true,cancel:true,sixDistinctScreens:true,normalPadEdit:true,manualProjectSave:true,autosaveReload:true,mobile:true,errors};
  fs.writeFileSync(`${dir}/results.json`,JSON.stringify(result,null,2)+'\n');console.log(result);
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
