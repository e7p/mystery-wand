#include <TVout.h>
//#include <font4x6.h>
#include <avr/pgmspace.h>
#include "sound.h"
#include <util/delay.h>

const uint8_t PROGMEM bootlogo[] = {
	0xF0, 0x01, 0x81, 0x8F, 0x8F, 0x3E, 0x60,
	0xD8, 0x01, 0x83, 0xCC, 0xD9, 0xB3, 0xD0,
	0xD8, 0x01, 0x86, 0x6C, 0xD9, 0xB3, 0xB0,
	0xDB, 0xDD, 0x87, 0xEF, 0x99, 0xBF, 0x60,
	0xDE, 0xF1, 0x86, 0x6C, 0xD9, 0xB6, 0x00,
	0xDE, 0xFD, 0x86, 0x6C, 0xD9, 0xB3, 0x00,
	0xDE, 0xCD, 0x86, 0x6C, 0xD9, 0xB3, 0x00,
	0xF3, 0xF9, 0xF6, 0x6F, 0x8F, 0x33, 0x00
};

TVout TV;

unsigned long nextmillis = 0;

void start_logo() {
	uint8_t frame = 0;
	do {
		if(TV.millis() > nextmillis) {
			nextmillis = TV.millis() + 20;
			frame++;
			if(frame < 8+8) {
				TV.fill(BLACK);
				for(uint8_t y = 0; y < frame-8; y++) {
					for(uint8_t x = 0; x < 7; x++) {
						TV.screen[5+y*16+x] = pgm_read_byte(&(bootlogo[49+(y-frame+9)*7+x]));
					}
				}
			} else if(frame < 8+8+44) {
				TV.fill(BLACK);
				for(uint8_t y = 0; y < 8; y++) {
					for(uint8_t x = 0; x < 7; x++) {
						TV.screen[5+(y+frame-7-8)*16+x] = pgm_read_byte(&(bootlogo[y*7+x]));
					}
				}
			} else if(frame == 8+8+44) {
				play_note_ch1(38, 255, 90, -10, 0);
			} else if(frame == 8+8+44+2) {
				play_note_ch1(47, 255, 90, -6, 0);
			}
		}
	} while(frame < 128);
	TV.fill(BLACK);
}

