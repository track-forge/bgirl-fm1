#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-only
"""Original, size-specific BGIRL SVG strokes -> existing four-bit icon cells.
Only the deliberately small SVG vocabulary below is supported; unsupported edits
fail the build. Runtime still uses AI*_DATA / cv_alpha_hc, never an SVG renderer.
"""
from pathlib import Path
import xml.etree.ElementTree as ET
from PIL import Image, ImageDraw
import aa_raster as ar

NAMES = ("bg_stage", "bg_lab", "bg_cut", "bg_vinyl")
ROOT = Path(__file__).resolve().parents[1] / "assets" / "bgirl-icons"


def raster(name, size, gamma=1.0):
    ss = 16
    svg = ET.parse(ROOT / f"{name}-{size}.svg").getroot()
    assert svg.attrib["viewBox"] == f"0 0 {size} {size}"
    img = Image.new("L", (size * ss, size * ss))
    d = ImageDraw.Draw(img)
    for e in svg:
        tag = e.tag.rsplit("}", 1)[-1]
        if tag == "title":
            continue
        width = round(float(e.attrib.get("stroke-width", "1")) * ss)
        if tag == "polyline":
            pts = [tuple(float(v) * ss for v in p.split(",")) for p in e.attrib["points"].split()]
            d.line(pts, fill=255, width=width, joint="curve")
        elif tag == "circle":
            x,y,r=[float(e.attrib[k])*ss for k in ("cx","cy","r")]
            d.ellipse((x-r,y-r,x+r,y+r),outline=255,width=width)
        elif tag == "rect":
            x, y, w, h = [float(e.attrib[k]) * ss for k in ("x", "y", "width", "height")]
            d.rectangle((x, y, x + w, y + h), outline=255, width=width)
        else:
            raise ValueError(f"unsupported SVG element {tag}")
    img = img.resize((size, size), Image.Resampling.BOX)
    values = [ar.alpha4(v, gamma) for v in img.tobytes()]
    return ar.pack(values), any(values)
