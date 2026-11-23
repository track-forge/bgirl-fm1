#!/usr/bin/env python3
"""Capture read-only BGIRL FM-1 USB console status as timestamped CSV."""
import argparse
import csv
from datetime import datetime, timezone
import os
import select
import sys
import termios
import time
import tty

FIELDS = ("uptime_ms", "cpu_pct", "audio_max_us", "audio_late", "voices_shed", "playing")


def status(fd):
    os.write(fd, b"status\r")
    deadline = time.monotonic() + 5
    pending = b""
    while select.select([fd], [], [], max(0, deadline - time.monotonic()))[0]:
        chunk = os.read(fd, 4096)
        if not chunk:
            raise OSError("USB console disconnected")
        pending += chunk
        while b"\r\n> " in pending:
            reply, pending = pending.split(b"\r\n> ", 1)
            lines = reply.decode("ascii", errors="replace").splitlines()
            if not any(line.startswith("felucca ") for line in lines):
                continue
            values = {}
            for line in lines:
                key, _, value = line.partition(" ")
                if key in FIELDS:
                    values[key] = int(value)
            if not all(key in values for key in FIELDS):
                raise ValueError("Incomplete USB console status reply")
            return [values[key] for key in FIELDS]
        if time.monotonic() >= deadline:
            break
    raise TimeoutError("No complete status reply from the FM-1 console")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("device", help="FM-1 USB serial device, e.g. /dev/ttyACM0")
    parser.add_argument("--samples", type=int, default=1, help="status samples to capture")
    parser.add_argument("--interval", type=float, default=1, help="seconds between polls")
    parser.add_argument("--label", default="", help="phase label written to every row")
    parser.add_argument("--no-header", action="store_true", help="append rows to an existing CSV")
    args = parser.parse_args()
    if args.samples < 1 or not 0 <= args.interval < float("inf"):
        parser.error("samples must be positive and interval finite and nonnegative")

    fd = os.open(args.device, os.O_RDWR | os.O_NOCTTY)
    try:
        original = termios.tcgetattr(fd)
        tty.setraw(fd)
        try:
            writer = csv.writer(sys.stdout, lineterminator="\n")
            if not args.no_header:
                writer.writerow(("utc", "phase", *FIELDS))
            for index in range(args.samples):
                if index:
                    time.sleep(args.interval)
                values = status(fd)
                writer.writerow((datetime.now(timezone.utc).isoformat(), args.label, *values))
                sys.stdout.flush()
        finally:
            try:
                termios.tcsetattr(fd, termios.TCSANOW, original)
            except termios.error:
                pass
    finally:
        os.close(fd)


if __name__ == "__main__":
    try:
        main()
    except (OSError, ValueError, termios.error) as error:
        raise SystemExit(str(error)) from None
