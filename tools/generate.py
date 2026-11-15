#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Browser-only entry into upstream's generators; no device toolchain/package needed."""
import sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'felucca/tools'))
import build
build.generate()
