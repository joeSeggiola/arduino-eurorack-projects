MIDI 4+1
========

A DIY Arduino-powered MIDI to 4x CV/gate interface in 6HP, with both polyphonic and monophonic voice allocation modes.

**[Arduino code][1]** | **[3D model][2]** | **[YouTube demo][5]** | [🗨️ **Mod Wiggler**][3] | [🗨️ **Lines**][4]

[1]: midi4plus1.ino
[2]: plate.stl
[3]: https://www.modwiggler.com/forum/viewtopic.php?t=231861
[4]: https://llllllll.co/t/midi-4-1-arduino-powered-polyphonic-and-monophonic-midi-to-4x-cv-gate-interface-in-6hp/32543
[5]: https://youtu.be/g9WwDo7eYi4

Features and limitations
------------------------

* MIDI DIN input and 4x CV/gate outputs with gate LEDs.
* Button for cycling through four different modes, with coloured mode LED:
   * **Poly** (red): four-voices polyphony with priority to last, LRU strategy and voice stealing;
   * **Poly-first** (orange): four-voices polyphony with priority to first and first-available strategy;
   * **Split poly+mono** (green): split keyboard with three-voices polyphony on the left, and monophony on the right (priority to last);
   * **Split mono+poly** (yellow): same as above, but flipped.
* Additional gate output that stays high while at least one polyphonic voice is active (logic OR), useful for single-filter setups.
* Voices lock with a long-press of the mode button: all gates of currently held polyphonic voices stay high, ignoring key releases until next reallocation.

The DACs range is 0-4V, so only the 4 center octaves are covered. The get more, it is necessary to add amplifiers 
on CV outputs, which will probably require a better DACs calibration process. Split modes splits the keyboard 
on middle C. Both lowest MIDI octave and split octave are easily configurable in code.

Circuit
-------

The following is just a hint for a possible circuit implementation.

![](schematic.png)

Pictures
--------

<img src="pictures/IMG_20200307_181436.jpg" width="420"> <img src="pictures/IMG_20200413_181507.jpg" width="420">

Thanks
------

- Emilie Gillet's [CVpal][10] (polyphonic voice allocator and monophonic notes stack)
- Befaco [MIDI Thing][11]
- François Best's [Arduino MIDI Library][12]

[10]: https://github.com/pichenettes/cvpal
[11]: https://github.com/Befaco/midithing
[12]: https://github.com/FortySevenEffects/arduino_midi_library
