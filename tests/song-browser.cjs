// SPDX-License-Identifier: GPL-3.0-only
const {chromium}=require(process.env.PLAYWRIGHT_MODULE||'playwright');
const assert=require('node:assert/strict'),fs=require('node:fs');
(async()=>{
 const {semantic,diff,toWire,CAPABILITY}=await import('../felucca/web/emu/song-snapshot.mjs');
 const browser=await chromium.launch({headless:true,args:['--no-sandbox'],...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{})});
 const artifacts='tmp/song-browser';fs.mkdirSync(artifacts,{recursive:true});
 try{
 const ctx=await browser.newContext({acceptDownloads:true}),page=await ctx.newPage(),errors=[];
 await ctx.addInitScript(()=>{
  const AC=window.AudioContext;window.AudioContext=class extends AC{constructor(o){super(o);window.testAC=this;}};
  const AW=window.AudioWorkletNode;window.AudioWorkletNode=class extends AW{constructor(...a){super(...a);window.testNode=this;}};
 });
 page.on('pageerror',e=>errors.push(e.message));
 const url=process.env.BGIRL_URL||'http://127.0.0.1:8813';
 const on=async p=>{await p.goto(url);await p.locator('#go').click();await p.waitForFunction(()=>document.querySelector('#start').hidden);await p.waitForTimeout(1300);};
 await on(page);
 const req=(type,doc,deadline=Date.now()+8000)=>page.evaluate(({type,doc,deadline})=>new Promise((resolve,reject)=>{
  const id=Math.random();const timer=setTimeout(()=>reject(Error('test request timeout')),10000);
  const fn=e=>{if(e.data.type==='song-result'&&e.data.id===id){clearTimeout(timer);testNode.port.removeEventListener('message',fn);resolve(e.data);}};
  testNode.port.addEventListener('message',fn);testNode.port.postMessage({type,doc,deadline,id});
 }),{type,doc,deadline});
 const read=async()=>{const r=await req('song-capture');assert(!r.error,r.error);return r.doc;};
 const exportFile=async p=>{
   const downloaded=p.waitForEvent('download');await p.locator('#song-export').click();const download=await downloaded;
   const file=`${artifacts}/export-${Date.now()}.bgirl.json`;await download.saveAs(file);return JSON.parse(fs.readFileSync(file,'utf8'));
 };
 const upload=async(doc,accept=true,p=page)=>{
   p.once('dialog',d=>accept?d.accept():d.dismiss());
   await p.locator('#song-file').setInputFiles({name:'agent-edit.bgirl.json',mimeType:'application/json',buffer:Buffer.from(JSON.stringify(doc))});
   await p.waitForFunction(()=>/Imported, stopped|Import cancelled|Snapshot failed|Stop playback|Missing\//.test(document.querySelector('#song-status').textContent));
 };
 const tap=async name=>{await page.getByRole('button',{name,exact:true}).click({delay:90});await page.waitForTimeout(300);};
 const turn=async(name)=>{await page.getByRole('slider',{name,exact:true}).press('ArrowUp');await page.waitForTimeout(180);};
 const flash=()=>page.evaluate(()=>{const d=JSON.parse(localStorage.getItem('bgirl-felucca-v1-flash')||'{}');delete d._portableSong;return d;});
 // Populate destination slot A through the real SAVE controls, not a test setter.
 for(let i=0;i<4;i++)await tap('SAVE');await turn('KNOB 4');await tap('OCT+');await tap('OCT+');await tap('HOME');
 assert(Object.keys(await flash()).length>0);
 const original=await exportFile(page);
 assert.equal(original.capability,CAPABILITY);assert.equal(original.song.tracks.length,6);
 // Immediate normal-panel unsaved sound/mix edit, before the 10-second autosave.
 await turn('KNOB 1');const live=await exportFile(page);assert(diff(semantic(original),semantic(live)).length>0);
 // Two canonical musical edits from an external agent, using the exported file.
 let edited=structuredClone(live);edited.title='Agent audition';
 edited.song.tracks[5].parameters.P_PAN=15;
 edited.song.tracks[4].steps[63]={notes:[57,0,0,0],n:1,time:0,flags:0,velocity:99,hits:0,accents:0,probability:0};
 edited.song.motion={enabledTracks:63,events:[{track:5,step:63,parameter:'P_PAN',lock:true,value:12}]};
 edited.song.tracks[4].parameters.P_E7=8; // OWN: embedded custom patch, not a factory selector
 edited.song.tracks[4].fm6[0]=(edited.song.tracks[4].fm6[0]+1)%99;
 toWire(edited);
 const stored=await flash();
 await upload(edited);await page.waitForTimeout(400);
 const actual=await read();assert.equal(actual.title,edited.title);assert.deepEqual(semantic(actual),semantic(edited));assert.deepEqual(await flash(),stored);
 // Ordinary post-import edits checkpoint as a paired full song, not B6S1 rows.
 await turn('KNOB 1');await page.waitForTimeout(2300);
 const postEdit=await read();assert(diff(semantic(postEdit),semantic(edited)).length>0);edited=postEdit;
 // Saved slots/libraries survive refresh; browser checkpoint owns the whole imported song.
 await page.reload();await page.locator('#go').click();await page.waitForFunction(()=>document.querySelector('#start').hidden);await page.waitForTimeout(1300);
 assert.deepEqual(semantic(await read()),semantic(edited));assert.deepEqual(await flash(),stored);
 // Fresh profile receives the downloaded/agent-edited file, not origin flash.
 const freshCtx=await browser.newContext();const fresh=await freshCtx.newPage();await on(fresh);await upload(edited,true,fresh);
 const freshDoc=await exportFile(fresh);assert.deepEqual(semantic(freshDoc),semantic(edited));await freshCtx.close();
 // Cancellation preserves latched controls, music and storage.
 await page.getByRole('button',{name:'GLO',exact:true}).click({modifiers:['Shift']});await page.waitForTimeout(200);
 const latch=await page.locator('.latched .face.btn').count();assert(latch>0);
 await upload(edited,false);assert.equal(await page.locator('.latched .face.btn').count(),latch);
 await page.getByRole('button',{name:'GLO',exact:true}).click({modifiers:['Shift']});await page.waitForTimeout(250);
 const before=await read(),beforeStorage=await flash();
 const bad=structuredClone(edited);bad.song.tracks[5].parameters.P_PAN='loud';
 await page.locator('#song-file').setInputFiles({name:'bad.json',mimeType:'application/json',buffer:Buffer.from(JSON.stringify(bad))});
 await page.waitForFunction(()=>document.querySelector('#song-status').textContent.includes('expected integer'));
 assert.deepEqual(semantic(await read()),semantic(before));assert.deepEqual(await flash(),beforeStorage);
 const expired=await req('song-import',original,Date.now()-1000);assert(expired.error.includes('expired'));
 assert.deepEqual(semantic(await read()),semantic(before));
 const missing=structuredClone(edited);missing.song.tracks[3].vocalIdentity[0]^=1;
 missing.resources.vocalBanks=missing.song.tracks[3].vocalIdentity.map(v=>v.toString(16).padStart(2,'0')).join('')?[missing.song.tracks[3].vocalIdentity.map(v=>v.toString(16).padStart(2,'0')).join('')]:[];
 const rejected=await req('song-import',missing);assert(rejected.error.includes('Missing/mismatched'));
 assert.deepEqual(semantic(await read()),semantic(before));
 // Multi-source SONG: original instruments + bundled source patterns; named slots untouched.
 const arranged=structuredClone(edited);arranged.title='Portable arrangement';
 const a=structuredClone(edited.song),b=structuredClone(edited.song);
 a.tracks[1].steps[0].notes[0]=40;b.tracks[1].steps[0].notes[0]=52;
 arranged.dependencies=[{id:'pattern-0',song:a},{id:'pattern-1',song:b}];
 arranged.song.arrangement.count=2;
 arranged.song.arrangement.rows[0]={sources:['pattern-0','pattern-1','pattern-0','pattern-1','pattern-0','pattern-1'],repeat:1};
 arranged.song.arrangement.rows[1]={sources:['pattern-1','pattern-0','pattern-1','pattern-0','pattern-1','pattern-0'],repeat:2};
 await upload(arranged);await page.waitForTimeout(400);
 assert.deepEqual(semantic(await read()),semantic(arranged));assert.deepEqual(await flash(),beforeStorage);
 await page.evaluate(()=>{window.analyser=testAC.createAnalyser();analyser.fftSize=4096;testNode.connect(analyser);});
 // GLO => SONG; normal sequencer starts through inherited panel controls.
 await tap('GLO');await tap('PLAY / STOP');await page.waitForTimeout(1700);
 const peak=await page.evaluate(()=>{const a=new Float32Array(4096);analyser.getFloatTimeDomainData(a);return Math.max(...a.map(Math.abs));});assert(peak>.0001&&peak<=1);
 const busy=await req('song-capture');assert(busy.error.includes('Stop playback'));
 await tap('PLAY / STOP');await tap('HOME');await page.waitForTimeout(600);
 await page.reload();await page.locator('#go').click();await page.waitForFunction(()=>document.querySelector('#start').hidden);await page.waitForTimeout(1300);
 assert.deepEqual(semantic(await read()),semantic(arranged));assert.deepEqual(await flash(),beforeStorage);
 // New project detaches all source state, including persisted checkpoint.
 page.once('dialog',d=>d.accept());await page.locator('#blank').click();await page.waitForTimeout(500);
 assert.equal((await read()).dependencies.length,0);assert.equal(await page.evaluate(()=>JSON.parse(localStorage.getItem('bgirl-felucca-v1-flash'))._portableSong),undefined);
 // FULL is a hardware estimate only; export/import still carries all 384 steps.
 page.once('dialog',d=>d.accept());await page.locator('#six-dense').click();await page.waitForTimeout(500);
 const dense=await exportFile(page);assert((await page.locator('#song-status').textContent()).includes('FULL'));
 await upload(dense);assert.deepEqual(semantic(await read()),semantic(dense));
 await page.setViewportSize({width:390,height:844});assert.equal(await page.evaluate(()=>document.documentElement.scrollWidth>innerWidth),false);
 await exportFile(page);await page.screenshot({path:`${artifacts}/mobile.png`,fullPage:true});
 await page.setViewportSize({width:1300,height:1100});await page.screenshot({path:`${artifacts}/desktop.png`,fullPage:true});
 // Storage-denied mode is truthful and export remains possible.
 await page.evaluate(()=>{Storage.prototype.setItem=()=>{throw Error('denied')};});
 page.once('dialog',d=>d.accept());
 await page.locator('#song-file').setInputFiles({name:'session.json',mimeType:'application/json',buffer:Buffer.from(JSON.stringify(edited))});
 await page.waitForFunction(()=>document.querySelector('#song-status').textContent.includes('SESSION ONLY'));
 await exportFile(page);
 assert.deepEqual(errors,[]);
 const results={normalUnsavedEdit:true,actualDownloadUpload:true,agentParameterNoteMotionCustomFM6:true,freshProfile:true,refresh:true,cancelPreservesLatch:true,invalidAtomic:true,expiredRequestAtomic:true,missingBankAtomic:true,twoSourceArrangement:true,slotsUnchanged:true,audioPeak:peak,denseFULL:true,mobile:true,storageDenied:true,errors,humanListening:'not performed'};
 fs.writeFileSync(`${artifacts}/results.json`,JSON.stringify(results,null,2)+'\n');console.log(results);
 }finally{await browser.close();}
})().catch(e=>{console.error(e);process.exitCode=1;});
