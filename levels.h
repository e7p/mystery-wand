#include <avr/pgmspace.h>

extern uint8_t level_scroll_x;
extern const uint8_t *level_scroll_data;
extern uint8_t row_counter;

void load_level(uint8_t level);
void scroll_level(uint8_t amount);