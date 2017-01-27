#include <avr/pgmspace.h>
#include "gfx.h"

#include "levels/leveldata.cpp"

uint8_t level_scroll_x;
const uint8_t *level_scroll_data;
uint8_t row_counter;

extern uint8_t player_xpos, player_ypos;

void load_level(uint8_t level) {
	level_scroll_data = level_tiles[level];
	level_scroll_x = 0;
	row_counter = 0;
	for(uint8_t x = 0; x <= 16; x++) {
		for(uint8_t y = 0; y < 12; y++) {
			bg_tiles[y*17+x] = pgm_read_byte(level_scroll_data++);
		}
	}
	player_xpos = 16;
	player_ypos = 96;
	for(int8_t y = 11; y >= 0; y--) {
		player_ypos = y*8+8;
		if(!(bg_tiles[y*17+2] & 0x20)) {
			break;
		}
	}
}

void scroll_level(uint8_t amount) {
	level_scroll_x += amount;
	while(level_scroll_x >= 8) {
		level_scroll_x -= 8;
		shiftleft_bg_tiles(&level_scroll_data);
		row_counter++;
	}
}