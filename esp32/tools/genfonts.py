#!/usr/bin/env python3
"""Generate Adafruit GFX font headers for the display.

Adafruit's own fontconvert only emits a contiguous codepoint range, which would
drag in some 200 unused glyphs to reach the Norwegian letters. This renders an
explicit character set instead, mapping the non-ASCII ones into a private index
space just above ASCII so the GFXfont stays contiguous and small.

The device never runs this. Headers are generated once and committed; run it
again only when the character set or a size changes.

    python3 tools/genfonts.py
"""

import os
import sys
import urllib.request

from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.abspath(__file__))
CACHE = os.path.join(HERE, ".fonts")
OUT = os.path.abspath(os.path.join(HERE, "..", "src", "fonts"))

GOOGLE_FONTS = "https://raw.githubusercontent.com/google/fonts/main/ofl"
OFL_URL = "https://scripts.sil.org/OFL"

PLEX_NOTICE = [
    "Derived from IBM Plex Mono.",
    'Copyright (c) 2017 IBM Corp. with Reserved Font Name "Plex".',
]

PLEX_FOOTNOTE = [
    "Named Mono* rather than Plex*: OFL section 3 restricts a Modified",
    "Version's use of a Reserved Font Name. Do not rename it back.",
]

# Both families are SIL Open Font License 1.1. Rasterising outlines into GFX
# bitmaps is a "Modified Version" under OFL section 1, so the notices below
# travel with the generated headers and the licence texts are vendored beside
# them. IBM Plex reserves the name "Plex", which OFL section 3 forbids a
# Modified Version from using, hence the neutral Mono* names further down.
SOURCES = {
    "Archivo.ttf": {
        "url": f"{GOOGLE_FONTS}/archivo/Archivo%5Bwdth,wght%5D.ttf",
        "ofl": f"{GOOGLE_FONTS}/archivo/OFL.txt",
        "licence": "OFL-Archivo.txt",
        "notice": [
            "Copyright 2020 The Archivo Project Authors",
            "(https://github.com/Omnibus-Type/Archivo).",
        ],
    },
    "IBMPlexMono-Regular.ttf": {
        "url": f"{GOOGLE_FONTS}/ibmplexmono/IBMPlexMono-Regular.ttf",
        "ofl": f"{GOOGLE_FONTS}/ibmplexmono/OFL.txt",
        "licence": "OFL-IBMPlexMono.txt",
        "notice": PLEX_NOTICE,
        "footnote": PLEX_FOOTNOTE,
    },
    "IBMPlexMono-SemiBold.ttf": {
        "url": f"{GOOGLE_FONTS}/ibmplexmono/IBMPlexMono-SemiBold.ttf",
        "ofl": f"{GOOGLE_FONTS}/ibmplexmono/OFL.txt",
        "licence": "OFL-IBMPlexMono.txt",
        "notice": PLEX_NOTICE,
        "footnote": PLEX_FOOTNOTE,
    },
}

# Characters above ASCII that the design needs, each given an index just past
# the printable range. The UTF-8 text helper maps codepoints onto these.
# No em or en dashes. The design used an em dash for the dry-hour marker and an
# en dash for time ranges; both are plain hyphens instead.
EXTRAS = ["æ", "ø", "å", "Æ", "Ø", "Å", "·", "°", "…"]
EXTRA_BASE = 0x7F

ASCII = [chr(c) for c in range(0x20, 0x7F)]

# Latin text plus the symbols that appear in running copy.
TEXT_SET = ASCII + EXTRAS

# The hero numerals show digits, a degree sign, and a minus for temperatures
# below freezing.
NUMERAL_SET = [chr(c) for c in range(0x30, 0x3A)] + ["-", "°"]

