// SPDX-License-Identifier: GPL-3.0-only
import fs from 'node:fs';import assert from 'node:assert/strict';
import{fromWire,toWire,parse,semantic,diff,summary,CAPABILITY,MAX_BYTES}from '../felucca/web/emu/song-snapshot.mjs';
const {instance:{exports:e}}=await WebAssembly.instantiate(fs.readFileSync('felucca/build/emu/felucca.wasm'),{});
e._initialize();e.web_nor_erase();e.web_boot();for(let i=0;i<600;i++)e.web_render(128);
const capture=()=>{const n=e.web_snapshot_capture();assert(n);return fromWire(new Int32Array(e.memory.buffer,e.web_snapshot_buffer(),n).slice());};
const original=capture(),w=toWire(original);
assert.equal(e.web_snapshot_validate(w.length),0);
assert.deepEqual(semantic(fromWire(w)),semantic(original));
const rejects=[
 d=>d.version=2,d=>d.capability='future',d=>d.format='bad',d=>d.song.selectedTrack=6,
 d=>d.song.tracks.pop(),d=>d.song.tracks[5].steps.pop(),d=>d.song.tracks[0].parameters.P_PAN=1.5,
 d=>d.song.tracks[0].parameters.P_PAN='1',d=>d.song.tracks[0].parameters.P_LEVEL=-1,
 d=>d.song.tracks[0].fm6.pop(),d=>d.song.tracks[0].fm6[0]=128,d=>d.song.tracks[0].steps[63].notes[0]=128,
 d=>d.song.tracks[0].steps[63].n=5,d=>d.song.tracks[0].steps[63].probability=113,
 d=>d.song.tracks[0].steps[63].hits=0+(d.song.tracks[0].steps[63].accents=1)-1,
 d=>d.song.tracks[0].parameters.P_VHOLD=1,d=>d.song.tracks[0].parameters.unknown=2,
 d=>d.resources.vocalBanks=[],d=>d.song.arrangement.count=17,d=>d.song.arrangement.rows[0].sources[0]='pattern-x',
 d=>d.dependencies=[{id:'pattern-0',song:d.song},{id:'pattern-0',song:d.song}],
 d=>d.song.arrangement.count=1,
 d=>d.song.motion.events=[{track:5,step:63,parameter:'P_PAN',lock:true,value:12},{track:5,step:63,parameter:'P_PAN',lock:false,value:13}],
 d=>d.song.motion.events=[{track:5,step:64,parameter:'P_PAN',lock:true,value:12}],
 d=>d.song.motion.events=[{track:5,step:63,parameter:'P_VHOLD',lock:true,value:12}],
 d=>d.requiredCapabilities=['unknown'],d=>d.song.globals.G_BPM=NaN
];
for(const change of rejects){const bad=structuredClone(original);change(bad);assert.throws(()=>toWire(bad));}
assert.throws(()=>parse('{"format":'));assert.throws(()=>parse(' '.repeat(MAX_BYTES+1)));
// Semantic comparison excludes title, creation time and producer/build provenance.
const metadata=structuredClone(original);metadata.title='different label';metadata.metadata={created:'tomorrow',producer:{build:'other'}};
assert.deepEqual(diff(semantic(original),semantic(metadata)),[]);
const edit=structuredClone(original);edit.song.tracks[5].parameters.P_PAN=15;edit.song.tracks[4].steps[63].notes[0]=57;
const changes=diff(semantic(original),semantic(edit));assert.equal(changes.length,2);assert(changes.some(x=>x.includes('tracks[5].parameters.P_PAN')));assert(changes.some(x=>x.includes('tracks[4].steps[63].notes[0]')));
new Int32Array(e.memory.buffer,e.web_snapshot_buffer(),w.length).set(toWire(edit));assert.equal(e.web_snapshot_apply(w.length),0);
for(let i=0;i<50;i++)e.web_render(128);assert.deepEqual(semantic(capture()),semantic(edit));
// Representative malformed raw-WASM fields are bounded before narrowing/loops.
for(const index of [0,1,14,15,40,41,42,43,200,500,w.length-1]){
 new Int32Array(e.memory.buffer,e.web_snapshot_buffer(),w.length).set(w);
 new Int32Array(e.memory.buffer,e.web_snapshot_buffer(),w.length)[index]=0x7fffffff;
 assert.notEqual(e.web_snapshot_apply(w.length),0,`bad wire ${index}`);
}
assert.deepEqual(semantic(capture()),semantic(edit));
fs.mkdirSync('tmp/song-fixtures',{recursive:true});
fs.writeFileSync('tmp/song-fixtures/export.bgirl.json',JSON.stringify(original,null,2)+'\n');
fs.writeFileSync('tmp/song-fixtures/agent-edited.bgirl.json',JSON.stringify(edit,null,2)+'\n');
console.log(`song JSON: ${rejects.length+2} negative cases, canonical wire/WASM round trip, raw-boundary atomic rejection, two-path agent diff PASS`);
console.log(summary(original),CAPABILITY);
