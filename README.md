# 3keys_1knob
Custom firmware for a 3-key + rotary encoder macropad (https://hackaday.io/project/189914)

### compile:
`$ make bin`

### compile & flash to pad:
- if on original firmware: connect P1.5 to GND and connect USB
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
