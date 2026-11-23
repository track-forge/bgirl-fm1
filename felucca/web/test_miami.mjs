// SPDX-License-Identifier: GPL-3.0-only
import assert from "node:assert/strict";
import {readFileSync} from "node:fs";
import {BackupConnection, bkUnpack} from "./fm1backup.js";
import {checkMiamiImage, provisionMiami} from "./fm1miami.js";

const image = readFileSync(process.argv[2]);
const bad = Uint8Array.from(image);
bad[bad.length - 1] ^= 1;
assert.throws(() => checkMiamiImage(bad));

const input = {}, writes = [];
let states = [0, 3, 0], interrupt = true;
const output = {send(frame) {
  assert.deepEqual(frame.slice(0, 5), [240, 125, 70, 54, 82]);
  const [op, slot, ...args] = frame.slice(5, -1);
  let reply = [op, slot, 0];
  if (op === 0) reply = [1, ...states];
  else if (op === 1) {
    assert.equal(args[0], states[slot]);
    assert.deepEqual(bkUnpack(args.slice(1), 24), Uint8Array.from(image.subarray(0, 24)));
    states[slot] = 2;
    writes.length = 0;
  } else if (op === 2) {
    const off = args[0] | args[1] << 7 | args[2] << 14;
    assert.deepEqual(bkUnpack(args.slice(3), Math.min(256, image.length - off)),
                     Uint8Array.from(image.subarray(off, off + 256)));
    writes.push(off);
    if (interrupt) {
      interrupt = false;
      throw new Error("disconnected");
    }
  } else if (op === 3) states[slot] = 1;
  input.onmidimessage({data: [240, 125, 70, 54, 82, ...reply, 247]});
}};
const connection = new BackupConnection(input, output, 0x36);
const request = (req, opt) => connection.request(req, opt);
const noop = () => {};
states = [3, 3, 3];
assert.equal(await provisionMiami(request, image, noop), null);
assert.deepEqual(states, [3, 3, 3]);
assert.equal(writes.length, 0);
states = [0, 3, 3];
await assert.rejects(provisionMiami(request, image, noop), /disconnected/);
assert.deepEqual(states, [2, 3, 3]);
assert.equal(await provisionMiami(request, image, noop), null);
assert.deepEqual(states, [2, 3, 3]);
assert.deepEqual(await provisionMiami(request, image, noop, {retry: true}), {slot: 0, skipped: false});
assert.equal(writes.length, Math.ceil((image.length - 512) / 256));
assert.deepEqual(await provisionMiami(request, image, noop), {slot: 0, skipped: true});
connection.close();
console.log("Miami installer: image validation, full-slot preservation, interruption, retry and no rewrite PASS");
