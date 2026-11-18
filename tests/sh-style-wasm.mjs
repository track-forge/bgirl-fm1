// SPDX-License-Identifier: GPL-3.0-only
import fs from 'node:fs';import assert from 'node:assert/strict';
const {instance}=await WebAssembly.instantiate(fs.readFileSync('felucca/build/emu/felucca.wasm'),{});const ex=instance.exports;ex._initialize();
const native=new Float32Array(new Uint8Array(fs.readFileSync('felucca/build/host/sh-style-native.f32')).buffer);let offset=0;
for(let sound=0;sound<2;sound++) {
 ex.web_nor_erase();ex.web_boot();assert.equal(ex.web_bgirl_sh_style_sound(sound),0);
 for(let n=0;n<4096;n+=128)ex.web_render(128);
 ex.web_midi(0x90,60,100);let energy=0;
 for(let n=0;n<88200;n+=128) {
  if(n===44160)ex.web_midi(0x80,60,0);
  const m=Math.min(128,88200-n);ex.web_render(m);
  const l=new Float32Array(ex.memory.buffer,ex.web_out_l(),m),r=new Float32Array(ex.memory.buffer,ex.web_out_r(),m);
  for(let i=0;i<m;i++)for(const v of [l[i],r[i]]){assert(Number.isFinite(v)&&Math.abs(v)<=1);assert.equal(v,native[offset++],`sample ${offset}`);energy+=v*v;}
 }assert(energy>.00001);
}
assert.equal(offset,native.length);console.log('SH-STYLE native/WASM: 176400 frames bit-identical, finite/bounded/nonzero, including release PASS');
