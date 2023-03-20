#ifndef USER_USB_RAM
#error "This example needs to be compiled with out_a USER USB setting"
#error "Tools--> USB Settings--> USER CODE w/266 USB Ram"
#endif

#include "src/userUsbHidKeyboard/USBHIDKeyboard.h"
#include <WS2812.h>

// 3keys
uint8_t BUTTON_LEFT = 11;
uint8_t BUTTON_RIGHT = 16;
uint8_t BUTTON_MIDDLE = 17;
uint8_t state_left_prev = 0;
uint8_t state_middle_prev = 0;
uint8_t state_right_prev = 0;

// 1knob
uint8_t OUTA = 30;
uint8_t OUTB = 31;
uint8_t SW = 33;
uint8_t sw_prev = 0;
enum {RSTAT_SETTLED=0, RSTAT_A, RSTAT_AB, RSTAT_B, RSTAT_FULL};
uint8_t rotary_stat_prev = RSTAT_SETTLED;
uint8_t rotary_stat_seen[4] = {0};

// RGB
uint8_t RGB = 34;
#define NUM_LEDS 3
#define COLOR_PER_LEDS 3
#define NUM_BYTES (NUM_LEDS*COLOR_PER_LEDS)
__xdata uint8_t ledData[NUM_BYTES];


void jump_to_bootloader() { // TODO: doesn't seem to work
	USB_CTRL = 0;
	EA = 0; //Disabling all interrupts is required.
	delayMicroseconds(50000);
	delayMicroseconds(50000);
	__asm__ ("lcall #0x3800");  //Jump to bootloader code
	while(1);
}


void setup() {
	pinMode(BUTTON_LEFT, INPUT_PULLUP);
	pinMode(BUTTON_RIGHT, INPUT_PULLUP);
	pinMode(BUTTON_MIDDLE, INPUT_PULLUP);
	pinMode(OUTA, INPUT_PULLUP);
	pinMode(OUTB, INPUT_PULLUP);
	pinMode(SW, INPUT_PULLUP);
	pinMode(RGB, OUTPUT);

	USBInit();
}

void loop() {
	uint8_t state_left = digitalRead(BUTTON_LEFT) ? 0 : 1;
	uint8_t state_middle = digitalRead(BUTTON_MIDDLE) ? 0 : 1;
	uint8_t state_right = digitalRead(BUTTON_RIGHT) ? 0 : 1;

	/////////////////////////////////////////
	///
	/// 3Keys
	///
	if(state_left != state_left_prev) {
		state_left_prev = state_left;
		if(state_left) {
			Keyboard_press('1');
		}
		else {
			Keyboard_release('1');
		}
	}

	if(state_middle != state_middle_prev) {
		state_middle_prev = state_middle;
		if(state_middle) {
			Keyboard_press('2');
		}
		else {
			Keyboard_release('2');
		}
	}

	if(state_right != state_right_prev) {
		state_right_prev = state_right;
		if(state_right) {
			Keyboard_press('3');
		}
		else {
			Keyboard_release('3');
		}
	}

	/////////////////////////////////////////
	///
	/// 1Knob
	///
	uint8_t out_a = digitalRead(OUTA) ? 0 : 1;
	uint8_t out_b = digitalRead(OUTB) ? 0 : 1;
	uint8_t sw = digitalRead(SW) ? 0 : 1;
	uint8_t rotary_status = (out_a && out_b) ? RSTAT_AB:
									   out_a ? RSTAT_A:
									   out_b ? RSTAT_B:
							rotary_stat_prev ? RSTAT_FULL:
											   RSTAT_SETTLED;

	if(out_a && out_b) {
		rotary_stat_seen[RSTAT_AB] = 1;
	}
	else if(out_a) {
		rotary_stat_seen[RSTAT_A] = 1;
	}
	else if(out_b) {
		rotary_stat_seen[RSTAT_B] = 1;
	}

	if(sw_prev != sw) {
		sw_prev = sw;
		if(sw) {
			Keyboard_press('s');
		}
		else {
			Keyboard_release('s');
		}
	}

	if(rotary_status == RSTAT_FULL && rotary_stat_prev == RSTAT_A && rotary_stat_seen[RSTAT_B] && rotary_stat_seen[RSTAT_AB]) {
		// Rotary encoder finished clockwise move
		Keyboard_press('r');
		Keyboard_release('r');
		memset(rotary_stat_seen, 0, sizeof(rotary_stat_seen));
		rotary_status = RSTAT_SETTLED;
	}
	else if(rotary_status == RSTAT_FULL && rotary_stat_prev == RSTAT_B && rotary_stat_seen[RSTAT_A] && rotary_stat_seen[RSTAT_AB]) {
		// Rotary encoder finished counter clockwise move
		Keyboard_press('l');
		Keyboard_release('l');
		memset(rotary_stat_seen, 0, sizeof(rotary_stat_seen));
		rotary_status = RSTAT_SETTLED;
	}
	else if(rotary_status == RSTAT_FULL) {
		// Rotary encoder got back to starting position
		memset(rotary_stat_seen, 0, sizeof(rotary_stat_seen));
		rotary_status = RSTAT_SETTLED;
	}
	rotary_stat_prev = rotary_status;

	/////////////////////////////////////////
	///
	/// RGB
	///
	uint8_t ledData_prev[NUM_BYTES] = {0};
	memcpy(ledData_prev, ledData, NUM_BYTES);
	if(state_left) {
		set_pixel_for_GRB_LED(ledData, 0, 255, 0, 0);
	}
	else {
		set_pixel_for_GRB_LED(ledData, 0, 0, 255, 0);
	}

	if(state_middle) {
		set_pixel_for_GRB_LED(ledData, 1, 255, 0, 0);
	}
	else {
		set_pixel_for_GRB_LED(ledData, 1, 0, 255, 0);
	}

	if(state_right) {
		set_pixel_for_GRB_LED(ledData, 2, 255, 0, 0);
	}
	else {
		set_pixel_for_GRB_LED(ledData, 2, 0, 255, 0);
	}

	if(memcmp(ledData, ledData_prev, NUM_BYTES)) {
		neopixel_show_P3_4(ledData, NUM_BYTES);
	}
}
