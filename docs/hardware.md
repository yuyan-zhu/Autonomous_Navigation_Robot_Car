# Hardware Notes

## Parts List

| Component | Model / Spec | Qty | Notes |
|---|---|---|---|
| Microcontroller | Raspberry Pi Pico 2 (RP2350) | 1 | Headers soldered; powered over USB during development |
| Motor driver | L9110S dual-channel module | 1 | About 0.8 A continuous per channel |
| Motors | Yellow TT gear motor (DC, 2 wires) | 2 | No built-in encoder |
| Wheels | TT-compatible rubber wheels | 2 | Measured diameter 67.2 mm |
| Caster | Metal swivel caster | 1 | Rear support |
| Chassis | 2WD acrylic chassis plate | 1 | |
| Encoder discs | Black plastic code disc, 20 slots | 2 | Mounted on inner end of motor shafts |
| Encoder sensors | LM393 slot-type IR speed sensor (3.3-5 V, digital output D0) | 2 | Single-phase |
| Ultrasonic sensor | HC-SR04 (5 V) | 1 | ECHO needs a 1 kΩ / 2 kΩ voltage divider to 3.3 V |
| Servo | SG90 micro servo | 1 | Powered from the 5 V rail, not from Pico 3V3 |
| Battery holder (temporary) | Kit holder, ___ x AA | 1 | For early motor tests |
| Battery holder | 6 x AA with ON/OFF switch, wire leads | 1 | Main motor supply |
| Batteries | AA NiMH rechargeable, 1.2 V, 2000 mAh | 6 | About 7.2-8.4 V in series |
| Buck converter | LM2596 adjustable (4-40 V in, 2 A continuous), screw terminals, LED voltmeter | 1 | Set to 5.0 V output |
| Capacitors | Electrolytic (100 µF, ≥16 V) and ceramic (0.1 µF, "104") | - | Supply decoupling and motor noise |
| Diodes | 1N5819 Schottky | - | For future VSYS power path |
| Resistors | 1 kΩ, 2 kΩ | - | HC-SR04 ECHO divider |
| Prototyping | Breadboard, jumper wires (M-M, M-F, F-F) | - | |

### Tools

- Digital multimeter (continuity, DC voltage, resistance)
- Small screwdrivers (Phillips and flat)
- Tape measure, marker, masking tape

## L9110S Motor Driver

- Input header (header facing you, left to right): `B-1A, B-2A, GND, VCC, A-1A, A-1B`
  - Silkscreen `B-1A / B-2A` = B-IA / B-IB; `A-1A / A-1B` = A-IA / A-IB (inconsistent labeling by the manufacturer)
- Output terminals: MOTOR B on the left, MOTOR A on the right
- Control logic:

  | IA | IB | Result |
  |---|---|---|
  | PWM | 0 | Forward |
  | 0 | PWM | Reverse |
  | 0 | 0 | Coast |

- Photos: `img/l9110s_front.jpg`, `img/l9110s_back.jpg`

### Pin Assignment

| Driver pin | Pico pin | Function |
|---|---|---|
| A-1A (A-IA) | GP2 | Left motor PWM A |
| A-1B (A-IB) | GP3 | Left motor PWM B |
| B-1A (B-IA) | GP4 | Right motor PWM A |
| B-2A (B-IB) | GP5 | Right motor PWM B |
| GND | GND | Common ground |
| VCC | Battery + | Motor supply |

- Channel A (MOTOR A terminal) -> left motor; channel B (MOTOR B terminal) -> right motor
- Motor wire polarity in the terminals does not matter; direction is corrected with `MOTOR_x_INVERTED` in `config.h`

## Motors

- Output shaft passes through the gearbox: wheel on the outer end, code disc on the inner end

| | Left | Right |
|---|---|---|
| Winding resistance (min reading, lead resistance subtracted) | TBD | TBD |
| Estimated stall current at 7.2 V | TBD | TBD |

## Encoders

- Type: single-phase optical (slotted code disc + slot-type IR sensor)
  - Measures speed but not direction; direction is taken from the commanded motor direction
- Code disc: 20 slots (both discs counted), D-shaped center hole
- Expected PPR: 20 per wheel revolution (rising edge only), to be verified in step 6
  - Counting both edges would give 40 per revolution
- Wiring: VCC -> Pico 3V3, GND -> common ground, D0 -> GP6 (left) / GP7 (right)

## Power

- Pico: USB 5 V (development); buck converter 5 V is not connected to the Pico yet
- Motors: battery -> L9110S VCC
- 5 V rail (buck converter): servo, ultrasonic sensor, and later Pico VSYS through a 1N5819
- All grounds tied together (battery -, L9110S GND, Pico GND, buck converter GND, sensor GND)
- Battery open-circuit voltage: TBD

## Dimensions

| Quantity | Value | Method |
|---|---|---|
| Wheel circumference | 211 mm | Jumper wire wrapped around the tire, then measured (may overestimate; calibrate in week 3) |
| Wheel diameter | 67.2 mm | 211 / pi (nominal 65 mm) |
| Track width | 129 mm | Tire centerline to centerline (outer-to-outer minus one tire width) |
| Distance per encoder pulse | 10.6 mm | 211 mm / 20 pulses |
