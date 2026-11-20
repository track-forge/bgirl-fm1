# SPDX-License-Identifier: GPL-3.0-only
"""Match each trio using one constant gain per clip, without limiting or EQ."""
import math
from pathlib import Path
import struct
import subprocess
import sys

out = Path(sys.argv[1])
for mix in ("solo", "bass"):
    stems = []
    for kit in ("bg-break", "808-raw", "cold-wire"):
        name = f"{kit}-{mix}"
        samples = tuple(x[0] for x in struct.iter_unpack("<f", (out / f"{name}.f32").read_bytes()))
        rms = math.sqrt(math.fsum(x*x for x in samples) / len(samples))
        stems.append((name, rms, max(map(abs, samples))))
    # Match complete phrase RMS, with at least 2 dB peak headroom on every file.
    target = min(10**(-24/20), *(10**(-2/20)*rms/peak for _, rms, peak in stems))
    for name, rms, peak in stems:
        gain = target / rms
        subprocess.run(["ffmpeg", "-loglevel", "error", "-y", "-f", "f32le", "-ar", "44100",
                        "-ac", "1", "-i", str(out / f"{name}.f32"), "-af", f"volume={gain:.12g}",
                        "-codec:a", "libmp3lame", "-b:a", "128k", str(out / f"{name}.mp3")], check=True)
        print(f"{name}: {20*math.log10(target):.2f} dBFS RMS, gain {20*math.log10(gain):+.2f} dB, "
              f"peak {20*math.log10(peak*gain):.2f} dBFS before encoding")
