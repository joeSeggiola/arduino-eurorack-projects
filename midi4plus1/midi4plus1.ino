
// CONFIGURATION =============================================================

// Pin 1 is reserved for MIDI RX.
// Pins A4 and A5 are reserved for the DAC I2C communication.

const byte MODE_BUTTON = A0; // Main button pin
const byte MODE_LEDS[] = { A1, A2, A3 }; // RGB LED pins for current mode display
const bool MODE_LEDS_PWM = true; // Set to FALSE to disable SoftPWM and adjust RGB brightness with resistors only 

const byte GATES[] { 3, 4, 5, 6 }; // Gate 1-4 pins
const byte GATE_OR = 7; // Auxiliary gate pin, high when at least one gate is high
const byte GATES_LEDS[] { 8, 9, 10, 11 }; // LEDs pins for gates display
const byte GATE_OR_LED = 12; // Auxiliary gate pin, high when at least one gate is high

const byte NOTE_ON_LED = 13; // LED pin for showing note-on signals

const unsigned int GATE_RETRIG_MS = 40; // Time between two consecutive gates in ms, to retrig envelopes
const bool GATE_RETRIG_MONO = false; // TRUE to force retrig also on monophonic modes, making legato impossible
const byte PITCH_BEND_SEMITONES = 2; // Picth-bend range in semitones
const byte SPLIT_MIDI_OCTAVE = 4; // Defines on which MIDI octave the keyboard will be split for poly+mono mode
const int LOWEST_MIDI_OCTAVE = 2; // CV out range is four octaves max, this defines which MIDI octave will be mapped to zero CV

const bool CLOCK = false; // TRUE to send MIDI clock to auxiliary gate pin (instead of OR)
const unsigned int CLOCK_PPQ = 24; // 24 PPQ to get a trigger every 1/4 note (MIDI standard), 12 PPQ for 1/8, 48 PPQ for 1/2, etc...
const unsigned int CLOCK_TRIG_MS = 40; // Trigger width for the clock output signal, in ms

const unsigned long BUTTON_LOCK_LONG_PRESS_MS = 500; // Button long-press duration to lock currently help polyphonic voices
const unsigned long BUTTON_DEBOUNCE_DELAY = 50; // Debounce delay for the button
const unsigned long LED_MODE_LOCK_DURATION_MS = 500; // How long the mode LED should be kept off to signal voices lock
const unsigned long LED_MIN_DURATION_MS = 50; // Minimum "on" duration for LEDs visibility

// DEBUGGING =================================================================

const bool DEBUG = false; // FALSE to disable debug messages on serial port

// By enabling debugging, the hardware serial port will be used for both debug
// messages and MIDI messages. Therefore, it's required to use a PC bridge to
// test hardware MIDI devices, like ttymidi: http://www.varal.org/ttymidi/

const bool DEBUG_WITH_TTYMIDI = false; // TRUE to use ttymidi bridge and print messages on serial

// Notes about debugging with ttymidi:
//  - run: ttymidi -s /dev/ttyUSB0 -b 9600 -v
//  - then connect the hardware MIDI device output to ttymidi input

// ===========================================================================

#include <EEPROM.h>
#include <MIDI.h>
#include <SoftPWM.h>
#include <Wire.h>

#include "lib/Button.cpp"
#include "lib/Led.cpp"
#include "lib/MCP4728.cpp"
#include "lib/MultiPointMap.cpp"

#include "mono.cpp"
#include "poly.cpp"

#define N 4 // Number of voices, each with its own CV/gate signals

#define MODE_POLY 0 // 4x polyphony with priority to last 
#define MODE_POLY_FIRST 1 // 4x polyphony with priority to first
#define MODE_POLY_MONO 2 // Keyboard split with 3x polyphony on lower keys, monophony on higher keys
#define MODE_MONO_POLY 3 // Keyboard split with monophony on lower key, 3x polyphony on higher keys
#define MODE_MONO 4 // 4x monophony on MIDI channels 1-4

#define MODE_POLY_RGB 0x330000 // Red
#define MODE_POLY_FIRST_RGB 0x003300 // Green
#define MODE_POLY_MONO_RGB 0x0000CC // Blue
#define MODE_MONO_POLY_RGB 0x3300CC // Pink
#define MODE_MONO_RGB 0x009966 // Teal

