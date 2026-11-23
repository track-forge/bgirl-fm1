// SPDX-License-Identifier: GPL-3.0-only
import {bkCrc, bkPack} from "./fm1backup.js";

const MIAMI_CMD = 82, MIAMI_DATA = 512;
const [MIAMI_EMPTY, MIAMI_READY, MIAMI_RESTORE, MIAMI_OCCUPIED] = [0, 1, 2, 3];

export function checkMiamiImage(image) {
  if (image.length !== 69634) throw new Error("Wrong Miami bank size");
  const view = new DataView(image.buffer, image.byteOffset, image.byteLength);
  const id = Array.from(image.subarray(16, 24), b => b.toString(16).padStart(2, "0")).join("");
  if (view.getUint32(0, true) !== 0x42564742 || view.getUint32(4, true) !== 1 ||
      view.getUint32(8, true) !== image.length - MIAMI_DATA || id !== "bb8f8cb449511455" ||
      view.getUint32(12, true) !== bkCrc(image.subarray(MIAMI_DATA)))
    throw new Error("Miami bank failed validation");
}

export async function miamiSlots(request) {
  const reply = await request([MIAMI_CMD, [0]], {timeout: 5000});
  if (reply.length !== 4 || reply[0] !== 1 || reply.slice(1).some(n => !Number.isInteger(n) || n < MIAMI_EMPTY || n > MIAMI_OCCUPIED))
    throw new Error("This firmware cannot provision Miami Bass");
  return reply.slice(1);
}

export async function provisionMiami(request, image, progress, {retry = false} = {}) {
  checkMiamiImage(image);
  const states = await miamiSlots(request);
  const ready = states.indexOf(MIAMI_READY);
  if (ready !== -1) return {slot: ready, skipped: true};
  const interrupted = retry ? states.indexOf(MIAMI_RESTORE) : -1;
  const slot = interrupted >= 0 ? interrupted : states.indexOf(MIAMI_EMPTY);
  if (slot < 0) return null;
  const send = async (op, args = []) => {
    const r = await request([MIAMI_CMD, [op, slot, ...args]], {timeout: 5000});
    if (r.length !== 3 || r[0] !== op || r[1] !== slot || r[2] !== 0)
      throw new Error(`USR${slot + 1}: bank write rejected (step ${op}, code ${r[2] ?? "unknown"})`);
  };
  await send(1, [states[slot], ...bkPack(image.subarray(0, 24))]);
  for (let off = MIAMI_DATA; off < image.length; off += 256) {
    const bytes = image.subarray(off, off + 256);
    await send(2, [off & 127, (off >>> 7) & 127, off >>> 14, ...bkPack(bytes)]);
    progress((off + bytes.length - MIAMI_DATA) / (image.length - MIAMI_DATA));
  }
  await send(3);
  if ((await miamiSlots(request))[slot] !== MIAMI_READY) throw new Error("Miami bank readback failed");
  return {slot, skipped: false};
}
