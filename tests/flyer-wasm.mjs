// SPDX-License-Identifier: GPL-3.0-only
import fs from 'node:fs';import assert from 'node:assert/strict';
const bytes=fs.readFileSync('felucca/build/emu/felucca.wasm');
async function run(delay,answer){
 const {instance}=await WebAssembly.instantiate(bytes,{}),ex=instance.exports;
 ex._initialize();ex.web_nor_erase();ex.web_master(650);ex.web_boot();
 assert.equal(ex.web_bgirl_flyer(0),0);
 const render=n=>{const out=[];for(let i=0;i<n;i+=128){const size=Math.min(128,n-i);ex.web_render(size);out.push(...new Float32Array(ex.memory.buffer,ex.web_out_l(),size));}return out;};
 render(delay);if(answer)assert.equal(ex.web_bgirl_flyer(1),0);
 const audio=render(88200);
 assert.equal(ex.web_bgirl_flyer(2),0);render(44100);
 assert.equal(ex.web_playing(),0);
 return audio;
}
for(const delay of [11025,33075,66150]){
 const beat=await run(delay,false),reply=await run(delay,true);
 let difference=0,peak=0;
 for(let i=0;i<reply.length;i++){assert(Number.isFinite(reply[i]));peak=Math.max(peak,Math.abs(reply[i]));difference+=(reply[i]-beat[i])**2;}
 assert(difference>0.001,'manual answer must add audible vocoder output, not just acknowledge');
 assert(peak<=1);console.log({delayFrames:delay,answerDifferenceEnergy:difference,peak});
}
