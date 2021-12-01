# Nissan Qashqai J10 CAN bus data

This file contains the list of CAN bus frame formats and semantics discovered by looking at the frames captured
on a manual-transmission diesel 2013 Nissan Qashqai J10.  A portion of the data here applies to other Nissan models like
Sentra, 370Z, X-Trail, Murano from the same period of time, and to a lesser extent to the automatic-transmission
and electric models (Nissan Leaf).  The automatic-transmission models will have frames 421 and/or 41f with
gearbox information which the manual models don't use.  Some specifics for other models can be found
[here](https://github.com/commaai/opendbc/blob/master/nissan_leaf_2018.dbc),
[here](https://github.com/commaai/opendbc/blob/master/nissan_x_trail_2017.dbc) and
[here](https://github.com/jackm/carhack/blob/master/nissan.md).

I captured the data using an off-the-shelf ESP32 board with an SN65HVD230 CAN transciever who's main function
is to automate the power mirrors folding and unfolding in my car.
[Schematics](https://www.digikey.com/schemeit/project/esp32-qashkai-folding-mirrors-a809da75fea642bfa70ecee4ab3926d9),
[tutorial](https://www.instructables.com/Build-an-Arduino-Into-a-Nissan-Qashqai-to-Automate/).

## Addressing

I follow the bit addressing in [this doc](https://github.com/jackm/carhack/blob/master/nissan.md) where CAN bus
message bytes in positions 1 to 8 are referenced by letters A-H and bits are numbered from 8th (MSB) to 1st (LSB).
Bit M in byte N is represented by `N.M`, e.g. the lowest bit of the 3rd byte is `C.1` while the highest bit in
byte 1 -- or the first bit of the entire messsages -- is `A.8`.  Frame IDs (PIDs) are hexadecimal.

## Periodic data frames

This lists individual values in the frames transmitted periodically by devices on the main CAN bus, i.e. frames
that don't need to be requested.  Multi-byte values are encoded as big-endian except where noted.

| Frame | Position in frame | Meaning | Format | Unit |
| --: | --- | --- | --- | --- |
| 002 | `A.8-B.1` | Steering wheel angle | 2's complement 16-bit integer, little-endian, positive values in right turn, negative in left turn, range of about -6000 to 6000 | 0.1 degree / LSB |
| 002 | `C.8-1`   | Steering wheel rate (TODO) |||
| 002 | `E.8-1`   | Message serial/timestamp | unsigned integer ||
||
| 160 | `A.8-B.5` | Pressure somewhere in the engine | 12-bit unsigned integer | possibly mmHg |
| 160 | `D.8-E.7` | Accelerator/throttle pedal position (soft-zone) | integer between 0 (depressed) and 792 (at or behind the stiff-zone threshold) ||
| 160 | `E.6`     | Accelerator/throttle pedal at stiff-zone threshold | boolean, 1 if at or behind threshold ||
| 160 | `G.8-H.7` | Same as `D.8-E.7` |||
||
| 180 | `A.8-B.2` | Engine revolutions | 15-bit unsigned integer | 0.25 RPM / LSB (possibly 1/4.096) |
||
| 19b | `E.8`     | Reverse gear / reverse light | boolean, 1 when in reverse ||
| 19b | `E.5`     | Reverse gear / reverse light | boolean, 1 when in reverse ||
||
| 1f9 | `C.8-D.2` | Engine RPM, same as `180 / A.8-B.2` |||
||
| 280 | `B.5-D.5` | A rapidly changing sensor value |||
| 280 | `F.8-G.1` | Logitudinal axis acceleration force | 2's complement 16-bit integer, positive when force towards front, i.e. when parked on a downward slope or decelerating, negative on upward slope or accelerating ||
||
| 284 | `A.8-B.1` | Front right wheel absolute speed | 16-bit unsigned integer, 0 when stopped, positive when rolling in either direction | 1/175th km/h/LSB (7000 at 40kmh) |
| 284 | `C.8-D.1` | Front left wheel absolute speed | 16-bit unsigned integer, 0 when stopped, positive when rolling in either direction | 1/175th km/h/LSB (7000 at 40kmh) |
| 284 | `E.8-1`   | Some related sensor data |||
| 284 | `G.8-H.1` | Message serial/timestamp | unsigned integer ||
||
| 285 | `A.8-B.1` | Rear right wheel absolute speed | 16-bit unsigned integer, 0 when stopped, positive when rolling in either direction | 1/175th km/h/LSB (7000 at 40kmh) |
| 285 | `C.8-D.1` | Rear left wheel absolute speed | 16-bit unsigned integer, 0 when stopped, positive when rolling in either direction | 1/175th km/h/LSB (7000 at 40kmh) |
| 285 | `E.8-1`   | Some related sensor data |||
| 285 | `G.8-H.1` | Message serial/timestamp | unsigned integer ||
||
| 2a0 | `B.8-C.1` | Lateral axis acceleration force | 16-bit unsigned integer, 0x8000 in equilibrium, higher values when force towards right, i.e. when parked with right wheels lower or turning left (in forward or in reverse), < 0x8000 when parked with right wheels higher or turning right (in forward or in reverse) ||
| 2a0 | `D.8-E.1` | Turn rate about vertical axis | 16-bit unsigned integer, 0x8000 no turn, higher values in left turn (right in reverse), < 0x8000 in right turn (left in reverse) ||
||
| 2de | `G.8-H.1` | Range at current fuel economy and fuel left (minus reserve, i.e. 0 km when fuel gauge in red zone), as shown on one of the dashboard panels | 16-bit unsigned integer | 0.1 km / LSB rounded to 1 km (10 LSBs) |
||
| 354 | `E.7`     | ESP/TCS disable button | boolean, 1 when button in ||
| 354 | `E.6`     | ESP/TCS off dash light | boolean, 1 when light on ||
| 354 | `E.4`     | ESP/TCS off dash light | boolean, 1 when light on ||
| 354 | `E.3`     | ESP/TCS off dash light | boolean, 1 when light on ||
| 354 | `E.1`     | ABS? off dash light | boolean, 1 when light on ||
| 354 | `G.5`     | Brake pedal switch / brake light / stop light | boolean, 1 when foot on pedal ||
||
| 358 | `A.1`     | Key inserted | boolean, 1 when key in ignition ||
| 358 | `B.7`     | Blower fan on (climate control) | boolean, 1 when fan running ||
| 358 | `C.6`     | Ignition ON but engine not running | boolean, 1 when true ||
| 358 | `F.2`     | Other-than-driver doors locked (even if some were unlocked manually) | boolean, 1 when locked ||
| 358 | `F.1`     | Driver door locked | boolean, 1 when locked ||
||
| 35d | `A.8`     | Ignition in ON | boolean, 1 when ON ||
| 35d | `A.7`     | Ignition in half ON (TODO) |||
| 35d | `A.6`     | Blower fan on (climate control) | boolean, 1 when fan running ||
| 35d | `A.3`     | Rear defrost heating | boolean, 1 when on ||
| 35d | `A.2`     | Rear defrost heating | boolean, 1 when on ||
| 35d | `A.1`     | A/C on | boolean, 1 when on ||
| 35d | `B.2`     | Dashboard power | boolean, 1 when on ||
| 35d | `B.1`     | Dashboard power | boolean, 1 when on ||
| 35d | `C.8`     | Wipers moving | boolean, 1 when moving ||
| 35d | `C.7`     | Wipers moving | boolean, 1 when moving ||
| 35d | `E.5`     | Brake pedal switch / brake light / stop light | boolean, 1 when foot on pedal ||
||
| 551 | `A.8-1`   | Engine temperature | 8-bit unsigned integer, `0x75` at middle of gauge range | deg C / LSB? |
| 551 | `B.8-1`   | Engine-related counter, rate proportional to revs, wraps at `0xff` | 8-bit integer | at idle increases at ~1.5 LSB / s |
| 551 | `C.8-1`   | Some other temperature |||
| 551 | `D.8-1`   | Possibly dashboard lights |||
| 551 | `E.1`     | Cruise speed/limit speed being set | boolean, 1 if being set currently ||
| 551 | `F.7-5`   || bitmap, `0b110` if setting cruise speed, `0b011` if limit speed, `0b001` if engaged? ||
| 551 | `F.4`     | Engine running? | boolean, 1 when running ||
| 551 | `F.4-1`   | Other engine status bits? |||
| 551 | `H.8-1`   | Engine-related counter, same as `B.8-1` |||
||
| 580 | `A.8-E.1` | Possibly a cruise-control sensor (mine might be broken, always zeros) |||
||
| 5c5 | `A.7`     | IGN not fully ON? |||
| 5c5 | `A.3`     | Handbrake / parking break engaged as indicated on dashboard | boolean, 1 when engaged ||
| 5c5 | `B.8-D.1` | Car odometer absolute value | 24-bit unsigned integer | km / LSB |
| 5c5 | `E.1`     | Driver seatbelt unbuckled | boolean, 1 when unbuckled ||
||
| 60d | `A.7`     | Passenger-side rear door open | boolean, 1 when open ||
| 60d | `A.6`     | Driver-side rear door open | boolean, 1 when open ||
| 60d | `A.5`     | Passenger door open | boolean, 1 when open ||
| 60d | `A.4`     | Driver door open | boolean, 1 when open ||
| 60d | `A.3`     | Position/parking/side lights on | boolean, 1 when on ||
| 60d | `A.2`     | Headlights (either low-beam or high-beam) on | boolean, 1 when either lights on ||
| 60d | `B.7`     | Right-turn signal or emergency lights on | boolean, 1 when on (delayed?) ||
| 60d | `B.6`     | Left-turn signal or emergency lights on | boolean, 1 when on (delayed?) ||
| 60d | `B.4`     | High-beam lights on (use together with `A.2` | boolean, 1 when on ||
| 60d | `B.3`     | IGN key position in ON or START | boolean, 1 when ON or START ||
| 60d | `B.2`     | IGN key position in ACC or ON | boolean, 1 when ACC or ON (can be interpreted as a bitmap together with `B.3`: `0b00` for OFF, `0b01` for ACC, `0b11` for ON, `0b10` for START) ||
| 60d | `B.1`     | Front fog lights on | boolean, 1 when on ||
| 60d | `C.5`     | Any door locked | boolean, 1 when locked ||
| 60d | `C.4`     | Any door locked | boolean, 1 when locked ||
| 60d | `C.3`     | Read fog lights on | boolean, 1 when on ||
| 60d | `D.6`     | Trunk door request / beep | boolean, 1 when on ||
| 60d | `D.2`     | Trunk door request / beep | boolean, 1 when on ||
||
| 625 | `A.6-5`   | `0b11` when ready to start? (injectors ready?) |||
| 625 | `B.7`     | Position/parking/side lights on | boolean, 1 when on (delayed?) ||
| 625 | `B.6`     | Headlights (either low-beam or high-beam) on | boolean, 1 when either lights on (delayed?) ||
| 625 | `B.5`     | High-beam lights on | boolean, 1 when on (delayed?) ||
| 625 | `B.4`     | Front fog lights on | boolean, 1 when on (delayed?) ||
| 625 | `C.7-1`   | Battery voltage | 8-bit unsigned integer | 1/16th V / LSB, e.g. `0xc0` is 12V |

## Missing data

Data available on some cars but not available on Nissan Qashqai J10 or not yet decoded:
* Fuel level
* Outside temperature
* Seatbelt status and occupancy sensor status
* Car VIN
* Current time and/or data
* Current GPS lat/lon/number of satellites

## Command frames

None yet
