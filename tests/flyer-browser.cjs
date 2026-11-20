// SPDX-License-Identifier: GPL-3.0-only
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict'),fs=require('node:fs');
(async()=>{
 const browser=await chromium.launch({headless:true,args:['--no-sandbox'],...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{})});
 try {
 const page=await browser.newPage(),errors=[];
 page.on('pageerror',e=>errors.push(e.message));
 await page.addInitScript(()=>{
  const AC=window.AudioContext;window.AudioContext=class extends AC{constructor(o){super(o);window.testAC=this;}};
  const AW=window.AudioWorkletNode;window.AudioWorkletNode=class extends AW{constructor(...a){super(...a);window.testNode=this;this.port.addEventListener('message',({data:m})=>{if(m.type==='flyer-result')(window.results??=[]).push(m);});}};
 });
 const url=process.env.BGIRL_FLYER_URL||'http://127.0.0.1:8841/bgirl-fm1/';
 await page.goto(url);
 await page.evaluate(()=>localStorage.setItem('bgirl-felucca-v1-flash',JSON.stringify({studioSentinel:'unchanged'})));
 const stored=await page.evaluate(()=>JSON.stringify({...localStorage}));
 assert.equal(await page.evaluate(()=>!!window.testAC),false);
 await page.locator('#start').focus();await page.keyboard.press('Enter');
 await page.waitForFunction(()=>!document.querySelector('#answer').disabled);
 await page.evaluate(()=>{window.meter=testAC.createAnalyser();meter.fftSize=4096;testNode.connect(meter);});
 const peak=()=>page.evaluate(()=>{let a=new Float32Array(4096);meter.getFloatTimeDomainData(a);return Math.max(...a.map(Math.abs));});
 let beatPeak=0;for(let i=0;i<8;i++){await page.waitForTimeout(100);beatPeak=Math.max(beatPeak,await peak());}
 assert(beatPeak>1e-5&&beatPeak<=1);
 for(const delay of [210,770,1450]){
  await page.waitForTimeout(delay);await page.locator('#answer').focus();await page.keyboard.press('Space');
  await page.waitForTimeout(100);assert((await page.evaluate(()=>window.results.filter(r=>r.action===1&&r.ok).length))>0);
 }
 await page.locator('#stop').click();await page.waitForFunction(()=>testAC.state==='suspended');
 assert(await page.locator('#answer').isDisabled());
 await page.locator('#start').click();await page.waitForFunction(()=>testAC.state==='running'&&!document.querySelector('#answer').disabled);
 await page.locator('#answer').click();await page.locator('#stop').click();await page.waitForFunction(()=>testAC.state==='suspended');
 assert.equal(await page.evaluate(()=>JSON.stringify({...localStorage})),stored);
 const out=process.env.BGIRL_ARTIFACTS||'.powderline/flyer';fs.mkdirSync(out,{recursive:true});
 for(const [name,width,height] of [['desktop',1300,1000],['mobile',390,844]]){
  await page.setViewportSize({width,height});await page.emulateMedia({reducedMotion:'reduce'});
  assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),false);
  await page.screenshot({path:out+'/'+name+'.png',fullPage:true});
 }
 await page.getByRole('link',{name:'Open full studio'}).click();
 assert.equal(new URL(page.url()).pathname,new URL('play/',url).pathname);
 assert(await page.locator('#go').isVisible());assert(await page.locator('#song-import').count());
 for(const asset of ['felucca.wasm','worklet.js','song-snapshot.mjs','listening/dry.flac']){
  assert.equal((await page.request.get(new URL(asset,page.url()).href)).status(),200,asset);
 }
 const touch=await browser.newPage({viewport:{width:390,height:844},hasTouch:true,isMobile:true});
 await touch.goto(url);await touch.locator('#start').tap();await touch.waitForFunction(()=>!document.querySelector('#answer').disabled);await touch.locator('#answer').tap();await touch.locator('#stop').tap();await touch.waitForFunction(()=>document.querySelector('#answer').disabled);await touch.close();
 const failed=await browser.newPage();await failed.route('**/felucca.wasm',r=>r.abort());await failed.goto(url);
 await failed.locator('#start').click();await failed.getByRole('status').filter({hasText:'Audio could not start'}).waitFor();
 assert(await failed.locator('figure img').isVisible());assert(await failed.getByRole('link',{name:'Open full studio'}).isVisible());
 assert.deepEqual(errors,[]);
 console.log({beatPeak,answerTimingsMs:[210,770,1450],stopRestart:true,storageUnchanged:true,routes:true,failureFallback:true,errors});
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
