#!/bin/env python3
from PIL import Image
import sys
import os

if len(sys.argv) < 2 or not os.path.isfile(sys.argv[1]):
	print("Error: no file supplied")
	sys.exit(1)

img = Image.open(sys.argv[1])
pixels = img.load()
for row in range(int(img.width / 8)):
	for y in range(img.height):
		byte = 0
		for x in range(8):
			if (type(pixels[x+row*8,y]) != int and (a == 0 for a in pixels[x+row*8,y][:3])) or (type(pixels[x+row*8,y]) == int and pixels[x+row*8,y] == 0):
				px = 0
			else:
				px = 1
			byte = (byte << 1) | px
		print("0x" + hex(byte)[2:].zfill(2).upper() + ", ", end="");
		if (y % 8 == 7):
			print(" // tile " + str(int(row*(img.height/8)+(y/8))))