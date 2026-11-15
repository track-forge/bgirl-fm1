#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Deterministic, offline whole/chop IMA bank. No private inputs by default."""
import argparse
import hashlib
import json
import math
from pathlib import Path
import struct
import sys
import tempfile
import wave

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT / 'felucca/tools'))
import sampleio

RATE = 22050
GAIN = 0.7071067811865476
MAX_SAMPLES = 22050 * 120  # bounded optional bank, not a device-fit claim


def sha(data):
    return hashlib.sha256(data).hexdigest()


def read_wav(path):
    data = path.read_bytes()
    if data[:4] != b'RIFF' or data[8:12] != b'WAVE' or len(data) < 44:
        raise ValueError(f'{path.name}: expected RIFF WAVE')
    fmt = pcm = None
    at = 12
    while at + 8 <= len(data):
        tag, size = struct.unpack_from('<4sI', data, at)
        block = data[at + 8:at + 8 + size]
        if len(block) != size:
            raise ValueError(f'{path.name}: truncated chunk')
        if tag == b'fmt ': fmt = block
        if tag == b'data': pcm = block
        at += 8 + size + (size & 1)
    if fmt is None or pcm is None or len(fmt) < 16:
        raise ValueError(f'{path.name}: missing fmt/data')
    codec, channels, rate, _, align, bits = struct.unpack_from('<HHIIHH', fmt)
    if channels not in (1, 2) or rate not in (22050, 24000, 44100) or (codec, bits) not in ((1, 16), (3, 32)):
        raise ValueError(f'{path.name}: supports mono/stereo PCM16 or float32 at 22050/24000/44100 Hz')
    if align != channels * bits // 8 or len(pcm) % align or not pcm:
        raise ValueError(f'{path.name}: invalid frame extent')
    values = [v[0] / 32768 if codec == 1 else v[0]
              for v in struct.iter_unpack('<h' if codec == 1 else '<f', pcm)]
    if any(not math.isfinite(v) or abs(v) > 1.4 for v in values):
        raise ValueError(f'{path.name}: nonfinite/out-of-range PCM')
    mono = [sum(values[i:i + channels]) / channels for i in range(0, len(values), channels)]
    return data, rate, channels, mono


def safe_file(base, name):
    path = (base / name).resolve()
    if not path.is_relative_to(base.resolve()) or not path.is_file():
        raise ValueError(f'missing/unsafe input: {name}')
    return path


