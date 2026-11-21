#!/usr/bin/env node
// SPDX-License-Identifier: GPL-3.0-only
// Schema artifact derived from the same parameter layout used by the validator.
import fs from 'node:fs';
import {layout,motionNames,CAPABILITY,PRE_KITS_CAPABILITY,FORMAT,VERSION} from '../../felucca/web/emu/song-snapshot.mjs';
const int=(min,max)=>({type:'integer',minimum:min,maximum:max});
const arr=(items,n)=>({type:'array',items,minItems:n,maxItems:n});
const obj=properties=>({type:'object',properties,required:Object.keys(properties),additionalProperties:false});
const ref=n=>({$ref:'#/$defs/'+n});
const step=obj({notes:arr(int(0,127),4),n:int(0,4),time:int(0,2),flags:int(0,255),velocity:int(0,127),hits:int(0,255),accents:int(0,255),probability:int(0,112)});
const track=obj({engine:int(0,layout.engines.length-1),preset:int(0,255),midiChannel:int(0,17),parameters:obj(Object.fromEntries(layout.parameters.map(k=>[k,k==='P_VHOLD'?{const:0}:int(-64,127)]))),fm6:arr(int(0,127),layout.fm6Size),vocalIdentity:arr(int(0,255),8),steps:arr(ref('step'),64)});
track.allOf=layout.engines.map(e=>({if:{properties:{engine:{const:e.id}}},then:e.id===1?false:{properties:{parameters:{properties:Object.fromEntries(layout.parameters.map((k,i)=>[k,{...int(...e.parameters[i]),...(e.forbidden[i]?{not:{enum:e.forbidden[i]}}:{})}]))}}}}));
const row=obj({sources:arr({enum:[null,'pattern-0','pattern-1','pattern-2','pattern-3']},6),repeat:int(0,16)});
const event=obj({track:int(0,5),step:int(0,63),parameter:{enum:motionNames},lock:{type:'boolean'},value:int(-64,127)});
const song=obj({selectedTrack:int(0,5),name:{type:'string',maxLength:12,pattern:'^[ -`{-~]*$'},globals:obj(Object.fromEntries(layout.globalIds.map((k,i)=>[k,int(...layout.globals[i])]))),tracks:arr(ref('track'),6),arrangement:obj({count:int(0,16),rows:arr(ref('row'),16)}),motion:obj({enabledTracks:int(0,63),events:{type:'array',items:ref('event'),maxItems:128}})});
const capability={enum:[CAPABILITY,PRE_KITS_CAPABILITY]};
const schema={...obj({format:{const:FORMAT},version:{const:VERSION},capability,title:{type:'string',maxLength:120},metadata:{type:'object'},resources:obj({factory:capability,vocalBanks:{type:'array',maxItems:30,items:{type:'string',pattern:'^[0-9a-f]{16}$'}},unresolved:{type:'array',maxItems:3,items:{type:'string'}}}),song:ref('song'),dependencies:{type:'array',maxItems:4,items:obj({id:{enum:['pattern-0','pattern-1','pattern-2','pattern-3']},song:ref('song')})}}),$schema:'https://json-schema.org/draft/2020-12/schema',$id:'https://track-forge.github.io/bgirl-fm1/song-v1.schema.json',$defs:{step,track,row,event,song},$comment:'Additional semantic checks (duplicate IDs/events, resource manifest, accents, source references, active repeats, byte limit) are enforced by shared song-snapshot.mjs and WASM.'};
schema.required=schema.required.filter(k=>k!=='metadata');
const text=JSON.stringify(schema)+'\n',path='docs/song-v1.schema.json';
if(process.argv.includes('--check')){if(fs.readFileSync(path,'utf8')!==text)throw new Error('stale schema');}else fs.writeFileSync(path,text);