#define CALIBRATION_RGB 0x3333CC // White

Button modeButton;
MCP4728 dac;
MultiPointMap calibration[4];
Led gateLed[N];
Led gateOrLed;
Led noteOnLed;

NoteStack mono[N];
VoiceAllocator poly;

byte mode = MODE_POLY;
byte voiceMidiNote[N]; // Current MIDI note for each voice
bool voiceActive[N]; // Current activation state for each voice
unsigned long voiceRetrigTime[N]; // Time the retrig interval started, zero if retrig is not occurring
bool voiceLocked[N]; // TRUE if the voice is currently locked
unsigned long voiceLockLedTime = 0; // Time the mode LED has been turned off to signal lock
int pitchBend; // Pitch-bend value (all voices in poly modes, monophonic voice only in split modes)
bool outputFlag; // TRUE if it's necessary to update the outputs

unsigned int clockCount = 0; // MIDI clock PPQ counter
bool clockRunning = false; // TRUE if MIDI start/continue message has been received
bool clockTrig = false; // TRUE if the trigger for the clock output is currently HIGH
unsigned long clockTrigTime = 0; // Time of the last trigger for the clock output, in ms
unsigned long clockTrigDuration = CLOCK_TRIG_MS; // Trigger width for the clock output signal, in ms

bool calibrating = false; // TRUE if currently running the calibration process
byte calibratingVoice; // Voice currently being calibrated
byte calibratingInterval; // Calibration point index for the current voice
int calibratingAddress; // EEPROM address to store the current calibration

const int MODE_EEPROM_ADDRESS = 0;
const int DAC_CALIBRATION_EEPROM_ADDRESS = 100;

#define MIDI_NOTE_TO_CV_FACTOR 83.333333 // 1000/12, i.e. 1000mV per octave
#define MIDI_PITCHBEND_MAX 8191 // Maximum value for pitch-bending, as given by MIDI.h library

const char NOTE_NAMES[12][3] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

// MIDI custom settings
// https://github.com/FortySevenEffects/arduino_midi_library/wiki/Using-custom-Settings
struct MIDISettings : public midi::DefaultSettings {
	static const long BaudRate = 31250;
	static const bool UseRunningStatus = false;
	static const bool Use1ByteParsing = false;
};
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, MIDISettings);

void setup() {
	
	// Init MIDI
	MIDI.begin(MIDI_CHANNEL_OMNI); // Listen to all channels
	MIDI.turnThruOff(); // Required after begin()
	
	// Debugging
	if (DEBUG || DEBUG_WITH_TTYMIDI) {
		Serial.begin(9600);
		if (DEBUG) debug("MIDI 4+1 - joeSeggiola");
	}
	
	// Setup I/O
	modeButton.init(MODE_BUTTON, BUTTON_DEBOUNCE_DELAY, true, true);
	pinMode(GATE_OR, OUTPUT);
	gateOrLed.init(GATE_OR_LED, LED_MIN_DURATION_MS);
	noteOnLed.init(NOTE_ON_LED, LED_MIN_DURATION_MS);
	for (byte i = 0; i < N; i++) {
		pinMode(GATES[i], OUTPUT);
		gateLed[i].init(GATES_LEDS[i]);
	}
	
	// Setup mode RGB LED
	if (MODE_LEDS_PWM) {
		SoftPWMBegin(); // Software PWM for the mode RGB LED
	} else {
		pinMode(MODE_LEDS[0], OUTPUT);
		pinMode(MODE_LEDS[1], OUTPUT);
		pinMode(MODE_LEDS[2], OUTPUT);
	}
	
	// Init I2C communication and DAC
	Wire.begin();
	Wire.setClock(400000); // Fast mode
	dac.init(Wire, 0);
	dac.selectVref(MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V);
	dac.selectPowerDown(MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL);
	dac.selectGain(MCP4728::GAIN::X2, MCP4728::GAIN::X2, MCP4728::GAIN::X2, MCP4728::GAIN::X2);
	
	// Load DACs calibration
	int calibrationAddress = DAC_CALIBRATION_EEPROM_ADDRESS;
	for (byte i = 0; i < 4; i++) {
		calibration[i].init(4000);
		calibrationAddress += calibration[i].load(calibrationAddress);
	}
	
	// If mode button is pressed on boot, start calibration process
	delay(100);
	if (modeButton.readOnce()) {
		setupCalibration();
	} else {
		setupMain();
	}
	
}

