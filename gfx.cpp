#include <TVout.h>
#include <avr/pgmspace.h>

#include "gfx.h"

#include "gfxdata.cpp"

const uint8_t *gfxdata[] = {NULL, tileset, wizard, wizard_mask, dragon, spider, logo,
	tileset+96, tileset+104, tileset+120, dragon+192, dragon+204,
	tileset+240, tileset+232, tileset+224};

extern TVout TV;

void draw_sprite(uint8_t x, uint8_t y, const uint8_t sprite, uint8_t width, uint8_t height) {
	uint16_t offset = (uint16_t)y*16+(uint16_t)x;
	const uint8_t *spritedata = gfxdata[sprite];
	for(; height != 0; height--) {
		for(uint8_t xp = 0; xp < width; xp++) {
			TV.screen[offset+xp] = pgm_read_byte(spritedata);
			spritedata++;
		}
		offset += 16;
	}
}

/*

0 C X X X X X X X X X X X X X X C0
1 D X X X X X X X X X X X X X X C1
2 X X X X X X X X X X X X X X X .
3 X X X X X X X X X X X X X X X .
X X X X X X X X X X X X X X X X .
X X X X X X X X X X X X X X X X .
X X X X X X X X X X X X X X X X .
X X X X X X X X X X X X X X X X .
X X X X X X X X X X X X X X X X .
X X X X X X X X X X X X X X X X .
A X X X X X X X X X X X X X X BE CA
B X X X X X X X X X X X X X X BF CB

*/

uint8_t bg_tiles[204];

/* scroll_x: between 0 and 8 (inclusive)
   0 means, that the first tile is fully visible on the left side of the screen
   8 means, that the last tile is fully visible on the right side of the screen
   values in between mean, that parts of first and last row are visible */
void draw_bg(uint8_t scroll_x) {
	for(uint8_t y = 1; y < 12; y++) {
		for(uint8_t i = 0; i < 8; i++) {
			uint8_t line, nextline = pgm_read_byte(&tileset[bg_tiles[y*17]*8+i]);
			for(uint8_t x = 0; x < 16; x++) {
				line = nextline;
				nextline = pgm_read_byte(&tileset[bg_tiles[x+y*17+1]*8+i]);
				if (scroll_x == 8) {
					line = nextline;
				} else if (scroll_x > 0) {
					line = (line << scroll_x) | (nextline >> (8-scroll_x));
				}
				TV.screen[((y*8+i)*16+x)] = line;
			}
		}
	}
}

uint8_t frame_counter;

void draw_object(const uint8_t object, const uint8_t mask,
	const uint8_t number_frames, uint8_t xoff, uint8_t y,
	uint8_t width, uint8_t height) {
	uint8_t offset = (frame_counter % number_frames)*width*height;
	uint8_t ymax = y + height;
	for(; y < ymax; y++) {
		uint8_t nextline, nextmaskline, line = 0, maskline = 0;
		for(uint8_t x = 0; x < width; x++) {
			if(xoff + (x+1)*8 < 128) {
				nextline = pgm_read_byte(&gfxdata[object][offset]);
				nextmaskline = (mask == GFXDATA_NULL ? 0xFF : pgm_read_byte(&gfxdata[mask][offset]));
				if(xoff % 8 != 0) {
					line |= (nextline >> (xoff % 8));
					maskline |= (nextmaskline >> (xoff % 8));
					TV.screen[(y*16)+xoff/8+x] = (TV.screen[(y*16)+xoff/8+x] & ~maskline) | line;
					line = (nextline << (8 - (xoff % 8)));
					maskline = (nextmaskline << (8 - (xoff % 8)));
				} else {
					line = nextline;
					maskline = nextmaskline;
					TV.screen[(y*16)+xoff/8+x] = (TV.screen[(y*16)+xoff/8+x] & ~maskline) | line;
				}
			}
			offset++;
		}
		if(xoff % 8 != 0) {
			TV.screen[(y*16)+xoff/8+width] = (TV.screen[(y*16)+xoff/8+width] & ~maskline) | line;
		}
	}
}

void shiftleft_bg_tiles(const uint8_t **nextcolumn) {
	for(uint8_t x = 0; x < 16; x++) {
		for(uint8_t y = 0; y < 12; y++) {
			bg_tiles[y*17+x] = bg_tiles[y*17+x+1];
		}
	}
	for(uint8_t y = 0; y < 12; y++) {
		bg_tiles[y*17+16] = pgm_read_byte((*nextcolumn)++);
	}
}