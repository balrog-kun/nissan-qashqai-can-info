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
| 160 | `A.8-B.5` | Varies with engine load, could be pressure value, > when revving up, < when revving down, usually follows `180: C.8-D5` and `180: D.4-E.1` | 12-bit unsigned integer ||
| 160 | `D.8-E.7` | Accelerator/throttle pedal position (soft zone, value remains at maximum when above 75% stroke, i.e. all stiff zone) | integer between 0 (released) and 792 (at or behind the stiff-zone threshold) ||
| 160 | `E.6`     | WOT -- Accelerator/throttle pedal in stiff zone, i.e. > 75% stroke | boolean, 1 if at or behind threshold ||
| 160 | `G.8-H.7` | Same as `D.8-E.7` -- according to brad370 these two values are compared for error detection |||
||
| 161 | `A.8-1`   | Some sensor value, varies with engine RPMs |||
| 161 | `B.8-1`   | Some voltage value, strictly goes down with electric current consumption (with a delay) when engine running, constant when stopped -- might be used for the idle RPM calcluation or similar |||
| 161 | `C.8-1`   | Same as above but also goes up with engine RPMs -- roughly follows the sum of `A.8-1` and `B.8-1` |||
| 161 | `D.1`     | WOT -- Accelerator/throttle pedal in stiff zone, i.e. > 75% stroke | boolean, 1 if behind threshold ||
| 161 | `E.8-1`   | Accelerator/throttle pedal position (soft zone, value remains at maximum when above 75% stroke, i.e. all stiff zone) | integer between 0 (released) and 0xfc (at or behind the stiff-zone threshold) ||
||
| 180 | `A.8-B.2` | Engine speed / revolutions | 15-bit unsigned integer | 0.25 RPM / LSB (service manual page MWI-27 says 8191.875 is displayed in case of malfunction, implying that the whole 16-bit value of `A.8-B.1` -- hypothetically `0xffff` in that case -- is simply divided by 8 to get the RPMs) |
| 180 | `C.8-D.5` | Varies with engine load, could be pressure value, usually follows `D.4-E.1` and `160: A.8-B.5` | 12-bit unsigned integer ||
| 180 | `D.4-E.1` | Varies with engine load, could be pressure value, usually follows `C.8-D.5` and `160: A.8-B.5` | 12-bit unsigned integer ||
||
| 19b | `A.8-5`   | Engine running status? |||
| 19b | `C.8-D.1` | Related to fuel consumption, up when revving up, low at constant revs, 0 when revving down (doesn't match fuel consumption calculated from the l/100km efficiency indication on dashboard multiplied by current velocity though) | 16-bit unsigned integer ||
| 19b | `E.8-5`   | Drive / reverse status? | `0b1111` for a moment after ignition switched to ON, `0b0000` when driving forward or stopped, `0b1001` in reverse, `0b0100` on cruise-control?, `0b0010` when gear-up request, `0b0101` when gear-down request ||
| 19b | `F.8-G.1` | A rather stable sensor value | 16-bit unsigned integer ||
||
| 1f9 | `A.7`     | A/C on while engine running | boolean, 1 when true ||
| 1f9 | `A.4`     | A/C on while engine running (after A/C off, goes to 0 before `A.7`) | boolean, 1 when true ||
| 1f9 | `C.8-D.2` | Engine RPM, same as `180: A.8-B.2` |||
||
| 215 | `B.7`     | Reverse gear / reverse light | boolean, 1 when in reverse ||
||
| 280 | `B.8-D.5` | A rapidly changing sensor value -- reacts to longitudinal axis acceleration |||
| 280 | `E.8-F.1` | Vehicle absolute speed, similar to `284: E.8-F.1` | 16-bit unsigned integer | 0.01 km/h / LSB (slightly higher) |
| 280 | `G.8-1`   | A rapidly changing sensor value -- reacts to longitudinal axis acceleration | 2's complement 16-bit integer, positive when force towards front, i.e. when parked on a downward slope or decelerating, negative on upward slope or accelerating ||
||
| 284 | `A.8-B.1` | Front right wheel absolute speed | 16-bit unsigned integer, 0 when stopped, positive when rolling in either direction | 1/175th km/h / LSB (7000 at 40kmh) |
| 284 | `C.8-D.1` | Front left wheel absolute speed | 16-bit unsigned integer, 0 when stopped, positive when rolling in either direction | 1/175th km/h / LSB (7000 at 40kmh) |
| 284 | `E.8-F.1` | Vehicle absolute speed | 16-bit unsigned integer | 0.01 km/h / LSB (slightly lower) |
| 284 | `G.8-H.1` | Message serial/timestamp | unsigned integer ||
||
| 285 | `A.8-B.1` | Rear right wheel absolute speed | 16-bit unsigned integer, 0 when stopped, positive when rolling in either direction | 1/175th km/h / LSB (7000 at 40kmh) |
| 285 | `C.8-D.1` | Rear left wheel absolute speed | 16-bit unsigned integer, 0 when stopped, positive when rolling in either direction | 1/175th km/h / LSB (7000 at 40kmh) |
| 285 | `E.8-1`   | Vehicle absolute speed | 8-bit unsigned integer | km/h |
| 285 | `G.8-H.1` | Message serial/timestamp | unsigned integer ||
||
| 2a0 | `B.8-C.1` | Lateral axis acceleration force | 16-bit unsigned integer, 0x8000 in equilibrium, higher values when force towards right, i.e. when parked with right wheels lower or turning left (in forward or in reverse), < 0x8000 when parked with right wheels higher or turning right (in forward or in reverse) | 0.0001274 g / LSB according to some specs |
| 2a0 | `D.8-E.1` | Turn rate about vertical axis | 16-bit unsigned integer, 0x8000 no turn, higher values in left turn (right in reverse), < 0x8000 in right turn (left in reverse) | 0.005 °/s / LSB according to some specs|
||
| 2de | `E.2`     | Efficiency unit is l/100km, as opposed to MPG or km/l | boolean, 1 when l/100km ||
| 2de | `G.8-H.1` | Distance-to-empty -- range at current fuel economy and fuel left (minus reserve, i.e. 0 km when fuel gauge in red zone), as shown on one of the dashboard panels | 16-bit unsigned integer | 0.1 km / LSB rounded to 1 km (10 LSBs) -- when efficiency unit set to l/100km |
||
| 354 | `A.8-B.1` | Vehicle absolute speed, similar to `284: E.8-F.1` | 16-bit unsigned integer | 0.01 km/h / LSB (slightly higher) |
| 354 | `E.7`     | ESP (VDC/TCS) disable button | boolean, 1 when button in ||
| 354 | `E.6`     | ESP (VDC/TCS) off dash light | boolean, 1 when light on ||
| 354 | `E.4`     | ESP (VDC/TCS) off dash light | boolean, 1 when light on ||
| 354 | `E.3`     | ESP (VDC/TCS) off dash light | boolean, 1 when light on ||
| 354 | `E.1`     | ABS? off dash light | boolean, 1 when light on ||
| 354 | `G.5`     | Brake pedal switch / brake light / stop light | boolean, 1 when foot on pedal ||
||
| 355 | `A.8-B.1` | Vehicle absolute speed, similar to `284: E.8-F.1` | 16-bit unsigned integer ||
| 355 | `C.8-D.1` | Vehicle absolute speed, similar to `284: E.8-F.1` | 16-bit unsigned integer ||
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
| 551 | `A.8-1`   | Engine coolant temperature | 8-bit unsigned integer, values in °C + 40, `0x75` at middle of gauge range, '0x5b' bottom of range | deg C / LSB (service manual page MWI-27 says 215ºC is displayed in data monitor in case of malfunction, suggesting the raw 8-bit value received -- hypothetically `0xff` in the malfunction case -- is offset by 40 and taken directly as degrees Celcius between -40 and 215) |
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
| 5c5 | `B.8-D.1` | Car odometer absolute value (mileage) | 24-bit unsigned integer | km / LSB |
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

## BCM diagnostic action PIDs (commands)

These are the BCM's service 0x30 PIDs that can be used to send commands and only send commands (there don't seem to be any useful information queries).  The service listens on address **0x745** and replies on address **0x765** but it allows communication only [as documented by Brad370z here](https://projectbytes.wordpress.com/2015/07/08/nissan-370z-can-control/), i.e. in a diagnostic session.  SID 0x30 is known as "I/O control by local ID" in some protocols.

These commands toggle the different lights, actuators, speakers and other devices connected to the BCM, plus it seems the BCM will automatically request power to the right rails on the car from the IPDM-E/R so that the commands mostly work even when the car is completely off.  The changes will time out after 5 seconds so these seem to be strictly diagnostic commands for quickly testing the different devices.  At the same time it seems the changes can be extended in time by resending the commands before the 5s run out.

As documented by Brad370z each command consists of the service number `0x30`, the PID/CID number, the function byte and the requested value.
Three functions seem to available:

| Function byte | Description |
| --: | --- |
| `0x00` | Sends a one-time command such as door unlocking.  Either this or `0x20` is supported by every PID, never both. |
| `0x01` | Queries whether given value is supported by this PID.  Every PID supports this function |
| `0x20` | Causes a temporary 5-second change to a device's state |

Most PID/CIDs support values 0 (off) and 1 (on).  For function `0x00` requesting the value 0 does nothing.  For function `0x20` the value 0 disables a device temporarily if it's on, and cancels a previously requested value that hasn't timed out yet.  For example if the left turn signal is currently blinking because the turn signal & wiper switch (stick/lever) is in the left position, requesting value will 1 will start blinking the right turn signal (and disable the left turn signal) for 5 seconds.  Requesting value 0 will disable either turn signal for 5s whether it was requested with the physical switch or a diagnostic command.

| PID | Function (`0x00`/`0x20`) | Values |
| --: | :-: | --- |
| 00 || _Supported PIDs 01-1f bitmask_: `32 20 c6 81` |
| 03 | `00` | 0: nothing, 1 & 2: _TODO_ -- beeps like the lock/unlock switch and a relay action can be heard from the BCM |
| 04 | `20` | 0, 1, 2: _TODO_ |
| 07 | `00` | 0: nothing, 1: Lock doors, 2: Unlock all doors, 3: Unlock driver door, 4: Unlock passenger doors |
| 0b | `20` | 0: Rear window defogging (defrost) off for 5s, 1: Rear window defogging (defrost) on for 5s |
| 11 | `20` | 0: Key beep off, 1: Key beep continuous tone for 5s |
| 12 | `20` | 0: Key beep off, 1: Key beep 4 short tones repeating for 5s (5 times) |
| 16 | `20` | 0: key beep off, 1: Key beep 4 short tones repeating for 5s (5 times) |
| 17 | `20` | 0: Roof light on for 5s, 1: Roof light on for 5s (same?) |
| 19 | `20` | 0: Roof light off, 1: Roof light on for 5s, 2: Roof light auto mode? in ACC on for 5s, in ON fade out and off for 5s |
| 20 || _Supported PIDs 21-3f bitmask_: `84 00 00 6d` |
| 21 | `20` | 0: No-key dashboard light off for 5s, 1: No-key dashboard light on for 5s |
| 26 | `00` | 0: nothing, 1: Trunk door open |
| 3a | `20` | 0: Position/parking/side lights off for 5s, 1: Position/parking/side lights on for 5s |
| 3b | `20` | 0: Low-/high-beam off for 5s, 1: Low-beam lights on for 5s, 2: High-beam lights on for 5s |
| 3d | `20` | 0: Front fog lights off for 5s, 1: Front fog lights on for 5s |
| 3e | `20` | 0: Rear fog lights off for 5s, 1: Rear fog lights on for 5s |
| 40 || _Supported PIDs 41-5f bitmask_: `09 90 00 01` |
| 45 | `20` | 0: Front wiper off for 5s, 1: Front wiper fast mode for 5s, 2: Front wiper slow mode for 5s, 3: front wiper one-shot mode |
| 48 | `20` | 0: Rear wiper off for 5s, 1: Rear wiper on for 5s |
| 49 | `00` | 0, 1: _TODO_ |
| 4c | `20` | 0: Turn signals/blinkers off for 5s, 1: Right turn signal on, left off for 5s, 2: Left turn signal on, right off for 5s |
| 60 || _Supported PIDs 61-7f bitmask_: `00 10 03 00` |
| 69 || MISSING |
| 77 | `20` | 0: Check oil dashboard light off for 5s, 1: Check oil dashboard light on for 5s |
| 78 | `20` | 0, 1: _TODO_ |

## Engine ECU diagnostic action PIDs (commands)

The engine computer seems to have its own service 0x30 to trigger diagnostic actions, with a similar syntax to the BCM's service 0x30.  The service 0x30 commands as well as the diagnostic session request (`02 10 c0`) now need to be sent to address **7e0** instead of **745**.  The function byte is always `00` and the value byte is apparently ignored, so the functions and values are not listed below.

These commands can't be used when the engine is running, error `22` (conditionsNotCorrect) is returned if they're attempted with the engine running.  Similarly some of the commands return error `22` if a previous related command is still in effect.  The PID/CIDs may be specific to the K9K engine or a subset of engines.

| PID | Action |
| --: | --- |
| 03 | Runs the radiator for 2-3 secs |
| 04 | Runs the radiator for 2-3 secs |
| 11 | _TODO_ |
| 12 | _TODO_ (2 or 3 quiet tick sounds in 2 sec intervals)
| 13 | _TODO_ |
| 14 | _TODO_ |
| 17 | _TODO_ |
| 18 | _TODO_ (makes a quiet ~5 Hz ticking noise from engine room for ~5 secs) |
| 22 | _TODO_ |
| 26 | _TODO_ |
| 35 | _TODO_ |
| 36 | _TODO_ |
| 37 | _TODO_ |
| 38 | _TODO_ |
| 41 | _TODO_ |
| 42 | _TODO_ |
| 43 | _TODO_ |

## IPDM-E/R diagnostic action PIDs (commands)

These are the diagnostic PIDs/CIDs for the IPDM-E/R ECU (or BCM?) at address **74d**.  Like before diagnostic session `0xc0` required.

| PID | Function (`0x00`/`0x20`) | Values |
| --: | :-: | --- |
| 00 || _Supported PIDs 01-1f bitmask_: `0b d2 00 00` |
| 05 | 00 | 0: nothing, 1: Very short honk |
| 07 | 20 | 0: Rear window defogging (defrost) off for 5s, 1: Rear window defogging (defrost) on for 5s |
| 08 | 20 | 0: Front wipers off for 5s, 1: Front wipers run slow for 5s, 2: Front wiper run fast for 5s |
| 09 | 20 | 0: Radiator off for 5s, 1, 2, 3: Run the radiator at different speeds | `` |
| 0a | 20 | 0, 1: _TODO_ triggers some big relay on for 5s |
| 0c | 00 | 0, 1: _TODO_ |
| 0f | 20 | 0: Front lights off for 5s, 1: Low/high-beam off for 5s, 2: Position and high-beam off for 5s, 3: Blink high-beam, position lights off for 5s, 4: Front fog lights on and low/high-beam/position lights off for 5s |

## Standard service 01 PIDs (current data)

This is the supported subset of the standard ECU service 01 PIDs just as [described on wikipedia](https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_01).  I captured the values with the engine off, ignition in ON on my 2013 manual transmission diesel J10.  Any PIDs not listed here seem to be unsupported.  The queries are sent to the address **7e0** and the replies are returned from address **7e8**.

| PID | Meaning | Unit (formula to get physical value) | Captured value |
| --: | --- | --- | --- |
| 00 | _Supported PIDs 01-1f bitmask_ || `98 3b 80 11` |
| 01 | Monitor status since DTCs cleared | bitmap | `00 64 80 00` (Diesel engine, Components test incomplete, Fuel test incomplete, EGR System test available) |
| 04 | Calculated engine load | % (100/255 A) | `00` |
| 05 | Engine coolant temperature | °C (A-40) | `4d` |
| 0b | Intake manifold absolute pressure | kPa (A) | `5e` |
| 0c | Engine speed | rpm (1/4(256A+B)) | `00 50` |
| 0d | Vehicle speed | km/h (A) | `00` |
| 0f | Intake air temperature | °C (A-40) | `51` |
| 10 | [Mass air flow sensor (MAF)](https://en.wikipedia.org/wiki/Mass_airflow_sensor) air flow rate | grams/sec (1/100(256A+B)) | `00 00` |
| 11 | Throttle position (doesn't seem to work, always 0?) | % (100/255 A) | `00` |
| 1c | OBD standards this vehicle conforms to | enum | `06` (EOBD-Europe) |
| 20 | _Supported PIDs 21-3f bitmask_ || `a0 01 80 00` |
| 21 | Distance traveled with malfunction indicator lamp (MIL) on | km (256A+B) | `00 00` |
| 23 | [Fuel Rail](https://en.wikipedia.org/wiki/Fuel_rail) Gauge Pressure | kPa (10(256A+B))| `00 00` |
| 30 | Warm-ups since codes cleared | count (A) | `13` |
| 31 | Distance traveled since codes cleared | km (256A+B) | `03 7c` |

## Standard service 09 PIDs (vehicle information)

As before this is the supported subset of the standard ECU service 09 PIDs just as [described on wikipedia](https://en.wikipedia.org/wiki/OBD-II_PIDs#Service_09).  Any PIDs not listed here seem to be unsupported, i.e. very little is actually supported and the VIN query (02), while available, returns no data.  The other two PIDs have lengths that don't match the specs.

| PID | Meaning | Value |
| --: | --- | --- |
| 00 | _Supported PIDs 01-1f bitmask_ | `54 00 00 00` |
| 02 | [Vehicle Identification Number](https://en.wikipedia.org/wiki/Vehicle_Identification_Number) (VIN) | `01 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff` |
| 04 | Calibration ID | `02 32 33 37 31 30 42 42 33 31 41 00 00 00 00 00 00 32 33 37 30 31 42 42 33 36 41 00 00 00 00 00 00` (ascii 23710BB31A 23701B36A) |
| 06 | Calibration Verification Numbers (CVN) Several CVN can be output (4 bytes each) the number of CVN and CALID must match | `01 44 d8 62 a7` |

## Non-standard service 22 PIDs

These are the manufacturer-specific service 22 PID/CIDs that can be queried for current values from sensors, internal BCM/Engine state and/or hardcoded information.  They're read in the same way as services 01 and 09 except for the PID/CID numbers being 16-bit.  SID 0x22 is known as "Read Data by Common Identifier" (CID, or Data Identifier - DID)

| PID | Meaning | Unit (format) | Captured value |
| --: | --- | --- | --- |
| 2000 | _Supported PIDs 2001-201f bitmask_ || `ff bf 14 59` |
| 2001 | Some engine temperature reading || `0c 6a` |
| 2002 | Engine speed/revolutions | 0.5 RPM / LSB | `00 50` |
| 2003 | Vehicle absolute speed || `00 00` |
| 2004 | Varies with engine load, could be pressure value, similar to `180: C.8-D.5` but higher resolution || `32 00` |
| 2005 | Battery voltage | 0.01 V / LSB | `04 cf` |
| 2006 | Car odometer absolute value (mileage) | km | `01 fd ad` |
| 2007 | (Same as 2001?) Some engine temperature reading || `0c 6a` |
| 2008 | A different engine temperature reading  || `0c 4e` |
| 2009 | _TODO_ || `03 b8` |
| 200b | Raw accelerator/throttle pedal angle (unclamped) || `02 e6` |
| 200c | Raw accelerator/throttle pedal angle (unclamped) || `01 73` |
| 200d | Some engine temperature reading || `00 5b` |
| 200e | Ignition in ON position || `01` |
| 200f | Brake pedal position: 0 when released, 1 when foot on pedal (barely pressed), 2 when pressed enough for the braking light to come on || `00` |
| 2010 | _TODO_ || `00` |
| 2014 | 1 when engine running || `00` |
| 2016 | 1 when engine running || `00` |
| 201a | _TODO_ || `00` |
| 201c | _TODO_ || `00` |
| 201d | _TODO_ || `00` |
| 2020 | _Supported PIDs 2021-203f bitmask_ || `fd 0c 0e ff` |
| 2021 | _TODO_ || `13 8d` |
| 2022 | _TODO_ || `13 8d` |
| 2023 | _TODO_ || `13 8d` |
| 2024 | Seems to be the idle RPM setpoint, changes with current demand | 0.25 RPM / LSB | `0c 80` |
| 2025 | 2 when brake pedal pressed (light on), 1 otherwise || `01` |
| 2026 | 2 when brake pedal pressed (light on), 1 otherwise || `01` |
| 2028 | Some dashboard lights? || `03` |
| 202d | _TODO_ || `00` |
| 202e | Accelerator/throttle pedal angle scaled to 0-3ff range but not clamped at 75% || `00 00` |
| 2035 | _TODO_ || `ff` |
| 2036 | _TODO_ || `00` |
| 2037 | Varies with engine load, could be pressure value, same as `160: A.8-B.5` but higher resolution || `2b ed` |
| 2039 | _TODO_ || `0f 95` |
| 203a | _TODO_ || `01` |
| 203b | _TODO_ || `01` |
| 203c | _TODO_ || `00` |
| 203d | _TODO_ || `01` |
| 203e | _TODO_ || `01` |
| 203f | _TODO_ || `00` |
| 2040 | _Supported PIDs 2041-205f bitmask_ || `ff 7f ff fb` |
| 2041 | _TODO_ || `01` |
| 2042 | _TODO_ || `00` |
| 2043 | _TODO_ || `01` |
| 2044 | Accelerator/throttle pedal angle scaled to 0-3ff range, clamped at 75% stroke || `00 00` |
| 2045 | _TODO_ || `03` |
| 2046 | _TODO_ || `00` |
| 2047 | _TODO_ || `01` |
| 2048 | 1 when engine stopped, 2 when running || `01` |
| 204a | _TODO_ || `00` |
| 204b | _TODO_ || `00` |
| 204c | 0 when cruise control disabled, 1 when limit speed engaged, 2 when setting limit speed, 5 when setting cruise speed, other values TODO || `00` |
| 204d | _TODO_ || `00` |
| 204e | _TODO_ || `24` |
| 204f | _TODO_ || `80` |
| 2050 | Vehicle absolute speed || `00 00` |
| 2051 | _TODO_ || `00` |
| 2052 | _TODO_ || `00` |
| 2053 | _TODO_ || `01` |
| 2054 | 1 when engine stopped, 7 when running? nope, something else || `07` |
| 2055 | _TODO_ || `07` |
| 2056 | 1 when engine running, 0 when stopped, 7 for a short while after stopping? || `00` |
| 2057 | Engine load-related value || `00 06` |
| 2058 | _TODO_ || `00` |
| 2059 | ASCD Clutch switch, 0 when release, 1 when depressed || `00` |
| 205a | 4 when RPMs above idle, 8 when engine stopped, 0x40 when idle.  Values 2 and 0x20 seen briefly while going from higher RPMs to idle || `08` |
| 205b | _TODO_ || `00` |
| 205c | 0 when IGN in OFF or ACC, 1 when in ON || `01` |
| 205d | 0 when accelerator/throttle pedal pressed, 1 when released || `01` |
| 205f | _TODO_ || `0f a0` |
| 2060 | _Supported PIDs 2061-207f bitmask_ || `ff ee 01 ff` |
| 2061 | Varies with engine load, could be a pressure value, similar to `180: C.8-D.5` but higher resolution || `32 00` |
| 2062 | Varies with engine load, could be a pressure value, similar to `180: C.8-D.5` but higher resolution || `32 00` |
| 2063 | Varies with engine load, could be a pressure value, different from `180: C.8-D.5` || `32 00` |
| 2064 | Battery voltage? Varies with engine RPM || `2b ed` |
| 2065 | Varies with engine load, could be a pressure value || `2b ed` |
| 2066 | Varies with engine load and RPMs || `06 13` |
| 2067 | Varies with engine load, could be a pressure value || `0d ad` |
| 2068 | Varies with engine load, could be a pressure value || `0d ad` |
| 2069 | Varies with engine load, could be fuel consumption or a pressure value || `00 00` |
| 206a | Engine running time in seconds (resets when ECU starts?) || `00 00` |
| 206b | Engine running time in seconds (resets every time IGN goes from OFF to ON?) || `00 00` |
| 206d | _TODO_ || `01` |
| 206e | _TODO_ || `01` |
| 206f | _TODO_ || `f0` |
| 2078 | Occasionally 2, _TODO_ || `00` |
| 2079 | _TODO_ || `00` |
| 207a | _TODO_ || `00` |
| 207b | _TODO_ || `00` |
| 207c | _TODO_ || `00` |
| 207d | _TODO_ || `00` |
| 207e | _TODO_ || `00` |
| 207f | _TODO_ || `00` |
| 2080 | _Supported PIDs 2081-209f bitmask_ || `ff 21 ff aa` |
| 2081 | _TODO_ || `00` |
| 2082 | _TODO_ || `00` |
| 2083 | _TODO_ || `00` |
| 2084 | _TODO_ || `01` |
| 2085 | _TODO_ || `01` |
| 2086 | _TODO_ || `00` |
| 2087 | _TODO_ || `01` |
| 2088 | _TODO_ || `01` |
| 208b | _TODO_ || `00` |
| 2090 | _TODO_ || `0f` |
| 2091 | _TODO_ || `0f` |
| 2092 | 1 when AC on || `00` |
| 2093 | _TODO_ || `02` |
| 2094 | _TODO_ || `00` |
| 2095 | 1 when engine stopped, 0 when running || `01` |
| 2096 | _TODO_ || `03` |
| 2097 | _TODO_ || `02` |
| 2098 | _TODO_ || `00 00` |
| 2099 | Goes up with electric current demand, goes down with engine RPMs || `eb` |
| 209b | _TODO_ || `00` |
| 209d | _TODO_ || `ff` |
| 209f | _TODO_ || `00 00` |
| 2100 | _Supported PIDs 2101-211f bitmask_ || `ff ff ff fd` |
| 2101 | _TODO_ || `0f` |
| 2102 | _TODO_ || `00 75 3f 01 c6 76 01 33 72 00 e2 95` |
| 2103 | _TODO_ || `04 7d 8b a7 05 0b 43 74 02 ff bc fc 04 8a 8d 47` |
| 2104 | _TODO_ || `00 00 00 00 00 00 00 00` |
| 2105 | _TODO_ || `01` |
| 2106 | _TODO_ || `01` |
| 2107 | _TODO_ || `01 a8 a2` |
| 2108 | _TODO_ || `00 00 00` |
| 2109 | _TODO_ || `00 0a` |
| 210a | A km counter || `37 37` |
| 210b | _TODO_ || `00 1f` |
| 210c | _TODO_, a counter, increases faster than km count || `5e ce` |
| 210d | _TODO_, a down counter || `00 f2 14 a8` |
| 210e | Absolute engine revolution conuter? Updates when stopping engine || `01 e0 51 22` |
| 210f | _TODO_ || `00` |
| 2110 | _TODO_ || `01 c6 76` |
| 2111 | _TODO_ || `01` |
| 2112 | _TODO_ || `00` |
| 2113 | _TODO_ || `0d 9c` |
| 2114 | _TODO_ || `00` |
| 2115 | _TODO_ || `00` |
| 2116 | _TODO_ || `0a d2` |
| 2117 | _TODO_ || `0c 26` |
| 2118 | _TODO_ || `2c ec` |
| 2119 | _TODO_ || `32 c8` |
| 211a | _TODO_ || `ff ff` |
| 211b | _TODO_ || `03 a4` |
| 211c | _TODO_ || `03 56 c0 68` |
| 211d | _TODO_ || `00` |
| 211e | _TODO_ || `00` |
| 2120 | _Supported PIDs 2121-213f bitmask_ || `ff f8 00 01` |
| 2121 | _TODO_ || `01` |
| 2122 | _TODO_ || `01` |
| 2123 | _TODO_ || `01` |
| 2124 | _TODO_ || `01` |
| 2125 | _TODO_ || `01` |
| 2126 | _TODO_ || `00` |
| 2127 | _TODO_ || `01` |
| 2128 | _TODO_ || `01` |
| 2129 | 1 when engine stopped, 0 when running || `01` |
| 212a | _TODO_ || `00` |
| 212b | _TODO_ || `00` |
| 212c | _TODO_ || `2c 01 fc 3c 01 fc 42 01 fc 6f 01 fc 6f 01 fc 70 01 fc 70 01 fc 70 01 fc 70 01 fd 4f 01 fd 55 01 fd 70 01 fd 71 01 fd 71 01 fd 72 01 fd 92 01 fd 93 01 fd 94 01 fd 95 01 fd 95 01 fd 9c` |
| 212d | _TODO_ || `00 10 9a 02 00 00 00 00 00 c0 00 78 76 74 72 70 6e 6c 6b 69 67 66 64 62 61 5f 5e 02 07 02 06 02 06 01 06 02 06 02 06 01 06 02 06 01 06 03 06` |
| 2140 | _Supported PIDs 2141-215f bitmask_ || `00 00 00 03` |
| 215f | 1 if engine has been started since ECU boot? || `00` |
| 2160 | _Supported PIDs 2161-217f bitmask_ || `f4 00 00 01` |
| 2161 | 1 when engine running || `00` |
| 2162 | _TODO_ || `00 98` |
| 2163 | Changes rapidly whlie engine running (values from a subset) || `00 00` |
| 2164 | _TODO_ || `00` |
| 2166 | _TODO_ || `00` |
| 2180 | _Supported PIDs 2181-219f bitmask_ || `96 fe f8 7e` |
| 2181 | _TODO_ || `00 00 00 00` |
| 2184 | _TODO_ || `ff ff` |
| 2186 | _TODO_ || `00` |
| 2187 | _TODO_ || `01` |
| 2189 | _TODO_ || `00` |
| 218a | _TODO_ || `00` |
| 218b | _TODO_ || `00` |
| 218c | _TODO_ || `00` |
| 218d | _TODO_ || `00` |
| 218e | _TODO_ || `00` |
| 218f | _TODO_ || `00` |
| 2191 | _TODO_ || `00 00` |
| 2192 | _TODO_ || `00` |
| 2193 | _TODO_ || `00` |
| 2194 | _TODO_ || `00` |
| 2195 | Occasionally 1 when blower is on but not reliably || `00` |
| 219a | _TODO_ || `00 00` |
| 219b | _TODO_ || `00 05` |
| 219c | _TODO_ || `00` |
| 219d | _TODO_ || `ff ff` |
| 219e | _TODO_ || `00 00` |
| 219f | _TODO_ || `00 00 00` |
| 2200 | _Supported PIDs 2201-221f bitmask_ || `00 00 00 01` |
| 2220 | _Supported PIDs 2221-223f bitmask_ || `ff de 00 01` |
| 2221 | _TODO_ || `00` |
| 2222 | _TODO_ || `00` |
| 2223 | _TODO_ || `00` |
| 2224 | _TODO_ || `00` |
| 2225 | _TODO_ || `00` |
| 2226 | _TODO_ || `00` |
| 2227 | _TODO_ || `00` |
| 2228 | _TODO_ || `00` |
| 2229 | _TODO_ || `01` |
| 222a | Climate control power usage? doesn't seem to react to other electrical loads || `00 37` |
| 222c | ASCD Clutch switch, 1 when depressed || `00` |
| 222d | ASCD Clutch switch, 1 when depressed || `00` |
| 222e | _TODO_ || `02` |
| 222f | _TODO_ || `00` |
| 2240 | _Supported PIDs 2241-225f bitmask_ || `00 00 00 01` |
| 2260 | _Supported PIDs 2261-227f bitmask_ || `06 00 00 01` |
| 2266 | _TODO_ || `00` |
| 2267 | _TODO_ || `00` |
| 2280 | _Supported PIDs 2281-229f bitmask_ || `60 20 00 00` |
| 2282 | _TODO_ || `00 00` |
| 2283 | _TODO_ || `00 00` |
| 228b | _TODO_ || `01 54` |
| 2400 | _Supported PIDs 2401-241f bitmask_ || `ff ab 62 05` |
| 2401 | _TODO_ || `03 b7` |
| 2402 | _TODO_, a sensor value, goes up with RPMs || `04 55` |
| 2403 | _TODO_, a sensor value, goes down with RPMs || `02 0e` |
| 2404 | _TODO_, a sensor value, some noise || `06 5b` |
| 2405 | _TODO_ || `88 00` |
| 2406 | _TODO_, engine temperature? || `7c 31` |
| 2407 | _TODO_, non-linear relation to RPMs and revving the engine up/down || `00 00` |
| 2408 | _TODO_, non-linear relation to RPMs and revving the engine up/down || `7e 70` |
| 2409 | _TODO_, a noisy sensor || `83 2c` |
| 240b | Same as 2401? _TODO_ || `03 b7` |
| 240d | _TODO_, goes up with RPMs || `0c 6c` |
| 240f | _TODO_ || `01` |
| 2410 | _TODO_ || `93 88` |
| 2412 | _TODO_, goes up with RPMs  || `00 00` |
| 2413 | _TODO_, goes up with RPMs || `1d bc` |
| 2417 | _TODO_, some engine sensor, little noise || `11 7e` |
| 241e | _TODO_, goes to 0 when engine running, quickly back to 9 when stopped || `09` |
| 2420 | _Supported PIDs 2421-243f bitmask_ || `9f d5 fd f1` |
| 2421 | _TODO_, non-linear relation to RPMs and revving the engine up/down || `0b 65` |
| 2424 | _TODO_, up with engine load || `0e a1` |
| 2425 | _TODO_ || `07 2e` |
| 2426 | _TODO_ || `07 25` |
| 2427 | _TODO_ || `00 c9` |
| 2428 | _TODO_ || `00 cb` |
| 2429 | _TODO_ || `02 10` |
| 242a | _TODO_ || `01 bf` |
| 242c | _TODO_ || `06 19` |
| 242e | _TODO_ || `04 de` |
| 2430 | _TODO_ || `00 00` |
| 2431 | _TODO_ || `00 00` |
| 2432 | _TODO_ || `23 2b` |
| 2433 | _TODO_ || `00` |
| 2434 | _TODO_ || `00` |
| 2435 | _TODO_ || `80 00` |
| 2436 | _TODO_ || `80 00` |
| 2438 | _TODO_, 0x2986 when running, then back to 0xaaa || `0a aa` |
| 2439 | _TODO_, up with RPMs then down over 20-30 seconds || `0e 92` |
| 243a | _TODO_, varies with engine load || `54 62` |
| 243b | _TODO_ || `09 1a` |
| 243c | _TODO_ || `54 62` |
| 2440 | _Supported PIDs 2441-245f bitmask_ || `c7 08 00 01` |
| 2441 | _TODO_, counter, goes faster with higher RPMs || `15 5b` |
| 2442 | _TODO_ || `0d a1` | dc6 after run, ea4 when running, f12 when running
| 2446 | _TODO_, up with RPMs || `01 e3` |
| 2447 | _TODO_, up with RPMs || `00 00` |
| 2448 | _TODO_, up with RPMs || `03 ac` |
| 244d | _TODO_, down when engine running then slowly recovers || `0c bd` |
| 2460 | _Supported PIDs 2461-247f bitmask_ || `ff fc 4b a7` |
| 2461 | _TODO_ || `00 00 00 03 00 00 00 3c 00 00 00 03 00 00 00 7d 00 00 00 0d 00 00 01 3f 00 00 00 0e 00 00 00 f8 00 00 00 04 00 00` |
| 2462 | _TODO_ || `00 00 00 00 00 00 00 00 00 67 00 3f 00 80 01 4c 01 06` |
| 2463 | _TODO_ || `ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff` |
| 2464 | _TODO_ || `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00` |
| 2465 | _TODO_ || `00 00 00 00 00 02 01 02 02 02` |
| 2466 | _TODO_ || `00 00 00 00 00 00 00 00 01 01` |
| 2467 | _TODO_ || `00 00 00 00 00 02 02 00 00 00` |
| 2468 | _TODO_ || `08 17 20 07 16 19 00 0a 17 1b` |
| 2469 | _TODO_ || `5a 4b 3d 35 2c 16 1b 24 20 1c` |
| 246a | _TODO_ || `00 00 00 00 00 00 00 00 00 00` |
| 246b | _TODO_ || `02 04 0a 00 02 04 08 00 02 04 08 00 02 04 08 00 02 04 0a 00` |
| 246c | _TODO_ || `12 15 14 04 15 10 1b 17 17 01` |
| 246d | _TODO_ || `00` |
| 246e | _TODO_ || `a7 10` |
| 2472 | Car odometer absolute value (mileage) | km | `01 fd ad` |
| 2475 | _TODO_ || `80 00` |
| 2477 | _TODO_ || `01` |
| 2478 | _TODO_ || `01` |
| 2479 | _TODO_ || `01` |
| 247b | _TODO_ || `72` |
| 247e | _TODO_ || `00` |
| 247f | _TODO_ || `0b 32` |
| 2480 | _Supported PIDs 2481-249f bitmask_ || `ff 80 0f 81` |
| 2481 | _TODO_ || `05` |
| 2482 | _TODO_ || `00 00` |
| 2483 | _TODO_ || `00 04` |
| 2484 | _TODO_ || `00 01` |
| 2485 | _TODO_ || `01` |
| 2486 | _TODO_ || `00` |
| 2487 | _TODO_ || `00 00` |
| 2488 | _TODO_ || `12 ee` |
| 2489 | _TODO_ || `03 a4` |
| 2495 | _TODO_ || `03 d4` |
| 2496 | _TODO_ || `03 d4` |
| 2497 | _TODO_ || `02 45` |
| 2498 | _TODO_ || `00 00` |
| 2499 | _TODO_ || `00` |
| 24a0 | _Supported PIDs 24a1-24bf bitmask_ || `1f ff ef fd` |
| 24a4 | _TODO_ || `00 01 01 00 01 01 01 01 00 00` |
| 24a5 | _TODO_ || `04 de` |
| 24a6 | _TODO_ || `02 26` |
| 24a7 | _TODO_ || `04 4c` |
| 24a8 | _TODO_ || `00` |
| 24a9 | _TODO_, odometer snapshot when? doesn't update on every trip || `01 fc b1` |
| 24aa | _TODO_ || `00` |
| 24ab | _TODO_ || `00` |
| 24ac | _TODO_ || `02 23 e2` |
| 24ad | _TODO_ || `02 24 bb` |
| 24ae | _TODO_ || `02 24 bb` |
| 24af | _TODO_ || `02` |
| 24b0 | _TODO_, odometer snapshot when? updates on every trip || `01 fd ad` |
| 24b1 | _TODO_ || `01 a4` |
| 24b2 | _TODO_ || `74` |
| 24b3 | _TODO_ || `01` |
| 24b5 | _TODO_ || `00 0d` |
| 24b6 | _TODO_ || `00 00` |
| 24b7 | _TODO_ || `12 80` |
| 24b8 | _TODO_ || `00 00` |
| 24b9 | _TODO_ || `00` |
| 24ba | _TODO_ || `00 00` |
| 24bb | _TODO_ || `00 00 00` |
| 24bc | _TODO_ || `00 00 fc` |
| 24bd | Same as 247f, _TODO_ || `0b 32` |
| 24be | _TODO_ || `03 a5` |
| 24c0 | _Supported PIDs 24c1-24df bitmask_ || `47 7f ef 31` |
| 24c2 | _TODO_ || `7f f9` |
| 24c6 | _TODO_ || `86 5a` |
| 24c7 | _TODO_ || `00 cb` |
| 24c8 | _TODO_ || `80 00` |
| 24ca | _TODO_ || `07 26` |
| 24cb | _TODO_ || `00` |
| 24cc | _TODO_ || `00` |
| 24cd | _TODO_ || `00` |
| 24ce | _TODO_, varies with engine load || `80 00` |
| 24cf | _TODO_, varies with engine load, slowly goes back to earlier values || `02 07` |
| 24d0 | _TODO_ || `00` |
| 24d1 | _TODO_ || `80 00` |
| 24d2 | _TODO_ || `80 00` |
| 24d3 | _TODO_ || `00 00` |
| 24d5 | _TODO_ || `13 97` |
| 24d6 | _TODO_ || `13 97` |
| 24d7 | _TODO_, goes up with RPMs || `03 ba` |
| 24d8 | _TODO_ || `00` |
| 24db | _TODO_ || `01` |
| 24dc | _TODO_ || `02` |
| 24e0 | _Supported PIDs 24e1-24ff bitmask_ || `67 7b ff d1` |
| 24e2 | _TODO_ || `00 00` |
| 24e3 | _TODO_ || `7f f9` |
| 24e6 | _TODO_ || `00` |
| 24e7 | _TODO_ || `00` |
| 24e8 | _TODO_ || `1f db` |
| 24ea | _TODO_ || `00 00 00` |
| 24eb | _TODO_ || `00 00` |
| 24ec | _TODO_ || `00 15 b6 89` |
| 24ed | _TODO_ || `03 03 03 00 05 04 03 03 0a 06` |
| 24ef | _TODO_ || `00 ab 00 ae 00 82 00 28 00 21 00 24 00 09 00 6e 00 03 00 05` |
| 24f0 | _TODO_, odometer snapshot when? doesn't update on every trip || `01 fd a9` |
| 24f1 | _TODO_, odometer snapshot when? doesn't update on every trip || `01 fd ad` |
| 24f2 | _TODO_ || `00 00` |
| 24f3 | _TODO_ || `09 1a` |
| 24f4 | _TODO_ || `00` |
| 24f5 | _TODO_ || `01` |
| 24f6 | _TODO_ || `00` |
| 24f7 | _TODO_ || `1f db` |
| 24f8 | _TODO_ || `1f db` |
| 24f9 | _TODO_ || `1f db` |
| 24fa | _TODO_ || `00 00` |
| 24fc | _TODO_ || `80 00` |
| 2500 | _Supported PIDs 2501-251f bitmask_ || `00 00 00 01` |
| 2520 | _Supported PIDs 2521-253f bitmask_ || `ff f8 00 01` |
| 2521 | _TODO_ || `80 d2` |
| 2522 | _TODO_ || `01` |
| 2523 | _TODO_ || `00` |
| 2524 | _TODO_ || `00 00` |
| 2525 | _TODO_ || `80 00` |
| 2526 | _TODO_ || `80 00` |
| 2527 | _TODO_ || `01 f4` |
| 2528 | _TODO_ || `25 1c` |
| 2529 | _TODO_ || `00 00` |
| 252a | _TODO_ || `00 00` |
| 252b | _TODO_ || `00 00` |
| 252c | _TODO_ || `05 78` |
| 252d | _TODO_ || `00` |
| 2540 | _Supported PIDs 2541-255f bitmask_ || `ff ff e0 01` |
| 2541 | _TODO_ || `80 00` |
| 2542 | _TODO_ || `80 00` |
| 2543 | Car odometer absolute value (mileage) | km | `01 fd ad` |
| 2544 | _TODO_ || `00` |
| 2545 | _TODO_ || `80 00` |
| 2546 | _TODO_ || `00 00` |
| 2547 | _TODO_ || `01` |
| 2548 | _TODO_ || `01` |
| 2549 | _TODO_ || `00` |
| 254a | Same as 242c? _TODO_ || `06 19` |
| 254b | _TODO_ || `00 00` |
| 254c | _TODO_ || `00 00` |
| 254d | _TODO_ || `00 00` |
| 254e | _TODO_ || `00` |
| 254f | _TODO_ || `00` |
| 2550 | _TODO_ || `01` |
| 2551 | _TODO_ || `00` |
| 2552 | _TODO_ || `00 00` |
| 2553 | _TODO_ || `00` |
| 2560 | _Supported PIDs 2561-257f bitmask_ || `00 00 00 11` |
| 257c | _TODO_ || `80 00` |
| 2580 | _Supported PIDs 2581-259f bitmask_ || `00 00 00 00` |
| 2800 | _Supported PIDs 2801-281f bitmask_ || `c5 fd e0 21` |
| 2801 | _TODO_ || `00 00` |
| 2802 | _TODO_ || `01 20` |
| 2806 | _TODO_ || `00 00` |
| 2808 | _TODO_ || `80 00` |
| 2809 | _TODO_ || `00 00` |
| 280a | _TODO_ || `00 00` |
| 280b | _TODO_ || `00 00` |
| 280c | _TODO_ || `00 00` |
| 280d | _TODO_ || `00 00` |
| 280e | _TODO_ || `00 00` |
| 2810 | _TODO_ || `80 00` |
| 2811 | _TODO_ || `80 00` |
| 2812 | _TODO_ || `80 00` |
| 2813 | _TODO_ || `80 00` |
| 281b | _TODO_ || `01` |
| 2820 | _Supported PIDs 2821-283f bitmask_ || `34 00 00 01` |
| 2823 | _TODO_ || `00` |
| 2824 | _TODO_ || `00` |
| 2826 | _TODO_ || `00 00` |
| 2840 | _Supported PIDs 2841-285f bitmask_ || `00 00 00 01` |
| 2860 | _Supported PIDs 2861-287f bitmask_ || `01 00 09 c1` |
| 2868 | _TODO_ || `00 4a ff 9c` |
| 2875 | _TODO_ || `00 00` |
| 2878 | _TODO_ || `00` |
| 2879 | _TODO_ || `00 00` |
| 287a | _TODO_ || `00` |
| 2880 | _Supported PIDs 2881-289f bitmask_ || `00 00 00 00` |
| 2c00 | _Supported PIDs 2c01-2c1f bitmask_ || `30 00 00 01` |
| 2c03 | _TODO_ || `01` |
| 2c04 | _TODO_ || `01` |
| 2c20 | _Supported PIDs 2c21-2c3f bitmask_ || `00 10 00 01` |
| 2c2c | _TODO_ || `00` |
| 2c40 | _Supported PIDs 2c41-2c5f bitmask_ || `00 00 00 00` |
| 3400 | _Supported PIDs 3401-341f bitmask_ || `00 3f ff f8` |
| 340b | _TODO_ || `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00` |
| 340c | _TODO_ || `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00` |
| 340d | _TODO_ || `00 00 00 00 00 00 00 00 00 00 00 00 00 00` |
| 340e | _TODO_ || `00 00 00 00 00 00 00 00 00 00 00 00 00 00` |
| 340f | _TODO_ || `00 00 00 00 00 00 00 00 00 00 00 00 00 00` |
| 3410 | _TODO_ || `00 00 00 00 00 00 00 00 00 00 00 00 00 00` |
| 3411 | _TODO_ || `00 00 00 00 00 00 00 00 00 00 00 00 00 00` |
| 3412 | _TODO_ || `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00` |
| 3413 | _TODO_ || `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00` |
| 3414 | _TODO_ || `00 00` |
| 3415 | _TODO_ || `00 00` |
| 3416 | _TODO_ || `00 00` |
| 3417 | _TODO_ || `00 00` |
| 3418 | _TODO_ || `00 00` |
| 3419 | _TODO_ || `00 00` |
| 341a | _TODO_ || `00 00` |
| 341b | _TODO_ || `00 00` |
| 341c | _TODO_ || `00 00` |
| 341d | _TODO_ || `00 00` |
| ef00 | _Supported PIDs ef01-ef1f bitmask_ || `58 3b 00 01` |
| ef02 | _TODO_ || `00 00` |
| ef04 | _TODO_ || `00` |
| ef05 | _TODO_ || `00` |
| ef0b | _TODO_ || `00` |
| ef0c | _TODO_ || `00 00` |
| ef0d | _TODO_ || `00` |
| ef0f | _TODO_ || `00` |
| ef10 | _TODO_ || `00 00` |
| ef20 | _Supported PIDs ef21-ef3f bitmask_ || `20 00 80 00` |
| ef23 | _TODO_ || `00 00` |
| ef31 | _TODO_ || `00 00` |
| f0e0 | _Supported PIDs f0e1-f0ff bitmask_ || `00 00 00 02` |
| f0ff | MISSING (query returns error) |||
| f180 | _Supported PIDs f181-f19f bitmask_ || `00 01 00 00` |
| f190 | _TODO_ || `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00` |
| f400 | _Supported PIDs f401-f41f bitmask_ || `98 3b 80 11` |
| f401 | Monitor status since DTCs cleared || `00 64 80 00` |
| f404 | Calculated engine load | 100/255 % / LSB | `00` |
| f405 | Engine coolant temperature | °C (offset by 40 (decimal)) | `54` |
| f40b | Intake manifold absolute pressure | kPa | `5e` |
| f40c | Engine speed | 0.25 RPM / LSB | `00 50` |
| f40d | Vehicle speed | km/h | `00` |
| f40f | Intake air temperature | °C (offset by 40 (decimal)) | `55` |
| f410 |  [Mass air flow sensor (MAF)](https://en.wikipedia.org/wiki/Mass_airflow_sensor) air flow rate | 0.01 grams/sec / LSB || `00 00` |
| f411 | Throttle position | 100/255 % / LSB | `00` |
| f41c | OBD standards this vehicle conforms to || `06` |
| f420 | _Supported PIDs f421-f43f bitmask_ || `a0 01 80 00` |
| f421 | Distance traveled with malfunction indicator lamp (MIL) on | km | `00 00` |
| f423 | [Fuel Rail](https://en.wikipedia.org/wiki/Fuel_rail) Gauge Pressure | 10 kPa / LSB | `00 00` |
| f430 | Warm-ups since codes cleared | literal | `13` |
| f431 | Distance traveled since codes cleared | km | `03 7c` |
| f600 | _Supported PIDs f601-f61f bitmask_ || `00 00 00 01` |
| f620 | _Supported PIDs f621-f63f bitmask_ || `00 00 80 01` |
| f631 | _TODO_ || `81 03 00 64 00 5a 00 c8 31 82 af 00 00 00 00 00 00` |
| f640 | _Supported PIDs f641-f65f bitmask_ || `00 00 00 01` |
| f660 | _Supported PIDs f661-f67f bitmask_ || `00 00 00 01` |
| f680 | _Supported PIDs f681-f69f bitmask_ || `00 00 00 01` |
| f6a0 | _Supported PIDs f6a1-f6bf bitmask_ || `00 00 40 01` |
| f6b2 | _TODO_ || `86 01 00 3d 00 00 00 18 b2 87 01 00 01 00 00 00 ff` |
| f6c0 | _Supported PIDs f6c1-f6df bitmask_ || `00 00 00 00` |
| f6e0 | _Supported PIDs f6e1-f6ff bitmask_ || `00 00 00 00` |
| f800 | _Supported PIDs f801-f81f bitmask_ || `55 00 00 00` |
| f802 | [Vehicle Identification Number](https://en.wikipedia.org/wiki/Vehicle_Identification_Number) (VIN) || `01 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff` |
| f804 | Calibration ID || `02 32 33 37 31 30 42 42 33 31 41 00 00 00 00 00 00 32 33 37 30 31 42 42 33 36 41 00 00 00 00 00 00` |
| f806 | Calibration Verification Numbers (CVN) Several CVN can be output (4 bytes each) the number of CVN and CALID must match || `01 44 d8 62 a7` |
| f808 | _TODO_ || `0a e8 53 d2` |
| fd00 | _Supported PIDs fd01-fd1f bitmask_ || `03 cf 1f ff` |
| fd07 | _TODO_ || `9f de` |
| fd08 | _TODO_ || `71 de` |
| fd09 | _TODO_ || `79 46` |
| fd0a | _TODO_ || `74 fe` |
| fd0d | _TODO_ || `45 48 48 48` |
| fd0e | _TODO_ || `46 47 48 48` |
| fd0f | _TODO_ || `46 4c 4c 4b` |
| fd10 | _TODO_ || `48 47 47 47` |
| fd14 | _TODO_ || `3f f5 3f f5 3f f5 3f f5 3f f5 3f f5 40 17 40 0e 3f df 3f df 3f f8 40 20 3f f2 3f f2 40 04 40 0e` |
| fd15 | _TODO_ || `01` |
| fd16 | _TODO_ || `01` |
| fd17 | _TODO_ || `01` |
| fd18 | _TODO_ || `01` |
| fd19 | _TODO_ || `8e e7` |
| fd1a | _TODO_ || `8f 08` |
| fd1b | _TODO_ || `8c af` |
| fd1c | _TODO_ || `8c a7` |
| fd1d | _TODO_ || `41 37` |
| fd1e | _TODO_ || `3f f2 3f f2 3f f2 3f f2 3f f2 3f f2 40 1a 40 12 40 12 40 12 40 24 40 2d 3f fb 3f fb 40 02 40 0a` |
| fd1f | _TODO_ || `3f f1 3f f1 3f f1 3f f1 3f f1 3f f1 40 1d 40 15 3f fa 3f fa 40 31 40 2c 3f fa 3f fa 40 12 40 10` |
| fd20 | _Supported PIDs fd21-fd3f bitmask_ || `00 00 00 1f` |
| fd3c | _TODO_ || `41 36` |
| fd3d | _TODO_ || `42 46` |
| fd3e | _TODO_ || `41 31` |
| fd3f | _TODO_ || `3f f5 3f f5 3f f5 3f f5 3f f5 3f f5 40 0e 40 05 3f f5 3f f5 40 15 40 18 40 07 40 07 40 04 40 03` |
| fd40 | _Supported PIDs fd41-fd5f bitmask_ || `60 0d fe 01` |
| fd42 | _TODO_ || `00` |
| fd43 | _TODO_ || `00` |
| fd4d | _TODO_ || `00 00` |
| fd4e | _TODO_ || `00` |
| fd50 | _TODO_ || `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 05 00 04 00 00 00 00 00 05 00 1d 00 0b 00 00 00 08 00 ca 00 4f 00 01 00 03 00 bd 00 bc 00 08 00 00 00 15 00 1e 00 18 00 00 00 03 00 07 00 00` |
| fd51 | _TODO_ || `00 00 00 00 00 00 00 00 00 cc c8 64 00 cc c3 73 00 cc e5 78 00 cc eb db 00 cc ef 6d 00 cc e6 aa` |
| fd52 | _TODO_ || `82 d8 83 22 83 17 81 40 82 d8 83 22 83 17 81 40 82 d8 83 22 83 17 81 40 81 05 82 ca 83 17 81 40 81 9c 83 31 82 29 81 40 81 14 82 29 82 f1 81 24 7f 1d 81 62 81 c2 81 bf 7f 1d 81 61 82 4e 7f 1d` |
| fd53 | _TODO_ || `00` |
| fd54 | _TODO_ || `00 00 00 00 00 02 00 03 00 04 00 05 00 05 00 06` |
| fd55 | _TODO_ || `00 00 00 00 00 00 00 00 00 cc c8 58 00 cc c8 3a 00 cc b8 ff 00 cc e3 13 00 cc d7 41 00 cc e5 c0` |
| fd56 | _TODO_ || `7b 2f 7b 2f 7b 2f 7b 84 7e 56 7c ba 7d cf 80 3f` |
| fd57 | _TODO_ || `00` |
| fd60 | _Supported PIDs fd61-fd7f bitmask_ || `00 81 0f 81` |
| fd69 | _TODO_ || `00 57` |
| fd70 | _TODO_ || `00` |
| fd75 | _TODO_ || `00 00` |
| fd76 | _TODO_ || `00 00` |
| fd77 | _TODO_ || `80 00` |
| fd78 | _TODO_ || `2c f5` |
| fd79 | _TODO_ || `00` |
| fd80 | _Supported PIDs fd81-fd9f bitmask_ || `57 f8 0f f9` |
| fd82 | _TODO_ || `26 e0` |
| fd84 | _TODO_ || `14 c9` |
| fd86 | _TODO_ || `14 e0` |
| fd87 | _TODO_ || `00 00` |
| fd88 | _TODO_ || `52 e1` |
| fd89 | _TODO_ || `23 78` |
| fd8a | _TODO_ || `19 80` |
| fd8b | _TODO_ || `22 75` |
| fd8c | _TODO_ || `00 00` |
| fd8d | _TODO_ || `54 62` |
| fd95 | _TODO_ || `00 00` |
| fd96 | _TODO_ || `00 00` |
| fd97 | _TODO_ || `00 00` |
| fd98 | _TODO_ || `00 00` |
| fd99 | _TODO_ || `00 00` |
| fd9a | _TODO_ || `00 00` |
| fd9b | _TODO_ || `80 00` |
| fd9c | _TODO_ || `80 00` |
| fd9d | _TODO_ || `80 00` |
| fda0 | _Supported PIDs fda1-fdbf bitmask_ || `30 00 07 f0` |
| fda3 | _TODO_ || `00` |
| fda4 | _TODO_ || `00` |
| fdb6 | _TODO_ || `00 00` |
| fdb7 | _TODO_ || `00 00` |
| fdb8 | _TODO_ || `00 19` |
| fdb9 | _TODO_ || `00 00` |
| fdba | _TODO_ || `00 19` |
| fdbb | _TODO_ || `00` |
| fdbc | _TODO_ || `00 20 00 20 00 20 00 20` |
| fee0 | _Supported PIDs fee1-feff bitmask_ || `00 00 00 02` |
| feff | MISSING (query returns error 12) |||

## BCM non-standard service 21 PIDs

These are some manufacturer-specific service 21 PID/CIDs that can be queried for BCM information.  They're read in the same way as services 01 and 09 but only within a diagnostic session (command `10 c0`).  In a normal session the reads will return error 0x80.  SID 0x21 is known as "Read Data by Local Identifier" in some protocols.

| PID | Meaning | Captured value |
| --: | --- | --- |
| 00 | _Supported PIDs 01-1f bitmask_ | `c0 00 00 01` |
| 01 || `d7 0a fc 95 28 04 20 d5 11 34 21 14 1e 98 14 cd 06 02 00 89 b8 02 0f bc 02 80 98 a0 00 00 40 88 88 00` |
| 02 || `42 52 30 33 30` (ascii BR030) |
| 20 | _Supported PIDs 21-3f bitmask_ | `80 00 00 01` |
| 21 | _TODO_ | `90 8c df` |
| 40 | _Supported PIDs 41-5f bitmask_ | `df fd 90 00` |
| 41 | `A.8`: IGN in ON, `A.7`: emergency lights switch, `A.6`: right turn switch, `A.5`: left turn switch, `A.4`: brake pedal (brake light switch) | `80 00 00 00 00` |
| 42 | `A.8`: IGN in ON, `A.7`: key inserted, `A.6`: driver door open, `A.5`: passenger door open, `A.4`: rear right door open, `A.3`: rear left door open`, `C.4`: IGN in ACC or ON, others _TODO_ | `c0 00 08 00 00 00 00 00` |
| 44 | `A.8`: IGN in ON, `A.7`: key inserted, `A.6`: driver door open, `A.4`: lights selector in the 3rd or 4th position, i.e. low-beam lights and/or position lights are on -- but 0 when the same lights are on in AUTO or triggered by the fog lights switch -- this bit strictly depends on the selector position, `A.1`: passenger door open, `B.8`: rear right door open, `B.7`: rear left door open`, `B.6`: trunk door open, others _TODO_ | `c0 00 00 00 00` |
| 45 | `A.8` and `A.7`: IGN in ON or START, `A.6-4`: 0b000: front wipers selector in OFF (2nd position), 0b001: front wipers selector in AUTO (3rd position), 0b010: front wipers selector in SLOW or one-shot (4th or 1st position), 0b100: front wipers selector in FAST (5th position), `B.5`: front wipers NOT moving right now, `E.8-7`: 0b00: rear wiper selector in OFF, 0b01: rear wiper selector in INTERMITTENT, 0b10: rear wiper selector in FAST, `E.5`: rear wiper moving right now, `E.4`: reverse gear, other bits _TODO_ | `c0 f0 00 00 00 00 00 00 00` |
| 46 | `A.8`: IGN in ACC or ON, `A.7`: IGN in ACC, other bits _TODO_ | `c0 00 00 00 00` |
| 47 | `A.8`: IGN in ON or START, `A.7`: key inserted, `A.4`: driver door open, `A.3`: passenger door open, `A.2`: rear right door open, `A.1`: rear left door open, `B.8`: trunk door open, `B.6`: IGN in ACC or ON, other bits _TODO_ | `c0 20 00 00 00 00 00 00 00 00` |
| 48 | `A.8`: IGN in ON or START, `A.7`: key inserted, `A.2`: IGN in ON or ACC, `B.7`: driver door open, `B.6`: passenger door open, `B.5`: rear right door open, `B.4`: rear left door open, `B.3`: trunk door open, other bits _TODO_ | `e0 00 00 00 c0 00 00 00 00` |
| 49 | `A.8`: IGN in ON or START, `A.7`: IGN in ON or ACC, `A.6`: high-beam switch (stick forward) on, `A.5-2`: 0b0000: light selector in OFF, 0b0001: light selector in AUTO, 0b0010: light selector in position/parking/side position, 0b1110: light selector in low-beam position, `A.1`: high-beam temporary switch (stick towards driver) on, `B.8-7`: 0b00: fog lights selector in 1st position (OFF), 0b10: fog lights selector in 2nd position (front fog lights), 0b11: fog lights selector in 3rd position (toggle rear fog lights+front on) `B.6`: driver door open, `B.5`: passenger door open, `B.4`: rear right door open, `B.3`: rear left door open, `B.2`: trunk door open, `B.1`: right turn switch on, `C.8`: left turn switch on, `C.3`: engine running, `G.8-7`: dashboard lights? other bits _TODO_ | `c0 00 00 00 00 00 40 1e 00 00` |
| 4a | `A.8`: right turn switch on, `A.7`: left turn switch on, `A.6`: high-beam switch (stick forward) on, `A.5-3,1`: 0b000x0: light selector in OFF, 0b000x1: light selector in AUTO, 0b001x0: light selector in position/parking/side position, 0b111x0: light selector in low-beam position, `A.2`: high-beam temporary switch (stick towards driver) on, `B.8-7`: 0b00: fog lights selector in 1st position (OFF), 0b10: fog lights selector in 2nd position (front fog lights), 0b11: fog lights selector in 3rd position (toggle rear fog lights+front on), `B.6-4`: 0b000: front wipers selector in OFF (2nd position), 0b001: front wipers selector in AUTO (3rd position), 0b010: front wipers selector in SLOW or one-shot (4th or 1st position), 0b100: front wipers selector in FAST (5th position), `B.3`: front wiper sprinkler switch on, `C.5-4`: 0b00: rear wiper selector in OFF, 0b01: rear wiper selector in INTERMITTENT, 0b10: rear wiper selector in FAST, `C.3`: rear wiper sprinkle switch on, other bits _TODO_ | `00 00 e0 00 00 00 00` |
| 4b | `B.4`: IGN in ON or START, `B.3`: IGN in ON or ACC, `B.2`: key inserted, other bits _TODO_ | `00 0e 80 00 00` |
| 4c | `A.8`: IGN in ON or START, `A.7`: blower fan on, `A.6`: A/C on, other bits _TODO_ | `80 00 00 00 00` |
| 4d | `A.8`: key inserted, `A.1`: IGN in ON or START, other bits _TODO_ | `81 00 00 00 00 00 00 00` |
| 4e | `A.8`: IGN in ON or START, `A.7`: IGN in ON or ACC, `A.6`: key inserted, other bits _TODO_ | `e0 00 00 00 00 00 00 00 00` |
| 50 | `A.8`: IGN in ON or START, `A.6`: key inserted, other bits _TODO_ | `d0 40 c0 00 00` |
| 51 | `A.8`: IGN in ON or START, `A.7`: brake pedal depressed (braking light on), other bits _TODO_ | `80 00 00 00 00` |
| 54 | `A.6`: blower fan on, `A.5`: TODO, `A.2,1`: TODO (1 after engine stopped, 2 when running?), `B.8-1`: might be coolant temperature, `C.8-1` might be battery voltage, `E.8-1`: engine revolutions / RPMs, `F.8-1` might be intake temperature, other bits _TODO_ | `81 53 9f 00 00 3a 00 00 00 00 00` |
| 82 || `32 4c 01 01 01 ff 01 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 01 ff ff ff ff ff ff ff ff ff ff ff 00` |
| 83 || `42 52 30 32 41 45 44 03 17 00 05 03 68 01 11 00 00 00 00 00 00 00 00 80` (ascii BR02AED) |
| 90 || `0b b8` |
| 91 || `03 e8` |
| 92 || `05` |
| 94 || `00` |

## Other ECUs non-standard service 21 PIDs

These are some manufacturer-specific service 21 PID/LIDs that can be queried for ECU information.  They're read in the same way as services 01 and 09 but there are no supported PID bitmasks for the LIDs after 0x80.  SID 0x21 is known as "Read Data by Local Identifier" in some protocols.

Address **745** (Engine ECU / ECM / ECMD?) LIDs:
| PID | Name | Captured value |
| --: | --- | --- |
| 80 || `42 42 33 36 41 46 34 42 45 30 36 36 39 52 00 f4 31 0e e6 a1 01 01 01 88` (ascii BB36AF4BE0669R) |
| 81 | VIN | `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00` |
| 82 || `95 03 00 00 00 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff` |
| 84 || `34 42 45 46 31 33 30 32 31 33 35 31 33 30 35 32 31 20 20 20` (ascii 4BEF1302135130521) |
| f0 | Product Number long | `33 36 31 30 52 46 34 42 45 30 36 36 39 52 00 f4 31 00 00 00 01 01 00 88` (ascii 3610RF4BE0669R)
| f1 || `30 30 30 30 30 46 46 4f 49 58 20 20 20 20 20 00 13 02 13 01 58 5c e5 63` (ascii 00000FFOIX) |
| fc || `33 36 31 30 52 46 34 42 45 30 36 36 39 52 00 f4 31 00 00 00 01 01 00 88` (ascii 3610RF4BE0669R) |
| fd || `30 30 30 30 30 46 46 4f 49 58 20 20 20 20 20 00 13 02 13 01 58 5c e5 63` (ascii 00000FFOIX) |
| fe | Product Number | `42 42 33 36 41 46 34 42 45 30 36 36 39 52 00 f4 31 0e e6 a1 02 01 00 88` (ascii BB36AF4BE0669R) |
| ff | HW Number| `42 42 33 31 41 4e 4d 55 4b 00 45 4f 4c 50 47 01 13 02 26 13 22 5c b8 12` (ascii BB31ANMUK EOLPG) |

Address **74d** (IPDM-E/R or BCM?) LIDs:
| PID | Name | Captured value |
| --: | --- | --- |
| 00 | _Supported PIDs 01-1f bitmask_ | `c0 01 00 00` |
| 01 || `d5 00 00 00 00` |
| 02 || `f0 40 27 c3 00 bc 05 d8 04` |
| 10 || `2f 30 ff 0f 32 00 be 9d 00 cc 01 00 00 00 00 00 00 f0` |
| 82 || `32 4c ff 01 01 ff ff 01 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff` |
| 83 || `4a 44 30 32 44 0d 44 08 07 00 00 00 00 00 76 35 2e 30 72 31 00 00 00 80` (ascii JD02D v5.0r1) |
| 91 || `0f 0a 14 0a 02` |
| 93 || `78 9e 28 03 7c 06 02 13 3b 00 00 00 00 00 00 00 00 00 00 00` |
| 95 || `00` |
| 97 || `00` |

Address **742** (EPS?) LIDs -- requires diagnostic session `0xc0`:
| PID | Name | Captured value |
| --: | --- | --- |
| 00 | _Supported PIDs 01-1f bitmask_ | `c4 00 00 00` |
| 01 || `0b f8 03 00 00 00 00 00 00 00 43 64 00 00 80 00 00 44 00 00 00` |
| 02 || `01` |
| 06 || `94 94 94 94 94 a2 a2 a2 a2 a2 a2` |
| 82 || `32 4c 01 00 01 00 01 00 ff ff ff ff ff ff ff ff ff ff ff ff ff 00 ff ff ff ff ff ff` |
| 83 || `42 52 30 31 44 42 41 05 02 39 43 30 31 35 87 00 00 00 01 01 00 00 00 80` (ascii BR01DBA 9C015) |
| 84 || `31 36 33 33 32 31 38 32 34 37 ff ff ff ff ff ff ff ff ff ff` (ascii 1633218247) |

Address **743** (Instrument panel/Odometer) LIDs:
| PID | Name | Captured value |
| --: | --- | --- |
| 00 | _Supported PIDs 01-1f bitmask_ | `e0 00 00 00` |
| 01 | Bytes 4-5 and 6-7: speedometer, bytes 8-10: odometer, bytes 11-12: engine RPMs (0.125 RPM unit), byte 14: engine coolant temperature - `6d` middle of gauge, `5b` bottom, bytes 17,18,33: dashboard lights, byte 17 bit 7: ESP/TCS off, bit 5: change oil?, bit 4: any door open, byte 33 bit 8: handbrake, bit 4: driver seatbelt undone, byte 63: cruise control state - `10` - speed limiter engaged, `20` - setting limit speed, `50` - setting cruise speed, byte 64: cruise control speed setpoint - `fe` when unset | `00 00 00 00 00 00 00 01 fd ec 00 00 33 45 00 00 10 0c 00 00 40 00 00 00 00 00 00 00 00 00 00 00 98 00 01 1e a0 2f 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 fe 00 00 41 c8 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00` |
| 02 || `fc fb fe 00 20 80 00 10 00 00 00 00 02 9a 07 e0 80 00 00 00 00 00 e4 00 00 00 00 00` |
| 03 || `03 03 03 00 ff ff 00 00 27 ff ff ff ff ff ff ff ff ff ff 27 ff ff ff ff ff 00 ff ff ff ff ff ff ff ff ff ff 00 ff ff ff ff ff ff ff ff ff ff ff 27 ff ff ff ff ff` |
| 82 || `00 00 ff ff 01 00 00 00 ff ff ff ff ff ff ff ff ff ff ff ff ff 00 ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff ff` |
| 83 || `42 52 35 30 45 14 46 02 18 01 06 06 08 06 00 01 00 16 00 01 ff ff ff 80` (ascii BR50E) |
| f0 | Product Number long | `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00` |
| f1 || `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00` |
| fe | Product Number | `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00` |
| ff | HW Number| `00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00` |

## Non-standard service 0x23 (ROM dump)

Apparently the Engine ECU (address **7e0**) and the BCM (address **745**) implement service 0x23 but I've been unable to get any data out of them so far.  SID 0x23 is known as "Read Memory by Address" in some procotols.  The message syntax is [as described in this blog post](https://leftoverpi.com/2020/01/23/reading-a-370z-ecu-rom/), i.e. the command consists of the length byte (`07`), the Service ID (`23`), the 32-bit big-endian read start address and the 16-bit big-endian number of bytes to read, e.g. `07 23 00 00 12 34 00 3f` to request 63 bytes starting at memory address 0x1234.  The command returns error `0x7f` if attempted in the normal session mode.  In that (370Z-specific) blog post those commands are executed in the `0xfb` session (`elevatedDiagSession`) but that mode is not supported on the J10 Qashqai.  However, executing the command in session `0xc0` mode (`startDiagSession`) resolves the `0x7f` error and instead error `0x12` (`subFunctionNotSupported-invalidFormat`) is then returned.  In session `0x85` error `0x11` is returned.  More research needed.

## TO DO

Write the [DBC file](https://www.csselectronics.com/pages/can-dbc-file-database-intro) for the periodic messages and the information PID queries perhaps using the free online editor.

Find out a message combination to stop the engine, perhaps by imitating the periodic messages.

Data available on some cars but not available on Nissan Qashqai J10 or yet to be decoded:
* Fuel level
* Outside temperature
* Outside brightness level (bright/dark, from the Light & Rain sensor)
* Seatbelt status and occupancy sensor status other than driver
* Car VIN
* Current time and/or date
* Current GPS lat/lon/number of satellites
