// SPDX-License-Identifier: GPL-3.0-only
// The same bounded validator/codec is used by the browser, worklet and offline CLI.
import {layout} from './snapshot-layout.mjs';
export {layout};
export const FORMAT='bgirl-song', VERSION=1, CAPABILITY='bgirl-six-clarity-127-v1-'+layout.identity, MAX_BYTES=2*1024*1024;
const fail=(p,m)=>{throw new Error(`${p}: ${m}`);};
const integer=(x,lo,hi,p)=>{if(!Number.isInteger(x)||x<lo||x>hi) fail(p,`expected integer ${lo}..${hi}`);return x;};
const object=(x,p)=>{if(!x||typeof x!=='object'||Array.isArray(x))fail(p,'expected object');return x;};
const array=(x,n,p)=>{if(!Array.isArray(x)||x.length!==n)fail(p,`expected ${n} entries`);return x;};
const keys=(x,ks,p)=>{object(x,p); if(Object.keys(x).some(k=>!ks.includes(k))||ks.some(k=>!(k in x)))fail(p,`expected fields ${ks.join(', ')}`);};
const fields=['notes','n','time','flags','velocity','hits','accents','probability'];
const bounds=[[0,4],[0,2],[0,255],[0,127],[0,255],[0,255],[0,112]];
// Explicit wire mapping, not a serialized C struct. No padding or hidden payload.
// B6S1 reserves these otherwise non-automatable IDs for dancer cues.
const visualMotion = ['CREW_MOVE','CREW_RATE','CREW_EFFECT','CREW_AMOUNT'];
const visualMax = [4,2,2,8];
export const motionNames = [...layout.motionParameters.map(i=>layout.parameters[i]), ...visualMotion];