void setupMain() {
	
	// Init voice allocators
	poly.init();
	for (byte i = 0; i < N; i++) {
		mono[i].init();
		voiceMidiNote[i] = 12; // Init voices to C0
	}
	
	bootAnimation();
	
	// Calculate appropriate trigger width for the clock output signal
	if (CLOCK) {
		unsigned long clockMaxBPM = 600;
		unsigned long clockMaxPeriod = ((60000L / clockMaxBPM) * CLOCK_PPQ) / 24; // Period at max BPM
		clockTrigDuration = min(CLOCK_TRIG_MS, (clockMaxPeriod * 0.8)); // Leave space to re-trig
	}
	
	// Set minimum "on" duration on gate LEDs
	for (byte i = 0; i < N; i++) {
		gateLed[i].setMinDurationMs(LED_MIN_DURATION_MS);
	}
	
	// Init mode from permanent storage
	byte initialMode = EEPROM.read(MODE_EEPROM_ADDRESS) % 5;
	setMode(initialMode);
	
	// MIDI callbacks
	MIDI.setHandleNoteOn(handleNoteOn);
	MIDI.setHandleNoteOff(handleNoteOff);
	MIDI.setHandlePitchBend(handlePitchBend);
	if (CLOCK) {
		MIDI.setHandleClock(handleClock);
		MIDI.setHandleStart(handleStart);
		MIDI.setHandleContinue(handleContinue);
		MIDI.setHandleStop(handleStop);
		MIDI.setHandleSongPosition(handleSongPosition);
	}
	
}

void setupCalibration() {
	
	// Start calibration process
	calibrating = true;
	calibratingVoice = 0;
	calibratingInterval = 0;
	calibratingAddress = DAC_CALIBRATION_EEPROM_ADDRESS;
	setModeLedColor(CALIBRATION_RGB);
	
	// MIDI callback
	MIDI.setHandleNoteOn(handleCalibrationOffset);
	
}

void loop() {
	
	MIDI.read();
	
	if (calibrating) {
		loopCalibration();
	} else {
		loopMain();
	}
	
	// Update LEDs
	gateOrLed.loop();
	noteOnLed.loop();
	for (byte i = 0; i < N; i++) gateLed[i].loop();
	
}

void loopMain() {
	
	// Check if retrig intervals are over
	for (byte i = 0; i < N; i++) {
		if (voiceRetrigTime[i] > 0) {
			if (voiceRetrigTime[i] + GATE_RETRIG_MS < millis()) {
				voiceRetrigTime[i] = 0;
				outputFlag = true;
			}
		}
	}
	
	// Update outputs if necessary
	if (outputFlag) {
		output();
		outputFlag = false;
	}
	
	// Update clock output
	if (CLOCK) {
		if (clockTrig && (clockTrigTime + clockTrigDuration < millis())) {
			digitalWrite(GATE_OR, LOW);
			clockTrig = false;
		}
	}
	
	// Check for mode button presses
	byte modeButtonPress = modeButton.readShortOrLongPressOnce(BUTTON_LOCK_LONG_PRESS_MS);
	if (modeButtonPress == 1) {
		setMode((mode + 1) % 5); // Short-press: cycle through modes
	} else if (modeButtonPress == 2) {
		voicesLock(); // Long-press: lock voices
	}
	
	// Re-set mode LED if necessary
	if (voiceLockLedTime > 0 && voiceLockLedTime + LED_MODE_LOCK_DURATION_MS < millis()) {
		voiceLockLedTime = 0;
		setModeLed();
	}
	
}

