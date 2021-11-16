Arduino Eurorack projects
=========================

DIY Eurorack projects with Arduino and C++ libraries.

> 🛒 *Some of these modules are for sale on **[Reverb](https://reverb.com/shop/joeseggiola)** and **[Tindie](https://www.tindie.com/stores/joeseggiola/)**, as PCB and panel kits or fully assembled!*

Modules
-------

Each module has its own detailed README file.

- [Clock divider](clock-divider/): clock divider in 4 HP.
- [Forks](forks/): two Bernoulli gates, clone of Mutable Instruments Branches.
- [In CV](in-cv/): virtual ensemble that plays Terry Riley's "In C" on CV/gate outputs.
- [MIDI 4+1](midi4plus1/): polyphonic and monophonic MIDI to 4x CV/gate interface in 6 HP.

Libraries and tools
-------------------

- [Button class](lib/Button.cpp): convenient reading methods, debouncing, combined single and long-press, internal pull-up usage.
- [Knob class](lib/Knob.cpp): analog value reading with low/high thresholds.
- [LED class](lib/Led.cpp): handles minimum duration to ensure visibility, implements blinking, toggle, flash.
- [MCP4728 class](lib/MCP4728.cpp): extends [Hideaki Tai's lib](https://github.com/hideakitai/MCP4728) to include optional LDAC; a sketch for [setting I2C address (device ID)](tools/mcp4728_addr) is provided.
- [MultiPointMap class](lib/MultiPointMap.cpp): maps values using a multi-linear scale that can be persisted in EEPROM, used to implement DACs calibration (adapted from Befaco [MIDI Thing](https://github.com/Befaco/midithing) and Emilie Gillet's [CVpal](https://github.com/pichenettes/cvpal)).
- [SR74HC595 class](lib/SR74HC595.cpp): simple wrapper around `shiftOut()` to handle 74HC595 shift registers.

License
-------

Code: [GPL 3.0](LICENSE), hardware: [CC BY-SA 3.0](https://creativecommons.org/licenses/by-sa/3.0/).
