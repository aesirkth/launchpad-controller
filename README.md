# Launch Pad Station

# Purpose

This software makes the Launch Pad Station work. The main functionalities are :

- Receive commands from the Ground Station
- Trigger the rocket fueling, venting and ignition
- Forward relevant commands to the rocket

# Description

The Launch Pad Station consists mainly of two electronics boards. The first one (*Launch Pad Station Board*) is embedded in the Launch Pad Station itself and is physically connected to the rocket fueling and ignition circuit as well as the rocket. It receives commands from the second board (*Launch Pad Station Gateway*) which is located on the Ground Station and connected to a laptop

Here is a description of the Rocket's systems taken from [aesirkth/GroundStation](https://github.com/aesirkth/GroundStation) :

![data_link](https://raw.githubusercontent.com/aesirkth/GroundStation/master/doc/diagrams/data_links.png)

Communication between the Launch Pad Station and the Ground Station is made through a LoRa link. The LoRa tranceivers used are the `RFM96W`

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
- 1x dual channels 5V relay (to power the solenoids on the fuel line)
- 2x single channel 5V relay (to power the ignition circuit)
- 1x fuel line (*not described here*)
- 1x ignition circuit (*not described here*)
- 1x Sigmundr rocket (*not described here*)
- 1x ombilical cables

The *RadioHead* library from airspayce is also required to control the LoRa modules with the Arduino boards. Use version 1.97 found [here](http://www.airspayce.com/mikem/arduino/RadioHead/RadioHead-1.97.zip)

# Setup

## Launch Pad Station Board

1. Upload `launchpadstation.ino` on the Arduino
2. Connect one 3S lipo battery to the DC-DC converter
3. Connect the DC-DC converter to the 5V input of the LPS prototype board 2.0
4. Connect the Launch Pad Station Board relay outputs to the relays
5. Connect two high current 3S lipo battery to the input of the dual channels relay
6. Connect the solenoids of the fuel line to the outputs of the dual channels relay
7. Connect one 3S lipo battery to the input of the single channel relay
8. Connect the ignition circuit to the output of the single channel relay
9. Connect the ombilicals to the rocket
10. (optional) connect the Arduino to a computer with a USB cable
11. (optional) control the Launch Pad Station with the GUI found on [aesirkth/GroundStation](https://github.com/aesirkth/GroundStation)

> Use (optional) steps when not using the Launch Pad Station Gateway

## Launch Pad Station Gateway

1. Upload `lpsgateway.ino` on the Arduino
2. Connect the Arduino to a computer with a USB cable
3. Control the Launch Pad Station with the GUI found on [aesirkth/GroundStation](https://github.com/aesirkth/GroundStation)

# Ignition sequence

![ignition_states](doc/LPS_ignition_states.png)
