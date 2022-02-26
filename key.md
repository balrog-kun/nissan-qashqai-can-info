# Messages 342 / 512 (PIN exchange)

Frames with Msg IDs 342 / 512 (two of each) can be seen exchanged by ECUs when ignition is moved from ACC to ON.  Frame 342 is a request and frame 512 is the response from another ECU.  One of the ECUs is likely the immobiliser.  A total of 4 frames is sent:

| ID | Contents | Description |
| --: | --- | --- |
| 342 | `03 20 xx yy` | Request verification, challenge is `xx yy` (16-bit number) |
| 512 | `03 23 uu vv` | Respond with response code `uu vv` (16-bit number) |
| 342 | `03 2c ff ff` | Acknowledge |
| 512 | `03 2c ff ff` | Acknowledge |

The initial `03` byte might be part of the command code (together with the `2x` byte that follows it) or a length indication for flow control.  The second byte (`2x`) determines the action requested or the response.

There's a 1:1 mapping between the challenge and response values.  The mapping is unrelated to the key remote or transponder codes, and doesn't seem to be dependent on the BCM PIN or another vehicle-specific value.  I've only tested it on my Qashqai J10 from 2012 though.  The response code calculation is simple enough that there doesn't seem to be much space for vehicle-specific logic.

After the initial exchange is completed, a new exchange can be started without restarting any ECUs.  The dashboard immobiliser light comes up after the initial 2 messages if no acknowlegements are exchanged in time.  Exactly 99 exchanges can be started (99 challenge/response round-trips) before the ECU stops responding.  It will also start blinking the immobiliser dashboard light at about 5Hz and will stay that way until reset by turning the key away from the ON position.  Turning it to ACC and back ON is enough to reset the ECU.  The engine will start just fine after initial exchange even if the immobiliser light is steady on or blinking.

The file [calc_key.py](calc_key.py) contains a basic implementation of the challenge response calculation that gives the exact `uu vv` response values for a given `xx yy` challenge value that my car would give.  The code is python 3 (for big integers) and takes an integer (big-endian `xx yy`) and returns an integer (big-endian `uu vv`).  The lower byte of the response is easy to calculate.  The upper byte on the other hand has an obvious relation between the its two halves or nibbles, but I haven't yet found a simple way to calculate it.  I resorted to using bitwise operations that reduce the number of possible values and using a small look-up table for the remaining unobvious values.  There's probably a simple arithmetic operation that gives the same result.
