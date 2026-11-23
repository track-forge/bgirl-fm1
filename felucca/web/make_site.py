#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
# Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments
"""Make the site (GitHub Pages):

  index.html                  redirect to the installer (the old URL keeps working)
  firmware/felucca-VER.fwsc   the package (+ LICENSE, LICENSING.md, LICENSES/: the package holds
                              JieLi SDK files under Apache-2.0, see LICENSING.md)
  webapp/installer/index.html index_pkg.html, self-contained (fm1pkg.js, fm1ota.js, metadata inlined)
  webapp/editor/index.html    editor.html (+ fukiai.ttf, FUKIAI-LICENSE.txt)
  src/                        not touched

  web/make_site.py build/felucca-X.Y.fwsc X.Y OUT_DIR

The package must be one made by tools/fm1pkg_make.py (Felucca's own loader, no vendor files).
Its identity is read from the package; the device must report it after
the install.
"""
import argparse
import json
import re
import shutil
from pathlib import Path

HERE = Path(__file__).resolve().parent
BLOCKS, BLK, KEEP = 20, 0x30, 0x2F


def strip_module(src):
    src = re.sub(r"^export\s+", "", src, flags=re.M)
    return re.sub(r"^import .*?;\n", "", src, flags=re.M)


def product_of(raw):
    """the package identity: one marker byte after each of the first 20 blocks (fm1pkg.js productOf)"""
    return "".join(chr((m - i - 1) & 0xFF) for i in range(BLOCKS) if (m := raw[i * BLK + KEEP]) != 0x7D)


def main(pkg, version, out, miami=None, installer_only=False):
    pkg, out = Path(pkg), Path(out)
    raw = pkg.read_bytes()
    product = product_of(raw)
    if product != "FM-1_247101" and not re.fullmatch(r"FM-1_9\d\d", product):
        raise SystemExit(f"{pkg}: identity {product!r} is not a recognized BGIRL or Felucca identity")
    if b"FELUCCA-LOADER-1" not in raw:              # Felucca loader marker: never publish a package with vendor files
        raise SystemExit(f"{pkg}: no Felucca loader in it; the site ships only fm1pkg_make.py packages "
                         "(a package patched from an official one carries vendor files)")
    html = (HERE / "index_pkg.html").read_text(encoding="utf-8")
    if installer_only:
        html = html.replace("<title>Felucca</title>", "<title>BGIRL FM-1 installer</title>")
        html = html.replace("<h1>Felucca</h1>", "<h1>BGIRL FM-1</h1>")
        html = html.replace("Felucca 1.0 is a field-testing release; please report problems on GitHub.",
                            "BGIRL FM-1 is custom firmware for the M-VAVE FM-1.")
        html = html.replace("Felucca 1.0 は実地テスト版です。不具合は GitHub で知らせてください。",
                            "BGIRL FM-1 は M-VAVE FM-1 用のカスタムファームウェアです。")
        html = html.replace("say(`Felucca ${meta.version}`)", "say(`BGIRL FM-1 ${meta.version}`)")
    lib = strip_module((HERE / "fm1pkg.js").read_text(encoding="utf-8")) + "\n" + \
        strip_module((HERE / "fm1ota.js").read_text(encoding="utf-8")) + "\n" + \
        strip_module((HERE / "fm1backup.js").read_text(encoding="utf-8")) + "\n" + \
        strip_module((HERE / "fm1miami.js").read_text(encoding="utf-8"))
    prefix = "bgirl-fm1" if installer_only else "felucca"
    name = f"{prefix}-{re.sub(r'[^A-Za-z0-9.-]', '-', version)}.fwsc"
    bank = Path(miami).read_bytes() if miami else None
    meta = json.dumps({"version": version, "product": product, "pkg": "../../firmware/" + name,
                       "miami": "../../firmware/miami.slot" if bank else None})
    for mark in ("/*LIB*/", "/*META*/"):
        if html.count(mark) != 1:
            raise SystemExit(f"index_pkg.html must contain {mark} once; update make_site.py")
    html = html.replace("/*LIB*/", lib).replace("/*META*/", meta)
    inst, ed, fw = out / "webapp" / "installer", out / "webapp" / "editor", out / "firmware"
    for d in (inst, fw):
        d.mkdir(parents=True, exist_ok=True)
    for old in fw.glob(f"{prefix}-*.fwsc"):          # one package: the current one
        old.unlink()
    (inst / "index.html").write_text(html, encoding="utf-8")
    shutil.copy(pkg, fw / name)
    if bank:
        (fw / "miami.slot").write_bytes(bank)
    lic = HERE.parent / "LICENSES"                  # the package holds JieLi SDK files (Apache-2.0): their
    (fw / "LICENSES").mkdir(exist_ok=True)          # licence travels next to it, with Felucca's own
    names = sorted(f.name for f in lic.glob("*.txt"))
    for n in names:
        shutil.copy(lic / n, fw / "LICENSES" / n)
    (fw / "LICENSES" / "index.html").write_text(    # the installer links this folder: Pages lists no folders
        '<!doctype html><meta charset="utf-8"><title>Felucca licences</title><h1>Licence texts</h1><ul>'
        + "".join(f'<li><a href="{n}">{n}</a></li>' for n in names)
        + '</ul><p><a href="../LICENSING.md">LICENSING.md</a> · <a href="../LICENSE">LICENSE (GPL-3.0)</a></p>\n',
        encoding="utf-8")
    for doc in ("LICENSE", "LICENSING.md"):
        shutil.copy(HERE.parent / doc, fw / doc)
    if installer_only:
        print(f"site: {out}: webapp/installer, firmware/{name} ({len(raw)} B, {product})")
        return
    ed.mkdir(parents=True, exist_ok=True)
    shutil.copy(HERE / "editor.html", ed / "index.html")
    for f in ("fukiai.ttf", "FUKIAI-LICENSE.txt", "fm1backup.js"):
        if (HERE / f).exists():
            shutil.copy(HERE / f, ed / f)
    (out / "index.html").write_text(
        '<!doctype html><meta charset="utf-8"><title>Felucca</title>'
        '<meta http-equiv="refresh" content="0; url=webapp/installer/">'
        '<a href="webapp/installer/">Felucca installer</a>\n', encoding="utf-8")
    print(f"site: {out}: webapp/installer ({len(html)} B), webapp/editor, firmware/{name} ({len(raw)} B, {product})")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("package")
    parser.add_argument("version")
    parser.add_argument("output")
    parser.add_argument("--miami", help="matching hardware build/gen/miami.slot")
    parser.add_argument("--installer-only", action="store_true", help="omit editor and homepage redirect")
    args = parser.parse_args()
    main(args.package, args.version, args.output, args.miami, args.installer_only)
