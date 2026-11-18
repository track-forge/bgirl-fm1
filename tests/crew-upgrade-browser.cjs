// SPDX-License-Identifier: GPL-3.0-only
// Supply the pre-upgrade PR38a429 WASM to exercise a real saved-settings upgrade.
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict'),fs=require('node:fs');
(async()=>{
 assert(process.env.BGIRL_CREW_PREVIOUS_WASM,'BGIRL_CREW_PREVIOUS_WASM must name the pre-upgrade WASM');
 const browser=await chromium.launch({headless:true,args:['--no-sandbox'],...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{})});
 try {
  const page=await browser.newPage(),errors=[];
  page.on('pageerror',e=>errors.push(e.message));
  await page.addInitScript(()=>{
   const AW=window.AudioWorkletNode;window.AudioWorkletNode=class extends AW{constructor(...a){super(...a);window.testNode=this;this.port.addEventListener('message',e=>{if(e.data.type==='crew-info')window.crewInfo=e.data.values;});}};
  });
  const info=async()=>{await page.evaluate(()=>{window.crewInfo=null;testNode.port.postMessage({type:'crew-info'});});await page.waitForFunction(()=>window.crewInfo);return page.evaluate(()=>window.crewInfo);};
  const tap=async(name,delay=90)=>{await page.getByRole('button',{name,exact:true}).click({delay});await page.waitForTimeout(220);};
  const turn=async(name,dir,count)=>{for(let k=0;k<count;k++){await page.getByRole('slider',{name,exact:true}).press(dir>0?'ArrowUp':'ArrowDown');await page.waitForTimeout(60);}};
  const on=async()=>{await page.locator('#go').click();await page.waitForFunction(()=>document.querySelector('#start').hidden);await page.waitForTimeout(1300);};
  const choose=async(view)=>{await tap('HOME',900);await turn('PRESETS',-1,(await info())[9]);await turn('PRESETS',1,7);await turn('KNOB 1',-1,5);await turn('KNOB 1',1,view);await tap('HOME');assert.equal((await info())[0],view);await page.waitForTimeout(2600);};
  const oldWasm=fs.readFileSync(process.env.BGIRL_CREW_PREVIOUS_WASM);
  await page.route('**/felucca.wasm',r=>r.fulfill({body:oldWasm,contentType:'application/wasm'}));
  await page.goto(process.env.BGIRL_URL||'http://127.0.0.1:8796');await on();
  await choose(0); // Save SCOPE using the genuine pre-migration firmware.
  const before=await page.evaluate(()=>({...localStorage}));assert(Object.keys(before).length>0);
  await page.reload();await on();assert.equal((await info())[0],0,'old firmware restores saved SCOPE');
  await page.unroute('**/felucca.wasm');
  await page.reload();await on();assert.equal((await info())[0],3,'new firmware upgrades saved SCOPE to CREW');
  await choose(1); // An intentional post-upgrade TRACKS selection must win later.
  await page.reload();await on();assert.equal((await info())[0],1,'new explicit view remains selected');
  assert.deepEqual(errors,[]);
  console.log(JSON.stringify({oldSavedScopeRestored:true,upgradedToCrew:true,laterTracksRestored:true,storageCleared:false,errors}));
 } finally {await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
