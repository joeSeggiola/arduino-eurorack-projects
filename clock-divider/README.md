Clock divider
=============

A DIY Arduino-powered clock divider in 4HP, with 3D-printed faceplate.

**[Arduino code](clock-divider.ino)** | **[3D model](plate.stl)** | **[Plate design](plate.svg)**

Features
--------

- Divides incoming clock signal by 2, 3, 4, 5, 6, 8, 16, 32 (configurable in code).
- Reset as trigger or manual button.
- Down-beat counting.
- Trigger mode: duration of incoming pulses is preserved on outputs.
- Gate-mode: duration of the output pulses is 50% of divided tempo, enabled by long-pressing the manual reset button.

Circuit
--------

The following is just a hint for a possible circuit implementation.

![](schematic.png)