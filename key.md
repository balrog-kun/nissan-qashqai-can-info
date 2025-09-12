# Messages 342 / 512 (PIN exchange)

Frames with Msg IDs 0x342 / 0x512 can be seen exchanged by ECUs when ignition is moved from ACC to ON and during key programming.  Frame 512 is a request and frame 342 is the response from another ECU.  One of the ECUs is likely the BCM's immobiliser function.

Each frame is 4 bytes.  The initial `03` byte is just the ISO 15765-2 CAN-TP header indicating a 3-byte payload follows.

The second byte (command byte) determines the action requested or the response and is followed by either: a 2-byte code (parameter) or `ff ff` when the command takes no parameter.  Part or all of the command byte might be designed as a bitfield but it's hard to speculate.

| Command | Parameter | In 512 | In 342 | Notes |
| --: | --- | --- | --- | --- |
| `20` | ✓ | ✓ |   ||
| `23` | ✓ |   | ✓ ||
| `2a` |   |   | ✓ | BCM unlock / Start key programming? |
| `2c` |   | ✓ | ✓ | Ack / Success / End of exchange (when switching to ON) |
| `31` |   |   | ✓ | Ack / Success (when progrmaming key) |
| `a4` |   |   | ✓ | Nack / Error (when programming key)  |
| `ad` | ✓ |   | ✓ ||
| `b0` |   | ✓ | ✓ | End of exchange (when programming key) |

# ACC -> ON exchange

When ignition moves to ON, a total of 4 frames is sent:

| ID | Contents | Description |
| --: | --- | --- |
| 342 | `03 20 xx xx` | Request authentication, challenge is `xx xx` (16-bit number) |
| 512 | `03 23 yy yy` | Respond with response `yy yy` (16-bit number) |
| 342 | `03 2c ff ff` | Acknowledge |
| 512 | `03 2c ff ff` | Acknowledge |

There's a 1:1 mapping between the challenge and response values.  The mapping seems to be related to the key transponder code, and may or may not be dependent on the BCM PIN or another vehicle-specific value.  I've only tested it on my Qashqai J10 from 2012.

After the initial exchange is completed, a new exchange can be started without restarting any ECUs by injecting a new 512 command.  The dashboard immobiliser light comes up after the initial 2 messages if no acknowlegements are exchanged in time.  Exactly 99 exchanges can be started (99 challenge/response round-trips) before the ECU stops responding.  It will also start blinking the immobiliser dashboard light at about 5Hz and will stay that way until reset by turning the key away from the ON position.  Turning it to ACC and back ON is enough to reset the ECU.  The engine will start just fine after initial exchange even if the immobiliser light is steady on or blinking, unlike when a non-transponder key is inserted which also causes the same signal to light up but engine won't start.

The file [calc_key.py](calc_key.py) contains an implementation of the challenge response calculation that gives the exact `yy yy` response values for a given `xx xx` challenge value that my car would give with one specific key, but it has since changed after replacing that key and reprogramming the immobiliser at a shop.  The python code no longer matches the values seen in my car but the values still show most of the same patterns.

The code is python 3 (for big integers) and takes an integer (big-endian `xx xx`) and returns an integer (big-endian `yy yy`).  The lower byte of the response is easy to calculate.  The upper byte on the other hand has a very obvious relation between its two halves or nibbles, but I haven't yet found a simple way to calculate it.  I resorted to using bitwise operations that reduce the number of possible values and using a small look-up table for the remaining unobvious values.  There's probably a simple arithmetic operation that gives the same result.

# Key programming

It seems that key programming is triggered with the SID 31 (Remote Activation of Routine) with routine local ID `01` and parameter `01` (`31 01 01`) sent to the ECU (frame ID 0x7e0).  That command returns an error before the frames 0x342 / 0x512 are exchanged though so more investigation is needed.  That command needs to be sent in session 0xc0 or 0x81, again more investigation needed.  The BCM (frame ID 0x745) may need to also be a in a specific mode for this to work.  The frames 0x342 and 0x512 are exchanged nearly half a second after the SID 31 routine.  There seems to be some exchange even in the failure case so it seems unlikely that this is just the BCM re-pairing with the ECU/ECM.

The ignition should be in ON.  On success, the immobiliser dashboard light flashes 5 times, on failure it stays on.
