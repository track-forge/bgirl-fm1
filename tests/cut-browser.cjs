// SPDX-License-Identifier: GPL-3.0-only
// Native WASM canvas, panel controls, real user-gesture AudioWorklet.
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict'),fs=require('node:fs');
(async()=>{
 const browser=await chromium.launch({headless:true,args:['--no-sandbox'],...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{})});
 const dir=process.env.BGIRL_CUT_ARTIFACTS||'local/cut-rollout/browser';fs.mkdirSync(dir,{recursive:true});
 try{
  const page=await browser.newPage({viewport:{width:1300,height:1000}}),errors=[];
  page.on('pageerror',e=>errors.push(e.message));
  await page.addInitScript(()=>{
   const AC=AudioContext;window.AudioContext=class extends AC{constructor(o){super(o);window.testAC=this;}};
   const AW=AudioWorkletNode;window.AudioWorkletNode=class extends AW{constructor(...a){super(...a);window.testNode=this;}};
  });
  const tap=async name=>{await page.getByRole('button',{name,exact:true}).click({delay:90});await page.waitForTimeout(300);};
  const turn=async(name,n=1)=>{for(let i=0;i<Math.abs(n);i++){await page.getByRole('slider',{name,exact:true}).press(n>0?'ArrowUp':'ArrowDown');await page.waitForTimeout(60);}};
  const info=()=>page.evaluate(()=>new Promise(resolve=>{const f=e=>{if(e.data.type==='crew-info'){testNode.port.removeEventListener('message',f);resolve(e.data.values);}};testNode.port.addEventListener('message',f);testNode.port.postMessage({type:'crew-info'});}));
  const pixels=()=>page.locator('#screen').evaluate(c=>c.toDataURL());
  const shot=async name=>fs.writeFileSync(`${dir}/${name}.png`,Buffer.from((await pixels()).split(',')[1],'base64'));
  let audioPeak=0,editChecks=0;
  for(let large=0;large<2;large++){
   await page.goto(process.env.BGIRL_URL||'http://127.0.0.1:8809');await page.evaluate(()=>localStorage.clear());await page.reload();
   await page.locator('#go').click();await page.waitForFunction(()=>document.querySelector('#start').hidden);await page.waitForTimeout(1500);
   if(large){await page.getByRole('button',{name:'HOME',exact:true}).click({delay:900});await page.waitForTimeout(300);await turn('PRESETS',2-(await info())[9]);await turn('KNOB 1');await tap('HOME');assert.equal((await info())[10],1);}
   await turn('ALGORITHM');assert.equal((await info())[2],1);
   for(const name of ['step','detail','automation']){
    await tap('SEQ');await shot(`${name}-${large}`);const before=await pixels();
    await turn('KNOB 1',3);await turn('KNOB 2',2);await turn('KNOB 3',-2);await turn('KNOB 4',2);
    assert.notEqual(await pixels(),before);editChecks++;await shot(`${name}-edited-${large}`);
   }
   await tap('HOME');
   // Save A through existing PROJECT controls, then create a real SONG section.
   for(let i=0;i<4;i++)await tap('SAVE');await turn('KNOB 4');await tap('OCT+');await tap('OCT+');
   await tap('HOME');await tap('GLO');await shot(`song-empty-${large}`);
   const before=await pixels();await turn('KNOB 3');assert.notEqual(await pixels(),before);editChecks++;
   await turn('KNOB 2',2);await turn('KNOB 4',2);await shot(`song-section-${large}`);
   await tap('HOME');await tap('PLAY / STOP');await page.waitForTimeout(700);
   const peak=await page.evaluate(()=>{const a=testAC.createAnalyser();a.fftSize=4096;testNode.connect(a);return new Promise(resolve=>setTimeout(()=>{const b=new Float32Array(4096);a.getFloatTimeDomainData(b);resolve(Math.max(...b.map(Math.abs)));a.disconnect();},120));});
   audioPeak=Math.max(audioPeak,peak);await tap('SEQ');await shot(`step-playing-${large}`);await tap('PLAY / STOP');
  }
  assert(audioPeak>.00001&&audioPeak<=1);assert.deepEqual(errors,[]);
  const results={normalAndLarge:true,panelNavigation:true,editChecks,savedProjectAndSongSection:true,audioPeak,errors,hardware:'not tested',visualApproval:'Doug review pending'};
  fs.writeFileSync(`${dir}/results.json`,JSON.stringify(results,null,2)+'\n');console.log(results);
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
