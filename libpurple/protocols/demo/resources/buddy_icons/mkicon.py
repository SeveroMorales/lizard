#!/usr/bin/env python3
#
# Purple - Internet Messaging Library
# Copyright (C) Pidgin Developers <devel@pidgin.im>
#
# Purple is the legal property of its developers, whose names are too numerous
# to list here.  Please refer to the COPYRIGHT file distributed with this
# source distribution.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, see <https://www.gnu.org/licenses/>.

import argparse
import colorsys
import hashlib
from pathlib import Path

from PIL import Image, ImageDraw, ImageFont

parser = argparse.ArgumentParser(
    description='Generate buddy icons from user names.')
parser.add_argument('name', nargs='+', help='The name(s) to use.')
parser.add_argument('-f', '--font',
                    default='/usr/share/fonts/urw-base35/D050000L.otf',
                    help='Path to (TrueType or OpenType) font to use.')
parser.add_argument('-s', '--size', default=96, type=int,
                    help='Size of buddy icons to produce.')
parser.add_argument('-o', '--output', default='.',
                    help='Directory in which to place files.')
args = parser.parse_args()


def calculate_colours_for_text(text):
    """
    Calculate the foreground and background colours from text.

    This is based on pidgin_color_calculate_for_text in Pidgin C code.
    """
    # Hash the string and get the first 2 bytes of the digest.
    checksum = hashlib.sha1()
    checksum.update(text.encode('utf-8'))
    digest = checksum.digest()

    # Calculate the hue based on the digest, scaled to 0-1.
    hue = (digest[0] << 8 | digest[1]) / 65535
    # Get the RGB values for the hue at full saturation and value.
    foreground = colorsys.hsv_to_rgb(hue, 1.0, 1.0)

    # Calculate the hue based on the end of the digest, scaled to 0-1.
    hue = (digest[-1] << 8 | digest[-2]) / 65535
    # Get the RGB values for the hue at full saturation and low value.
    background = colorsys.hsv_to_rgb(hue, 1.0, 0.2)

    # Finally calculate the foreground summing 20% of the inverted background
    # with 80% of the foreground.
    foreground = (
        (0.2 * (1 - bc)) + (0.8 * fc) for bc, fc in zip(background, foreground)
    )

    # Pillow requires colours in 0-255.
    return (tuple(int(c * 255) for c in foreground),
            tuple(int(c * 255) for c in background))


output_dir = Path(args.output)
font = ImageFont.truetype(args.font, size=int(args.size * 96/72))

for name in args.name:
    fore, back = calculate_colours_for_text(name)

    # Generate an image using the first letter of the name to choose a glyph
    # from the specified font. The default font generates some star-like or
    # flower-like symbols.
    img = Image.new('RGBA', (args.size, args.size), color=back)
    draw = ImageDraw.Draw(img)
    letter = name[0].upper()
    draw.text((args.size // 2, args.size - 1), letter,
              font=font, anchor='mb', fill=fore)

    img.save(output_dir / f'{name}.png', 'PNG')
