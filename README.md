# Launch Pad Station

# Purpose

This software makes the Launch Pad Station work. The main functionalities are :

- Receive commands from the Ground Station
- Trigger the rocket fueling, venting and ignition
- Forward relevant commands to the rocket

# Description

The Launch Pad Station consists mainly of two electronics boards. The first one (*Launch Pad Station Board*) is embedded in the Launch Pad Station itself and is physically connected to the rocket fueling and ignition circuit as well as the rocket. It receives commands from the second board (*Launch Pad Station Gateway*) which is located on the Ground Station and connected to a laptop

Here is a description of the rocket's systems taken from [aesirkth/GroundStation](https://github.com/aesirkth/GroundStation) :

![data_link](https://raw.githubusercontent.com/aesirkth/GroundStation/master/doc/diagrams/data_links.png)

Communication between the Launch Pad Station and the Ground Station is made through a LoRa link. The LoRa tranceivers used are the `RFM95W` running at 915 MHz

# Requirements

In order to make everything work you need :

- 1x Launch Pad Station Board 2.0 with:
  - 1x Arduino nano
  - 1x RFM9XW with a matching antenna
- 1x Launch Pad Station Gateway with:
  - 1x Arduino nano
  - 1x RFM9XW with a matching antenna
- 2x high current 3S lipo battery with matching voltage
- 2x 3S lipo battery
- 1x 12V to 5V DC-DC converter
- 1x dual channels 5V relay (solenoids relay)
- 1x single channel 5V relay (ignition relay)
- 1x fuel line (*not described here*)
- 1x ignition circuit (*not described here*)
- 1x Sigmundr rocket (*not described here*)
- 1x ombilical cables

The *RadioHead* library from airspayce is also required to control the LoRa modules with the Arduino boards. Use version 1.97 found [here](http://www.airspayce.com/mikem/arduino/RadioHead/RadioHead-1.97.zip)

# Setup

## Launch Pad Station Board

1. Upload `launchpadstation.ino` on the Arduino
2. Connect the 5V and GND pins of the relays to the 5V and GND pins on the *Launch Pad Station Board*
3. Connect the *Launch Pad Station Board* outputs to the relays:
   1. Connect Arduino pin `A2` to `IN1` on the ignition relay
   2. Connect Arduino pin `A3` to `IN2` on the solenoids relay (Fill)
   3. Connect Arduino pin `A4` to `IN1` on the solenoids relay (Vent)
4. Connect one 3S lipo battery to the DC-DC converter
5. Connect the DC-DC converter to the 5V input of the *Launch Pad Station Board*
6. Connect two high current 3S lipo battery to the input of the solenoids relay
7. Connect one 3S lipo battery to the input of the ignition relay
8. Connect the solenoids of the fuel line to the outputs of the solenoids relay
9. Connect the ignition circuit to the output of the ignition channel relay
10. Connect the ombilicals to the rocket
11. (optional) connect the Arduino to a computer with a USB cable
12. (optional) control the Launch Pad Station with the GUI found on [aesirkth/GroundStation](https://github.com/aesirkth/GroundStation)

> Use (optional) steps when not using the Launch Pad Station Gateway

## Launch Pad Station Gateway

1. Upload `lpsgateway.ino` on the Arduino
2. Connect the Arduino to a computer with a USB cable
3. Control the Launch Pad Station with the GUI found on [aesirkth/GroundStation](https://github.com/aesirkth/GroundStation)

# Ignition sequence

![ignition_states](doc/LPS_ignition_states.png)

>The commands are single bytes and defined in `launchpadstation.ino`

# Rocket control

3 wires are used to send commands to the rocket :

- Ground
- Telemetry enable/disable (Arduino pin `A0`)
- Calibration trigger (Arduino pin `A1`)

Send `CMD_TM_ENABLE` to set pin `A0` to `HIGH`

Send `CMD_TM_DISABLE` to set pin `A0` to `LOW`

Send `CMD_CA_TRIGGER` to set `A1` to `LOW` for 100 ms

The default state of pins `A0` and `A1` is `HIGH`

>The commands are single bytes and defined in `launchpadstation.ino`