void draw_sprite(uint8_t x, uint8_t y, uint8_t *sprite, uint8_t width, uint8_t height) {
	uint8_t offset = y*16+x;
	for(; height != UINT8_MAX; height--) {
		for(uint8_t xp = 0; xp < width; xp++) {
			TV.screen[offset+xp] = pgm_read_byte(sprite);
			sprite++;
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

const uint8_t PROGMEM level_tiles[] = {
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 3, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 3, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2
};

const uint8_t PROGMEM tileset[] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0xFF, 0x00, 0xFF, 0x00, 0x44, 0x00, 0x11, 0x00,
	0x44, 0x00, 0x11, 0x00, 0x44, 0x00, 0x11, 0x00,
	0x7E, 0x81, 0x81, 0x91, 0x89, 0x81, 0x81, 0x7E
};

const uint8_t PROGMEM wizard[] = {
	0x30, 0x58, 0xAC, 0xD6, 0xAC, 0xE5, 0xD9, 0xB9, // frame 0
	0x6E, 0x56, 0x6C, 0x54, 0x7C, 0x54, 0x7C, 0x28,
	0x00, 0x30, 0x58, 0xAC, 0xD6, 0xAC, 0xE5, 0xF9, // frame 1
	0xED, 0x56, 0x6E, 0x54, 0x7C, 0x54, 0x7C, 0x14,
	0x30, 0x58, 0xAC, 0xD6, 0xAC, 0xE4, 0xD9, 0xB9, // frame 2
	0x6F, 0x56, 0x6C, 0x54, 0x7C, 0x54, 0x7C, 0x48
};

const uint8_t PROGMEM wizard_mask[] = {
	0x30, 0x78, 0xFC, 0xFE, 0xFC, 0xFD, 0xD9, 0xB9, // frame 0
	0x7E, 0x7E, 0x7C, 0x7C, 0x7C, 0x7C, 0x7C, 0x28,
	0x00, 0x30, 0x78, 0xFC, 0xFE, 0xFC, 0xFD, 0xF9, // frame 1
	0xFD, 0x7E, 0x7E, 0x7C, 0x7C, 0x7C, 0x7C, 0x14,
	0x30, 0x78, 0xFC, 0xFE, 0xFC, 0xFC, 0xD9, 0xB9, // frame 2
	0x7F, 0x7E, 0x7C, 0x7C, 0x7C, 0x7C, 0x7C, 0x48,
};

const uint8_t jump[] = {
	0, 11, 18, 23, 24
};

/* scroll_x: between 0 and 8 (inclusive)
   0 means, that the first tile is fully visible on the left side of the screen
   8 means, that the last tile is fully visible on the right side of the screen
   values in between mean, that parts of first and last row are visible */
void draw_bg(uint8_t scroll_x) {
	for(uint8_t y = 0; y < 12; y++) {
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

void draw_object(const uint8_t *object, const uint8_t *mask,
	const uint8_t number_frames, uint8_t xoff, uint8_t y,
	uint8_t width, uint8_t height) {
	uint8_t offset = (frame_counter % number_frames)*width*height*8;
	uint8_t ymax = y + height*8;
	for(; y < ymax; y++) {
		uint8_t nextline, nextmaskline, line = 0, maskline = 0;
		for(uint8_t x = 0; x < width; x++) {
			nextline = pgm_read_byte(&object[offset]);
			nextmaskline = pgm_read_byte(&mask[offset]);
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
			offset++;
		}
		if(xoff % 8 != 0) {
			TV.screen[(y*16)+xoff/8+width] = (TV.screen[(y*16)+xoff/8+width] & ~maskline) | line;
		}
	}
}

void shiftleft_bg_tiles(const uint8_t *nextcolumn) {
	for(uint8_t x = 0; x < 16; x++) {
		for(uint8_t y = 0; y < 12; y++) {
			bg_tiles[y*17+x] = bg_tiles[y*17+x+1];
		}
	}
	for(uint8_t y = 0; y < 12; y++) {
		bg_tiles[y*17+16] = pgm_read_byte(nextcolumn++);
	}
}

uint8_t level_scroll_x;
const uint8_t *level_scroll_data;
uint8_t row_counter;

void load_level(const uint8_t *level) {
	level_scroll_data = level;
	level_scroll_x = 0;
	row_counter = 0;
	for(uint8_t x = 0; x <= 16; x++) {
		for(uint8_t y = 0; y < 12; y++) {
			bg_tiles[y*17+x] = pgm_read_byte(level_scroll_data++);
		}
	}
}

void scroll_level(uint8_t amount) {
	level_scroll_x += amount;
	while(level_scroll_x >= 8) {
		level_scroll_x -= 8;
		shiftleft_bg_tiles(level_scroll_data);
		level_scroll_data += 12;
		row_counter++;
	}
}

uint8_t player_xpos;

#define FLAG_COUNT_FRAMES 1
#define FLAG_WAIT_BUTTONUP 2
uint8_t flags = 0;
uint8_t player_jump = 0, player_extra_y;

int main() {
	sound_init();
	TV.begin(_PAL, 128, 96);
	TV.set_hbi_hook(&vblank_sound);
	DDRB |= (1 << PB4); // button input
	PORTB |= (1 << PB4);

	_delay_ms(200);
	//TV.draw_rect(0, 0, 127, 95, 1, -1);
	//TV.select_font(font4x6);
	//TV.print(2, 2, "Hello World!");

	//start_logo(); //TODO uncomment this
	load_level(level_tiles);
	frame_counter = 0;
	flags |= FLAG_COUNT_FRAMES;
	while(1) {
		if(!(flags & FLAG_WAIT_BUTTONUP) && player_jump == 0 && !(PINB & (1 << PB4))) {
			player_jump = 9;
			flags |= FLAG_WAIT_BUTTONUP;
		}
		if((flags & FLAG_WAIT_BUTTONUP) && (PINB & (1 << PB4))) {
			flags &= ~FLAG_WAIT_BUTTONUP;
		}
		if(player_jump > 4) {
			player_extra_y = jump[9-player_jump];
			player_jump--;
		} else if(player_jump > 0) {
			player_extra_y = jump[player_jump];
			player_jump--;
		} else {
			player_extra_y = 0;
		}
		draw_bg(level_scroll_x);
		if(row_counter < sizeof(level_tiles)/12-16) {
			scroll_level(1);
			player_xpos = 16;
		} else if(player_xpos < 128-28) {
			player_xpos++;
		} else {
			flags &= ~FLAG_COUNT_FRAMES;
			frame_counter = 0;
		}
		draw_object(wizard, wizard_mask, sizeof(wizard)/16, player_xpos, 96-16-16-player_extra_y, 1, 2);
		if(flags & FLAG_COUNT_FRAMES) {
			frame_counter++;
		}
		_delay_ms(15);
		// do nothing;
	}

	return 0;
}