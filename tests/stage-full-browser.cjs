// SPDX-License-Identifier: GPL-3.0-only
// Native WASM canvas, panel controls, real user-gesture AudioWorklet.
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict'),fs=require('node:fs');
(async()=>{
 const browser=await chromium.launch({headless:true,args:['--no-sandbox'],...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{})});
 const dir=process.env.BGIRL_STAGE_FULL_ARTIFACTS||'local/stage-rollout/browser';fs.mkdirSync(dir,{recursive:true});
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
  const six=()=>page.evaluate(()=>new Promise(resolve=>{const f=e=>{if(e.data.type==='six-info'){testNode.port.removeEventListener('message',f);resolve(e.data.values);}};testNode.port.addEventListener('message',f);testNode.port.postMessage({type:'six-info'});}));
  let audioPeak=0,editChecks=0;
  const home=async()=>{await tap('ENV');await tap('HOME');};
  const display=async(row,n)=>{await page.getByRole('button',{name:'HOME',exact:true}).click({delay:900});await page.waitForTimeout(300);await turn('PRESETS',row-(await info())[9]);await turn('KNOB 1',n);await tap('HOME');};
  for(let large=0;large<2;large++){
   await page.goto(process.env.BGIRL_URL||'http://127.0.0.1:8811');await page.evaluate(()=>localStorage.clear());await page.reload();
   await page.locator('#go').click();await page.waitForFunction(()=>document.querySelector('#start').hidden);await page.waitForTimeout(1500);
   if(large){await display(2,1);assert.equal((await info())[10],1);}
   for(let view=0;view<3;view++){
    await display(7,-5);await display(7,view);assert.equal((await info())[0],view);await home();
    await shot(`home-${view}-${large}`);await tap('PLAY / STOP');assert.equal((await info())[6],1);await page.waitForTimeout(300);await shot(`home-${view}-play-${large}`);await tap('PLAY / STOP');
   }
   // LEVELS follows the T5/T6 bank. Inactive K3/K4 cannot mutate any track's level.
   await turn('ALGORITHM',4);assert.equal((await info())[2],4);const bank=await six();await turn('KNOB 3',2);await turn('KNOB 4',2);assert.deepEqual((await six())[10],bank[10]);await shot(`levels-bank2-${large}`);
   await tap('HOME');await shot(`mixer-bank2-${large}`);const before=await pixels();await turn('KNOB 2',-3);await turn('KNOB 3',2);assert.notEqual(await pixels(),before);editChecks++;
   await tap('HOME');await shot(`clock-${large}`);await turn('KNOB 1',2);await turn('KNOB 2',2);await tap('PLAY / STOP');assert.equal((await info())[6],1);await page.waitForTimeout(500);await shot(`clock-play-${large}`);
   const peak=await page.evaluate(()=>{const a=testAC.createAnalyser();a.fftSize=4096;testNode.connect(a);return new Promise(resolve=>setTimeout(()=>{const b=new Float32Array(4096);a.getFloatTimeDomainData(b);resolve(Math.max(...b.map(Math.abs)));a.disconnect();},120));});audioPeak=Math.max(audioPeak,peak);await tap('PLAY / STOP');
   await turn('ALGORITHM',-4);await tap('ARP');await turn('KNOB 1');await turn('KNOB 3',2);await shot(`arp-${large}`);
   await tap('ARP');await turn('KNOB 1',2);await turn('KNOB 2',-2);await shot(`arp2-${large}`);await tap('ARP');await shot(`dance-${large}`);await turn('KNOB 3');await shot(`dance-ghost-${large}`);
   await tap('SCL');await turn('KNOB 1');await turn('KNOB 2');await shot(`scale-${large}`);await tap('SCL');await turn('KNOB 1');await turn('KNOB 3');await shot(`chord-${large}`);
  }
  assert(audioPeak>.00001&&audioPeak<=1);assert.deepEqual(errors,[]);
  const results={normalAndLarge:true,homeViews:3,panelNavigation:true,editChecks,inactiveLevelKnobs:true,mixerAndClock:true,arpScaleChordDance:true,audioPeak,errors,hardware:'not tested',visualApproval:'Doug review pending'};
  fs.writeFileSync(`${dir}/results.json`,JSON.stringify(results,null,2)+'\n');console.log(results);
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
