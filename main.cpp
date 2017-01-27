#include <TVout.h>
#include <font8x8.h>
#include <font4x6.h>
#include <avr/pgmspace.h>
#include "bootlogo.h"
#include "sound.h"
#include "gfx.h"
#include "levels.h"
#include "levels/leveldata.h"

TVout TV;

unsigned long nextmillis = 0;

const uint8_t jump[] = {
	1, 2, 4, 7, 7, 7
};

#define FLAG_COUNT_FRAMES 1
#define FLAG_WAIT_BUTTONUP 2
#define FLAG_PC_FALLING 4
#define FLAG_INPUT_SEQUENCE 8
#define FLAG_LEVEL_SCROLLS 16
#define FLAG_GAME_PAUSED 32
uint8_t flags = 0;
uint8_t player_jump = 0, player_xpos, player_ypos, player_fall = 0;

#define TOP_COLLISION 1
#define RIGHT_COLLISION 2
#define BOTTOM_COLLISION 4

uint8_t bg_collisions() {
	// which bg-tile is possibly colliding with the character?
	// player_ypos = baseline of character (pixels from top)
	// 16 = height of character (in pixels)
	// player_xpos = left position of character (pixels from left)
	// 8 = width of character (in pixels)
	// level_scroll_x = amount of pixels scrolled to the left (i.e. 8-level_scroll_x pixels of the first column are visible)

	//  v- first column (scrolled)
	// ----+------++------++------+
	//     |      ||      ||      |
	//     |      ||      ||      |
	//     |      ||      ||      |
	// ----+------++------++------+
	//     |      || v- player character
	//     |   +------+   ||      |
	//     |   |      |   ||      |
	// ----+---|      |---++------+

	// The player can be positioned in either one or two (like in the picture) columns
	// collisions can occur
	//  - while jumping (to the ceiling) -> collides with one / two of the tiles which are upmost covering the player
	//  - while running (to the right) -> collides with 1-3 of the tiles covering the player on the right side
	//  - while falling (to the bottom)
	//    -> collides with one / two of the tiles which are most bottom covering the player
	//    -> collides with the ground (line 96)
	uint8_t collisions = 0;
	uint8_t coll_tile = (player_ypos/8-3)*17+(player_xpos+level_scroll_x)/8;
	uint8_t top_collision = 0;
	uint8_t multitile = (player_xpos+level_scroll_x) % 8 ? 1 : 0;
	if(player_ypos % 8 == 1) {
		top_collision |= bg_tiles[coll_tile];
	}
	coll_tile += 17;
	if(top_collision & 0x20) {
		collisions |= TOP_COLLISION;
	}

	uint8_t right_collision = 0;
	if((player_xpos+level_scroll_x)%8 == 0) {
		right_collision |= bg_tiles[coll_tile+1+multitile] | bg_tiles[coll_tile+17+1+multitile];
		if(player_ypos % 8) {
			right_collision |= bg_tiles[coll_tile+2*17+1+multitile];
		}
	}
	if(right_collision & 0x20) {
		collisions |= RIGHT_COLLISION;
	}

	coll_tile += 2*17;
	if(player_ypos % 8) {
		coll_tile += 17;
	}
	uint8_t bottom_collision = 0;
	if(player_ypos % 8 == 0) {
		bottom_collision |= bg_tiles[coll_tile];
		if(multitile) {
			bottom_collision |= bg_tiles[coll_tile+1];
		}
		if(bottom_collision & 0x20) {
			collisions |= BOTTOM_COLLISION;
		}
	}
	return collisions;
}

void start_screen() {
	load_level(0);
	bg_tiles[8*17+4] = 8; // 4,8: tile[8]
	draw_bg(0);
	draw_sprite(4, 2*8, GFXDATA_LOGO, 9, 5*8); // 4,2: logo
	TV.print(6*8, 8*8, "START"); // 6,8: Text "START"
	while(PINB & (1 << PB4)) {
		nextmillis = TV.millis() + 15;
		while(TV.millis() < nextmillis) {
			// busy wait loop
		}
	}
	bg_tiles[8*17+4] = 12;
	draw_bg(0);
	draw_sprite(4, 2*8, GFXDATA_LOGO, 9, 5*8); // 4,2: logo
	TV.print(6*8, 8*8, "START"); // 6,8: Text "START"
	while(!(PINB & (1 << PB4))) {
		nextmillis = TV.millis() + 15;
		while(TV.millis() < nextmillis) {
			// busy wait loop
		}
	}
	nextmillis = TV.millis() + 15;
	while(TV.millis() < nextmillis) {
		// busy wait loop
	}
}

