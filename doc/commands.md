# Commands

The commands sent by the host computer must respect the following structure:

```
[Start byte][ID byte][Data byte 0][Data byte 1]
```

The commands are sent through the USB serial interface of the Teensy LC

Each frame must contain exactly 4 bytes. The bytes used are for the most part printable ascii characters to allow the user to easily send commands from a basic serial monitor

## Start byte

The start byte must be `0x26` (`"&"` in ascii encoding)

## ID byte

This byte is used to filter commands for the controller from commands for the gateway itself

- `0x63` (`"c"`): command for the controller
- `0x67` (`"g"`): command for the gateway

## Available commands

The data bytes contain the commands. Here are the commands available. All the commands return their return value + a line feed character (`0x0A`) + a new line character (`0x0D`)

### Send BONJOUR string

Makes the controller or the gateway send an identification string over serial

*Target:* controller, gateway<br>
*Data byte 1:* `0x42` (`"B"`)<br>
*Data byte 2:* `0x00` unused, can be any value<br>
*Returns:* `"LAUNCHPADCONTROLLER"`

### Activate/deactivate the main outputs

Set one of the main outputs high or low

*Target:* controller<br>
*Data byte 0:*
- `0x61` (`"a"`): for output 1 [P1]
- `0x62` (`"b"`): for output 2 [P2]
- `0x63` (`"c"`): for output 3 [P3]
- `0x64` (`"d"`): for output 4 [P4]

*Data byte 1:*
- `0x00`: set output low. A logic AND is applied to the LSb so (`"0"`) works to set the output low
- `0x01`: set output high. A logic AND is applied to the LSb so (`"1"`) works to set the output high

*Returns:* State frame (see **State frame** [below](#state-frame))

### Turn the servos

Update the rotation of one of the servos

*Target:* controller<br>
*Data byte 0:*
- `0x6A` (`"j"`): servo 1 [P5]
- `0x6B` (`"k"`): servo 2 [P6]
- `0x6C` (`"l"`): servo 3 [P7]

*Data byte 1:*
- `0x00` - `0xB4`: angle of the servo

*Returns:* State frame (see **State frame** [below](#state-frame))

The pulse width is set to be between adjustable boundaries. You can modify them in `platformio.ini`

```ini
...
[env]
...
build_flags = 
  ...
  -D SERVO_MIN_PULSE_WIDTH=1000 ; microseconds
  -D SERVO_MAX_PULSE_WIDTH=2000 ; microseconds
```

## State frame

The Launchpad Controler responds to all received commands with a data frame containing the updated state of its outputs

```
[Data byte 0][Data byte 1]...[Data byte N][0x0A][0x0D]
```

*Data byte 0:*
- *bit 0:* rfm_success, 1 if the LoRa chip is successfully initialized
- *bit 1:* output1, 1 if output 1 is enabled
- *bit 2:* output2
- *bit 3:* output3
- *bit 4:* output4

*Data byte 1:* `uint8_t` Current angle of servo 1<br>
*Data byte 2:* `uint8_t` Current angle of servo 2<br>
*Data byte 3:* `uint8_t` Current angle of servo 3<br>
*Data byte 4-5:* `int16_t` Battery 1 voltage, raw 12 bits measurement referenced to input voltage, MSB first<br>
*Data byte 6-7:* `int16_t` Battery 2 voltage, raw 12 bits measurement referenced to input voltage, MSB first<br>
*Data byte 8:* `int8_t` Latest RSSI value of the Launchpad Controller (`0` if LoRa init failed)<br>
*Data byte 9:* `int8_t` Latest RSSI value of the Launchpad Gateway (`0` if LoRa init failed)

## Examples

Ask the gateway to send back the identification string

```cpp
0x26, 0x67, 0x42, 0x00 // Or "&gB0"
```

Set output 1 high

```cpp
0x26, 0x63, 0x61, 0x01 // Or "&ca1"
```

Turn servo 3 to the middle position (90°)

```cpp
0x26, 0x63, 0x6C, 0x5A // Or "&clZ"
```
