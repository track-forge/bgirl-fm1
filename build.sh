#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")"
# This planning scaffold deliberately retains the upstream template identity.
case "${1:-}" in
  --host|--web) ;;
  *) echo "Usage: ./build.sh --host|--web (device/release setup is deferred)" >&2; exit 2 ;;
esac
if [[ $# != 1 ]]; then echo "Pass exactly one build target." >&2; exit 2; fi
exec python3 base/tools/build.py app "$1"
