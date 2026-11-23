# Miami Bass hardware starter bank (#69)

## Format and ownership

The hardware profile stores the existing trimmed Miami pack unchanged, at
32,768 Hz, IMA ADPCM low-nibble first. The browser build remains embedded at
103,360 audio bytes, ID `a433fc11a7cef65c`. The hardware content identity is
`bb8f8cb449511455`; project and preset formats already carry that identity, not
a slot index. A browser project naming its different bank does not silently bind
to this bank. The factory demo explicitly binds to the build's identity.

| Offset | Bytes | Content |
| ---: | ---: | --- |
| 0 | 24 | BGVB magic, little-endian version 1, payload length and CRC-32, 8-byte content ID |
| 24 | 488 | Reserved (erased in the transport image; the provisioner writes a BGVP marker at 256) |
| 512 | 156 | 13 triples of start/end/IMA checkpoint (little-endian 32-bit words) |
| 668 | 512 | 128 IMA seek checkpoints (little-endian 32-bit words) |
| 1180 | 68,454 | Original trimmed ADPCM |
| **Total** | **69,634** | **12,286 free bytes in one 81,920-byte slot** |

Firmware scans the plain XIP window on boot and after a successful provision.
It accepts only the generated header and expected payload CRC, not some other
bank claiming the same ID. Metadata and samples are read directly from XIP;
render performs no scan, allocation or I/O. An unavailable bank shows MISS
and produces silence on VOCALS rather than playing an unrelated sample.
The two other slots remain ordinary SAMPLE/SLICE/GRAIN slots.

## Installer

Generate with `BGIRL_HARDWARE_ASSETS=1 python3 tools/generate.py`. Build the
matching firmware with `BGIRL_HARDWARE_ASSETS=1 ./felucca/build.sh`. Once the
target link blockers below are resolved, stage the existing installer with
`python3 felucca/web/make_site.py PACKAGE VERSION OUTPUT --miami felucca/build/gen/miami.slot`.
The bank is a separate asset; nothing is appended to the firmware package.

- **First install:** check **Add Miami Bass starter bank**, install firmware,
  and select an empty slot after the device reconnects. Play the factory demo
  with transport after the bank shows as verified. Existing sessions/settings
  are not reset; reload the factory demo explicitly if auditioning an upgraded device.
- **Routine upgrade:** leave the option unchecked. Existing bank, samples and
  settings are left alone by provisioning. A matching bank is never rewritten.
- **Restore:** use **Install or restore Miami Bass only**. A slot recognized as
  damaged or interrupted can be selected for repair. Replacing other content
  requires choosing its slot and confirming; save an editor backup first.
  The archive keeps ordinary samples, not the reproducible starter bank.
- The firmware updater's application writes remain in `[0x4000, 0x93000)`.
  Provisioning is confined to one of the three existing slots in
  `[0xA0000, 0xDC000)`. It uses the same JEDEC gate as ordinary sample writes
  (`0x856014` / `0xB36014`); older firmware and unsupported parts cannot
  provision through this flow. Downgrading to older firmware loses the new
  editor protection even if the bytes survive the firmware update.
- Editor SysEx 82 on `7D 46 36`: op 0 lists protocol/version and three slot
  states (0 erased, 1 matching, 2 restore, 3 occupied); op 1 takes slot,
  expected state, and packed 24-byte header; op 2 streams strictly ordered
  256-byte packed chunks (short final chunk); op 3 verifies CRC and commits
  the header last. Write replies are op, slot, result (0 success). Ordinary
  sample commands reject BGVB/BGVP slots and list them with sentinel 127.
  A power cut can leave no playable bank; this is a restart-from-begin
  protocol, not an atomic firmware+bank transaction. If interrupted before
  the marker, the slot may instead appear occupied or erased.

## Evidence and remaining hardware work

`./tests/run_miami_slot_test.sh` compares the slot and embedded hardware
builds over every decoded PCM sample, every shot start/end and grid
checkpoint, note mapping, unchanged shot pitch behavior, and four rendered
dry/vocoded WAVs. It exercises B6S1 recall after relocation and corrupt-bank
rejection, plus NOR-emulated transfer/restore and installer MIDI framing.
Run the browser suite with browser assets, not the hardware-generated headers.

JieLi offline link diagnosis (same #68 hardware profile, original linker):
XIP fits, RAM overflow changes from 4,132 to 4,164 bytes, POOL overflow is
unchanged at 4,100 bytes, and the pre-existing missing atomic helper still
blocks the package. For size accounting only, a temporary enlarged RAM/POOL
map and placeholder unresolved compiler helpers allowed an offline section
comparison: application load sections (XIP text + RAM-text + init data)
549,164 → 480,656 bytes (**−68,508**); RAM 77,860 → 77,892 (**+32**); POOL
372,740 → 372,740 (**0**). These diagnostic ELFs are not runnable artifacts.
The real linker script and updater image extent were not widened.

No physical device was flashed. Attended validation, after the independent
target blockers are fixed: boot and listen to the factory demo, confirm slot
durability after power-cycle, upgrade without provisioning, and verify
interrupted bank restore while retaining the other slots and settings.