def package(input_dir, manifest_path, output):
    input_dir, manifest_path, output = map(Path, (input_dir, manifest_path, output))
    manifest = json.loads(manifest_path.read_text())
    chops = manifest['slices']
    sources = sorted({c['source'] for c in chops})
    if not 1 <= len(sources) <= 6 or manifest['source_count'] != len(sources) or manifest['slice_count'] != len(chops):
        raise ValueError('manifest source/slice count mismatch (1..6 parents required)')
    originals = sorted(p.name for p in input_dir.glob('*_candidate_float.wav'))
    if originals != sources:
        raise ValueError('original WAV inventory differs from manifest')
    actual = {p.relative_to(manifest_path.parent).as_posix() for p in manifest_path.parent.rglob('*.wav')}
    listed = {c['slice'] for c in chops}
    if len(listed) != len(chops) or actual != listed:
        raise ValueError('chop WAV inventory differs from manifest (duplicates/missing/extra)')
    entries, groups, pcm = [], [], []
    originals_pcm = {}
    for source in sources:
        originals_pcm[source] = read_wav(safe_file(input_dir, source))

    def add(path, parent, entry_id, original, boundaries):
        data, rate, channels, mono = read_wav(path)
        frames = len(mono)
        if rate == 44100:
            mono = [(mono[i] + mono[min(i + 1, frames - 1)]) / 2 for i in range(0, frames, 2)]
        elif rate != RATE:
            raw = mono
            mono = []
            for i in range((frames * RATE + rate - 1) // rate):
                q, r = divmod(i * rate, RATE)
                mono.append((raw[q] * (RATE-r) + raw[min(q+1,frames-1)] * r) / RATE)
        n = len(mono)
        if n < 64 or len(pcm) + n > MAX_SAMPLES:
            raise ValueError('entry too short or bank exceeds 120s runtime sample limit')
        fade = RATE * 3 // 1000
        converted = [round(v * GAIN * 32767 * (min(i, n - 1 - i, fade) / fade if original else 1))
                     for i, v in enumerate(mono)]
        start = len(pcm)
        pcm.extend(converted)
        entries.append(dict(id=entry_id, parent=parent, kind='whole' if original else 'chop',
                            source_sha256=sha(data), input_frames=frames, input_rate=rate, channels=channels,
                            start=start, end=len(pcm), samples=n, seconds=n / RATE,
                            input_boundaries=boundaries))

    for i, source in enumerate(sources):
        add(safe_file(input_dir, source), i, source.removesuffix('_candidate_float.wav'), True, None)
    groups.append(dict(name='WHOLE', first=0, count=len(sources), source_ids=sources))
    for i, source in enumerate(sources):
        rows = sorted((c for c in chops if c['source'] == source), key=lambda c: c['slice_index'])
        if not 1 <= len(rows) <= 32 or [c['slice_index'] for c in rows] != list(range(1, len(rows) + 1)):
            raise ValueError('group needs 1..32 consecutive slice_index values')
        first, end = len(entries), 0
        _, sr, channels, original = originals_pcm[source]
        for c in rows:
            a, b = round(c['start_seconds'] * sr), round(c['end_seconds'] * sr)
            if a != end or b <= a or b > len(original) or c['sample_rate'] != sr or c['channels'] != channels:
                raise ValueError('invalid/noncontiguous source boundaries or format')
            path = safe_file(manifest_path.parent, c['slice'])
            _, csr, cch, samples = read_wav(path)
            if len(samples) != b - a or csr != sr or cch != channels:
                raise ValueError('chop format/frame extent differs from manifest')
            if abs(c['duration_seconds'] - (b-a)/sr) > 0.000002 or c['edge_fade_ms'] != 3.0:
                raise ValueError('invalid duration/edge fade metadata')
            add(path, i, Path(c['slice']).stem, False, [a, b])
            end = b
        if end != len(original):
            raise ValueError('chops do not cover the whole original including tail')
        groups.append(dict(name=f'G{i+1}', first=first, count=len(rows), source_id=source))
    blob, _ = sampleio.ima_encode(pcm, None)
    blob = bytes(blob)
    grid = [k * len(pcm) // 128 for k in range(128)]
    starts = {e['start'] for e in entries} | set(grid)
    states, pred, idx = {}, 0, 0
    for n in range(len(pcm)):
        if n in starts: states[n] = (pred & 65535) | idx << 16
        code = (blob[n//2] >> (4*(n%2))) & 15
        step = sampleio.IMA_STEP[idx]
        delta = (step >> 3) + (step if code & 4 else 0) + (step >> 1 if code & 2 else 0) + (step >> 2 if code & 1 else 0)
        pred = max(-32768, min(32767, pred - delta if code & 8 else pred + delta))
        idx = max(0, min(88, idx + sampleio.IMA_IDX[code & 7]))
    for e in entries:
        e.update(checkpoint=states[e['start']], byte_offset=e['start']//2, nibble=e['start']%2)
    report = dict(version=1, codec='IMA4 low-nibble-first', rate=RATE, samples=len(pcm), bytes=len(blob),
                  audio_sha256=sha(blob), manifest_sha256=sha(manifest_path.read_bytes()), groups=groups, entries=entries,
                  grid_checkpoints=[dict(position=k, state=states[k]) for k in grid])
    identity = sha(json.dumps(report, sort_keys=True, separators=(',', ':')).encode())
    report['bank_sha256'] = identity
    h = '/* Generated by tools/package_vocals.py. Do not redistribute private banks. */\n'
    h += 'static const uint8_t BG_VOCAL_ID[8] = {' + ','.join(str(x) for x in bytes.fromhex(identity[:16])) + '};\n'
    h += 'static const uint8_t BG_VOCAL_DATA[] = {\n' + '\n'.join(','.join(map(str, blob[i:i+32])) + ',' for i in range(0, len(blob), 32)) + '\n};\n'
    h += f'#define BG_VOCAL_INIT {{.len={len(pcm)}u,.rate=32768u,.nseg=1,.seg={{{{0,0,{len(pcm)}u}}}}}}\n'
    h = h.replace('.nseg=1,.seg=', '.nseg=1,.grid={'+','.join(str(states[k]) for k in grid)+'},.seg=')
    h += 'static const struct { uint32_t a,b,st; } BG_VOCAL_ENTRIES[] = {\n' + ''.join(f'{{{e["start"]}u,{e["end"]}u,{e["checkpoint"]}u}},\n' for e in entries) + '};\n'
    h += 'static const struct { uint8_t first,n; } BG_VOCAL_GROUPS[] = {' + ','.join(f'{{{g["first"]},{g["count"]}}}' for g in groups) + '};\n'
    h += 'static const char *const BG_VOCAL_VIEWS[] = {' + ','.join(json.dumps(g['name']) for g in groups) + '};\n'
    for i, g in enumerate(groups):
        names = [f'W{j+1:02}' if i == 0 else f'C{j+1:02}' for j in range(g['count'])]
        h += f'static const char *const BG_VOCAL_NAMES_{i}[] = {{' + ','.join(json.dumps(n) for n in names) + '};\n'
    h += 'static const char *const *const BG_VOCAL_NAMES[] = {' + ','.join(f'BG_VOCAL_NAMES_{i}' for i in range(len(groups))) + '};\n'
    h += 'static const param_desc_t BG_VOCAL_STARTS[] = {' + ','.join(
        '{"START",F_ENUM,0,%d,0,BG_VOCAL_NAMES_%d,0}' % (g['count']-1,i) for i,g in enumerate(groups)) + '};\n'
    report['header_sha256'] = sha(h.encode())
    output.mkdir(parents=True, exist_ok=True)
    (output/'bgirl_vocals.h').write_text(h)
    (output/'bank.ima').write_bytes(blob)
    (output/'bank.json').write_text(json.dumps(report, indent=2) + '\n')
    print(f'vocals: {len(sources)} whole + {len(chops)} chops; {len(blob)} encoded bytes; id {identity}')
    return report


def fixture(output):
    """Rights-cleared existing original speech, no network/generation dependencies."""
    source = ROOT/'assets/speech'
    m = json.loads((source/'manifest.json').read_text())
    with tempfile.TemporaryDirectory() as temp:
        base = Path(temp)
        slices = base/'slices'
        slices.mkdir()
        with wave.open(str(source/m['phrases'][0]['dry_source'])) as w:
            params, raw = w.getparams(), w.readframes(w.getnframes())
        name = 'fixture_candidate_float.wav'
        with wave.open(str(base/name), 'wb') as w:
            w.setparams(params); w.writeframes(raw)
        rows = []
        for j, (a, b) in enumerate(((0, params.nframes//2), (params.nframes//2, params.nframes))):
            file = f'fixture_{j+1}.wav'
            chunk = list(struct.unpack('<%dh' % (b-a), raw[a*2:b*2]))
            fade = params.framerate*3//1000
            chunk = [round(v*min(i,len(chunk)-1-i,fade)/fade) for i,v in enumerate(chunk)]
            with wave.open(str(slices/file), 'wb') as w:
                w.setparams(params); w.writeframes(struct.pack('<%dh'%len(chunk), *chunk))
            rows.append(dict(source=name, slice=file, slice_index=j+1, start_seconds=a/params.framerate,
                             end_seconds=b/params.framerate, duration_seconds=(b-a)/params.framerate,
                             sample_rate=params.framerate, channels=1, edge_fade_ms=3.0))
        manifest = slices/'manifest.json'
        manifest.write_text(json.dumps(dict(source_count=1, slice_count=2, slices=rows)))
        return package(base, manifest, output)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--input-dir', type=Path)
    p.add_argument('--manifest', type=Path)
    p.add_argument('--output', required=True, type=Path)
    p.add_argument('--fixture', action='store_true')
    a = p.parse_args()
    try:
        if a.fixture:
            if a.input_dir or a.manifest: p.error('--fixture cannot use private inputs')
            fixture(a.output)
        else:
            if not a.input_dir or not a.manifest: p.error('explicit --input-dir and --manifest required')
            package(a.input_dir, a.manifest, a.output)
    except (ValueError, KeyError, OSError, struct.error) as e:
        p.exit(1, f'vocal packaging failed: {e}\n')

if __name__ == '__main__': main()
