// SPDX-License-Identifier: GPL-3.0-only
import fs from 'node:fs';import vm from 'node:vm';import assert from 'node:assert/strict';
const html=fs.readFileSync('felucca/web/editor.html','utf8');
const proto=html.slice(html.indexOf('/*PROTO-BEGIN*/'),html.indexOf('/*PROTO-END*/'));
const E=vm.runInNewContext(proto+'\n;({fmtValue,F})',{setTimeout,clearTimeout,setInterval,clearInterval,console,TextEncoder,TextDecoder});
const h=fs.readFileSync('felucca/firmware/src/bgirl_vocoder_time.h','utf8');
const values=h.match(/BG_REL_MS\[128\] = \{([^}]+)\}/)[1].split(',').map(x=>Math.fround(Number(x.replace('f',''))));
for(let i=0;i<128;i++)assert.deepEqual(Array.from(E.fmtValue({fmt:E.F.VOCREL},i)),[String(Math.floor(values[i]+.5)),'ms']);
for(let i=1;i<128;i++)assert.deepEqual(Array.from(E.fmtValue({fmt:E.F.VOCLEG},i)),[String(i+1),'ms']);
console.log('editor: all128 log and127 legacy millisecond labels agree with firmware table PASS');
