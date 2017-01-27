#include <avr/pgmspace.h>

#include "level0.cpp"
#include "level1.cpp"

const uint8_t *level_tiles[] = {tutorial_level, castle_level};
uint8_t level_widths[] = {72, 72};
uint8_t spiders_per_level[][3] = {{54}, {9, 36}};