// SPDX-License-Identifier: GPL-3.0-only
// Browser wrapper: real input messages, note releases, responsive layout and focus.
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict'),fs=require('node:fs');
(async()=>{
 const browser=await chromium.launch({headless:true,args:['--no-sandbox'],...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{})});
 const out=process.env.BGIRL_ARTIFACTS||'tmp/browser-shell';fs.mkdirSync(out,{recursive:true});
 try{
  const page=await browser.newPage({viewport:{width:1300,height:1000},hasTouch:true}),errors=[],requests=[];
  page.on('pageerror',e=>errors.push(e.message));
  page.on('request',r=>requests.push(r.url()));
  await page.addInitScript(()=>{
   window.sent=[];
   const AW=window.AudioWorkletNode;
   window.AudioWorkletNode=class extends AW{constructor(...args){super(...args);const original=this.port.postMessage.bind(this.port);this.port.postMessage=(m,...rest)=>{if(m.type!=='load')sent.push(structuredClone(m));return original(m,...rest);};}};
  });
  await page.goto(process.env.BGIRL_URL||'http://127.0.0.1:8814/bgirl-fm1/');
  await page.evaluate(()=>document.fonts.ready);
  assert.equal(await page.locator('.hit[tabindex="0"]').count(),0,'unpowered panel is not a keyboard trap');
  await page.keyboard.press('Tab');assert.equal(await page.locator('.skip-link').evaluate(e=>e===document.activeElement),true);
  await page.keyboard.press('Enter');assert.equal(await page.locator('#device').evaluate(e=>e===document.activeElement),true);
  const order=await page.evaluate(()=>!!(document.querySelector('#device').compareDocumentPosition(document.querySelector('.intro-card'))&Node.DOCUMENT_POSITION_FOLLOWING));assert(order);
  await page.locator('#go').click();await page.waitForFunction(()=>document.querySelector('#start').hidden);await page.waitForTimeout(1200);
  assert.equal(await page.locator('.hit[tabindex="0"]').count(),49);
  assert.equal(await page.getByRole('button',{name:'HOME',exact:true}).evaluate(e=>e===document.activeElement),true);
  const clear=()=>page.evaluate(()=>{sent.length=0;});
  const messages=()=>page.evaluate(()=>sent.filter(m=>['keys','buttons','enc'].includes(m.type)));
  // Native selects retain their arrows and letter search; links and buttons keep browser keys.
  await page.locator('#signature-sound').focus();await clear();
  await page.keyboard.press('ArrowDown');await page.keyboard.press('KeyZ');await page.keyboard.press('Digit1');
  assert.equal(await page.locator('#signature-sound').evaluate(e=>e.selectedIndex),1);assert.deepEqual(await messages(),[]);
  // Every on-screen note is keyboard playable, including correct key-up and focus-loss release.
  const note=page.getByRole('button',{name:'F3',exact:true});await note.focus();await clear();
  await page.keyboard.down('Space');await page.keyboard.down('Space');
  assert.equal(await note.getAttribute('aria-pressed'),'true');
  await page.keyboard.up('Space');assert.equal(await note.getAttribute('aria-pressed'),'false');
  assert.deepEqual((await messages()).filter(m=>m.type==='keys').map(m=>m.mask),[1,0]);
  await page.keyboard.down('Enter');await page.keyboard.press('Tab');
  assert.equal(await note.getAttribute('aria-pressed'),'false');await page.keyboard.up('Enter');
  // Focused panel buttons release if Tab moves away during the hold.
  const home=page.getByRole('button',{name:'HOME',exact:true});await home.focus();await clear();
  await page.keyboard.down('Space');assert.equal(await home.getAttribute('aria-pressed'),'true');
  await page.keyboard.press('Tab');assert.equal(await home.getAttribute('aria-pressed'),'false');await page.keyboard.up('Space');
  assert.deepEqual((await messages()).filter(m=>m.type==='buttons').map(m=>m.mask),[64,0]);
  // Relative encoder and global shortcuts still emit exactly one intended control event.
  const algorithm=page.getByRole('slider',{name:'ALGORITHM',exact:true});await algorithm.focus();await clear();
  await page.keyboard.press('ArrowUp');assert.deepEqual(await messages(),[{type:'enc',role:1,n:1}]);
  assert.match(await algorithm.getAttribute('aria-valuetext'),/Relative encoder/);
  await page.locator('h1').click();await clear();await page.keyboard.press('KeyZ');
  assert.deepEqual((await messages()).filter(m=>m.type==='keys').map(m=>m.mask),[1,0]);
  // Lost pointer capture must not leave a knob dragging or a key sounding.
  await page.evaluate(()=>{sent.length=0;});
  const box=await note.boundingBox();await page.mouse.move(box.x+box.width/2,box.y+box.height/2);await page.mouse.down();await page.mouse.up();
  assert.equal(await note.getAttribute('aria-pressed'),'false');
  await page.mouse.down();await note.evaluate(e=>e.releasePointerCapture(1));await page.mouse.move(box.x+2,box.y+2);await page.mouse.up();
  assert.equal(await note.getAttribute('aria-pressed'),'false','capture loss releases note');
  const knobBox=await algorithm.boundingBox();await page.mouse.move(knobBox.x+knobBox.width/2,knobBox.y+knobBox.height/2);await page.mouse.down();
  await algorithm.evaluate(e=>e.releasePointerCapture(1));await clear();await page.mouse.move(knobBox.x,knobBox.y-50);await page.mouse.up();
  assert.deepEqual(await messages(),[],'capture loss ends encoder drag');
  await clear();await note.tap();assert.deepEqual((await messages()).filter(m=>m.type==='keys').map(m=>m.mask),[1,0]);
  await home.click({modifiers:['Shift']});assert.equal(await home.getAttribute('aria-pressed'),'true');
  await home.click();assert.equal(await home.getAttribute('aria-pressed'),'false');
  // Both native layouts, narrow phone and 200%-equivalent CSS viewport; no clipped controls.
  const sizes=[['desktop',1300,1000],['mobile',390,844],['narrow',320,740],['landscape',844,390],['zoom-equivalent',650,500]];
  for(const [name,width,height]of sizes){
   await page.setViewportSize({width,height});await page.waitForTimeout(200);
   assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),false,name+' horizontal overflow');
   assert.equal(await page.locator('.hit').count(),49);
   const bounds=await page.locator('.hit').evaluateAll(es=>es.every(e=>{const b=e.getBoundingClientRect(),p=e.parentElement.getBoundingClientRect();return b.x>=p.x-1&&b.right<=p.right+1&&b.y>=p.y-1&&b.bottom<=p.bottom+1;}));assert(bounds,name+' panel targets inside device');
   if(['desktop','mobile'].includes(name)){await page.locator('#device').scrollIntoViewIfNeeded();await page.screenshot({path:`${out}/${name}.png`});await page.screenshot({path:`${out}/${name}-full.png`,fullPage:true});}
  }
  // Orientation rebuild preserves the focused control and releases removed held targets.
  await page.setViewportSize({width:1300,height:1000});await algorithm.focus();
  await page.setViewportSize({width:390,height:844});await page.waitForTimeout(200);
  assert.equal(await algorithm.evaluate(e=>e===document.activeElement),true);
  const contrast=await page.evaluate(()=>{
   const style=getComputedStyle(document.documentElement),get=n=>style.getPropertyValue(n).trim();
   const lum=hex=>{const a=hex.slice(1).match(/../g).map(v=>parseInt(v,16)/255).map(v=>v<=.04045?v/12.92:((v+.055)/1.055)**2.4);return a[0]*.2126+a[1]*.7152+a[2]*.0722;};
   return [['body','--fg','--bg',4.5],['support','--dim','--body',4.5],['panel labels','--label','--part',4.5],['action ink','--bg','--lit',4.5],['focus','--focus','--bg',3],['control edges','--edge','--part',3]].map(([name,a,b,min])=>{const x=lum(get(a)),y=lum(get(b));return {name,ratio:(Math.max(x,y)+.05)/(Math.min(x,y)+.05),min};});
  });
  for(const c of contrast)assert(c.ratio>=c.min,c.name+' contrast');
  // Honor reduced motion for LED breath; firmware animation remains independently controlled.
  await page.emulateMedia({reducedMotion:'reduce'});
  await page.evaluate(()=>document.querySelector('.face.btn').parentElement.classList.add('breath'));
  assert.equal(await page.locator('.breath .face').first().evaluate(e=>getComputedStyle(e).animationName),'none');
  assert.deepEqual(errors,[]);
  assert(requests.every(u=>new URL(u).origin===new URL(page.url()).origin),'no external runtime font/assets');
  const result={viewports:sizes,keyboardNotes:'Enter/Space, key-up, focus-loss, repeat',buttonFocusRelease:true,selectIsolation:true,relativeAndGlobalInput:true,captureLossRelease:true,contrast,focusAfterStartupAndResize:true,panelTargets:49,unpoweredTargetsSkipped:true,reducedMotion:true,noExternalRequests:true,errors,scope:'Browser wrapper only; no spoken firmware readout or hardware claim'};
  fs.writeFileSync(`${out}/results.json`,JSON.stringify(result,null,2)+'\n');console.log(result);
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
