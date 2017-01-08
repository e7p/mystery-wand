#!/bin/env python3
from PIL import Image
import sys
import os

if len(sys.argv) < 2 or not os.path.isfile(sys.argv[1]):
	print("Error: no file supplied")
	sys.exit(1)

img = Image.open(sys.argv[1])
pixels = img.load()
counter = 0
for y in range(img.height):
	byte = 0
	for x in range(img.width):
		if (type(pixels[x,y]) != int and (a == 0 for a in pixels[x,y][:3])) or (type(pixels[x,y]) == int and pixels[x,y] == 0):
			px = 0
		else:
			px = 1

		byte = (byte << 1) | px
		if(x % 8 == 7):
			if(counter%int(img.width/8) == 0):
				print()
			print("0x" + hex(byte)[2:].zfill(2).upper() + ", ", end="");
			byte = 0
			counter = counter + 1
print()