void loopCalibration() {
	
	// Button advance through calibration points and voices
	if (modeButton.readOnce()) {
		calibratingInterval++;
		if (calibratingInterval == calibration[calibratingVoice].size()) {
			
			// Voice calibration completed, save and advance to next voice
			calibratingAddress += calibration[calibratingVoice].save(calibratingAddress);
			calibratingVoice++;
			calibratingInterval = 0;
			
			// Calibration completed?
			if (calibratingVoice == N || calibratingVoice == 4) {
				calibrating = false;
				for (byte i = 0; i < N; i++) gateLed[i].off();
				setModeLedColor(0x000000);
				delay(1000);
				setupMain();
				return;
			}
			
		}
	}
	
	// Show which voice is currently being calibrated
	for (byte i = 0; i < N; i++) {
		gateLed[i].set(i == calibratingVoice);
	}
	
	if (millis() % 50 == 0) {
		
		// Update DAC values
		unsigned int size = calibration[calibratingVoice].size();
		unsigned int step = calibration[calibratingVoice].getStep();
		unsigned int value = step * (calibratingInterval + 1);
		dac.analogWrite(
			calibration[0].map(calibratingVoice == 0 ? value : (calibratingVoice > 0 ? step * size : 0)), 
			calibration[1].map(calibratingVoice == 1 ? value : (calibratingVoice > 1 ? step * size : 0)), 
			calibration[2].map(calibratingVoice == 2 ? value : (calibratingVoice > 2 ? step * size : 0)), 
			calibration[3].map(calibratingVoice == 3 ? value : (calibratingVoice > 3 ? step * size : 0))
		);
		
		// Update gates
		for (byte i = 0; i < N; i++) {
			digitalWrite(GATES[i], calibratingVoice == i);
		}
		
	}
	
}

void setMode(byte m) {
	
	// Setup polyphonic allocator
	mode = m;
	switch (mode) {
		case MODE_POLY:
			poly.setMode(VoiceAllocator::Mode::LAST);
			poly.setSize(N);
			break;
		case MODE_POLY_FIRST:
			poly.setMode(VoiceAllocator::Mode::FIRST);
			poly.setSize(N);
			break;
		case MODE_POLY_MONO:
		case MODE_MONO_POLY:
			poly.setMode(VoiceAllocator::Mode::LAST);
			poly.setSize(N - 1);
			break;
	}
	
	// Mode selection on permanent storage
	EEPROM.update(MODE_EEPROM_ADDRESS, mode);
	
	setModeLed();
	reset();
	
}

void voicesLock() {
	
	// If any voice is locked, unlock it
	bool unlocked = false;
	for (byte i = 0; i < N; i++) {
		if (voiceLocked[i]) {
			voiceLocked[i] = false;
			voiceActive[i] = false;
			outputFlag = true;
			unlocked = true;
		}
	}
	
	if (mode != MODE_MONO) {
		
		// Lock currently held polyphonic voices
		if (!unlocked) {
			for (byte i = 0; i < N; i++) {
				if (voiceActive[i] && !isNoteForMonophony(voiceMidiNote[i])) {
					voiceLocked[i] = true;
				}
			}
			if (DEBUG) debugVoices();
		}
		
		// Turn off the mode LED to signal (un)locking
		voiceLockLedTime = millis();
		if (MODE_LEDS_PWM) {
			SoftPWMSet(MODE_LEDS[0], 0);
			SoftPWMSet(MODE_LEDS[1], 0);
			SoftPWMSet(MODE_LEDS[2], 0);
		} else {
			digitalWrite(MODE_LEDS[0], LOW);
			digitalWrite(MODE_LEDS[1], LOW);
			digitalWrite(MODE_LEDS[2], LOW);
		}
		
	}
	
}

void reset() {
	
	// Reset gates; CVs don't need reset, they'll keep the last note value and that's fine
	for (byte i = 0; i < N; i++) {
		digitalWrite(GATES[i], LOW);
		gateLed[i].off();
		voiceActive[i] = false;
		voiceLocked[i] = false;
		voiceRetrigTime[i] = 0;
	}
	digitalWrite(GATE_OR, LOW);
	gateOrLed.off();
	
	// Reset allocators
	for (byte i = 0; i < N; i++) {
		mono[i].clear();
	}
	poly.clear();
	
	pitchBend = 0;
	outputFlag = true;
	
}

