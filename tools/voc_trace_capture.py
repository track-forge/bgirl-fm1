#!/usr/bin/env python3
"""Arm or dump the FM-1 vocoder trace; do not run another CDC reader."""
import argparse
import csv
import os
import select
import sys
import termios
import time
import tty

FIELDS = ("id", "total_ticks", "nested_ticks", "parts_ticks", "capture_ticks",
          "bank_ticks", "bank_calls", "fx_ticks", "song_tick", "voices_end", "flags")


def exchange(fd, command):
    os.write(fd, f"voctrace {command}\r".encode("ascii"))
    data = bytearray()
    deadline = time.monotonic() + 5
    while select.select([fd], [], [], max(0, deadline - time.monotonic()))[0]:
        chunk = os.read(fd, 4096)
        if not chunk:
            break
        data.extend(chunk)
        if b"trace_ack " in data and data.endswith(b"\r\n> "):
            report = {}
            steps = []
            for line in data.decode("ascii").splitlines():
                key, _, value = line.partition(" ")
                if key in FIELDS or key == "trace_ack":
                    report[key] = int(value)
                elif key == "step":
                    steps.append(int(value))
            return report, steps
        if time.monotonic() >= deadline:
            break
    raise TimeoutError("Incomplete voctrace response")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("device")
    parser.add_argument("action", choices=("arm", "dump"))
    args = parser.parse_args()
    fd = os.open(args.device, os.O_RDWR | os.O_NOCTTY)
    original = termios.tcgetattr(fd)
    try:
        tty.setraw(fd)
        termios.tcflush(fd, termios.TCIFLUSH)
        armed = args.action == "arm"
        exchange(fd, "on" if armed else "off")
        for _ in range(10):
            report, _ = exchange(fd, "")
            if bool(report["trace_ack"] & 1) == armed:
                break
            time.sleep(.05)
        else:
            raise TimeoutError("Render IRQ did not acknowledge trace mode")
        if armed:
            print(f"Trace armed: epoch {report['trace_ack']}")
            return
        writer = csv.writer(sys.stdout)
        writer.writerow(("row", *FIELDS, *(f"step_{i}" for i in range(1, 7))))
        for i in range(13):
            report, steps = exchange(fd, f"row {i}")
            if len(steps) != 6:
                raise ValueError(f"Unexpected number of tracks in trace row {i}")
            writer.writerow((i, *(report[key] for key in FIELDS), *steps))
    finally:
        termios.tcsetattr(fd, termios.TCSANOW, original)
        os.close(fd)


if __name__ == "__main__":
    main()
