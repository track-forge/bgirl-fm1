// SPDX-License-Identifier: GPL-3.0-only
import {readFileSync} from 'node:fs';
const {instance:{exports:ex}} = await WebAssembly.instantiate(readFileSync(process.argv[2]));
ex._initialize();
ex.web_nor_erase();
ex.web_boot();
function advance(blocks) {
  for (let i=0; i<blocks; i++) ex.web_render(1024);
}
advance(110);
ex.web_buttons(1 << 10); // PLAY: drive the real CREW HOME animation offline.
advance(4);
ex.web_buttons(0);
const frames = [];
for (let i=0; i<24; i++) {
  advance(4);
  frames.push(Buffer.from(new Uint8Array(ex.memory.buffer,ex.web_screen(),240*240*2)));
}
if (frames.every(frame => frame.equals(frames[0]))) throw new Error('CREW capture did not move');
process.stdout.write(Buffer.concat(frames));
