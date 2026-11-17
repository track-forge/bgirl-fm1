// SPDX-License-Identifier: GPL-3.0-only
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict'),fs=require('node:fs');
(async()=>{
 const browser=await chromium.launch({headless:true,args:['--no-sandbox'],...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{})});
 const artifacts=process.env.BGIRL_ARTIFACTS||'local/clarity/browser';fs.mkdirSync(artifacts,{recursive:true});
 try {
  const page=await browser.newPage({viewport:{width:1300,height:1100}}),errors=[];page.on('pageerror',e=>errors.push(e.message));
  await page.addInitScript(()=>{
   const AC=window.AudioContext;window.AudioContext=class extends AC{constructor(o){super(o);window.testAC=this;}};
   const AW=window.AudioWorkletNode;window.AudioWorkletNode=class extends AW{constructor(...a){super(...a);window.testNode=this;}};
  });
  await page.goto(process.env.BGIRL_URL||'http://127.0.0.1:8794');
  const on=async()=>{await page.locator('#go').click();await page.waitForFunction(()=>document.querySelector('#start').hidden);await page.waitForTimeout(1200);};
  const info=()=>page.evaluate(()=>new Promise((resolve,reject)=>{const timer=setTimeout(()=>reject(new Error('clarity-info timeout')),5000);const fn=e=>{if(e.data.type==='clarity-info'){clearTimeout(timer);testNode.port.removeEventListener('message',fn);resolve(e.data.values);}};testNode.port.addEventListener('message',fn);testNode.port.postMessage({type:'clarity-info'});}));
  const tap=async name=>{await page.getByRole('button',{name,exact:true}).click({delay:90});await page.waitForTimeout(650);};
  const turn=async(name,n)=>{for(let i=0;i<Math.abs(n);i++)await page.getByRole('slider',{name,exact:true}).press(n>0?'ArrowUp':'ArrowDown');await page.waitForTimeout(300);};
  const fixture=async id=>{page.once('dialog',d=>d.accept());await page.locator('#'+id).click();await page.waitForTimeout(500);};
  await on();await fixture('clarity-whole');let q=await info();assert.deepEqual(q.slice(0,7),[2,1,94,127,0,0,151]);
  await page.getByRole('button',{name:'GLO',exact:true}).click({modifiers:['Shift']});
  const before=await info();page.once('dialog',d=>d.dismiss());await page.locator('#clarity-chops').click();assert.deepEqual(await info(),before);assert.equal(await page.locator('.latched .face.btn').count(),1);
  await fixture('clarity-whole');assert.equal(await page.locator('.latched .face.btn').count(),0);
  await page.evaluate(()=>{window.analyser=testAC.createAnalyser();analyser.fftSize=4096;testNode.connect(analyser);});
  const peak=()=>page.evaluate(()=>{const a=new Float32Array(4096);analyser.getFloatTimeDomainData(a);return Math.max(...a.map(Math.abs));});
  const peaks={};
  for(const id of ['clarity-whole','clarity-chops','clarity-arrangement','clarity-carrier-only','clarity-dry']) {
   await fixture(id);assert.equal((await info())[5],0);await tap('PLAY / STOP');let p=0;
   for(let i=0;i<10;i++){await page.waitForTimeout(100);p=Math.max(p,await peak());}assert(p>.00001&&p<=1);peaks[id]=p;
   await tap('PLAY / STOP');assert.equal((await info())[8],0);
  }
  await fixture('clarity-whole');await tap('FX');await tap('FX');
  await turn('KNOB 2',-2);assert.equal((await info())[0],0);await turn('KNOB 2',1);assert.equal((await info())[0],1);await turn('KNOB 2',1);assert.equal((await info())[0],2);
  await tap('PLAY / STOP');await tap('FX'); // input
  await turn('KNOB 1',1);assert.equal((await info())[7],1);await page.waitForTimeout(300);assert.equal((await info())[8],1);
  await tap('FX'); // color
  await turn('KNOB 4',40);q=await info();assert.equal(q[2],127);assert.equal(q[6],500);
  await page.screenshot({path:`${artifacts}/release500.png`,fullPage:true});
  await turn('KNOB 4',-127);q=await info();assert.equal(q[2],0);assert.equal(q[6],5);
  await tap('PLAY / STOP');assert.equal((await info())[7],0);
  // Set preset back through loader, then normal panel save and fresh-instance restore.
  await page.locator('#clarity-wet').selectOption('117');await page.locator('#clarity-bands').selectOption('1');
  await fixture('clarity-whole');q=await info();assert.deepEqual(q.slice(0,7),[1,1,94,117,0,0,151]);
  await page.waitForTimeout(15000);await page.reload();await on();assert.deepEqual((await info()).slice(0,7),q.slice(0,7));
  await tap('SAVE');await tap('SAVE');await tap('SAVE');await tap('SAVE');await turn('KNOB 4',1);await tap('OCT+');await tap('OCT+');
  const flash=await page.evaluate(()=>localStorage.getItem('bgirl-felucca-v1-flash'));assert(flash);
  await fixture('clarity-arrangement');assert.equal((await info())[5],0);
  await page.setViewportSize({width:390,height:844});await page.waitForTimeout(200);
  assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),false);
  await page.screenshot({path:`${artifacts}/mobile.png`,fullPage:true});
  const gallery=await page.request.get(new URL('listening/clarity/manifest.json',page.url()).href);assert.equal(gallery.status(),200);
  assert.equal((await gallery.json()).cases.length,13);
  assert.deepEqual(errors,[]);console.log({peaks,modePanel:'8/12/24',releasePanel:'5/500ms',freeze:true,cancel:true,persistence:true,mobile:true,pageErrors:errors,humanApproval:'pending'});
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