void output() {
	
	// Update CVs (DACs)
	unsigned int dacValues[4];
	for (byte i = 0; i < 4; i++) {
		if (i < N) {
			int pitchBendValue = pitchBend;
			if (mode == MODE_POLY_MONO || mode == MODE_MONO_POLY) { // Pitch-bend is for monophonic voice only
				if (!isNoteForMonophony(voiceMidiNote[i])) pitchBendValue = 0;
			}
			dacValues[i] = getMidiNoteCV(voiceMidiNote[i], pitchBendValue);
		} else {
			dacValues[i] = 0;
		}
	}
	dac.analogWrite(
		calibration[0].map(dacValues[0]), 
		calibration[1].map(dacValues[1]), 
		calibration[2].map(dacValues[2]), 
		calibration[3].map(dacValues[3])
	);
	
	// Update gates
	for (byte i = 0; i < N; i++) {
		bool active = voiceActive[i] && voiceRetrigTime[i] == 0;
		digitalWrite(GATES[i], active);
		gateLed[i].set(active);
	}
	
	// Update OR gate
	if (!CLOCK) {
		bool gateOrActive = false;
		byte gateOrFirstVoice = mode == MODE_MONO_POLY ? 1 : 0;
		byte gateOrLastVoice = mode == MODE_POLY_MONO ? N - 2 : N - 1;
		for (byte i = gateOrFirstVoice; i <= gateOrLastVoice; i++) {
			gateOrActive |= voiceActive[i];
		}
		digitalWrite(GATE_OR, gateOrActive);
		gateOrLed.set(gateOrActive);
	}
	
	if (DEBUG) debugVoices();
	
}

void setModeLed() {
	switch (mode) {
		case MODE_POLY: setModeLedColor(MODE_POLY_RGB); break;
		case MODE_POLY_FIRST: setModeLedColor(MODE_POLY_FIRST_RGB); break;
		case MODE_POLY_MONO: setModeLedColor(MODE_POLY_MONO_RGB); break;
		case MODE_MONO_POLY: setModeLedColor(MODE_MONO_POLY_RGB); break;
		case MODE_MONO: setModeLedColor(MODE_MONO_RGB); break;
	}
}

void setModeLedColor(unsigned long color) {
	if (MODE_LEDS_PWM) {
		SoftPWMSet(MODE_LEDS[0], (color >> 16) & 0xFF);
		SoftPWMSet(MODE_LEDS[1], (color >> 8) & 0xFF);
		SoftPWMSet(MODE_LEDS[2], color & 0xFF);
	} else {
		digitalWrite(MODE_LEDS[0], (color >> 16) & 0xFF ? HIGH : LOW);
		digitalWrite(MODE_LEDS[1], (color >> 8) & 0xFF ? HIGH : LOW);
		digitalWrite(MODE_LEDS[2], color & 0xFF ? HIGH : LOW);
	}
}

void handleNoteOn(byte channel, byte note, byte velocity) {
	noteOnLed.flash();
	if (isNoteForMonophony(note)) {
		if (mode == MODE_MONO && (channel == 0 || channel > N)) return;
		mono[getMonophonyStackIndex(channel)].noteOn(note);
		byte i = getMonophonyVoiceIndex(channel);
		if (GATE_RETRIG_MONO && voiceActive[i]) {
			voiceRetrigTime[i] = millis(); // If already playing a note, start a retrig interval to avoid legato
		}
		voiceMidiNote[i] = note;
		voiceActive[i] = true;
		outputFlag = true;
	} else {
		int voice = poly.noteOn(note);
		if (voice > -1) {
			byte i = getPolyphonyVoiceIndex(voice);
			if (voiceActive[i] && voiceMidiNote[i] != note) {
				voiceRetrigTime[i] = millis(); // If that voice was already playing a different note, start a retrig interval
			}
			voiceMidiNote[i] = note;
			voiceActive[i] = true;
			voiceLocked[i] = false; // Unlock the voice in case it was locked
			outputFlag = true;
		}
	}
}

void handleNoteOff(byte channel, byte note, byte velocity) {
	if (isNoteForMonophony(note)) {
		if (mode == MODE_MONO && (channel == 0 || channel > N)) return;
		int newNote = mono[getMonophonyStackIndex(channel)].noteOff(note);
		byte i = getMonophonyVoiceIndex(channel);
		if (newNote > -1) {
			if (GATE_RETRIG_MONO && voiceActive[i] && voiceMidiNote[i] != newNote) {
				voiceRetrigTime[i] = millis(); // If will now play a different note, start a retrig interval to avoid legato
			}
			voiceMidiNote[i] = newNote;
			voiceActive[i] = true;
		} else {
			voiceActive[i] = false;
		}
		outputFlag = true;
	} else {
		int voice = poly.noteOff(note);
		if (voice > -1) {
			byte i = getPolyphonyVoiceIndex(voice);
			if (!voiceLocked[i]) { // De-activate voice if not locked
				voiceActive[i] = false;
				outputFlag = true;
			}
		}
	}
}

