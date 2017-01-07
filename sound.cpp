/* AVR sound engine made by e7p - you are allowed to use and modify it under
 * the terms of GPLv3. For more information contact git@e7p.de */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <string.h>

#define C_FREQ(hz) (15000/(hz))

const uint8_t PROGMEM notes[] = { // starting at C4 (MIDI-Note 36)
	  C_FREQ(65.41),   C_FREQ(69.30),   C_FREQ(73.42),   C_FREQ(77.79), // C4
	  C_FREQ(82.41),   C_FREQ(87.31),   C_FREQ(92.50),   C_FREQ(98.00), // E4
	 C_FREQ(103.83),  C_FREQ(110.00),  C_FREQ(116.54),  C_FREQ(123.47), // Ab4
	 C_FREQ(130.81),  C_FREQ(138.59),  C_FREQ(146.83),  C_FREQ(155.56), // C5
	 C_FREQ(164.81),  C_FREQ(174.61),  C_FREQ(185.00),  C_FREQ(196.00), // E5
	 C_FREQ(207.65),  C_FREQ(220.00),  C_FREQ(233.08),  C_FREQ(246.94), // Ab5
	 C_FREQ(261.63),  C_FREQ(277.18),  C_FREQ(293.66),  C_FREQ(311.13), // C6
	 C_FREQ(329.63),  C_FREQ(349.23),  C_FREQ(369.99),  C_FREQ(392.00), // E6
	 C_FREQ(415.30),  C_FREQ(440.00),  C_FREQ(466.16),  C_FREQ(493.88), // Ab6
	 C_FREQ(523.25),  C_FREQ(554.37),  C_FREQ(587.33),  C_FREQ(622.25), // C7
	 C_FREQ(659.26),  C_FREQ(698.46),  C_FREQ(739.99),  C_FREQ(783.99), // E7
	 C_FREQ(830.61),  C_FREQ(880.00),  C_FREQ(932.33),  C_FREQ(987.77), // Ab7
};

volatile uint8_t volumeCh1 = 0;
volatile int8_t envCh1 = 0;
//volatile uint8_t volumeCh4 = 0;
//volatile int8_t envCh4 = 0;
volatile uint8_t phaseCh1 = 0;
volatile int8_t sweepCh1 = 0;
volatile uint8_t pulselengthCh1 = 0; // 58,8 Hz - 7500,0 Hz possible
volatile uint8_t lengthCh1 = 0;
//volatile uint8_t lengthCh4 = 0;
volatile uint8_t timePrescale = 0;

#if 0
volatile uint8_t noiseStatus = 0;
volatile uint8_t noisecount = 0;
volatile uint8_t noiseprescale = 0;

volatile uint8_t newNoiseStatus;
volatile uint8_t newLengthCh1;
volatile uint8_t newLengthCh4;
volatile uint8_t newVolumeCh1;
volatile uint8_t newVolumeCh4;
volatile uint8_t newPulselengthCh1;
volatile uint8_t newOCR2A;

void sound_loop() { // as often as you like to
	uint8_t noiseCarry = ((noiseStatus >> 7) ^ (noiseStatus >> 2) ^ 1) & 0x01;
	newNoiseStatus = (noiseStatus << 1) | noiseCarry;

	// TODO: make pulse widths better configurable
	uint8_t threshold = (uint16_t)volumeCh1 * (uint16_t)pulselengthCh1 / 2 / 255; //  pulselengthCh1 / 2; //
	uint8_t squareout;
	if(phaseCh1 > threshold){
		squareout = 1;
	} else {
		squareout = 0;
	}

	if((timePrescale & 0x3F) == 0) { // 234 Hz
		if(lengthCh1 > 0 && lengthCh1 < 255) { // if not 0 or 255 subtract one
			newLengthCh1 = lengthCh1 - 1;
		}
		if(lengthCh4 > 0 && lengthCh4 < 255) { // if not 0 or 255 subtract one
			newLengthCh4 = lengthCh4 - 1;
		}
	}
	if((timePrescale & 0x7F) == 0) { // 117 Hz

	}
	if(timePrescale == 0) { // 59 Hz
		if(lengthCh1 > 0) {
			newVolumeCh1 = volumeCh1 + envCh1;
		}
		if(lengthCh4 > 0) {
			newVolumeCh4 = volumeCh4 + envCh4;
		}
		newPulselengthCh1 = pulselengthCh1 + sweepCh1;
	}
	uint8_t mixCh1 = (squareout && lengthCh1 > 0) ? volumeCh1 : 0;
	uint8_t mixCh4 = ((noiseStatus & 1) && lengthCh4 > 0) ? volumeCh4 : 0;
	newOCR2A = (uint16_t)mixCh1 + (uint16_t)mixCh4 - ((uint16_t)mixCh1 * (uint16_t)mixCh4)/256;
}
#endif

// TODO: make vblank routine as long as necessary and possible, to enable more sound features...
// the problem was, that with an enhanced sound engine, the routine took too long so the picture
// was very distorted. Now there is a much more simple routine in use, but theoretically parts
// which are commented out on the upper part could be partly (maybe more efficiently, probably
// using inline assembler) inserted for more features.

void vblank_sound() { // 15 kHz and FAST (<2us)
	phaseCh1++;
	if(phaseCh1 > pulselengthCh1) {
		phaseCh1 = 0;
	}
	if(phaseCh1 > pulselengthCh1 / 2 && lengthCh1 > 0){
		OCR2A = volumeCh1;
	} else {
		OCR2A = 0;
	}
	timePrescale--;
	if((timePrescale & 0x3F) == 0) { // 234 Hz
		if(lengthCh1 > 0) { // if not 0 or 255 subtract one
			lengthCh1--;
		}
	}
	if(timePrescale == 0) { // 59 Hz
		if(lengthCh1 > 0) {
			volumeCh1 = volumeCh1 + envCh1;
		}
		pulselengthCh1 += sweepCh1;
	}
}

void sound_init() {
	TCCR2A = (1 << COM2A1) | (1 << COM2A0) | (1 << WGM21) | (1 << WGM20);
	TCCR2B = (1 << CS20);
	OCR2A = 0;

	DDRB |= (1 << PB3);
}

void play_note_ch1(uint8_t note, uint8_t volume, uint8_t length, uint8_t env, uint8_t sweep) {
	envCh1 = env;
	sweepCh1 = sweep;
	pulselengthCh1 = pgm_read_byte(&(notes[note]));
	lengthCh1 = length;
	volumeCh1 = volume;
}