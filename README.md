# Launchpad Controller <!-- omit in toc -->

This is the embedded software on the Launchpad Controller to be used for the Mjollnir project.
It controls all the equipement on the Launchpad and is remotely operated from the Ground Station

The Launchpad Controller is an upgraded version of what was used for the Sigmundr (2019) and Odin (2018) projects. Check the previous versions here:

- [v1.0](https://github.com/aesirkth/launchpad-controller/tree/v1.0) - Odin
- [v2.0](https://github.com/aesirkth/launchpad-controller/tree/v2.0) - Sigmundr

*The work for the Mjollnir project is still ongoing*

- [Description](#description)
- [Installation](#installation)
- [Commands](#commands)
  - [Start byte](#start-byte)
  - [ID byte](#id-byte)
  - [Available commands](#available-commands)
    - [Send BONJOUR string](#send-bonjour-string)
    - [Activate/deactivate the main outputs](#activatedeactivate-the-main-outputs)
    - [Turn the servos](#turn-the-servos)
    - [State frame](#state-frame)
  - [Examples](#examples)
- [Folder structure](#folder-structure)

# Description

The Launchpad Controller runs on a Teensy LC microcontroller from [PJRC](https://www.pjrc.com/teensy/teensyLC.html) and the code is built around the cross-platform development tool [PlatformIO](https://platformio.org/)

The following outputs are available on the board:

- 2x 12V/15A DC
- 2x 24V/15A DC
- 3x 6V servo-compatible outputs
- 3x 12V-24V/2A H-bridge (output voltage selectable by jumper)

# Installation

Install PlatformIO Core on your system. The documentation is [here](https://platformio.org/install/cli)

Connect the board to your system using a micro USB cable

Build and upload the code to the Launchpad Controller using PlatformIO cli

```sh
platformio run -t upload -e controller
```

Currently, a second Launchpad Controller is needed on the Ground Station side to send the commands. Build the code and upload it

```sh
platformio run -t upload -e gateway
```

# Commands

The commands sent by the host computer must respect the following structure:

```
[Start byte][ID byte][Data byte 1][Data byte 2]
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

*Target:* controller, gateway

*Data byte 1:* `0x42` (`"B"`)

*Data byte 2:* `0x00` unused, can be any value

*Returns:* `"LAUNCHPADCONTROLLER"`

### Activate/deactivate the main outputs

Set one of the main outputs high or low

*Target:* controller

*Data byte 1:*

- `0x61` (`"a"`): for output 1 [P1]
- `0x62` (`"b"`): for output 2 [P2]
- `0x63` (`"c"`): for output 3 [P3]
- `0x64` (`"d"`): for output 4 [P4]

*Data byte 2:*

- `0x00`: set output low. A logic AND is applied to the LSb so (`"0"`) works to set the output low
- `0x01`: set output high. A logic AND is applied to the LSb so (`"1"`) works to set the output high

*Returns:* State frame (see **State frame** [below](#state-frame))

### Turn the servos

Update the rotation of one of the servos

*Target:* controller

*Data byte 1:*

- `0x6A` (`"j"`): servo 1 [P5]
- `0x6B` (`"k"`): servo 2 [P6]
- `0x6C` (`"l"`): servo 3 [P7]

*Data byte 2:*

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

### State frame

This is the frame returned by the Launchpad Controller after it has received a valid command

TODO

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

# Folder structure

```cpp
├── doc/
├── include/
│   └── hardware_definition.h   // Pinout and hardware constants
├── lib/
├── src/                        // Main code
│   ├── controller.cpp
│   ├── controller.h
│   ├── gateway.cpp
│   └── gateway.h
├── test/                       // Code for unit tests
│   ├── test_main.cpp
│   └── test_main.h
├── platformio.ini              // PlatformIO configuration file
└── README.md                   // This file
```