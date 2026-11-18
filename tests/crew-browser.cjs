// SPDX-License-Identifier: GPL-3.0-only
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict'),fs=require('node:fs');
(async()=>{
 const browser=await chromium.launch({headless:true,args:['--no-sandbox'],...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{})});
 try {
  const page=await browser.newPage({viewport:{width:1300,height:1100}}),errors=[];
  page.on('pageerror',e=>errors.push(e.message));
  await page.addInitScript(()=>{
   const AC=window.AudioContext;window.AudioContext=class extends AC{constructor(o){super(o);window.testAC=this;}};
   const AW=window.AudioWorkletNode;window.AudioWorkletNode=class extends AW{constructor(...a){super(...a);window.testNode=this;this.port.addEventListener('message',e=>{if(e.data.type==='crew-info')window.crewInfo=e.data.values;if(e.data.load!==undefined)(window.loads??=[]).push(e.data.load);});}};
  });
  const artifacts=process.env.BGIRL_CREW_ARTIFACTS||'tmp/crew-browser';fs.mkdirSync(artifacts,{recursive:true});
  const info=async()=>{await page.evaluate(()=>{window.crewInfo=null;testNode.port.postMessage({type:'crew-info'});});await page.waitForFunction(()=>window.crewInfo);return page.evaluate(()=>window.crewInfo);};
  const tap=async(name,delay=90)=>{await page.getByRole('button',{name,exact:true}).click({delay});await page.waitForTimeout(220);};
  const turn=async(name,dir=1,count=1)=>{for(let k=0;k<count;k++){await page.getByRole('slider',{name,exact:true}).press(dir>0?'ArrowUp':'ArrowDown');await page.waitForTimeout(60);}};
  const key=async down=>page.evaluate(down=>testNode.port.postMessage({type:'keys',mask:down?1:0}),down);
  const on=async()=>{await page.locator('#go').click();await page.waitForFunction(()=>document.querySelector('#start').hidden);await page.waitForTimeout(1300);};
  await page.goto(process.env.BGIRL_URL||'http://127.0.0.1:8796');await on();
  let q=await info();assert.equal(q[0],3);assert.equal(q[1],1);assert.equal(q[2],0);
  const idle=q[5];await page.waitForTimeout(400);assert.equal((await info())[5],idle);
  await page.evaluate(()=>{window.analyser=testAC.createAnalyser();analyser.fftSize=4096;testNode.connect(analyser);});
  await tap('PLAY / STOP');await page.waitForTimeout(700);
  const peak=await page.evaluate(()=>{const a=new Float32Array(4096);analyser.getFloatTimeDomainData(a);return Math.max(...a.map(Math.abs));});assert(peak>.0001 && peak<=1);
  const screens=[];
  for(let t=0;t<6;t++) {
   q=await info();assert.equal(q[2],t);assert.equal(q[4],[0,1,2,3,4,0][t]);assert.equal(q[1],1);
   screens.push(await page.locator('#screen').evaluate(c=>c.toDataURL()));
   await page.locator('#screen').screenshot({path:`${artifacts}/track-${t+1}.png`});
   await turn('ALGORITHM');
  }
  assert.equal(new Set(screens).size,6);for(let i=0;i<24;i++){await turn('ALGORITHM');assert.equal((await info())[2],(i+1)%6);}
  await turn('ALGORITHM',-1);q=await info();assert.equal(q[2],5);assert.equal(q[3]&768,0); // Empty T6 while other tracks play.
  await turn('ALGORITHM',-1,4); // T2 bass
  // Engine knob remains a normal musical edit; screenshot keeps the real labels.
  await turn('KNOB 1');await tap('HOME');assert.equal((await info())[1],0); // MIXER
  await turn('KNOB 4',1);await tap('HOME');await tap('HOME');q=await info();assert.equal(q[1],1);assert.equal(q[7],1);assert(q[3]&1024);
  await page.locator('#screen').screenshot({path:`${artifacts}/muted.png`});
  await tap('HOME');await turn('KNOB 4',-1);await tap('HOME');await tap('HOME');
  await tap('PLAY / STOP');await page.waitForTimeout(1500);
  await key(true);await page.waitForTimeout(150);assert((await info())[3]&256);await key(false);
  // Hidden ARP family owns the display and does not paint CREW.
  await tap('ARP');const before=(await info())[5];await key(true);await page.waitForTimeout(400);await key(false);assert.equal((await info())[5],before);
  await tap('HOME');assert.equal((await info())[1],1);
  // Six-track GLO bank follows T5/T6; inactive knobs remain covered by six-browser.cjs.
  await turn('ALGORITHM',1,4);const glo=page.getByRole('button',{name:'GLO',exact:true});await glo.hover();await page.mouse.down();await page.waitForTimeout(800);
  q=await info();assert.equal(q[1],0);const covered=q[5];await page.waitForTimeout(350);assert.equal((await info())[5],covered);await page.mouse.up();await page.waitForTimeout(300);
  // Normal DISPLAY menu: COLOR/STYLE/LARGE, then HOME row7. Preserve the normal controls.
  await tap('HOME',900);await turn('KNOB 1');await turn('PRESETS');await turn('KNOB 1');await turn('PRESETS');await turn('KNOB 1');
  await tap('HOME');q=await info();assert.equal(q[10],1);assert.equal(q[12],1);await page.locator('#screen').screenshot({path:`${artifacts}/large-line.png`});
  await page.screenshot({path:`${artifacts}/desktop.png`,fullPage:true});
  // Save all four HOME choices through the menu and reload real NOR/localStorage settings.
  for(const view of [0,1,2,3]) {
   await tap('HOME',900);await turn('PRESETS',-1,(await info())[9]);await turn('PRESETS',1,7);await turn('KNOB 1',-1,5);await turn('KNOB 1',1,view);await tap('HOME');
   assert.equal((await info())[0],view);await page.waitForTimeout(2400);
   await page.reload();await on();assert.equal((await info())[0],view);
  }
  await page.setViewportSize({width:390,height:844});await page.waitForTimeout(200);assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),false);
  await page.screenshot({path:`${artifacts}/mobile.png`,fullPage:true});
  assert.deepEqual(errors,[]);
  const result={tracks:6,visibleDancers:1,distinctScreens:new Set(screens).size,peak,viewsRestored:[0,1,2,3],mute:true,stoppedLive:true,emptyTrackIsolated:true,hiddenAndCoveredNoPaint:true,loads:await page.evaluate(()=>window.loads||[]),errors,humanArtApproval:'pending',hardware:'unverified'};
  fs.writeFileSync(`${artifacts}/results.json`,JSON.stringify(result,null,2)+'\n');console.log(result);
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
