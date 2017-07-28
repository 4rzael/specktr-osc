# specktr-osc
A multiplateform OSC client that listen to a Specktr MIDI glove and outputs OSC messages.

![](https://i.imgur.com/hZenrNp.png)

This software is intended to be used with the [Specktr ](http://specktr.com/fr/) MIDI glove, but should work with any MIDI hardware.

## Notes :

* This software depends on the [JUCE](https://www.juce.com/) library.  
* Because of this dependency, it is released using the GPLv3 License.

* MIDI compatibility may not be very extended yet. Post issues if needed.

* This program has only be tested on Windows 10 64-bits. (version 1703). It is compatible with MIDI-BLE, but isn't perfect : [see here](https://forum.juce.com/t/experimental-support-for-the-windows-runtime-midi-api/21049)

* The code is pretty horrible. Didn't have time. Sorry.

## OSC routes :

In the OSC topic, variables are noted as `{variable_name}`. Those are substituted with real values before being sent.

Routes by message_type :

* `note_on` => `NOTE_NUMBER:int VELOCITY:int`  
* `note_off` => `NOTE_NUMBER:int VELOCITY:int`  
* `program_change` => `NEW_PROGRAM:int`  
* `pitch_bend` => `PITCH_BEND_VALUE:int`
* `aftertouch` => `AFTERTOUCH_VALUE:int`  
* `channel_pressure` => `PRESSURE_VALUE:int`  
* `control_change` (CC) => `CONTROL_NUMBER:int CONTROL_VALUE:int`  

