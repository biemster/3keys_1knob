// ===================================================================================
// Project:   MacroPad Mini for CH551, CH552 and CH554
// Version:   v1.1
// Year:      2023
// Author:    Stefan Wagner
// Github:    https://github.com/wagiminator
// EasyEDA:   https://easyeda.com/wagiminator
// License:   http://creativecommons.org/licenses/by-sa/3.0/
// ===================================================================================
//
// Description:
// ------------
// Firmware example implementation for the MacroPad Mini.
//
// References:
// -----------
// - Blinkinlabs: https://github.com/Blinkinlabs/ch554_sdcc
// - Deqing Sun: https://github.com/DeqingSun/ch55xduino
// - Ralph Doncaster: https://github.com/nerdralph/ch554_sdcc
// - WCH Nanjing Qinheng Microelectronics: http://wch.cn
//
// Compilation Instructions:
// -------------------------
// - Chip:  CH551, CH552 or CH554
// - Clock: min. 12 MHz internal
// - Adjust the firmware parameters in include/config.h if necessary.
// - Make sure SDCC toolchain and Python3 with PyUSB is installed.
// - Press BOOT button on the board and keep it pressed while connecting it via USB
//   with your PC.
// - Run 'make flash'.
//
// Operating Instructions:
// -----------------------
// - Connect the board via USB to your PC. It should be detected as a HID keyboard.
// - Press a macro key and see what happens.
// - To enter bootloader hold down key 1 while connecting the MacroPad to USB. All
//   NeoPixels will light up white as long as the device is in bootloader mode 
//   (about 10 seconds).


// ===================================================================================
// Libraries, Definitions and Macros
// ===================================================================================

// Libraries
#include <config.h>                         // user configurations
#include <system.h>                         // system functions
#include <delay.h>                          // delay functions
#include <neo.h>                            // NeoPixel functions
#include <usb_conkbd.h>                     // USB HID consumer keyboard functions

// Prototypes for used interrupts
void USB_interrupt(void);
void USB_ISR(void) __interrupt(INT_NO_USB) {
  USB_interrupt();
}

// structur with key details
struct key {
  char code;
  uint8_t last;
};

// ===================================================================================
// NeoPixel Functions
// ===================================================================================

// NeoPixel variables
__idata uint8_t neo1 = NEO_MAX;             // brightness of NeoPixel 1
__idata uint8_t neo2 = NEO_MAX;             // brightness of NeoPixel 2
__idata uint8_t neo3 = NEO_MAX;             // brightness of NeoPixel 3

// Update NeoPixels
void NEO_update(void) {
  EA = 0;                                   // disable interrupts
  NEO_writeColor(neo1, 0, 0);               // NeoPixel 1 lights up red
  NEO_writeColor(0, neo2, 0);               // NeoPixel 2 lights up green
  NEO_writeColor(0, 0, neo3);               // NeoPixel 3 lights up blue
  EA = 1;                                   // enable interrupts
}

// Read EEPROM (stolen from https://github.com/DeqingSun/ch55xduino/blob/ch55xduino/ch55xduino/ch55x/cores/ch55xduino/eeprom.c)
uint8_t eeprom_read_byte (uint8_t addr){
  ROM_ADDR_H = DATA_FLASH_ADDR >> 8;
  ROM_ADDR_L = addr << 1; //Addr must be even
  ROM_CTRL = ROM_CMD_READ;
  return ROM_DATA_L;
}

// handle key press
void handle_key(uint8_t current, struct key * key, uint8_t * neo) {
  if(current != key->last) {                // state changed?
    key->last = current;                    // update last state flag
    if(current) {                           // key was pressed?
      *neo = NEO_MAX;                       // light up corresponding NeoPixel
      NEO_update();                         // update NeoPixels NOW!
      KBD_press(key->code);                 // press
    }
    else {                                  // key was released?
      KBD_release(key->code);               // release
    }
  }
  else if(key->last) {                      // key still being pressed?
    *neo = NEO_MAX;                         // keep NeoPixel on
  }
}

// ===================================================================================
// Main Function
// ===================================================================================
void main(void) {
  // Variables
  struct key
      key1 = { .last = 0 },                 // struct for key 1
      key2 = { .last = 0 },                 // struct for key 2
      key3 = { .last = 0 };                 // struct for key 3
  uint8_t knobswitchlast = 0;               // last state of knob switch
  __idata uint8_t i;                        // temp variable
  uint8_t currentKnobKey;                   // current key to be sent by knob

  // Enter bootloader if key 1 is pressed
  NEO_init();                               // init NeoPixels
  if(!PIN_read(PIN_KEY1)) {                 // key 1 pressed?
    NEO_latch();                            // make sure pixels are ready
    for(i=9; i; i--) NEO_sendByte(NEO_MAX); // light up all pixels
    BOOT_now();                             // enter bootloader
  }

  // Setup
  CLK_config();                             // configure system clock
  DLY_ms(5);                                // wait for clock to settle
  KBD_init();                               // init USB HID keyboard
  WDT_start();                              // start watchdog timer

  // TODO: Read eeprom for key characters
  key1.code = (char)eeprom_read_byte(0);
  key2.code = (char)eeprom_read_byte(1);
  key3.code = (char)eeprom_read_byte(2);
  char knobsw_char = (char)eeprom_read_byte(3);
  char knobclockwise_char = (char)eeprom_read_byte(4);
  char knobcounterclockwise_char = (char)eeprom_read_byte(5);

  // Loop
  while(1) {
    handle_key(!PIN_read(PIN_KEY1), &key1, &neo1);
    handle_key(!PIN_read(PIN_KEY2), &key2, &neo2);
    handle_key(!PIN_read(PIN_KEY3), &key3, &neo3);

    // Handle knob switch
    if(!PIN_read(PIN_ENC_SW) != knobswitchlast) {
      knobswitchlast = !knobswitchlast;
      if(knobswitchlast) {
        NEO_update();
        KBD_press(knobsw_char);
      }
      else {
        KBD_release(knobsw_char);
      }
    }
    else if(knobswitchlast) {
    }

    // Handle knob
    currentKnobKey = 0;                              // clear key variable
    if(!PIN_read(PIN_ENC_A)) {                       // encoder turned ?
      if(PIN_read(PIN_ENC_B)) {
        currentKnobKey = knobclockwise_char;         // clockwise?
      }
      else {
        currentKnobKey = knobcounterclockwise_char;  // counter-clockwise?
      }
      DLY_ms(10);                                    // debounce
      while(!PIN_read(PIN_ENC_A));                   // wait until next detent
    }

    if(currentKnobKey) {
      KBD_type(currentKnobKey);                      // press and release corresponding key ...
    }

    // Update NeoPixels
    NEO_update();
    if(neo1 > NEO_GLOW) neo1--;             // fade down NeoPixel
    if(neo2 > NEO_GLOW) neo2--;             // fade down NeoPixel
    if(neo3 > NEO_GLOW) neo3--;             // fade down NeoPixel
    DLY_ms(5);                              // latch and debounce
    WDT_reset();                            // reset watchdog
  }
}
