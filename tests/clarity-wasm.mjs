// SPDX-License-Identifier: GPL-3.0-only
import fs from 'node:fs';import assert from 'node:assert/strict';
const bytes=fs.readFileSync(process.argv[2]||'felucca/build/emu/felucca.wasm');
const {instance}=await WebAssembly.instantiate(bytes,{});const ex=instance.exports;ex._initialize();
const native=new Float32Array(new Uint8Array(fs.readFileSync(process.argv[3]||'felucca/build/host/clarity-native.f32')).buffer);let offset=0,energy=0;
for(let b=1;b<=2;b++) {
 ex.web_nor_erase();ex.web_boot();assert.equal(ex.web_bgirl_clarity(0,b,94,0,127),0);
 for(let n=0;n<4096;n+=128)ex.web_render(128);
 ex.web_buttons(1<<10);ex.web_render(128);ex.web_buttons(0);ex.web_render(128);
 for(let n=0;n<88200;n+=128) {
  const m=Math.min(128,88200-n);ex.web_render(m);
  const l=new Float32Array(ex.memory.buffer,ex.web_out_l(),m),r=new Float32Array(ex.memory.buffer,ex.web_out_r(),m);
  for(let i=0;i<m;i++)for(const v of [l[i],r[i]]){assert.equal(v,native[offset++],`sample ${offset}`);energy+=v*v;}
 }
}
assert.equal(offset,native.length);assert(energy>0);console.log('12/24 clarity native/WASM: 176400 frames bit-identical, nonzero PASS');