# name, source, variable weight, pixel size, character set
FONTS = [
    ("ArchivoBlack56", "Archivo.ttf", 900, 56, NUMERAL_SET),
    ("ArchivoBold19", "Archivo.ttf", 700, 19, TEXT_SET),
    ("ArchivoSemiBold16", "Archivo.ttf", 600, 16, TEXT_SET),
    ("MonoSemiBold15", "IBMPlexMono-SemiBold.ttf", None, 15, TEXT_SET),
    ("MonoRegular13", "IBMPlexMono-Regular.ttf", None, 13, TEXT_SET),
]

THRESHOLD = 128


def index_of(char):
    """Map a character to its slot in the font's index space."""
    if char in EXTRAS:
        return EXTRA_BASE + EXTRAS.index(char)

    return ord(char)


def ensure_sources():
    os.makedirs(CACHE, exist_ok=True)

    os.makedirs(OUT, exist_ok=True)

    for name, source in SOURCES.items():
        path = os.path.join(CACHE, name)

        if not os.path.exists(path):
            print(f"fetching {name}")
            urllib.request.urlretrieve(source["url"], path)

        # The licence has to ship next to what it covers.
        licence = os.path.join(OUT, source["licence"])

        if not os.path.exists(licence):
            print(f"fetching {source['licence']}")
            urllib.request.urlretrieve(source["ofl"], licence)


def load(source, weight, size):
    font = ImageFont.truetype(os.path.join(CACHE, source), size)

    if weight is not None:
        # Archivo's variation axes are Weight then Width.
        font.set_variation_by_axes([weight, 100])

    return font


def render(font, char, size):
    """Rasterise one glyph, returning its packed bits and placement.

    Offsets follow the GFXglyph convention: relative to a cursor sitting on the
    baseline, with yOffset negative above it. Glyph bitmaps are a continuous
    bitstream, not padded to a byte at each row.
    """
    pad = size * 2
    canvas = Image.new("L", (size * 4, size * 4), 0)

    # Anchor "ls" puts the origin at the left edge, on the baseline.
    ImageDraw.Draw(canvas).text((pad, pad), char, font=font, fill=255, anchor="ls")

    mono = canvas.point(lambda v: 255 if v >= THRESHOLD else 0, mode="1")
    box = mono.getbbox()
    advance = round(font.getlength(char))

    if box is None:  # whitespace has no ink
        return b"", 0, 0, advance, 0, 0

    x0, y0, x1, y1 = box

    bits = bytearray()
    acc = 0
    count = 0

    for y in range(y0, y1):
        for x in range(x0, x1):
            acc = (acc << 1) | (1 if mono.getpixel((x, y)) else 0)
            count += 1

            if count == 8:
                bits.append(acc)
                acc = 0
                count = 0

    if count:
        bits.append(acc << (8 - count))

    return bytes(bits), x1 - x0, y1 - y0, advance, x0 - pad, y0 - pad


