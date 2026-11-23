#!/usr/bin/env python3
"""Run an explicit FM-1_906 Miami probe against the bench's USB CDC console."""
import argparse
import os
import select
import termios
import time
import tty
import zlib
from datetime import datetime, timezone
from pathlib import Path

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument("device")
parser.add_argument("log", type=Path)
parser.add_argument("backup", type=Path, help="Known-good full ROM backup")
parser.add_argument("stage", choices=("boot", "window", "header", "crc", "mount"))
args = parser.parse_args()

rom = args.backup.read_bytes()
header = rom[0xA0000:0xA0018]
payload = rom[0xA0200:0xA0200 + 69122]
expected_crc = 0xA09C57F3
if len(rom) != 0x100000 or header[:4] != b"BGVB" or zlib.crc32(payload) != expected_crc:
    parser.error("backup does not contain the expected Miami bank in slot 1")

with args.log.open("a", encoding="utf-8") as log:
    def record(message):
        print(f"{datetime.now(timezone.utc).isoformat()} {message}", file=log, flush=True)
        os.fsync(log.fileno())

    def exchange(command, detail=""):
        record(f"TX {command} {detail}".rstrip())
        start = time.monotonic()
        os.write(fd, (command + "\r").encode("ascii"))
        reply = b""
        while not reply.endswith(b"\r\n> "):
            remaining = start + 5 - time.monotonic()
            if remaining <= 0 or not select.select([fd], [], [], remaining)[0]:
                raise TimeoutError(f"{command}: incomplete response: {reply!r}")
            data = os.read(fd, 4096)
            if not data:
                raise OSError("CDC disconnected")
            reply += data
        record(f"RX {time.monotonic() - start:.3f}s {reply!r}")
        lines = reply.decode("ascii", errors="replace").splitlines()
        if any(line.startswith("ERR ") for line in lines):
            raise RuntimeError(f"{command}: probe rejected")
        return lines

    fd = os.open(args.device, os.O_RDWR | os.O_NOCTTY)
    original = termios.tcgetattr(fd)
    try:
        tty.setraw(fd)
        # Allow DTR greeting to arrive before discarding stale console bytes.
        time.sleep(0.25)
        termios.tcflush(fd, termios.TCIFLUSH)
        status = exchange("status")
        if "felucca v0.6-miamistep" not in status:
            raise RuntimeError("Expected v0.6-miamistep; no probe sent")
        if args.stage in ("window", "header", "mount"):
            reply = exchange("miami " + args.stage)
            expected = "OK mounted read-only" if args.stage == "mount" else "OK " + args.stage
            if expected not in reply:
                raise RuntimeError(f"Missing {expected!r}")
            if args.stage == "header" and "header=" + header.hex().upper() not in reply:
                raise RuntimeError("Header differs from the ROM backup")
        if args.stage == "crc":
            for offset in range(0, len(payload), 1024):
                count = min(1024, len(payload) - offset)
                reply = exchange("miami chunk", f"expected_offset={offset} length={count}")
                expected = ["OK chunk", f"offset {offset}", f"length {count}"]
                if offset + count == len(payload):
                    expected[0] = "OK valid"
                    expected.append(f"crc 0x{expected_crc:08X}")
                if not all(line in reply for line in expected):
                    raise RuntimeError(f"Unexpected response for chunk at {offset}")
        record(f"PASS {args.stage}")
    except (OSError, RuntimeError) as error:
        record(f"FAIL {args.stage}: {error}")
        raise SystemExit(str(error)) from error
    finally:
        try:
            termios.tcsetattr(fd, termios.TCSANOW, original)
        except termios.error:
            pass  # A reset may have removed the device.
        os.close(fd)