function projectIO(input,read,path='song') {
 const out={},words=[];let pos=0;
 const val=(x,lo,hi,p)=>{if(read)x=input[pos++];integer(x,lo,hi,p);if(!read)words.push(x);return x;};
 const p=read?{}:input;
 if(!read)keys(p,['selectedTrack','name','globals','tracks','arrangement','motion'],path);
 out.selectedTrack=val(p.selectedTrack,0,5,path+'.selectedTrack');
 if(!read && (typeof p.name!=='string'||p.name.length>12||!/^[\x20-\x7e]*$/.test(p.name)||/[a-z]/.test(p.name)))fail(path+'.name','expected at most 12 uppercase printable ASCII characters');
 const name=[];for(let i=0;i<12;i++)name.push(val(read?0:p.name.charCodeAt(i)||0,0,126,path+'.name'));
 out.name=String.fromCharCode(...name).replace(/\0+$/,'');
 out.globals={};if(!read)keys(p.globals,layout.globalIds,path+'.globals');
 layout.globalIds.forEach((id,i)=>out.globals[id]=val(p.globals?.[id],...layout.globals[i],`${path}.globals.${id}`));
 if(!read)array(p.tracks,6,path+'.tracks');
 out.tracks=Array.from({length:6},(_,t)=>{
   const tr=read?{}:p.tracks[t],q={},tp=`${path}.tracks[${t}]`;
   if(!read)keys(tr,['engine','preset','midiChannel','parameters','fm6','vocalIdentity','steps'],tp);
   q.engine=val(tr.engine,0,layout.engines.length-1,tp+'.engine');
   if(q.engine===1)fail(tp+'.engine','retired DIGITAL engine is unsupported');
   q.preset=val(tr.preset,0,255,tp+'.preset');q.midiChannel=val(tr.midiChannel,0,17,tp+'.midiChannel');
   q.parameters={};if(!read)keys(tr.parameters,layout.parameters,tp+'.parameters');
   layout.parameters.forEach((id,i)=>{
     const v=val(tr.parameters?.[id],...(id==='P_VHOLD'?[0,0]:layout.engines[q.engine].parameters[i]),tp+'.parameters.'+id);
     if(layout.engines[q.engine].forbidden[i]?.includes(v))fail(tp+'.parameters.'+id,'noncanonical legacy/bank selector; use a canonical value with embedded FM6 patch');
     q.parameters[id]=v;
   });
   if(!read)array(tr.fm6,layout.fm6Size,tp+'.fm6');
   q.fm6=Array.from({length:layout.fm6Size},(_,i)=>val(tr.fm6?.[i],0,127,tp+`.fm6[${i}]`));
   if(!read)array(tr.vocalIdentity,8,tp+'.vocalIdentity');
   q.vocalIdentity=Array.from({length:8},(_,i)=>val(tr.vocalIdentity?.[i],0,255,tp+'.vocalIdentity'));
   if(!read)array(tr.steps,64,tp+'.steps');
   q.steps=Array.from({length:64},(_,i)=>{
     const s=read?{}:tr.steps[i],v={},sp=tp+`.steps[${i}]`;
     if(!read){keys(s,fields,sp);array(s.notes,4,sp+'.notes');}
     v.notes=Array.from({length:4},(_,j)=>val(s.notes?.[j],0,127,sp+`.notes[${j}]`));
     fields.slice(1).forEach((f,j)=>v[f]=val(s[f],...bounds[j],sp+'.'+f));
     if(v.accents&~v.hits)fail(sp+'.accents','accent must refer to a hit');return v;
   });return q;
 });
 if(!read)keys(p.arrangement,['count','rows'],path+'.arrangement');
 out.arrangement={count:val(p.arrangement?.count,0,16,path+'.arrangement.count'),rows:[]};
 if(!read)array(p.arrangement.rows,16,path+'.arrangement.rows');
 for(let i=0;i<16;i++){
   const r=read?{}:p.arrangement.rows[i],rp=path+`.arrangement.rows[${i}]`;
   if(!read){keys(r,['sources','repeat'],rp);array(r.sources,6,rp+'.sources');}
   const sources=Array.from({length:6},(_,t)=>{let x=read?0:r.sources[t];if(!read)x=x===null?4:Number(/^pattern-[0-3]$/.test(x)?x.slice(-1):NaN);const v=val(x,0,4,rp+'.sources');return v===4?null:`pattern-${v}`;});
   out.arrangement.rows.push({sources,repeat:val(r.repeat,i<out.arrangement.count?1:0,16,rp+'.repeat')});
 }
 if(!read)keys(p.motion,['enabledTracks','events'],path+'.motion');
 out.motion={enabledTracks:val(p.motion?.enabledTracks,0,63,path+'.motion.enabledTracks'),events:[]};
 if(!read && !Array.isArray(p.motion.events))fail(path+'.motion.events','expected array');
 const count=val(read?0:p.motion.events.length,0,128,path+'.motion.events');
 const seen=new Set();
 for(let i=0;i<count;i++){
   const e=read?{}:p.motion.events[i],ep=path+`.motion.events[${i}]`;
   if(!read)keys(e,['track','step','parameter','lock','value'],ep);
   const place=val(read?0:integer(e.track,0,5,ep+'.track')*64+integer(e.step,0,63,ep+'.step'),0,383,ep+'.place');
   if(!read && (typeof e.lock!=='boolean'||!motionNames.includes(e.parameter)))fail(ep,'invalid parameter/lock');
   const visual=visualMotion.indexOf(e.parameter);
   const id=visual>=0 ? 25+visual : layout.parameters.indexOf(e.parameter);
   const param=val(read?0:id+(e.lock?128:0),0,255,ep+'.parameter');
   const code=param&127, cue=code-25, isCue=cue>=0&&cue<visualMotion.length;
   if(isCue ? param&128 : !layout.motionParameters.includes(code))fail(ep+'.parameter','unknown parameter or cue lock');
   const value=val(e.value,isCue?0:-64,isCue?visualMax[cue]:127,ep+'.value'),key=`${place}/${code}`;
   if(seen.has(key))fail(ep,'duplicate track/step/parameter');seen.add(key);
   out.motion.events.push({track:place>>6,step:place&63,parameter:isCue?visualMotion[cue]:layout.parameters[code],lock:!!(param&128),value});
 }
 return read?{project:out,used:pos}:words;
}
export function fromWire(words,producer={}) {
 const mask=integer(words[0],0,15,'mask');let pos=1;
 const song=projectIO(words.slice(pos),true);pos+=song.used;
 const dependencies=[];
 for(let s=0;s<4;s++)if(mask>>s&1){const q=projectIO(words.slice(pos),true);pos+=q.used;dependencies.push({id:`pattern-${s}`,song:q.project});}
 if(pos!==words.length)fail('wire','unexpected trailing values');
 return {format:FORMAT,version:VERSION,capability:CAPABILITY,title:song.project.name||'Untitled',metadata:{producer},resources:resourcesFor(song.project,dependencies),song:song.project,dependencies};
}
function resourcesFor(song,deps){
 const vocals=new Set(),unresolved=new Set();
 for(const q of [song,...deps.map(d=>d.song)])for(let t=0;t<6;t++){
  const tr=q.tracks[t];
  if(tr.engine===4||tr.engine===8){
    const srcs=[tr.parameters.P_E0,...q.motion.events.filter(e=>e.track===t&&e.parameter==='P_E0').map(e=>e.value)];
    for(const src of srcs)if(src>=5)unresolved.add(`private-user-sample-${src-4}`);
  }
  if(tr.engine!==13)continue;
  const srcs=[tr.parameters.P_E0,...q.motion.events.filter(e=>e.track===t&&e.parameter==='P_E0').map(e=>e.value)];
  for(const src of srcs){if(src===6)vocals.add(tr.vocalIdentity.map(v=>v.toString(16).padStart(2,'0')).join(''));if(src>=1&&src<=3)unresolved.add(`private-user-sample-${src}`);}
 }
 return {factory:CAPABILITY,vocalBanks:[...vocals].sort(),unresolved:[...unresolved].sort()};
}
// The previous layout has identical wire fields; only DRUM's range/preset list grew.
export const PRE_KITS_CAPABILITY='bgirl-six-clarity-127-v1-bb83fb2d0515bc0b';
export function toWire(doc){
 object(doc,'document');
 if(Object.keys(doc).some(k=>!['format','version','capability','title','metadata','resources','song','dependencies'].includes(k)))fail('document','unknown required/top-level field; optional extensions belong in metadata');
 for(const k of ['format','version','capability','title','resources','song','dependencies'])if(!(k in doc))fail(k,'required');
 if(doc.format!==FORMAT||doc.version!==VERSION||![CAPABILITY,PRE_KITS_CAPABILITY].includes(doc.capability))fail('document','unsupported format/version/capability');
 if(doc.metadata!==undefined)object(doc.metadata,'metadata');
 if(typeof doc.title!=='string'||doc.title.length>120)fail('title','expected string <=120 characters');
 if(!Array.isArray(doc.dependencies)||doc.dependencies.length>4)fail('dependencies','at most four sources');
 let mask=0;const deps=[...doc.dependencies].sort((a,b)=>String(a.id).localeCompare(String(b.id)));
 for(const d of deps){keys(d,['id','song'],'dependency');if(!/^pattern-[0-3]$/.test(d.id))fail('dependency.id','expected pattern-0..3');const bit=1<<Number(d.id.slice(-1));if(mask&bit)fail('dependency.id','duplicate');mask|=bit;}
 const words=[mask,...projectIO(doc.song,false)];
 for(const d of deps)words.push(...projectIO(d.song,false,`dependencies.${d.id}`));
 for(const r of doc.song.arrangement.rows.slice(0,doc.song.arrangement.count))for(const id of r.sources)if(id!==null&&!deps.some(d=>d.id===id))fail('arrangement','missing dependency '+id);
 const expected=resourcesFor(doc.song,deps);
 if(!doc.resources || Object.keys(doc.resources).length!==3 || doc.resources.factory!==doc.capability || JSON.stringify(doc.resources.vocalBanks)!==JSON.stringify(expected.vocalBanks) || JSON.stringify(doc.resources.unresolved)!==JSON.stringify(expected.unresolved))fail('resources','manifest must match musical references');
 if(words.length>65536)fail('document','logical capacity exceeded');
 return Int32Array.from(words);
}
export function parse(text){if(typeof text!=='string'||new TextEncoder().encode(text).length>MAX_BYTES)fail('file','maximum 2 MiB');let d;try{d=JSON.parse(text);}catch{fail('file','invalid/truncated JSON');}toWire(d);return d;}
export function semantic(doc){toWire(doc);const d=fromWire(toWire(doc));return {capability:d.capability,resources:d.resources,song:d.song,dependencies:d.dependencies};}
export function diff(a,b,path=''){if(JSON.stringify(a)===JSON.stringify(b))return [];if(a&&b&&typeof a==='object'&&typeof b==='object')return [...new Set([...Object.keys(a),...Object.keys(b)])].flatMap(k=>diff(a[k],b[k],path+(Array.isArray(a)?`[${k}]`:(path?'.':'')+k)));return [`${path}: ${JSON.stringify(a)} -> ${JSON.stringify(b)}`];}
export function summary(d){toWire(d);return `${d.title}: 6 tracks, ${d.song.arrangement.count} sections, ${d.dependencies.length} pattern dependencies, ${d.song.motion.events.length} motion/locks; ${d.resources.unresolved.length} unresolved resources`;}
export const errors={1:'Stop playback and release controls before importing.',2:'Snapshot failed firmware field validation.',3:'Missing/mismatched vocal bank/group or unresolved private sample. Install the matching bank; private user samples are not portable in v1.',4:'Arrangement source is missing.'};
