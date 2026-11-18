// SPDX-License-Identifier: GPL-3.0-only
import fs from 'node:fs';import assert from 'node:assert/strict';
const {instance:{exports:ex}}=await WebAssembly.instantiate(fs.readFileSync('felucca/build/emu/felucca.wasm'),{});
ex._initialize();ex.web_nor_erase();ex.web_boot();
for(let i=0;i<60032;i+=128)ex.web_render(128);
assert.equal(ex.web_bgirl_project(1),0);
for(let i=0;i<4096;i+=128)ex.web_render(128);
// Identical normal PLAY gesture and block boundaries to native.
ex.web_buttons(1<<10);ex.web_render(128);ex.web_buttons(0);ex.web_render(128);assert.equal(ex.web_playing(),1);
const native=new Float32Array(new Uint8Array(fs.readFileSync('felucca/build/host/mega-native.f32')).buffer);
let offset=0;
for(let n=0;n<44100*15;n+=128){const count=Math.min(128,44100*15-n);ex.web_render(128);
 const l=new Float32Array(ex.memory.buffer,ex.web_out_l(),128),r=new Float32Array(ex.memory.buffer,ex.web_out_r(),128);
 for(let i=0;i<count;i++)for(const v of [l[i],r[i]])assert.equal(v,native[offset++],`sample ${offset}`);
}
assert.equal(offset,native.length);console.log('Mega: two complete loops /661500 native-WASM frames bit-identical PASS');
