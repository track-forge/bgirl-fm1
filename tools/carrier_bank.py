#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Generate the single local ANALOG wrapper: inherited + carrier + SH-STYLE."""
import sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from tools.sh_style.bank import generate
generate('--check' in sys.argv)
