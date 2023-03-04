#!/usr/bin/env python3
# talkatu
# Copyright (C) 2017-2020 Gary Kramlich <grim@reaperworld.com>
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
# You should have received a copy of the GNU General Public License
# along with this library; if not, see <https://www.gnu.org/licenses/>.

"""
This script will run the command specified via command line arguments,
making sure broadwayd is running and will be used.
"""

import os
import subprocess
import sys


def main():
	# start broadway
	broadwayd  = subprocess.Popen(['gtk4-broadwayd'])

	# run the unit test but set the GDK_BACKEND envvar to broadway
	env = {**os.environ, 'GDK_BACKEND': 'broadway'}

	try:
		proc = subprocess.run(args=sys.argv[1:], env=env)
	finally:
		# kill broadway
		broadwayd.kill()

	# return the exit code of the unit test
	sys.exit(proc.returncode)


if __name__ == '__main__':
	main()
