#!/usr/bin/env node
// SPDX-License-Identifier: GPL-3.0-only
import fs from 'node:fs';
import {parse,summary,semantic,diff} from '../../felucca/web/emu/song-snapshot.mjs';
try {
 const [cmd,a,b]=process.argv.slice(2);
 if(!['validate','inspect','diff'].includes(cmd)||!a||(cmd==='diff'&&!b))throw new Error('Usage: node tools/song-snapshots/song.mjs validate|inspect FILE | diff BEFORE AFTER');
 const load=p=>{if(fs.statSync(p).size>2*1024*1024)throw new Error('maximum 2 MiB');return parse(fs.readFileSync(p,'utf8'));};
 const song=load(a);
 if(cmd==='diff')console.log(diff(semantic(song),semantic(load(b))).join('\n')||'No musical changes');
 else console.log((cmd==='validate'?'VALID · ':'')+summary(song));
 if(song.resources.unresolved.length)throw new Error('Unresolved: '+song.resources.unresolved.join(', '));
} catch(e){console.error(e.message);process.exitCode=1;}
