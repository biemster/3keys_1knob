# 3keys_1knob
Custom firmware for a 3-key + rotary encoder macropad (https://hackaday.io/project/189914)

### compile:
`$ make bin`

### compile & flash to pad:
- if on original firmware:
    - open the keypad and locate the 16-pin IC next to two of the key mounts
    - connect P1.5 (pin 3) to GND (pin 14) and connect USB
    - if this doesn't work, try the CH552 default of connecting 3.6 (pin 12) to 3v3 (pin 16)
- if on this firmware: press key1 while connecting USB
- `$ make flash`

### configure keys:
1. dump the current build: `$ isp55e0 --data-dump flashdata.bin`
2. edit the first 6 bytes of this binary to the [key codes](./include/usb_conkbd.h) you want to use for:
    - each of the three keys,
    - the knob switch,
    - turning the knob clockwise, then
    - turning the knob counterclockwise
3. write it back: `$ isp55e0 --data-flash flashdata.bin`
