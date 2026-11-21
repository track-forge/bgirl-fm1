// SPDX-License-Identifier: GPL-3.0-only
// Real Chromium / AudioWorklet: normal loading, panel edits, MIDI and origin saves.
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict'),fs=require('node:fs');
(async()=>{
 const browser=await chromium.launch({headless:true,args:['--no-sandbox'],...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{})});
 const dir=process.env.BGIRL_ARTIFACTS||'local/sh-style/browser';fs.mkdirSync(dir,{recursive:true});
 try {
  const page=await browser.newPage({viewport:{width:1300,height:1100}}),errors=[];page.on('pageerror',e=>errors.push(e.message));
  await page.addInitScript(()=>{
   const AC=AudioContext;window.AudioContext=class extends AC{constructor(o){super(o);window.testAC=this;}};
   const AW=AudioWorkletNode;window.AudioWorkletNode=class extends AW{constructor(...a){super(...a);window.testNode=this;}};
  });
  const on=async()=>{await page.locator('#go').click();await page.waitForFunction(()=>document.querySelector('#start').hidden);await page.waitForTimeout(1300);};
  const info=()=>page.evaluate(()=>new Promise((resolve,reject)=>{const timer=setTimeout(()=>reject(new Error('sh-style-info timeout')),5000);const fn=e=>{if(e.data.type==='sh-style-info'){clearTimeout(timer);testNode.port.removeEventListener('message',fn);resolve(e.data.tracks);}};testNode.port.addEventListener('message',fn);testNode.port.postMessage({type:'sh-style-info'});}));
  const tap=async name=>{await page.getByRole('button',{name,exact:true}).click({delay:90});await page.waitForTimeout(250);};
  const turn=async(name,up=true)=>{await page.getByRole('slider',{name,exact:true}).press(up?'ArrowUp':'ArrowDown');await page.waitForTimeout(200);};
  const load=async sound=>{await page.locator('#sh-style-sound').selectOption({index:sound});await page.locator('#sh-style-load').click();await page.waitForTimeout(300);};
  await page.goto(process.env.BGIRL_URL||'http://127.0.0.1:8798');await on();
  assert.equal(await page.locator('[aria-label="SH-ISH collection"] h2').textContent(),'SH-ISH');
  assert.deepEqual(await page.locator('#sh-style-sound option').allTextContents(),['Crunchy Bass · 1-1','Velvet Pulse · saved patch, slot unknown','Noisy Dark Sweep · T1-6','Slayer Bass · T1-7','Electro Droplet · T2-8']);
  page.once('dialog',d=>d.accept());await page.locator('#blank').click();await page.waitForTimeout(500);
  // All five ordinary presets available on all six selected tracks; other tracks untouched.
  for(let t=0;t<6;t++){
   for(let sound=0;sound<5;sound++){
    const before=await info();await load(sound);const after=await info();
    assert.equal(after[t][0],0);assert.equal(after[t][1],13+sound);assert.equal(after[t][2],[106,25,70,56,69][sound]);assert.equal(after[t][10],1);
    for(let k=0;k<6;k++)if(k!==t)assert.deepEqual(after[k].slice(0,11),before[k].slice(0,11));
   }await turn('ALGORITHM');
  }
  // Normal PRESETS ordering: appended IDs, then continue to unchanged FM6 BG-first ordering.
  await load(0);await turn('PRESETS');assert.equal((await info())[0][1],14);
  for (let preset=15;preset<=17;preset++) {await turn('PRESETS');assert.equal((await info())[0][1],preset);}
  await turn('PRESETS');assert.deepEqual((await info())[0].slice(0,2),[12,8]);
  const peaks=[];
  for(let sound=0;sound<5;sound++){
   if(sound===1)await page.setViewportSize({width:390,height:844});
   await load(sound);await tap('HOME');await tap('ENV');const before=(await info())[0][4];
   await turn('KNOB 1');const edited=(await info())[0][4];assert.notEqual(edited,before,'normal attack knob edits preset');
   await page.evaluate(()=>{window.analyser=testAC.createAnalyser();analyser.fftSize=4096;testNode.connect(analyser);});
   const peak=()=>page.evaluate(()=>{const a=new Float32Array(4096);analyser.getFloatTimeDomainData(a);return Math.max(...a.map(Math.abs));});
   await page.evaluate(()=>testNode.port.postMessage({type:'midi',data:[0x90,60,100]}));await page.waitForTimeout(200);
   const p=await peak();assert(p>.000001&&p<=1);peaks.push(p);
   await page.evaluate(()=>testNode.port.postMessage({type:'midi',data:[0x80,60,0]}));await page.waitForTimeout(2500);assert((await peak())<.00001);
   await tap('PLAY / STOP');await tap('PLAY / STOP');await page.waitForTimeout(1500);assert((await peak())<.00001);
   await tap('SAVE');await tap('OCT+');await tap('OCT+');
   await page.waitForTimeout(15500); // normal autosave, not a test-only persistence call
   const q=(await info())[0].slice(0,11);assert.equal(q[4],edited);
   await page.screenshot({path:`${dir}/${sound?'mobile-'+sound:'desktop'}.png`,fullPage:true});
   await page.reload();await on();assert.deepEqual((await info())[0].slice(0,11),q);
   assert(await page.evaluate(()=>!!localStorage.getItem('bgirl-felucca-v1-flash')));
   assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),false);
  }
  assert.deepEqual(errors,[]);
  const result={sounds:5,selectedTracks:6,panelBrowsing:true,panelEdits:true,noteOffStop:true,saveReloadAll:true,desktopMobile:true,peaks,pageErrors:errors};
  fs.writeFileSync(`${dir}/results.json`,JSON.stringify(result,null,2)+'\n');console.log(result);
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
