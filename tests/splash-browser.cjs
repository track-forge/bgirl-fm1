// SPDX-License-Identifier: GPL-3.0-only
// Capture the first real AudioWorklet framebuffer: no boot-delay/test-only renderer.
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict'),fs=require('node:fs');
(async()=>{
 const header=fs.readFileSync('felucca/build/gen/bgirl_logo.h','utf8');
 const parse=name=>[...header.match(new RegExp(name+'\\[\\d+\\] = \\{([\\s\\S]*?)\\}'))[1].matchAll(/0x([0-9a-f]+)/g)].map(m=>parseInt(m[1],16));
 const palette=parse('BG_LOGO_PALETTE'),alpha=parse('BG_LOGO_ALPHA'),packed=parse('BG_LOGO_PIXELS');
 const width=+header.match(/#define BG_LOGO_W (\d+)u/)[1],height=+header.match(/#define BG_LOGO_H (\d+)u/)[1];
 const x0=Math.floor((240-width)/2),y0=Math.floor((240-height)/2);
 assert.equal(x0,8);assert.equal(y0,47); // center the unchanged 224x145 artwork to the nearest pixel
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
  const unswap=c=>((c>>8)|(c<<8))&65535,bg=unswap(first[0]);
  const blend=(color,a)=>{
   const percent=Math.floor((a*100+127)/255);let value=0;
   for(const [shift,mask] of [[11,31],[5,63],[0,31]]){
    const start=(bg>>shift)&mask,d=(((color>>shift)&mask)-start)*percent;
    value|=(start+Math.trunc((d+(d>=0?50:-50))/100))<<shift;
   }
   return value;
  };
  let transparentPixels=0,outsidePixels=0;
  for(let y=0;y<height;y++)for(let x=0;x<width;x++){
   const pos=y*width+x,idx=(pos&1)?packed[pos>>1]&15:packed[pos>>1]>>4;
   assert.equal(unswap(first[(y0+y)*240+x0+x]),blend(palette[idx],alpha[idx]),`initial logo pixel ${x},${y}`);
   if(alpha[idx]===0)transparentPixels++;
  }
  assert(transparentPixels>0,'source transparency must survive packing');
  for(let y=0;y<240;y++)for(let x=0;x<240;x++){
   if(x<x0||x>=x0+width||y<y0||y>=y0+height){
    assert.equal(unswap(first[y*240+x]),bg,`only background outside logo at ${x},${y}`);outsidePixels++;
   }
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
  assert.deepEqual(errors,[]);const result={logoPixels:width*height,transparentPixels,outsidePixels,placement:{x:x0,y:y0,width,height},text:'none',background:'current palette, no backing rectangle',bootTransition:'original timing, splash replaced by instrument',panelTargets:49,inscription:'absent in desktop/mobile',errors,hardware:'not tested'};
  fs.writeFileSync(`${out}/results.json`,JSON.stringify(result,null,2)+'\n');console.log(result);
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
