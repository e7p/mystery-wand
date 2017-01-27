#include <TVout.h>
#include <avr/pgmspace.h>
#include "sound.h"

extern TVout TV;
extern unsigned long nextmillis;

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

void boot_logo() {
	uint8_t frame = 0;
	do {
		if(TV.millis() > nextmillis) {
			nextmillis = TV.millis() + 30;
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