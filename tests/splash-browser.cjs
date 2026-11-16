// SPDX-License-Identifier: GPL-3.0-only
// Capture the first real AudioWorklet framebuffer: no boot-delay/test-only renderer.
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict'),fs=require('node:fs');
(async()=>{
 const header=fs.readFileSync('felucca/build/gen/bgirl_logo.h','utf8');
 const parse=name=>[...header.match(new RegExp(name+'\\[\\d+\\] = \\{([\\s\\S]*?)\\}'))[1].matchAll(/0x([0-9a-f]+)/g)].map(m=>parseInt(m[1],16));
 const palette=parse('BG_LOGO_PALETTE'),packed=parse('BG_LOGO_PIXELS');
 const browser=await chromium.launch({headless:true,args:['--no-sandbox'],...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{})});
 try {
  const page=await browser.newPage({viewport:{width:1300,height:1000}}),errors=[];
  page.on('pageerror',e=>errors.push(e.message));
  await page.addInitScript(()=>{
   const AW=window.AudioWorkletNode;
   window.AudioWorkletNode=class extends AW{constructor(...args){super(...args);this.port.addEventListener('message',e=>{
    if(e.data.fb){if(!window.splashFrame)window.splashFrame=Array.from(e.data.fb);window.latestFrame=Array.from(e.data.fb);}
   });}};
  });
  await page.goto(process.env.BGIRL_URL||'http://127.0.0.1:8782');
  await page.evaluate(()=>document.fonts.ready);await page.locator('#go').click();
  await page.waitForFunction(()=>window.splashFrame);
  const first=await page.evaluate(()=>window.splashFrame);assert.equal(first.length,240*240);
  for(let y=0;y<145;y++)for(let x=0;x<224;x++){
   const pos=y*224+x,idx=(pos&1)?packed[pos>>1]&15:packed[pos>>1]>>4;
   const c=first[(28+y)*240+8+x],rgb565=((c>>8)|(c<<8))&65535;
   assert.equal(rgb565,palette[idx],`initial logo pixel ${x},${y}`);
  }
  const out=process.env.BGIRL_ARTIFACTS||'tmp/splash-browser';fs.mkdirSync(out,{recursive:true});
  // Export the captured first browser frame at native size, even on a slow screenshot runner.
  const png=await page.evaluate(fb=>{
   const c=document.createElement('canvas');c.width=c.height=240;const ctx=c.getContext('2d'),im=ctx.createImageData(240,240);
   fb.forEach((v,k)=>{const p=((v>>8)|(v<<8))&65535;im.data[k*4]=(p>>11)*255/31;im.data[k*4+1]=((p>>5)&63)*255/63;im.data[k*4+2]=(p&31)*255/31;im.data[k*4+3]=255;});
   ctx.putImageData(im,0,0);return c.toDataURL().split(',')[1];
  },first);
  fs.writeFileSync(`${out}/splash-native.png`,Buffer.from(png,'base64'));
  await page.waitForTimeout(1100);assert.notDeepEqual(await page.evaluate(()=>window.latestFrame),first);
  assert.equal(await page.locator('#start').evaluate(e=>e.hidden),true);
  for(const [name,width,height] of [['desktop',1300,1000],['mobile',390,844]]){
   await page.setViewportSize({width,height});await page.waitForTimeout(200);
   assert.equal(await page.locator('#circuit-mark').count(),0);
   assert(!/BG[–-]01\s*\/\s*BATTLE CIRCUIT/i.test(await page.locator('main').innerText()));
   assert.equal(await page.locator('.hit').count(),49);
   assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),false);
   await page.screenshot({path:`${out}/${name}.png`,fullPage:true});
  }
  assert.deepEqual(errors,[]);const result={logoPixels:224*145,bootTransition:'original timing, splash replaced by instrument',panelTargets:49,inscription:'absent in desktop/mobile',errors,hardware:'not tested'};
  fs.writeFileSync(`${out}/results.json`,JSON.stringify(result,null,2)+'\n');console.log(result);
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
