In CV
=====

A DIY Arduino-powered virtual ensemble that plays Terry Riley's "In C" on CV/gate outputs.

**[Arduino code][1]** | **[3D model][2]** | **[YouTube demo][3]**

[1]: in-cv.ino
[2]: plate.stl
[3]: https://youtu.be/ea2zLXFY1C4

Features
--------

* Six performers that play "In C" patterns on 1V/oct CV/gate outputs.
* Each performer has a button to advance through the 53 patterns, or to pause at the end of the current loop (long-press).
* LEDs show when a note is played, but will start blinking if the performer is left behind by three or more patterns.
* External clock input to control playback speed.
* Main button to show late performers, or to reset everything to initial state (long-press).
* In the initial state every performer plays a steady C with no gate (for tuning), until the advance button is pressed and the first pattern starts being played.

Circuit
-------

The following is just a hint for a possible circuit implementation.

![](schematic.png)

Patterns data generation
------------------------

Arduino code reads the definitions of the 53 patterns of "In C" from [`patterns/patterns.h`][20]. This file is not hand-written, but automatically generated using [`patterns/cli.js`][21], a Node.js command line script. This script translates music notation contained in [`patterns/patterns.txt`][22] into performant and memory-efficient data.

That said, you can write your own patterns and sequences to make the module play just about everything. Edit the TXT file (one sequence per line), install [Node.js][23] and then run this in the `patterns/` folder:

	npm install
	npm start -- patterns.txt

[20]: patterns/patterns.h
[21]: patterns/cli.js
[22]: patterns/patterns.txt
[23]: https://nodejs.org/en/

Pictures
--------

<img src="pictures/IMG_20190720_185259.jpg" width="420"> <img src="pictures/IMG_20190726_192809.jpg" width="420"> <img src="pictures/IMG_20190727_130919.jpg" width="420"> <img src="pictures/IMG_20190727_131911.jpg" width="420">

Thanks
------

- [Tero Parviainen][10]'s [analysis and JavaScript implementation][11] of "In C"

[10]: https://teropa.info/
[11]: https://teropa.info/blog/2017/01/23/terry-rileys-in-c.html
