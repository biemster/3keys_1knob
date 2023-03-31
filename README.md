# 3keys_1knob
Custom firmware for a 3-key + rotary encoder macropad (https://hackaday.io/project/189914)

### compile:
`$ make bin`

### compile & flash to pad:
- if on original firmware: connect P1.5 to GND and connect USB
- if on this firmware: press key1 while connecting USB
- `$ make flash`

### configure keys:
1. `$ isp55e0 --data-dump flashdata.bin`
2. edit first 6 bytes of this binary (3 keys, plus 3 for the knob), and write it back:
3. `$ isp55e0 --data-flash flashdata.bin`
