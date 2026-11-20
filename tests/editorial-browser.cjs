// SPDX-License-Identifier: GPL-3.0-only
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict'),fs=require('node:fs');
(async()=>{
 const browser=await chromium.launch({headless:true,args:['--no-sandbox'],...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{})});
 try{
 const page=await browser.newPage({viewport:{width:1300,height:1000}}),errors=[];
 page.on('pageerror',e=>errors.push(e.message));
 await page.addInitScript(()=>{const A=window.Audio;window.Audio=class extends A{constructor(...a){super(...a);window.excerpt=this;}};});
 await page.goto(process.env.BGIRL_FLYER_URL||'http://127.0.0.1:8852/bgirl-fm1/');
 assert.deepEqual(await page.locator('article section').evaluateAll(es=>es.map(e=>e.id)),['voice','patches','sampler','arp']);
 assert(await page.evaluate(()=>excerpt.paused));
 await page.evaluate(()=>localStorage.setItem('editorial-sentinel','untouched'));
 for(const clip of ['dry','saw','trio','sub-clean','sub-drive','hit','pocket']){
  const b=page.locator('[data-clip="'+clip+'"]').first();await b.scrollIntoViewIfNeeded();await b.focus();await page.keyboard.press('Enter');
  await page.waitForFunction(()=>!excerpt.paused&&excerpt.currentTime>0);
  assert((await page.evaluate(()=>excerpt.src)).endsWith(clip+'.wav'));
  assert.equal(await page.locator('[aria-pressed=true]').count(),1);
 }
 await page.locator('#stop-excerpt').click();assert(await page.evaluate(()=>excerpt.paused));
 await page.locator('[data-clip=dry]').click();await page.waitForFunction(()=>!excerpt.paused);
 await page.locator('#start').click();await page.waitForFunction(()=>!document.querySelector('#answer').disabled);
 assert(await page.evaluate(()=>excerpt.paused));
 await page.locator('[data-clip=saw]').first().click();await page.waitForFunction(()=>!excerpt.paused);
 assert(await page.locator('#answer').isDisabled());
 await page.locator('#stop-excerpt').click();
 for(const image of await page.locator('img').all()){assert(await image.evaluate(i=>i.complete&&i.naturalWidth>0));}
 const out='.powderline/editorial';fs.mkdirSync(out,{recursive:true});
 for(const [name,width,height] of [['desktop',1300,1000],['mobile',390,844]]){
  await page.setViewportSize({width,height});await page.emulateMedia({reducedMotion:'reduce'});
  await page.screenshot({path:out+'/'+name+'.png',fullPage:true});
  assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),false);
  await page.locator('#arp').scrollIntoViewIfNeeded();assert(await page.evaluate(()=>excerpt.paused));
 }
 assert.equal(await page.evaluate(()=>localStorage.getItem('editorial-sentinel')),'untouched');
 const levels=await (await page.request.get(new URL('editorial/manifest.json',page.url()).href)).json();
 assert.equal(levels['sub-clean'].matched_rms,levels['sub-drive'].matched_rms);
 assert.notEqual(levels.hit.raw_rms,levels.pocket.raw_rms);
 for(const a of await page.locator('a[href]').all()){const href=await a.getAttribute('href');if(!href.startsWith('https:'))assert((await page.request.get(new URL(href,page.url()).href)).ok(),href);}
 const touch=await browser.newPage({viewport:{width:390,height:844},hasTouch:true,isMobile:true});
 await touch.goto(process.env.BGIRL_FLYER_URL||'http://127.0.0.1:8852/bgirl-fm1/');
 await touch.locator('[data-clip=hit]').tap();await touch.getByRole('status').filter({hasText:'Playing: Voice hit alone'}).waitFor();
 await touch.locator('#stop-excerpt').tap();await touch.getByRole('status').filter({hasText:'Excerpt stopped.'}).waitFor();await touch.close();
 assert.deepEqual(errors,[]);console.log('PASS: seven real excerpts, keyboard, exclusive audio, Stop, stills, prefix links, mobile/reduced motion, untouched storage.');
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
