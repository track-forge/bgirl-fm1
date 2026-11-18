// SPDX-License-Identifier: GPL-3.0-only
// Real panel navigation + AudioWorklet. No fake HTML LCD or direct UI setter.
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict'),fs=require('node:fs');
(async()=>{
 const browser=await chromium.launch({headless:true,args:['--no-sandbox'],...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{})});
 const dir=process.env.BGIRL_STAGE_ARTIFACTS||'local/stage-lab-cut/browser-after';fs.mkdirSync(dir,{recursive:true});
 try{
  const page=await browser.newPage({viewport:{width:1300,height:1000}}),errors=[];
  page.on('pageerror',e=>errors.push(e.message));
  await page.addInitScript(()=>{
   const AC=AudioContext;window.AudioContext=class extends AC{constructor(o){super(o);window.testAC=this;}};
   const AW=AudioWorkletNode;window.AudioWorkletNode=class extends AW{constructor(...a){super(...a);window.testNode=this;}};
  });
  await page.goto(process.env.BGIRL_URL||'http://127.0.0.1:8807');
  await page.locator('#go').click();await page.waitForFunction(()=>document.querySelector('#start').hidden);await page.waitForTimeout(1500);
  const tap=async(name,delay=90)=>{await page.getByRole('button',{name,exact:true}).click({delay});await page.waitForTimeout(280);};
  const turn=async(name,n=1)=>{for(let i=0;i<Math.abs(n);i++){await page.getByRole('slider',{name,exact:true}).press(n>0?'ArrowUp':'ArrowDown');await page.waitForTimeout(65);}};
  const info=()=>page.evaluate(()=>new Promise(resolve=>{const f=e=>{if(e.data.type==='crew-info'){testNode.port.removeEventListener('message',f);resolve(e.data.values);}};testNode.port.addEventListener('message',f);testNode.port.postMessage({type:'crew-info'});}));
  const shot=async name=>fs.writeFileSync(`${dir}/${name}.png`,Buffer.from(await page.locator('#screen').evaluate(c=>c.toDataURL().split(',')[1]),'base64'));
  const clip=async name=>{for(let i=0;i<12;i++){if(name.startsWith("lab") && i%3===0)await turn("KNOB 1",i<6?1:-1);await shot(`${name}-frame-${String(i).padStart(2,'0')}`);await page.waitForTimeout(100);}};
  const large=async on=>{await tap('HOME',900);await turn('PRESETS',-(await info())[9]);await turn('PRESETS',2);await turn('KNOB 1',on?1:-1);await tap('HOME');assert.equal((await info())[10],on?1:0);};
  await page.evaluate(()=>{window.meter=testAC.createAnalyser();meter.fftSize=4096;testNode.connect(meter);});
  const peak=()=>page.evaluate(()=>{const a=new Float32Array(4096);meter.getFloatTimeDomainData(a);return Math.max(...a.map(Math.abs));});
  let audioPeak=0;
  for(let mode=0;mode<2;mode++){
   if(mode){
    // This browser context belongs only to this acceptance run. Start LARGE's
    // same navigation recipe fresh, not at EDIT's remembered last family page.
    await page.evaluate(()=>localStorage.clear());await page.reload();
    await page.locator('#go').click();await page.waitForFunction(()=>document.querySelector('#start').hidden);await page.waitForTimeout(1500);
    await large(true);
    await page.evaluate(()=>{window.meter=testAC.createAnalyser();meter.fftSize=4096;testNode.connect(meter);});
   }
   await tap('HOME');
   // HOME cycles through MIXER/CLOCK if already home. Return from ENV after the
   // first iteration; initially fresh startup is already CREW, so reset via ENV.
   await tap('ENV');await tap('HOME');
   let q=await info();while(q[2]!==0){await turn('ALGORITHM');q=await info();}
   await shot(`stage-${mode}-stop`);await tap('PLAY / STOP');await page.waitForTimeout(500);audioPeak=Math.max(audioPeak,await peak());
   await shot(`stage-${mode}-play`);await clip(`stage-${mode}`);await tap('PLAY / STOP');
   await turn('ALGORITHM',4);await tap('EDIT');await tap('EDIT');await tap('EDIT');
   if(!process.env.BGIRL_STAGE_BASELINE)await page.waitForFunction(()=>{
    const a=document.querySelector('#screen').getContext('2d').getImageData(10,34,65,20).data;
    let ink=0;for(let i=0;i<a.length;i+=4)if(a[i]>110&&a[i+1]>110&&a[i+2]>110)ink++;
    return ink>70;
   },null,{timeout:15000});
   await shot(`lab-${mode}-stop`);const before=await page.locator('#screen').evaluate(c=>c.toDataURL());
   await turn('KNOB 1',1);await turn('KNOB 2',2);await turn('KNOB 3',4);await turn('KNOB 4',-2);
   assert.notEqual(await page.locator('#screen').evaluate(c=>c.toDataURL()),before);await shot(`lab-${mode}-edit`);await clip(`lab-${mode}`);
   await tap('HOME');await turn('ALGORITHM',2);await tap('SEQ');await shot(`cut-${mode}-stop`);
   await tap('PLAY / STOP');await turn('KNOB 1',18);await turn('KNOB 2',2);await shot(`cut-${mode}-edit`);await clip(`cut-${mode}`);await tap('PLAY / STOP');
   await tap('ENV');await shot(`env-${mode}-stop`);await turn('KNOB 1',5);await turn('KNOB 2',-3);await shot(`env-${mode}-edit`);
  }
  assert(audioPeak>.00001&&audioPeak<=1);assert.deepEqual(errors,[]);
  await page.setViewportSize({width:390,height:844});await page.waitForTimeout(200);assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),false);await page.screenshot({path:`${dir}/mobile.png`,fullPage:true});
  const result={audioPeak,normalAndLarge:true,physicalNavigation:true,knobEdits:true,errors,approval:'Doug review pending',hardware:'not tested'};
  fs.writeFileSync(`${dir}/results.json`,JSON.stringify(result,null,2)+'\n');console.log(result);
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
