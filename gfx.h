#define GFXDATA_NULL                 0

#define GFXDATA_TILESET              1
#define GFXDATA_WIZARD               2
#define GFXDATA_WIZARD_MASK          3
#define GFXDATA_DRAGON               4
#define GFXDATA_SPIDER               5
#define GFXDATA_LOGO                 6
#define GFXDATA_FILLED_DOT           7
#define GFXDATA_FILLED_LONG_A        8
#define GFXDATA_FILLED_LONG_B        9
#define GFXDATA_DRAGON_PHALLUS       10
#define GFXDATA_DRAGON_PHALLUS_START 11
#define GFXDATA_LIVES_0              12
#define GFXDATA_LIVES_1              13
#define GFXDATA_LIVES_2              14

extern uint8_t bg_tiles[];
extern uint8_t frame_counter;

void draw_sprite(uint8_t x, uint8_t y, const uint8_t sprite, uint8_t width, uint8_t height);
void draw_bg(uint8_t scroll_x);
void draw_object(const uint8_t object, const uint8_t mask,
	const uint8_t number_frames, uint8_t xoff, uint8_t y,
	uint8_t width, uint8_t height);
void shiftleft_bg_tiles(const uint8_t **nextcolumn);