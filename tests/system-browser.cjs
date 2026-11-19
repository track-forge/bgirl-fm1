// SPDX-License-Identifier: GPL-3.0-only
// Real panel, native WASM screen, AudioWorklet and persisted browser flash.
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict'),fs=require('node:fs');
(async()=>{
 const browser=await chromium.launch({headless:true,args:['--no-sandbox'],...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{})});
 const dir=process.env.BGIRL_SYSTEM_ARTIFACTS||'local/system-rollout/browser';fs.mkdirSync(dir,{recursive:true});
 try{
  const page=await browser.newPage({viewport:{width:1300,height:1000}}),errors=[];page.on('pageerror',e=>errors.push(e.message));
  await page.addInitScript(()=>{const AW=AudioWorkletNode;window.AudioWorkletNode=class extends AW{constructor(...a){super(...a);window.testNode=this;}};});
  const tap=async(name,delay=90)=>{await page.getByRole('button',{name,exact:true}).click({delay});await page.waitForTimeout(260);};
  const turn=async(name,n=1)=>{for(let k=0;k<Math.abs(n);k++){await page.getByRole('slider',{name,exact:true}).press(n>0?'ArrowUp':'ArrowDown');await page.waitForTimeout(60);}};
  const info=()=>page.evaluate(()=>new Promise(resolve=>{const f=e=>{if(e.data.type==='crew-info'){testNode.port.removeEventListener('message',f);resolve(e.data.values);}};testNode.port.addEventListener('message',f);testNode.port.postMessage({type:'crew-info'});}));
  const pixels=()=>page.locator('#screen').evaluate(c=>c.toDataURL());
  const shot=async(name)=>fs.writeFileSync(`${dir}/${name}.png`,Buffer.from((await pixels()).split(',')[1],'base64'));
  const flash=()=>page.evaluate(()=>localStorage.getItem('bgirl-felucca-v1-flash'));
  let pages=0,menus=0,layers=0;
  for(let large=0;large<2;large++){
   await page.goto(process.env.BGIRL_URL||'http://127.0.0.1:8812');await page.evaluate(()=>localStorage.clear());await page.reload();
   await page.locator('#go').click();await page.waitForFunction(()=>document.querySelector('#start').hidden);await page.waitForTimeout(1600);
   // LARGE through the real DISPLAY menu, then all tabs and their row navigation.
   await tap('HOME',900);
   if(large){await turn('PRESETS',2);await turn('KNOB 1');assert.equal((await info())[10],1);}
   for(let tab=0;tab<6;tab++){
    await shot(`menu-${tab}-${large}`);const before=await pixels();await turn('PRESETS');assert.notEqual(await pixels(),before);menus++;
    if(tab===4){
     await turn('PRESETS',3);await page.waitForTimeout(300);let old=await pixels();await turn('KNOB 1');assert.notEqual(await pixels(),old);
     await turn('PRESETS');await page.waitForTimeout(300);old=await pixels();await turn('KNOB 1');assert.notEqual(await pixels(),old);await shot(`channels56-${large}`);
    }
    await turn('ALGORITHM');
   }
   await tap('HOME');
   const titles=['user','presets','phrases','project','tools'];
   for(let p=0;p<5;p++){
    await tap('SAVE');await shot(`${titles[p]}-${large}`);const before=await pixels();await turn('KNOB 1');assert.notEqual(await pixels(),before);pages++;
   }
   // A save opens NAME, cancel leaves persistent sectors untouched.
   await tap('SAVE');await turn('KNOB 1',2);await tap('OCT+');await shot(`name-${large}`);
   const unchanged=await flash();await turn('KNOB 2');await tap('OCT-');assert.equal(await flash(),unchanged);
   // Save a named sound, revisit the occupied slot, cancel overwrite, then accept rename.
   await tap('OCT+');await turn('KNOB 2');await tap('OCT+');await page.waitForTimeout(500);
   assert(Object.keys(JSON.parse(await flash()||'{}')).length>0);await shot(`saved-user-${large}`);
   await tap('OCT+');await shot(`overwrite-${large}`);const saved=await flash();await tap('OCT-');assert.equal(await flash(),saved);
   // Hold maps on both banks. ENV/LFO are momentary pages, GLO/FX/SEQ/REC are maps.
   for(const bank of [0,4]){
    if(bank)await turn('ALGORITHM',4);
    for(const button of ['GLO','ENV','LFO','FX','SEQ','REC']){
     await tap('HOME');const btn=page.getByRole('button',{name:button,exact:true});await btn.hover();await page.mouse.down();await page.waitForTimeout(900);
     const held=await pixels();await shot(`hold-${button}-${bank}-${large}`);await page.mouse.up();await page.waitForTimeout(350);
     // If the map latches, the same button closes it. HOME is then a clean return.
     await tap(button);await tap('HOME');assert.notEqual(await pixels(),held);layers++;
    }
   }
   await page.reload();await page.locator('#go').click();await page.waitForFunction(()=>document.querySelector('#start').hidden);await page.waitForTimeout(1200);
   assert.equal((await info())[10],large);assert(Object.keys(JSON.parse(await flash()||'{}')).length>0);
  }
  assert.deepEqual(errors,[]);const results={normalAndLarge:true,pages,menus,layers,namingCancel:true,soundSave:true,overwriteCancel:true,reloadPreservesSettingsAndSectors:true,errors,hardware:'not tested',visualApproval:'Doug review pending'};
  fs.writeFileSync(`${dir}/results.json`,JSON.stringify(results,null,2)+'\n');console.log(results);
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
