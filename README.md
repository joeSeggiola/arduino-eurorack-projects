Arduino Eurorack projects
=========================

DIY Eurorack projects with Arduino and common C++ libraries.

Modules
-------

Each module has its own detailed README file.

- [Clock divider](clock-divider/): clock divider in 4HP.
- [Forks](forks/): two Bernoulli gates, clone of Mutable Instruments Branches.
- [In CV](in-cv/): virtual ensemble that plays Terry Riley's "In C" on CV/gate outputs.

Libraries and tools
-------------------

- [Button class](lib/Button.cpp): convenient reading methods, debouncing, combined single and long-press, internal pull-up usage.
- [Knob class](lib/Knob.cpp): analog value reading with low/high thresholds.
- [LED class](lib/Led.cpp): handles minimum duration to ensure visibility, implements blinking, toggle, flash.
- [MCP4728 class](lib/MCP4728.cpp): extends [Hideaki Tai's lib](https://github.com/hideakitai/MCP4728) to include DACs calibration and optional LDAC. Sketches for [setting I2C address (device ID)](tools/mcp4728_addr) and [guiding the calibration process](tools/mcp4728_calibration) are provided.
- [MM74HC595M class](lib/MM74HC595M.cpp): simple wrapper around `shiftOut()` to handle 74HC595 shift registers.

License
-------

Code: [GPL 3.0](LICENSE), everything else: free domain.