void handlePitchBend(byte channel, int bend) {
	pitchBend = bend;
	outputFlag = true;
}

void handleClock() {
	if (clockRunning) {
		if (clockCount == 0) {
			digitalWrite(GATE_OR, HIGH);
			clockTrig = true;
			clockTrigTime = millis();
			gateOrLed.flash();
		}
		clockCount = (clockCount + 1) % CLOCK_PPQ;
	}
}

void handleStart() {
	clockCount = 0;
	clockRunning = true;
}

void handleContinue() {
	clockRunning = true;
}

void handleStop() {
	clockRunning = false;
}

void handleSongPosition(unsigned int beats) {
	clockCount = (beats * 6) % CLOCK_PPQ; // MIDI beat = a 16th note = 6 pulses (24 PPQ / 4)
}

void handleCalibrationOffset(byte channel, byte note, byte velocity) {
	if (calibrating) {
		noteOnLed.flash();
		gateOrLed.flash();
		int offset = note >= (4 + 1) * 12 ? 1 : -1; // Split the keyboard in half on middle C
		int v = calibration[calibratingVoice].get(calibratingInterval); // Current point value
		calibration[calibratingVoice].set(calibratingInterval, max(0, v + offset)); // Move current point
	}
}

bool isNoteForMonophony(byte note) {
	bool higherKey = note >= (SPLIT_MIDI_OCTAVE + 1) * 12; // Check keyboard split
	return ( // Return TRUE if the pressed note should be used for monophony
		(mode == MODE_MONO) ||
		(mode == MODE_POLY_MONO && higherKey) ||
		(mode == MODE_MONO_POLY && !higherKey)
	);
}

byte getMonophonyStackIndex(byte channel) {
	return mode == MODE_MONO ? ((channel - 1) % N) : 0;
}

byte getMonophonyVoiceIndex(byte channel) {
	if (mode == MODE_MONO) return (channel - 1) % N;
	return mode == MODE_MONO_POLY ? 0 : N - 1;
}

byte getPolyphonyVoiceIndex(byte i) {
	return mode == MODE_MONO_POLY ? i + 1 : i;
}

unsigned int getMidiNoteCV(byte note, int pitchBendValue) {
	int lowestMidiNote = (LOWEST_MIDI_OCTAVE + 1) * 12;
	float noteForCV = (float)note - lowestMidiNote;
	if (pitchBendValue != 0) noteForCV += ((float)pitchBendValue / MIDI_PITCHBEND_MAX) * PITCH_BEND_SEMITONES;
	return min(4000, max(0, round(noteForCV * MIDI_NOTE_TO_CV_FACTOR)));
}

String getMidiNoteName(byte note) {
	String s = String(NOTE_NAMES[note % 12]);
	s.concat((note / 12) - 1); // Octave
	return s;
}

void debug(String line) {
	if (DEBUG) {
		if (DEBUG_WITH_TTYMIDI) {
			Serial.write((byte)0xFF);
			Serial.write((byte)0x00);
			Serial.write((byte)0x00);
			Serial.write((byte)line.length());
			Serial.print(line);
			Serial.flush();
		} else {
			Serial.println(line);
		}
	}
}

void debugMidiNote(String line, byte note) {
	if (DEBUG) {
		String m = String(line);
		m.concat(" ");
		m.concat(note);
		m.concat(" (");
		m.concat(getMidiNoteName(note));
		m.concat(")");
		debug(m);
	}
}

void debugVoices() {
	if (DEBUG) {
		String m = String("Voices: ");
		for (byte i = 0; i < N; i++) {
			m.concat(voiceActive[i] ? (voiceLocked[i] ? "{" : "[") : ".");
			m.concat(getMidiNoteName(voiceMidiNote[i]));
			m.concat(voiceActive[i] ? (voiceLocked[i] ? "}" : "]") : ".");
		}
		debug(m);
	}
}

void bootAnimation() {
	
	// Turn on all LEDs
	for (byte i = 0; i < N; i++) {
		gateLed[i].on();
		delay(100);
	}
	gateOrLed.on();
	delay(200);
	
	// Turn them off
	for (byte i = 0; i < N; i++) {
		gateLed[i].off();
		delay(100);
	}
	gateOrLed.off();
	delay(100);
	gateOrLed.loop();
	delay(200);
	
}