def generate(name, source, weight, size, charset):
    font = load(source, weight, size)

    glyphs = {}
    bitmap = bytearray()

    for char in charset:
        data, width, height, advance, x_offset, y_offset = render(font, char, size)

        glyphs[index_of(char)] = {
            "offset": len(bitmap),
            "width": width,
            "height": height,
            "advance": advance,
            "x": x_offset,
            "y": y_offset,
            "char": char,
        }

        bitmap.extend(data)

    first = min(glyphs)
    last = max(glyphs)
    ascent, descent = font.getmetrics()

    described = f"{source}, {size}px"
    if weight is not None:
        described = f"{source} weight {weight}, {size}px"

    lines = [
        "// Generated by tools/genfonts.py. Do not edit.",
        f"// {described}, {len(charset)} glyphs.",
        "//",
        *[f"// {line}" for line in SOURCES[source]["notice"]],
        "//",
        "// This file is a Modified Version of that font under the SIL Open",
        "// Font License 1.1, and is licensed under the OFL rather than this",
        f"// project's licence. See {SOURCES[source]['licence']}, or",
        f"// {OFL_URL}",
        *(
            ["//", *[f"// {line}" for line in SOURCES[source]["footnote"]]]
            if "footnote" in SOURCES[source]
            else []
        ),
        "#pragma once",
        "#include <Adafruit_GFX.h>",
        "",
        f"const uint8_t {name}Bitmaps[] PROGMEM = {{",
    ]

    for i in range(0, len(bitmap), 12):
        lines.append("    " + ", ".join(f"0x{b:02X}" for b in bitmap[i : i + 12]) + ",")

    lines += ["};", "", f"const GFXglyph {name}Glyphs[] PROGMEM = {{"]

    for index in range(first, last + 1):
        glyph = glyphs.get(index)

        if glyph is None:
            lines.append("    {0, 0, 0, 0, 0, 0},  // unused")
            continue

        # A trailing backslash would continue the // comment onto the next line.
        label = {" ": "space", "\\": "backslash"}.get(glyph["char"], glyph["char"])
        lines.append(
            f"    {{{glyph['offset']}, {glyph['width']}, {glyph['height']}, "
            f"{glyph['advance']}, {glyph['x']}, {glyph['y']}}},  // {label}"
        )

    lines += [
        "};",
        "",
        f"const GFXfont {name} PROGMEM = {{(uint8_t *){name}Bitmaps,",
        f"                                (GFXglyph *){name}Glyphs, 0x{first:02X},",
        f"                                0x{last:02X}, {ascent + descent}}};",
        "",
        f"// Approx. {len(bitmap) + (last - first + 1) * 7 + 7} bytes",
        "",
    ]

    os.makedirs(OUT, exist_ok=True)

    with open(os.path.join(OUT, f"{name}.h"), "w") as handle:
        handle.write("\n".join(lines))

    total = len(bitmap) + (last - first + 1) * 7 + 7

    print(
        f"{name:20s} {len(bitmap):6d} B bitmap  {last - first + 1:3d} slots  "
        f"yAdvance {ascent + descent:2d}  ~{total} B"
    )

    return total


README = """# Fonts

Bitmap fonts for the display, generated by `esp32/tools/genfonts.py`. Do not
edit anything in this directory by hand, including this file; change the
generator and run it again.

## Licence

**These files are under the SIL Open Font License 1.1, not the AGPL that covers
the rest of this repository.**

Rendering a typeface's outlines into bitmaps produces a Modified Version under
OFL section 1 ("by changing formats"), and section 5 requires such a version to
be distributed entirely under the OFL. The requirement does not reach the
firmware that draws with them, so bundling the two is fine.

| Files | Source | Notice |
| --- | --- | --- |
| `Archivo*.h` | [Archivo](https://github.com/Omnibus-Type/Archivo) | Copyright 2020 The Archivo Project Authors |
| `Mono*.h` | [IBM Plex Mono](https://github.com/IBM/plex) | Copyright (c) 2017 IBM Corp. with Reserved Font Name "Plex" |

IBM Plex reserves the name "Plex". OFL section 3 forbids a Modified Version
from using a Reserved Font Name, though it qualifies that the restriction
"only applies to the primary font name as presented to the users" \u2014 which a C
identifier inside firmware arguably is not. The derived fonts are named `Mono*`
anyway: it costs nothing and avoids the argument, particularly since "IBM
Plex" is also a registered trademark. Archivo declares no reserved name, so
`Archivo*` needs no such treatment.

Naming either origin in documentation, as above, is nominative use and is what
section 4 permits when acknowledging contributions. Both families additionally
carry trademarks ("Archivo" by Omnibus-Type, "IBM Plex" by IBM Corp), which are
separate from the licence.

Both source files set `fsType = 0`, so they carry no embedding restriction of
their own.

Full licence texts: `OFL-Archivo.txt`, `OFL-IBMPlexMono.txt`.
"""


def write_readme():
    """The licence carve-out must survive a wipe and regenerate."""
    with open(os.path.join(OUT, "README.md"), "w") as handle:
        handle.write(README)


def main():
    ensure_sources()
    write_readme()

    total = sum(generate(*spec) for spec in FONTS)
    print(f"\ntotal flash: ~{total} B")


if __name__ == "__main__":
    sys.exit(main())
