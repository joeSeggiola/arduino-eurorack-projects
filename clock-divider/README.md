Clock divider
=============

A DIY Arduino-powered clock divider in 4HP, with 3D-printed faceplate.

**[Arduino code][1]** | **[PCB][2]** | [🗨️ **Mod Wiggler**][4]

[1]: clock-divider.ino
[2]: pcb/
[4]: https://modwiggler.com/forum/viewtopic.php?t=214669

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

Pictures
--------

### New PCB build

<img src="pictures/IMG_20210702_130542.jpg" width="420"> <img src="pictures/IMG_20210702_130610.jpg" width="420"> 

### Old [3D-printed](3d/) build

<img src="pictures/IMG_20190107_215258.jpg" width="420"> <img src="pictures/IMG_20190112_224337.jpg" width="420"> <img src="pictures/IMG_20190306_214855.jpg" width="420"> <img src="pictures/IMG_20190307_225219.jpg" width="420">