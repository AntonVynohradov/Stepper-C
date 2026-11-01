#******************************************************************************
# Stepper-C: Stepper Motor Control Implementation
#
# Project:     Stepper-C - Hardware-Agnostic Stepper Motor Control Library
# Description: Make the dark background of a raster hero illustration transparent,
#              so that it blends into the landing page and the PDF cover.
#
# Author:      Anton Vynohradov
# Email:       avynohradov@systemfromscratch.com
#
# License:     Apache License 2.0
#
# Copyright (c) 2026 Anton Vynohradov
#
# SPDX-License-Identifier: Apache-2.0
#*****************************************************************************

"""Turn the background of a dark hero illustration (a generated render, for example from
Canva) into transparency.

A generated image has its own near-black background. On the landing page and on the PDF
cover it sits on a navy gradient with a blue radial glow, so its rectangle shows. This
script keeps every pixel that differs from the background colour (the board, the device,
the glowing signal) and fades the rest to transparent:

    alpha = smoothstep(low, high, max(|R - Rb|, |G - Gb|, |B - Bb|))

The background colour (Rb, Gb, Bb) is the median of the four corners unless given.
Very dark parts of the subject become slightly transparent too; that is invisible on the
dark backgrounds the image is made for. Only Pillow is needed (a WeasyPrint dependency).

    python tools/docs/hero_alpha.py hero.jpg docs/assets/images/hero-dark.png
    python tools/docs/hero_alpha.py hero.jpg out.png --low 4 --high 48 --preview check.png
"""

from __future__ import annotations

import argparse
import statistics
import sys

from PIL import Image, ImageChops


def smoothstep_lut(low: int, high: int) -> list:
    """Lookup table 0..255 -> 0..255 of smoothstep(low, high, x)."""
    lut = []
    for x in range(256):
        t = min(max((x - low) / float(high - low), 0.0), 1.0)
        lut.append(round(255 * t * t * (3 - 2 * t)))
    return lut


def corner_colour(im: Image.Image) -> tuple:
    """Median colour of the four corner pixels."""
    w, h = im.size
    corners = [im.getpixel(p) for p in ((0, 0), (w - 1, 0), (0, h - 1), (w - 1, h - 1))]
    return tuple(int(statistics.median(c[k] for c in corners)) for k in range(3))


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__.split("\n\n")[0])
    ap.add_argument("source", help="dark illustration (JPG or PNG)")
    ap.add_argument("output", help="PNG with a transparent background")
    ap.add_argument("--background", help="background colour as R,G,B (default: corners)")
    ap.add_argument("--low", type=int, default=4, help="difference that stays transparent")
    ap.add_argument("--high", type=int, default=48, help="difference that is fully opaque")
    ap.add_argument("--preview", help="also write the result over a lighter navy, to check")
    args = ap.parse_args()
    if not 0 <= args.low < args.high <= 255:
        ap.error("need 0 <= low < high <= 255")

    im = Image.open(args.source).convert("RGB")
    bg = (tuple(int(v) for v in args.background.split(",")) if args.background
          else corner_colour(im))
    diff = ImageChops.difference(im, Image.new("RGB", im.size, bg))
    r, g, b = diff.split()
    alpha = ImageChops.lighter(ImageChops.lighter(r, g), b).point(
        smoothstep_lut(args.low, args.high))
    out = im.copy()
    out.putalpha(alpha)
    out.save(args.output, optimize=True)
    print(f"wrote {args.output} ({im.size[0]}x{im.size[1]}, background {bg})")

    if args.preview:
        check = Image.new("RGBA", im.size, (14, 40, 80, 255))
        check.alpha_composite(out)
        check.convert("RGB").save(args.preview)
        print(f"wrote {args.preview}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
