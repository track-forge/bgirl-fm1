#!/usr/bin/env python3
"""Send only the live-music object to the BGIRL FM-1 editor."""
import argparse
import os
from pathlib import Path
import select
import time
import zlib

HEADER = bytes.fromhex("f0 7d 46 36")
PROJECT_SIZE = 3840
PUT = 67


def u32(value):
    return bytes((value >> (7 * i)) & 127 for i in range(5))


def pack7(data):
    packed = bytearray()
    for offset in range(0, len(data), 7):
        group = data[offset:offset + 7]
        packed.append(sum((byte >> 7) << i for i, byte in enumerate(group)))
        packed.extend(byte & 127 for byte in group)
    return bytes(packed)


def transaction(data):
    yield bytes([0, 0]) + u32(len(data)) + u32(zlib.crc32(data))
    for offset in range(0, len(data), 256):
        yield bytes([1, 0]) + u32(offset) + pack7(data[offset:offset + 256])
    yield bytes([2, 0])


def frame(command, args=b""):
    return HEADER + bytes([command]) + args + b"\xf7"


def exchange(fd, command, args=b""):
    outgoing = frame(command, args)
    prefix = HEADER + bytes([command])
    deadline = time.monotonic() + 3
    reply = bytearray()
    while time.monotonic() < deadline:
        readable, writable, _ = select.select(
            [fd], [fd] if outgoing else [], [], max(0, deadline - time.monotonic())
        )
        if writable:
            outgoing = outgoing[os.write(fd, outgoing):]
        if not readable:
            continue
        incoming = os.read(fd, 1024)
        if not incoming:
            raise OSError("MIDI device disconnected")
        for byte in incoming:
            if byte == 0xF0:
                reply = bytearray([byte])
            elif byte == 0xF7:
                if reply.startswith(prefix):
                    return bytes(reply[len(prefix):])
                reply.clear()
            elif byte < 0x80 and reply:
                reply.append(byte)
            elif byte < 0xF8:
                reply.clear()
    raise TimeoutError(f"No reply for command {command}, arguments {args[:2].hex()}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=["probe", "load", "frames"])
    parser.add_argument("device", nargs="?", help="ALSA raw MIDI node, e.g. /dev/snd/midiC2D0")
    parser.add_argument("--ram-probe", action="store_true",
                        help="test load only on v0.2-ramprobe, without Miami vocals")
    parser.add_argument("--song", type=Path, default=Path(__file__).parent / "fixtures" / "ticker.b6s1")
    args = parser.parse_args()
    if args.action != "frames" and not args.device:
        parser.error("specify the FM-1 raw MIDI device")
    if args.action != "probe":
        data = args.song.read_bytes()
        if len(data) != PROJECT_SIZE or data[:4] != b"B6S1":
            raise ValueError("Expected a 3840-byte B6S1 live project")
    if args.action == "frames":
        import sys
        sys.stdout.buffer.write(b"".join(frame(PUT, op) for op in transaction(data)))
        return
    fd = os.open(args.device, os.O_RDWR | os.O_NONBLOCK)
    try:
        info = exchange(fd, 1)
        version, sep, fields = info.partition(b"\x00")
        if not version.startswith(b"FELUCCA ") or not sep or len(fields) < 6:
            raise ValueError(f"Unexpected editor identity: {info!r}")
        names_and_caps = fields[5:].split(b"\x00", fields[0])
        if len(names_and_caps) != fields[0] + 1 or names_and_caps[-1][:1] != b"\x06":
            raise ValueError("Editor did not report six tracks")
        print(version.decode("ascii"), flush=True)
        if args.ram_probe:
            if version != b"FELUCCA v0.2-ramprobe" or args.action != "load":
                raise ValueError("--ram-probe is only for loading on the diagnostic firmware")
            print("RAM-only load test: Miami vocals unavailable.", flush=True)
        else:
            states = exchange(fd, 82, b"\x00")
            if len(states) != 4 or states[0] != 1 or any(s > 3 for s in states[1:]):
                raise ValueError(f"Unexpected Miami bank status: {states!r}")
            print("Miami slots (0 empty, 1 CRC-ready, 2 incomplete, 3 occupied):", *states[1:], flush=True)
            if args.action == "probe":
                return
            if 1 not in states[1:]:
                raise ValueError("No validated Miami bank; no song sent")
        for op in transaction(data):
            if op[0] == 2:
                print("All chunks accepted; sending live restore COMMIT", flush=True)
            reply = exchange(fd, PUT, op)
            if reply != op[:2] + b"\x00":
                raise ValueError(f"PUT {op[0]} refused/unexpected reply: {reply.hex()}")
        print("Live restore ACK; transport remains under manual control.", flush=True)
    finally:
        os.close(fd)


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError) as error:
        raise SystemExit(str(error)) from None
