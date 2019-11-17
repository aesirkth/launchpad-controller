# LaunchPadStation

# Purpose

This software is embedded in the Launch Pad Station. The main functionalities are :

- Receive commands from the Ground Station
- Trigger the rocket fueling, venting, defueling and ignition
- Forward relevant commands to the rocket

# Requirements

In order to make everything work you need :

- 1x LPS prototype board 2.0 with:
  - 1x Arduino nano
  - 1x RFM9XW with a matching antenna
- 2x 3S lipo battery with matching voltage
- 2x 2S lipo battery
- 1x 7.2V to 5V DC-DC converter
- 1x dual channels 5V relay (to power the solenoids on the fuel line)
- 2x single channel 5V relay (to power the ignition circuit)
- 1x fuel line (*not described here*)
- 1x ignition circuit (*not described here*)
- 1x Sigmundr rocket
- 1x ombilical cables

# Setup

1. Connect one 2S lipo battery to the DC-DC converter
2. Connect the DC-DC converter to the 5V input of the LPS prototype board 2.0
3. Connect the LPS prototype board relay outputs to the relays
4. Connect two 3S lipo battery to the input of the dual channels relay
5. Connect the solenoids of the fuel line to the outputs of the dual channels relay
6. Connect one 2s battery to the input of the single channel relay
7. Connect the ignition circuit to the output of the single channel relay
8. Connect the ombilicals to the rocket
9. (optional) connect the Arduino to a computer with an USB cable

# Ignition sequence

![ignition_states](doc/LPS_ignition_states.png)
