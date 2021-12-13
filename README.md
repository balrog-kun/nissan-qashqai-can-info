# Nissan Qashqai J10 CAN bus data

This file contains the list of CAN bus frame formats and semantics discovered by looking at the frames captured
on a manual-transmission diesel 2013 Nissan Qashqai J10.  A portion of the data here applies to other Nissan models like
Juke, Pulsar, Sentra, 370Z, X-Trail, Murano from the same period of time, and to a lesser extent to the
automatic-transmission and electric models (Nissan Leaf).  The automatic-transmission models will have frames 421 and/or
41f, 176 with gearbox information which the manual models don't use.  Some specifics for other models can be found
[here](https://github.com/commaai/opendbc/blob/master/nissan_leaf_2018.dbc),
[here](https://github.com/commaai/opendbc/blob/master/nissan_x_trail_2017.dbc) and
[here](https://github.com/jackm/carhack/blob/master/nissan.md).

Nissan Qashqai, unlike many other cars, only has one major CAN bus with between 6 and 12 devices connected to it
(depending on model configuration) and it's the same bus you can access through the Data Link Connector (connector M4 in
the service manual schematics) or the comfort unit.  The service manuals refer to that bus as "CAN communication" or
"CAN COMM".  There is one other isolated bus using the CAN protocol and it connects the Audio unit to the NAVI unit
who is the master device on that bus.  That bus is referred to as "AV communication" or "AV COMM" in the service manuals.
This file always refers to the main CAN bus.  Both busses run at 500kbps with 11-bit addresses.

I captured the data using an off-the-shelf ESP32 board with an SN65HVD230 CAN transciever who's main function
is to automate the power mirrors folding and unfolding in my car.
[Schematics](https://www.digikey.com/schemeit/project/esp32-qashkai-folding-mirrors-a809da75fea642bfa70ecee4ab3926d9),
[tutorial](https://www.instructables.com/Build-an-Arduino-Into-a-Nissan-Qashqai-to-Automate/).

## Addressing

This file follows the bit addressing in [this doc](https://github.com/jackm/carhack/blob/master/nissan.md) where CAN
bus message bytes in positions 1 to 8 are referenced by letters A-H and bits are numbered from 8th (MSB) to 1st (LSB).
Bit M in byte N is represented by `N.M`, e.g. the lowest bit of the 3rd byte is `C.1` while the highest bit in
byte 1 -- or the first bit of the entire messsages -- is `A.8`.  Frame IDs (addresses) are hexadecimal.

## Periodic data frames

This lists individual values in the frames transmitted periodically by devices on the main CAN bus, i.e. frames
that don't need to be requested.  Multi-byte values are encoded as big-endian except where noted.

TODO: map signals to their names in chart in service manual LAN.pdf.
TODO: map values to their names in chart on service manual INL-38 or DLK-610.

| Frame | Position in frame | Meaning | Format | Unit |
| --: | --- | --- | --- | --- |
| 002 | `A.8-B.1` | Steering wheel angle | 2's complement 16-bit integer, little-endian, positive values in right turn, negative in left turn, range of about -6000 to 6000 | 0.1 degree / LSB |
| 002 | `C.8-1`   | Steering wheel rate (TODO) |||
| 002 | `E.8-1`   | Message serial/timestamp | unsigned integer ||
||
| 160 | `A.8-B.5` | RPM rate of change or pressure somewhere in the engine > when revving up, < when revving down | 12-bit unsigned integer ||
| 160 | `D.8-E.7` | Accelerator/throttle pedal position (soft zone, value remains at maximum when above 75% stroke, i.e. all stiff zone) | integer between 0 (released) and 792 (at or behind the stiff-zone threshold) ||
| 160 | `E.6`     | Accelerator/throttle pedal in stiff zone, i.e. > 75% stroke | boolean, 1 if at or behind threshold ||
| 160 | `G.8-H.7` | Same as `D.8-E.7` -- according to brad370 these two values are compared for error detection |||
||
| 161 | `A.8-1`   | Some sensor value, varies with engine RPMs |||
| 161 | `B.8-1`   | Some voltage value, strictly goes down with electric current consumption (with a delay) when engine running, constant when stopped -- might be used for the idle RPM calcluation or similar |||
| 161 | `C.8-1`   | Same as above but also goes up with engine RPMs -- roughly follows the sum of `A.8-1` and `B.8-1` |||
||
| 180 | `A.8-B.2` | Engine revolutions | 15-bit unsigned integer | 0.25 RPM / LSB (service manual page MWI-27 says 8191.875 is displayed in case of malfunction, implying that the whole 16-bit value of `A.8-B.1` -- hypothetically `0xffff` in that case -- is simply divided by 8 to get the RPMs) |
||
| 19b | `A.8-5`   | Engine running status? |||
| 19b | `C.8-D.5` | Fuel consumption | 16-bit unsigned integer ||
| 19b | `E.8-5`   | Drive / reverse status? | `0b1111` for a moment after ignition switched to ON, `0b0000` when driving forward or stopped, `0b1001` in reverse, `0b0100` on cruise-control?, `0b0010` when gear-up request, `0b0101` when gear-down request ||
| 19b | `F.8-G.1` | A rather stable sensor value | 16-bit unsigned integer ||
||
| 1f9 | `A.7`     | A/C on while engine running | boolean, 1 when true ||
| 1f9 | `A.4`     | A/C on while engine running (after A/C off, goes to 0 before `A.7`) | boolean, 1 when true ||
| 1f9 | `C.8-D.2` | Engine RPM, same as `180 / A.8-B.2` |||
||
| 215 | `B.7`     | Reverse gear / reverse light | boolean, 1 when in reverse ||
||
| 280 | `B.8-D.5` | A rapidly changing sensor value -- reacts to longitudinal axis acceleration |||
| 280 | `E.8-F.1` | Total absolute speed, similar to `284/E.8-F.1` | 16-bit unsigned integer | 0.01 km/h / LSB (slightly higher) |
| 280 | `G.8-1`   | A rapidly changing sensor value -- reacts to longitudinal axis acceleration | 2's complement 16-bit integer, positive when force towards front, i.e. when parked on a downward slope or decelerating, negative on upward slope or accelerating ||
||
| 284 | `A.8-B.1` | Front right wheel absolute speed | 16-bit unsigned integer, 0 when stopped, positive when rolling in either direction | 1/175th km/h/LSB (7000 at 40kmh) |
| 284 | `C.8-D.1` | Front left wheel absolute speed | 16-bit unsigned integer, 0 when stopped, positive when rolling in either direction | 1/175th km/h/LSB (7000 at 40kmh) |
| 284 | `E.8-F.1` | Total absolute speed | 16-bit unsigned integer | 0.01 km/h / LSB (slightly lower) |
| 284 | `G.8-H.1` | Message serial/timestamp | unsigned integer ||
||
| 285 | `A.8-B.1` | Rear right wheel absolute speed | 16-bit unsigned integer, 0 when stopped, positive when rolling in either direction | 1/175th km/h/LSB (7000 at 40kmh) |
| 285 | `C.8-D.1` | Rear left wheel absolute speed | 16-bit unsigned integer, 0 when stopped, positive when rolling in either direction | 1/175th km/h/LSB (7000 at 40kmh) |
| 285 | `E.8-1`   | Total absolute speed | 8-bit unsigned integer | km/h |
| 285 | `G.8-H.1` | Message serial/timestamp | unsigned integer ||
||
| 2a0 | `B.8-C.1` | Lateral axis acceleration force | 16-bit unsigned integer, 0x8000 in equilibrium, higher values when force towards right, i.e. when parked with right wheels lower or turning left (in forward or in reverse), < 0x8000 when parked with right wheels higher or turning right (in forward or in reverse) ||
| 2a0 | `D.8-E.1` | Turn rate about vertical axis | 16-bit unsigned integer, 0x8000 no turn, higher values in left turn (right in reverse), < 0x8000 in right turn (left in reverse) ||
||
| 2de | `E.2`     | Efficiency unit is l/100km, as opposed to MPG or km/l | boolean, 1 when 1/100km ||
| 2de | `G.8-H.1` | Distance-to-empty -- range at current fuel economy and fuel left (minus reserve, i.e. 0 km when fuel gauge in red zone), as shown on one of the dashboard panels | 16-bit unsigned integer | 0.1 km / LSB rounded to 1 km (10 LSBs) -- when efficiency unit set to l/100km |
||
| 354 | `A.8-B.1` | Total absolute speed, similar to `284/E.8-F.1` | 16-bit unsigned integer | 0.01 km/h / LSB (slightly higher) |
| 354 | `E.7`     | ESP (VDC/TCS) disable button | boolean, 1 when button in ||
| 354 | `E.6`     | ESP (VDC/TCS) off dash light | boolean, 1 when light on ||
| 354 | `E.4`     | ESP (VDC/TCS) off dash light | boolean, 1 when light on ||
| 354 | `E.3`     | ESP (VDC/TCS) off dash light | boolean, 1 when light on ||
| 354 | `E.1`     | ABS? off dash light | boolean, 1 when light on ||
| 354 | `G.5`     | Brake pedal switch / brake light / stop light | boolean, 1 when foot on pedal ||
||
| 355 | `A.8-B.1` | Total absolute speed, similar to `284/E.8-F.1` | 16-bit unsigned integer ||
| 355 | `C.8-D.1` | Total absolute speed, similar to `284/E.8-F.1` | 16-bit unsigned integer ||
||
| 358 | `A.1`     | Key inserted | boolean, 1 when key in ignition ||
| 358 | `B.7`     | Blower fan on (climate control) | boolean, 1 when fan running ||
| 358 | `D.6`     | Likely oil pressure dashboard warning light | boolean, 1 when true ||
| 358 | `F.2`     | Other-than-driver doors locked (even if some were unlocked manually) | boolean, 1 when locked ||
| 358 | `F.1`     | Driver door locked | boolean, 1 when locked ||
||
| 35d | `A.8`     | Ignition in ON | boolean, 1 when ON ||
| 35d | `A.7`     | Ignition in half ON (TODO) |||
| 35d | `A.6`     | Blower fan on (climate control) | boolean, 1 when fan running ||
| 35d | `A.3`     | Rear window defogging (defrost) | boolean, 1 when on ||
| 35d | `A.2`     | Rear window defogging (defrost) | boolean, 1 when on ||
| 35d | `A.1`     | A/C on | boolean, 1 when on ||
| 35d | `B.2`     | Dashboard power | boolean, 1 when on ||
| 35d | `B.1`     | Dashboard power | boolean, 1 when on ||
| 35d | `C.8`     | Wipers moving | boolean, 1 when moving ||
| 35d | `C.7`     | Wipers moving | boolean, 1 when moving ||
| 35d | `E.5`     | Brake pedal switch / brake light / stop light | boolean, 1 when foot on pedal ||
||
| 551 | `A.8-1`   | Engine coolant temperature | 8-bit unsigned integer, values in ºC + 40, `0x75` at middle of gauge range, '0x5b' bottom of range | deg C / LSB (service manual page MWI-27 says 215ºC is displayed in data monitor in case of malfunction, suggesting the raw 8-bit value received -- hypothetically `0xff` in the malfunction case -- is offset by 40 and taken directly as degrees Celcius between -40 and 215) |
| 551 | `B.8-1`   | Engine-related counter, rate proportional to revs, wraps at `0xff` -- fuel consumed? | 8-bit integer | at idle increases at ~1.5 LSB / s |
| 551 | `C.4-1`   | Some sensor reading, changes gradually from `0x4` to `0xa` when blower fan spins up and back when it spins down but is independent of blower speed (may include all of `C.8-1`) | integer ||
| 551 | `D.8-1`   | Possibly dashboard lights |||
| 551 | `E.8-1`   | ASCD cruise speed/limit speed and status | target speed in km/h when engaged, 1 if being set currently | km/h |
| 551 | `F.7-5`   || bitmap, `0b110` when setting ASCD cruise speed, `0b011` when setting limit speed, `0b100` if cruise engaged, `0b001` if limit engaged, `0b101` after cruise dis-enagaged?, 0s otherwise ||
| 551 | `F.4`     | Engine running? | boolean, 1 when running ||
| 551 | `F.4-1`   | Other engine status bits? |||
| 551 | `H.8-1`   | Engine-related counter, same as `B.8-1` |||
||
| 5c5 | `A.7`     | IGN not fully ON? |||
| 5c5 | `A.3`     | Handbrake / parking brake engaged as indicated on dashboard | boolean, 1 when engaged ||
| 5c5 | `B.8-D.1` | Car odometer absolute value | 24-bit unsigned integer | km / LSB |
| 5c5 | `E.1`     | Driver seatbelt unbuckled | boolean, 1 when unbuckled ||
| 5c5 | `F.4-1`   | Might be oil level |||
||
| 5dd | `B.8-C.1` | Some down counter, not fuel though | 16-bit unsigned integer ||
||
| 5e1 | `A.8-B.1` | Rapidly changing counter | 16-bit unsigned integer ||
||
| 5e4 | `A.2`     | Some status bit, true before engine start, false after start (also after stop), not any dashboard light | boolean ||
| 5e4 | `B.8-C.1` | Rapidly changing counter | 16-bit unsigned integer ||
||
| 60d | `A.7`     | Rear RH door open | boolean, 1 when open ||
| 60d | `A.6`     | Rear LH door open | boolean, 1 when open ||
| 60d | `A.5`     | Passenger door open | boolean, 1 when open ||
| 60d | `A.4`     | Driver door open | boolean, 1 when open ||
| 60d | `A.3`     | Position/parking/side lights on | boolean, 1 when on ||
| 60d | `A.2`     | Headlights (either low-beam or high-beam) on | boolean, 1 when either lights on ||
| 60d | `B.7`     | Right-turn signal or emergency lights on | boolean, 1 when on (delayed?) ||
| 60d | `B.6`     | Left-turn signal or emergency lights on | boolean, 1 when on (delayed?) ||
| 60d | `B.4`     | High-beam lights on (use together with `A.2`) | boolean, 1 when on ||
| 60d | `B.3`     | IGN switch ON or START (BCM input 38) | boolean, 1 when ON or START ||
| 60d | `B.2`     | IGN switch ACC or ON (BCM input 37) | boolean, 1 when ACC or ON (can be interpreted as a bitmap together with `B.3`: `0b00` for OFF, `0b01` for ACC, `0b11` for ON, `0b10` for START) ||
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
| 625 | `C.7-1`   | Battery voltage (possibly Diesel-only) | 8-bit unsigned integer | 1/16th V / LSB, e.g. `0xc0` is 12V |

## Missing data

Data available on some cars but not available on Nissan Qashqai J10 or to be yet decoded:
* Fuel level
* Outside temperature
* Outside brightness level (bright/dark, from the Light & Rain sensor)
* ASCD clutch switch
* Seatbelt status and occupancy sensor status
* Car VIN
* Current time and/or data
* Current GPS lat/lon/number of satellites

## Command frames

None yet
