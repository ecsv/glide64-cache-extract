#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# analyze_tex, Nintendo64 hires texture analyzing tool for debugging
#
# Copyright (C) 2013  Sven Eckelmann <sven@narfation.org>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

import os
import sys
from PIL import ImageFile
ImageFile.MAXBLOCK = 1000000 # default is 64k
from PIL import Image
import re

def main():
	arguments = sys.argv[1:]
	if len(arguments) == 0:
		sys.stderr.write('Usage: %s DIRECTORY\n' % (sys.argv[0]))
		sys.exit(1)

	files = get_files(arguments[0])
	images = analyze_image(arguments[0], files)
	print_info(images)

def get_files(directory):
	importfiles = []

	for root, dirs, files in os.walk(directory):
		for file in files:
			path = os.path.join(root, file)
			path = os.path.relpath(path, directory)
			importfiles.append(path)

	return importfiles

def count_trailing_zeros(x):
	zeros = 0

	while ((1 << zeros) & x) == 0:
		zeros += 1

	return zeros
	

def get_min_square_size(width, height):
	w = count_trailing_zeros(width)
	h = count_trailing_zeros(height)
	shift = min(w, h)
	return (width >> shift, height >> shift)

def get_hires_params(name):
	pos = name.find('#')
	if pos == -1:
		return None

	rest = name[pos:]
	parse = re.search('#(?P<chksum>[\dA-Fa-f]{8})#(?P<fmt>[\dA-Fa-f]{1})#(?P<siz>[\dA-Fa-f]{1})(#(?P<palchksum>[\dA-Fa-f]{8}))?', rest)
	if not parse:
		sys.stderr.write('Could parse filename %s\n' % (name))
		return None

	if parse.group('palchksum'):
		chksum = parse.group('palchksum') + parse.group('chksum')
	else:
		chksum = "00000000" + parse.group('chksum')

	return {'crc': chksum, 'format': parse.group('fmt'), 'size': parse.group('siz') }

def analyze_image(directory, files):
	images = []
	for f in files:
		im = Image.open(os.path.join(directory, f))
		if not im:
			sys.stderr.write('Could not open %s\n' % (f))
			continue

		width, height = get_min_square_size(im.size[0], im.size[1])
		folder = os.path.dirname(f)
		name = os.path.basename(f)
		hires = get_hires_params(name)

		if not hires:
			continue

		info = {'crc': hires['crc'], 'minwidth': width, 'minheight': height, 'size': hires['size'], 'format': hires['format'], 'folder': folder}
		images.append(info)

	return images

def print_info(images):
	for image in images:
		print('%s\t%d\t%d\t%s\t%s\t%s' % (image['crc'], image['minwidth'], image['minheight'], image['size'], image['format'], image['folder']))

if __name__ == '__main__':
	main()
