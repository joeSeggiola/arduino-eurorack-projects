Forks
=====

Arduino-powered clone of Mutable Instrument's [Branches][1], with a 3D-printed faceplate.

**[Arduino code](forks.ino)** | **[3D model](plate.stl)** | **[Plate design](plate.svg)**

Features and limitations
------------------------

- Two Bernoulli gates similar to the ones found in Mutable Instrument's Branches.
- Both Latch and Toggle modes are enabled with two independent dedicated switches.
- Manual input button, it can be used as a manual trigger/gate generator.

Compared to [Branches][1], it has some drawbacks due to Arduino usage with minimal circuitry around:

- Probability CV input range is unipolar: a negative input will be treated as 0%, and an input higher than 5V as 100%.
- Probability CV input is simply summed to the value of the knob: with the knob fully counter-clockwise the probability is given by the CV input only, turning the knob clockwise will "push" the probability toward 100%, up to fully clockwise where the CV input doesn't matter any more.
- The second channel input is not normalized to the first one.

Circuit
-------

The following is a hint for a possible circuit implementation. Only one Bernoulli gate is laid out, the second is an exact copy.

![](schematic.png)

[1]: https://mutable-instruments.net/modules/branches/