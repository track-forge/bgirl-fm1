// SPDX-License-Identifier: GPL-3.0-only
// Capture the real default CREW HOME framebuffer with the browser build.
import {readFileSync} from 'node:fs';
const {instance:{exports:ex}} = await WebAssembly.instantiate(readFileSync(process.argv[2]));
ex._initialize();
ex.web_nor_erase();
ex.web_boot();
const advance = blocks => { for (let i=0;i<blocks;i++) ex.web_render(1024); };
advance(110);
ex.web_buttons(1 << 10); // PLAY: capture real accepted-note CREW animation, without audio.
advance(4);
ex.web_buttons(0);
for (let frame=0;frame<24;frame++) {
  advance(4);
  const src = new Uint16Array(ex.memory.buffer,ex.web_screen(),240*240);
  const rgb = Buffer.alloc(src.length*3);
  for (let i=0;i<src.length;i++) {
    const p = (src[i] >> 8) | (src[i] << 8);
    rgb[i*3] = ((p >> 11) & 31)*255/31;
    rgb[i*3+1] = ((p >> 5) & 63)*255/63;
    rgb[i*3+2] = (p & 31)*255/31;
  }
  process.stdout.write(rgb);
}
