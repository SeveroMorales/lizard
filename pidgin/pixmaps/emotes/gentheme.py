#!/usr/bin/env python3
#
# Pidgin - Internet Messenger
# Copyright (C) Pidgin Developers <devel@pidgin.im>
#
# Pidgin is the legal property of its developers, whose names are too numerous
# to list here.  Please refer to the COPYRIGHT file distributed with this
# source distribution.
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, see <https://www.gnu.org/licenses/>.

import sys

input = sys.argv[1]
output = sys.argv[2]

with open(input, 'rb') as r, open(output, 'wb') as w:
    for line in r.readlines():
        if line.startswith((b'_Name=', b'_Description=', b'_Author=')):
            line = line[1:]
        w.write(line)
