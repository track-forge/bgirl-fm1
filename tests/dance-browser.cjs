// SPDX-License-Identifier: GPL-3.0-only
// Actual Chromium AudioWorklet + inherited panel; diagnostics are read-only.
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict'), fs=require('node:fs');
(async()=>{
 const browser=await chromium.launch({headless:true,args:['--no-sandbox'],...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{})});
 try {
  const page=await browser.newPage({viewport:{width:1300,height:1100}}),errors=[],loads=[];
  page.on('pageerror',e=>errors.push(e.message));
  await page.addInitScript(()=>{
   const AC=window.AudioContext;window.AudioContext=class extends AC{constructor(o){super(o);window.testAC=this;}};
   const AW=window.AudioWorkletNode;window.AudioWorkletNode=class extends AW{constructor(...a){super(...a);window.testNode=this;this.port.addEventListener('message',e=>{if(e.data.type==='dance-info')window.danceInfo=e.data.values;if(e.data.load!==undefined)(window.testLoads??=[]).push(e.data.load);});}};
  });
  await page.goto(process.env.BGIRL_URL||'http://127.0.0.1:8775');
  await page.locator('#go').click();await page.waitForFunction(()=>document.querySelector('#start').hidden);await page.waitForTimeout(1200);
  const info=async()=>{await page.evaluate(()=>{window.danceInfo=null;testNode.port.postMessage({type:'dance-info'});});await page.waitForFunction(()=>window.danceInfo);return page.evaluate(()=>window.danceInfo);};
  const tap=async name=>{await page.getByRole('button',{name,exact:true}).click({delay:90});await page.waitForTimeout(400);};
  const knob=async(n,dir,count=1)=>{for(let k=0;k<count;k++){await page.getByRole('slider',{name:'KNOB '+n,exact:true}).press(dir>0?'ArrowUp':'ArrowDown');await page.waitForTimeout(45);}};
  const track=async dir=>{await page.getByRole('slider',{name:'ALGORITHM',exact:true}).press(dir>0?'ArrowUp':'ArrowDown');await page.waitForTimeout(180);};
  const screen=()=>page.locator('#screen').evaluate(c=>c.toDataURL());
  const key=async down=>page.evaluate(down=>testNode.port.postMessage({type:'keys',mask:down?1:0}),down);
  const artifacts=process.env.BGIRL_ARTIFACTS||'tmp/dance-browser';fs.mkdirSync(artifacts,{recursive:true});
  // T2 analog bass, with real ARP family cycling: ARP -> ARP2 -> DANCE -> ARP.
  await track(1);await tap('ARP');await tap('ARP');await tap('ARP');assert.equal((await info())[1],1);
  await tap('ARP');assert.equal((await info())[1],0);await tap('ARP');
  // ARP2 HOLD, swing and probability are the existing controls.
  await knob(1,1,4);await knob(3,1);assert.equal((await info())[7],1);
  await knob(2,-1,2);const prob=(await info())[6];assert(prob<127);await knob(2,1,2);
  await tap('ARP');await knob(1,1);assert((await info())[4]>0);
  const idle=await info();await page.waitForTimeout(350);assert.equal((await info())[0],idle[0]);
  await key(true);await page.waitForTimeout(650);await key(false);await page.waitForTimeout(250);
  let a=await info();assert(a[8]>0 && a[0]&512);const crisp=await screen();
  await page.screenshot({path:`${artifacts}/crisp-desktop.png`,fullPage:true});
  await page.locator('#screen').screenshot({path:`${artifacts}/crisp-panel.png`});
  await knob(3,1);assert.equal((await info())[2],1);await page.waitForTimeout(600);assert.notEqual(await screen(),crisp);
  await page.screenshot({path:`${artifacts}/ghost-desktop.png`,fullPage:true});
  await page.locator('#screen').screenshot({path:`${artifacts}/ghost-panel.png`});
  const rate=(await info())[5];await knob(2,1);assert.notEqual((await info())[5],rate);
  // Leave / re-enter while HOLD runs; switch track must show its own untouched state.
  await tap('HOME');a=await info();assert.equal(a[1],0);await page.waitForTimeout(400);assert.notEqual((await info())[0],a[0]);
  await tap('ARP');assert.equal((await info())[1],1);
  await track(1);assert.equal((await info())[0],0);await track(-1);assert((await info())[0]&512);
  // Stop/restart sequencer does not freeze internally clocked held arp.
  await tap('PLAY / STOP');await tap('PLAY / STOP');a=await info();await page.waitForTimeout(300);assert.notEqual((await info())[0],a[0]);
  // Busy electro groove (includes vocoder), rapid arp, both visual styles.
  await tap('PLAY / STOP');
  for(const style of [0,1]) {await knob(3,style?1:-1);await page.waitForTimeout(1700);loads.push({style,...await page.evaluate(()=>({samples:(window.testLoads||[]).slice(-2)}))});}
  await tap('PLAY / STOP');
  // Release HOLD through normal ARP2. No free-running dance after all notes end.
  await tap('ARP');await tap('ARP');await knob(3,-1);await page.waitForTimeout(300);assert.equal((await info())[8],0);
  await tap('ARP');a=await info();await page.waitForTimeout(300);assert.equal((await info())[0],a[0]);
  // LARGE and alternate palette/style through the existing HOME-held menu.
  await page.getByRole('button',{name:'HOME',exact:true}).click({delay:900});await page.waitForTimeout(250);
  await knob(1,1); // COLOR next palette
  await page.getByRole('slider',{name:'PRESETS',exact:true}).press('ArrowUp');await page.waitForTimeout(150);
  await knob(1,1); // STYLE LINE
  await page.getByRole('slider',{name:'PRESETS',exact:true}).press('ArrowUp');await page.waitForTimeout(150);
  await knob(1,1); // LARGE ON
  await tap('HOME');await tap('ARP');assert.equal((await info())[1],1);
  await page.locator('#screen').screenshot({path:`${artifacts}/large-line-panel.png`});
  // Save full project in A through normal SAVE family and confirmation.
  await tap('SAVE');await tap('SAVE');await tap('SAVE');await knob(1,1);await tap('OCT+');await tap('OCT+');
  await page.waitForFunction(()=>Object.keys(JSON.parse(localStorage.getItem('bgirl-felucca-v1-flash')||'{}')).length>0);
  await page.setViewportSize({width:390,height:844});await page.waitForTimeout(200);
  assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),false);
  await tap('ARP');await page.screenshot({path:`${artifacts}/mobile.png`,fullPage:true});
  await page.reload();await page.locator('#go').click();await page.waitForFunction(()=>document.querySelector('#start').hidden);await page.waitForTimeout(1300);
  assert.equal((await info())[2],0);assert.equal((await info())[0],0); // session style/progression not persisted
  assert.deepEqual(errors,[]);
  const result={navigation:'ARP/ARP2/DANCE cycle',acceptedArp:'HOLD release, hidden progression, track isolation, no-notes idle, rate/mode/swing/prob controls',persistence:'panel project save and reload; style resets',loads,errors,hardware:'unverified',humanListening:'not performed'};
  fs.writeFileSync(`${artifacts}/results.json`,JSON.stringify(result,null,2)+'\n');console.log(result);
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