uint8_t current_level;

#define START_LEVEL 0
#define FINAL_LEVEL 1

int main() {
	sound_init();
	TV.begin(_PAL, 128, 96);
	TV.set_hbi_hook(&vblank_sound);
	DDRB |= (1 << PB4); // button input
	PORTB |= (1 << PB4);

	nextmillis = TV.millis() + 1000;
	while(TV.millis() < nextmillis) {
		// busy wait loop
	}
	boot_logo();
	unsigned long lastpress = 0;
	uint8_t store_flags = 0, input_seq = 0;
	uint8_t shoot = 0;
	uint8_t dragon_phallus_left;
	uint8_t player_lives;
	uint16_t score;
	while(1) {
		TV.select_font(font8x8);
		dragon_phallus_left = 4;
		player_lives = 6;
		current_level = START_LEVEL;
		score = 0;
		start_screen();
		TV.select_font(font4x6);
		while(1) {
			load_level(current_level);
			frame_counter = 0;
			uint8_t spiders_in_level[3];
			int8_t active_spider_positions[3], active_spider_direction[3];
			for(uint8_t i = 0; i < 3; i++) {
				spiders_in_level[i] = spiders_per_level[current_level][i];
				active_spider_positions[i] = 0;
			}
			flags |= FLAG_COUNT_FRAMES;
			while(1) {
				if(!(flags & (FLAG_INPUT_SEQUENCE)) && !(PINB & (1 << PB4))) {
					// input sequence started
					store_flags = flags;
					flags = FLAG_INPUT_SEQUENCE | FLAG_GAME_PAUSED;
				}
				if(flags & FLAG_INPUT_SEQUENCE) {
					if(flags & FLAG_WAIT_BUTTONUP && (PINB & (1 << PB4))) {
						input_seq <<= 2;
						if(TV.millis() - lastpress < 200) {
							input_seq |= 1;
						} else {
							input_seq |= 2;
						}
						flags &= ~FLAG_WAIT_BUTTONUP;
						lastpress = TV.millis();
					} else if(!(flags & FLAG_WAIT_BUTTONUP)) {
						if(PINB & (1 << PB4)) {
							// button up... check for how long
							if(TV.millis() - lastpress > 150 || input_seq & 0xC0) {
								flags = store_flags;
								if(input_seq == 0x01) {
									if(!(flags & FLAG_PC_FALLING) && player_jump == 0) {
										player_jump = 6;
										play_note_ch1(34, 255, 100, -10, -2);
									}
								} else if(input_seq == 0x09) {
									// shoot!!
									shoot = 1;
									play_note_ch1(37, 255, 100, -20, 4);
								}
								input_seq = 0;
							}
						} else {
							lastpress = TV.millis();
							flags |= FLAG_WAIT_BUTTONUP;
						}
					}
				}
				if(!(flags & FLAG_GAME_PAUSED)) {
					uint8_t collisions;
					if(player_jump > 0) {
						player_ypos -= jump[--player_jump];
						collisions = bg_collisions();
						// if collision (on head), set player_jump to 0 and player_ypos accordingly. else:
						if(collisions & TOP_COLLISION) {
							player_jump = 0;
							player_ypos = ((player_ypos/8)+1)*8;
							// TODO: sound (ughh, hit a ceiling)
						}
					} else {
						collisions = bg_collisions();
					}
					if(collisions & RIGHT_COLLISION) {
						flags &= ~FLAG_LEVEL_SCROLLS;
					} else {
						flags |= FLAG_LEVEL_SCROLLS;
					}
					// check whether there is a solid tile under the character... if not, player has to fall and can not jump while this
					if(!player_jump) {
						if(!(collisions & BOTTOM_COLLISION)) {
							flags |= FLAG_PC_FALLING;
							player_ypos += jump[player_fall];
							if(player_fall < 5) {
								player_fall++;
							}
							if(bg_tiles[(player_ypos/8-1)*17+(player_xpos+level_scroll_x)/8] & 0x20 || ((player_xpos+level_scroll_x) % 8 && bg_tiles[(player_ypos/8-1)*17+(player_xpos+level_scroll_x)/8+1] & 0x20)) {
								player_ypos -= (player_ypos%8)+8;
							}
							collisions = bg_collisions();
						}
						if(collisions & BOTTOM_COLLISION) {
							flags &= ~FLAG_PC_FALLING;
							player_fall = 0;
						}
					}
					if(player_ypos > 96) {
						player_ypos = 16;
					}
					// if there is a collision with an enemy
						// check whether the enemy is right under the character. if so, then hurt the enemy and let the player jump maybe
						// else hurt the player
					if(flags & FLAG_LEVEL_SCROLLS && level_scroll_x == 0) {
						for(uint8_t i = 0; i < 3; i++) {
							if(spiders_in_level[i] > 0)
								spiders_in_level[i]--;
						}
					}
					for(uint8_t i = 0; i < 3; i++) {
						if(active_spider_positions[i] == 0 && spiders_in_level[i] <= 15 && spiders_in_level[i] > 0) {
							active_spider_positions[i] = spiders_in_level[i]*8;
							active_spider_direction[i] = -1;
						}
						if(active_spider_positions[i] >= player_xpos && active_spider_positions[i] <= player_xpos + 7 && player_ypos >= 76 && player_ypos < 80+16) {
							/*if(flags & FLAG_PC_FALLING) {
								active_spider_positions[i] = 0; // spider dead (sound)
								spiders_in_level[i] = 0;
							} else {*/
							// game over!
							if(player_lives > 0) {
								player_lives--;
								play_note_ch1(20, 255, 100, -40, 8);
							}
							/*current_level = -1;
							break;*/
							//}
						}
					}
					// end boss damage?
					/*if(current_level == 255) {
						break;
					}*/
				}
				draw_bg(level_scroll_x);
				uint8_t draw_input_seq = input_seq, draw_pos = 4;
				while(draw_input_seq != 0) {
					switch(draw_input_seq & (0xC0)) {
						case 0x40:
							draw_sprite(draw_pos++, 24, GFXDATA_FILLED_DOT, 1, 8);
							break;
						case 0x80:
							draw_sprite(draw_pos++, 24, GFXDATA_FILLED_LONG_A, 1, 8);
							draw_sprite(draw_pos++, 24, GFXDATA_FILLED_LONG_B, 1, 8);
							break;
						default:
							// nothing
							break;
					}
					draw_input_seq <<= 2;
				}
				if(flags & FLAG_GAME_PAUSED) {
					for(uint8_t i = 0; i < 3; i++) {
						if(active_spider_positions[i] != 0) {
							draw_object(GFXDATA_SPIDER, GFXDATA_NULL, 2, active_spider_positions[i], 76, 1, 4);
						}
					}					
				} else {
					for(uint8_t i = 0; i < 3; i++) {
						if(active_spider_positions[i] != 0) {
							int8_t spider_walk = active_spider_direction[i] - (flags & FLAG_LEVEL_SCROLLS ? 1 : 0);
							if(active_spider_positions[i] + spider_walk <= 0) {
								active_spider_positions[i] = 0;
								spiders_in_level[i] = 0;
							} else {
								active_spider_positions[i] += spider_walk;
								if(bg_tiles[9*17+(active_spider_positions[i]+level_scroll_x)/8+(active_spider_direction[i]+1)/2] & 0x20) {
									active_spider_positions[i] -= spider_walk;
									active_spider_direction[i] = -active_spider_direction[i];
								}
								draw_object(GFXDATA_SPIDER, GFXDATA_NULL, 2, active_spider_positions[i], 76, 1, 4);
								if(shoot) {
									shoot--;
									TV.draw_line(player_xpos + 7, player_ypos - 10, active_spider_positions[i] + 1, 78, 1); // pew
									active_spider_positions[i] = 0;
									spiders_in_level[i] = 0;
									score += 100;
								}
							}
						}
					}
					if(row_counter < level_widths[current_level]-16) {
						if(flags & FLAG_LEVEL_SCROLLS) {
							scroll_level(1);
						}
						player_xpos = 16;
					} else if(player_xpos < 128-28) {
						if(current_level == FINAL_LEVEL) {
							if(!dragon_phallus_left || player_xpos < (10-dragon_phallus_left)*8) {
								player_xpos++;
							} else {
								// hurt! sound
								if(player_lives > 0) {
									player_lives--;
									play_note_ch1(20, 255, 100, -40, 8);
								}
							}
						} else {
							player_xpos++;
						}
					} else {
						flags &= ~FLAG_COUNT_FRAMES;
						frame_counter = 0;
						break;
					}
				}
				if(current_level == FINAL_LEVEL && row_counter >= 65-16) {
					if(dragon_phallus_left) {
						uint8_t dragon_x = 128-(row_counter-65+16)*8-level_scroll_x;
						if(shoot > 0) {
							TV.draw_line(player_xpos + 7, player_ypos - 10, dragon_x-(dragon_phallus_left-1)*8, 73, 1);
							shoot--;
							score += 1000;
							dragon_phallus_left--;
						}
						if(dragon_phallus_left > 1 && dragon_x-(dragon_phallus_left-1)*8 < 128) {
							draw_object(GFXDATA_DRAGON_PHALLUS, GFXDATA_NULL, 1, dragon_x-(dragon_phallus_left-1)*8, 9*8, 3, 4);
						}
						draw_object(GFXDATA_DRAGON, GFXDATA_NULL, 2, dragon_x, 6*8, 3, 4*8);
						if(dragon_phallus_left > 1) {
							draw_object(GFXDATA_DRAGON_PHALLUS_START, GFXDATA_NULL, 1, dragon_x, 9*8, 1, 4);
						}
					}
					if(!dragon_phallus_left && row_counter >= 72-16) {
						// won!
						bg_tiles[13+17*8] = 0x2E;
						bg_tiles[14+17*8] = 0x3E;
						bg_tiles[13+17*9] = 0x2F;
						bg_tiles[14+17*9] = 0x3F;
					}
				}
				draw_object(GFXDATA_WIZARD, GFXDATA_WIZARD_MASK, 3, player_xpos, player_ypos-16, 1, 16);
				if(flags & FLAG_COUNT_FRAMES) {
					frame_counter++;
				}
				if(player_lives == 0) {
					current_level = -1;
					break;
				}
				while(shoot) {
					TV.draw_line(player_xpos + 7, player_ypos - 10, rand() % 128, rand() % 96, 1); // pew
					shoot--;
				}
				for(uint8_t i = 0; i < 16*8; i++) {
					TV.screen[i] = 0;
				}
				TV.print(0, 0, "Score:");
				TV.print(7*4, 0, score);
				TV.print(128-(8*4+3*8), 0, "Health:");
				uint8_t draw_lives = player_lives;
				for(uint8_t i = 0; i < 3; i++) {
					uint8_t max_lives = draw_lives;
					if(max_lives > 2) max_lives = 2;
					draw_sprite(16-(3-i), 0, GFXDATA_LIVES_0 + max_lives, 1, 8);
					draw_lives -= max_lives;
				}
				nextmillis += 50;
				while(TV.millis() < nextmillis) {
					// busy wait loop
				}
			}
			// level completed melody
			score += 5000;
			play_note_ch1(0, 255, 100, -20, 0);
			nextmillis += 100;
			while(TV.millis() < nextmillis) {
				// busy wait loop
			}
			play_note_ch1(4, 255, 100, -20, 0);
			nextmillis += 100;
			while(TV.millis() < nextmillis) {
				// busy wait loop
			}
			play_note_ch1(7, 255, 100, -20, 0);
			nextmillis += 100;
			while(TV.millis() < nextmillis) {
				// busy wait loop
			}
			play_note_ch1(12, 255, 100, -20, 0);
			nextmillis += 700;
			while(TV.millis() < nextmillis) {
				// busy wait loop
			}
			if(current_level < FINAL_LEVEL) {
				current_level++;
			} else {
				break;
			}
		}
	}

	return 0;
}