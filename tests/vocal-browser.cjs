// SPDX-License-Identifier: GPL-3.0-only
// Real browser/AudioWorklet; signal + panel evidence, not intelligibility approval.
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict'),fs=require('node:fs');
(async()=>{
 const browser=await chromium.launch({headless:true,args:['--no-sandbox'],...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{})});
 try {
  const page=await browser.newPage({viewport:{width:1300,height:1100}}),errors=[];
  page.on('pageerror',e=>errors.push(e.message));
  await page.addInitScript(()=>{
   const AC=window.AudioContext;window.AudioContext=class extends AC{constructor(o){super(o);window.testAC=this;}};
   const AW=window.AudioWorkletNode;window.AudioWorkletNode=class extends AW{constructor(...a){super(...a);window.testNode=this;}};
  });
  await page.goto(process.env.BGIRL_URL||'http://127.0.0.1:8773/');
  await page.getByRole('button',{name:'Switch on',exact:true}).click();
  await page.waitForFunction(()=>document.querySelector('#vocal-status').textContent.startsWith('Installed bank'));
  await page.waitForTimeout(1200);
  const bank=await page.locator('#vocal-status').innerText();
  await page.evaluate(()=>{window.meter=testAC.createAnalyser();meter.fftSize=4096;testNode.connect(meter);});
  const peak=()=>page.evaluate(()=>{const a=new Float32Array(4096);meter.getFloatTimeDomainData(a);return Math.max(...a.map(Math.abs));});
  const tap=async name=>{await page.getByRole('button',{name,exact:true}).click({delay:90});await page.waitForTimeout(180);};
  const peaks={},artifacts=process.env.BGIRL_ARTIFACTS||'tmp/vocal-browser';fs.mkdirSync(artifacts,{recursive:true});
  // Cancel must preserve the project; load only on explicit confirmation.
  page.once('dialog',d=>d.dismiss());await page.locator('#vocal-whole-dry').click();assert.equal(await page.locator('#project-status').innerText(),'');
  for(const id of ['vocal-whole-dry','vocal-whole-wet','vocal-chops-dry','vocal-chops-wet']) {
   page.once('dialog',d=>d.accept());await page.locator('#'+id).click();await page.waitForTimeout(350);
   assert.match(await page.locator('#project-status').innerText(),/^Loaded/);
   await tap('PLAY / STOP');peaks[id]=0;
   for(let i=0;i<(id.includes("whole")?45:25);i++){await page.waitForTimeout(100);peaks[id]=Math.max(peaks[id],await peak());}
   assert(peaks[id]>.00001&&peaks[id]<=1,`${id}: ${peaks[id]}`);
   await page.screenshot({path:`${artifacts}/${id}.png`,fullPage:true});
   await tap('PLAY / STOP');await page.waitForTimeout(2500);assert((await peak())<.0001);
  }
  // Normal EDIT panel, group/START knob changes, chromatic hardware key gesture.
  await tap('EDIT');const before=await page.locator('#screen').evaluate(c=>c.toDataURL());
  await page.getByRole('slider',{name:'KNOB 2',exact:true}).press('ArrowDown');await page.waitForTimeout(180);
  const after=await page.locator('#screen').evaluate(c=>c.toDataURL());assert.notEqual(before,after);
  await page.screenshot({path:`${artifacts}/normal-edit.png`,fullPage:true});
  // Dry tap must continue after key release, with no sequencer running.
  page.once('dialog',d=>d.accept());await page.locator('#vocal-whole-dry').click();await page.waitForTimeout(250);
  await page.locator('h1').click();await page.keyboard.down('KeyZ');await page.waitForTimeout(80);await page.keyboard.up('KeyZ');
  let tapPeak=0;for(let i=0;i<4;i++){await page.waitForTimeout(100);tapPeak=Math.max(tapPeak,await peak());}assert(tapPeak>.00001);
  await page.waitForTimeout(4000);assert((await peak())<.0001);
  // Existing SAVE and autosave browser storage survive reload, selection not reset.
  await tap('SAVE');await tap('OCT+');await tap('OCT+');
  await page.waitForFunction(()=>Object.keys(JSON.parse(localStorage.getItem('bgirl-felucca-v1-flash')||'{}')).length>0);
  // Save a full project too, through SAVE > PROJECT (not just a user preset).
  await tap('HOME');for(let i=0;i<4;i++){await tap('SAVE');await page.waitForTimeout(500);}
  await page.getByRole('slider',{name:'KNOB 4',exact:true}).press('ArrowUp');await tap('OCT+');await tap('OCT+');
  await page.waitForTimeout(400);
  await page.reload();await page.getByRole('button',{name:'Switch on',exact:true}).click();
  await page.waitForFunction(()=>document.querySelector('#vocal-status').textContent.startsWith('Installed bank'));await page.waitForTimeout(1200);
  // Reload the saved project from slot A using the ordinary panel.
  for(let i=0;i<4;i++){await tap('SAVE');await page.waitForTimeout(500);}
  await page.getByRole('slider',{name:'KNOB 3',exact:true}).press('ArrowUp');await tap('OCT+');await tap('OCT+');
  await tap('HOME');await tap('EDIT');
  const restored=await page.evaluate(()=>new Promise(resolve=>{
   const fn=e=>{if(e.data.type==='vocal-info'){testNode.port.removeEventListener('message',fn);resolve(e.data.state);}};
   testNode.port.addEventListener('message',fn);testNode.port.postMessage({type:'vocal-info'});
  }));
  assert.equal(restored&15,6,'saved project restores VOCALS, not startup SPEECH');
  assert.equal((restored>>4)&7,0,'WHOLE view restored');assert(restored&4096,'bank fingerprint matches');
  await page.screenshot({path:`${artifacts}/restored-edit.png`,fullPage:true});
  await page.setViewportSize({width:390,height:844});await page.waitForTimeout(150);
  assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),false);
  await page.screenshot({path:`${artifacts}/mobile.png`,fullPage:true});assert.deepEqual(errors,[]);
  const result={bank,peaks,tapPeak,normalPanelChanged:true,projectRestoredState:restored,pageErrors:errors,humanListening:'pending'};
  fs.writeFileSync(`${artifacts}/results.json`,JSON.stringify(result,null,2)+'\n');console.log(result);
 } finally {await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
