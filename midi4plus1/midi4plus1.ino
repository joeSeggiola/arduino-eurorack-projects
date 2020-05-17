
// CONFIGURATION =============================================================

// Pin 1 is reserved for MIDI RX (PIN0->220Ω->DIN5, 5V->220Ω->DIN4).
// Pins A4 and A5 are reserved for the DAC I2C communication.

const byte MODE_BUTTON = A0; // Main button pin
const byte MODE_LEDS[] = { A1, A2 }; // Bi-color LED pins for current mode display

const byte GATES[] { 3, 4, 5, 6 }; // Gate 1-4 pins
const byte GATE_OR = 7; // Auxiliary gate pin, high when at least one gate is high
const byte GATES_LEDS[] { 8, 9, 10, 11 }; // LEDs pins for gates display
const byte GATE_OR_LED = 12; // Auxiliary gate pin, high when at least one gate is high

const byte NOTE_ON_LED = 13; // LED pin for showing note-on signals

const unsigned int GATE_RETRIG_MS = 40; // Time between two consecutive gates in ms, to retrig envelopes
const byte PITCH_BEND_SEMITONES = 2; // Picth-bend range in semitones
const byte SPLIT_MIDI_OCTAVE = 4; // Defines on which MIDI octave the keyboard will be split for poly+mono mode
const int LOWEST_MIDI_OCTAVE = 2; // CV out range is four octaves max, this defines which MIDI octave will be mapped to zero CV

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
#include <Wire.h>

#include "lib/Button.cpp"
#include "lib/Led.cpp"
#include "lib/MCP4728.cpp"

#include "mono.cpp"
#include "poly.cpp"

#define N 4 // Number of voices, each with its own CV/gate signals

#define MODE_POLY 0 // 4x polyphony with priority to last 
#define MODE_POLY_FIRST 1 // 4x polyphony with priority to first
#define MODE_POLY_MONO 2 // Keyboard split with 3x polyphony on lower keys, monophony on higher keys
#define MODE_MONO_POLY 3 // Keyboard split with monophony on lower key, 3x polyphony on higher keys

Button modeButton;
Led modeLed[2];
MCP4728 dac;
Led gateLed[N];
Led gateOrLed;
Led noteOnLed;

NoteStack mono;
VoiceAllocator poly;

byte mode = MODE_POLY;
byte voiceMidiNote[N]; // Current MIDI note for each voice
bool voiceActive[N]; // Current activation state for each voice
unsigned long voiceRetrigTime[N]; // Time the retrig interval started, zero if retrig is not occurring
bool voiceLocked[N]; // TRUE if the voice is currently locked
unsigned long voiceLockLedTime = 0; // Time the mode LED has been turned off to signal lock
int pitchBend; // Pitch-bend value (all voices in poly modes, monophonic voice only in split modes)
bool outputFlag; // TRUE if it's necessary to update the outputs

const int MODE_EEPROM_ADDRESS = 0;

#define MIDI_NOTE_TO_CV_FACTOR 83.333333 // 1000/12, i.e. 1000mV per octave
#define MIDI_PITCHBEND_MAX 8191 // Maximum value for pitch-bending, as given by MIDI.h library

const char NOTE_NAMES[12][3] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };

// MIDI custom settings
// https://github.com/FortySevenEffects/arduino_midi_library/wiki/Using-custom-Settings
struct MIDISettings : public midi::DefaultSettings {
	static const bool UseRunningStatus = false;
	static const bool Use1ByteParsing = false;
};
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, MIDI, MIDISettings);

void setup() {
	
	// Init MIDI
	MIDI.setHandleNoteOn(handleNoteOn);
	MIDI.setHandleNoteOff(handleNoteOff);
	MIDI.setHandlePitchBend(handlePitchBend);
	MIDI.begin(MIDI_CHANNEL_OMNI); // Listen to all channels
	MIDI.turnThruOff(); // Required after begin()
	
	// Debugging
	if (DEBUG || DEBUG_WITH_TTYMIDI) {
		Serial.begin(9600);
		if (DEBUG) debug("MIDI 4+1 - joeSeggiola");
	}
	
	// Setup I/O
	modeButton.init(MODE_BUTTON, BUTTON_DEBOUNCE_DELAY, true, true);
	modeLed[0].init(MODE_LEDS[0]);
	modeLed[1].init(MODE_LEDS[1]);
	pinMode(GATE_OR, OUTPUT);
	gateOrLed.init(GATE_OR_LED, LED_MIN_DURATION_MS);
	noteOnLed.init(NOTE_ON_LED, LED_MIN_DURATION_MS);
	for (byte i = 0; i < N; i++) {
		pinMode(GATES[i], OUTPUT);
		gateLed[i].init(GATES_LEDS[i], LED_MIN_DURATION_MS);
	}
	
	// Init I2C communication and DAC
	Wire.begin();
	Wire.setClock(400000); // Fast mode
	dac.init(Wire, 0);
	dac.selectVref(MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V);
	dac.selectPowerDown(MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL);
	dac.selectGain(MCP4728::GAIN::X2, MCP4728::GAIN::X2, MCP4728::GAIN::X2, MCP4728::GAIN::X2);
	
	// DAC calibration
	dac.calibrate(0, -11, 0.990099010);
	dac.calibrate(1,  -9, 0.997008973);
	dac.calibrate(2,  -5, 1.000250063);
	dac.calibrate(3,  -2, 0.988142292);
	
	// Init voice allocators
	mono.init();
	poly.init();
	for (byte i = 0; i < N; i++) voiceMidiNote[i] = 12; // Init voices to C0
	
	bootAnimation();
	
	// Init mode from permanent storage
	byte initialMode = EEPROM.read(MODE_EEPROM_ADDRESS) % 4;
	setMode(initialMode);
	
}

void loop() {
	
	MIDI.read();
	
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
	
	// Check for mode button presses
	byte modeButtonPress = modeButton.readShortOrLongPressOnce(BUTTON_LOCK_LONG_PRESS_MS);
	if (modeButtonPress == 1) {
		setMode((mode + 1) % 4); // Short-press: cycle through modes
	} else if (modeButtonPress == 2) {
		voicesLock(); // Long-press: lock voices
	}
	
	// Re-set mode LED if necessary
	if (voiceLockLedTime > 0 && voiceLockLedTime + LED_MODE_LOCK_DURATION_MS < millis()) {
		voiceLockLedTime = 0;
		setModeLed();
	}
	
	// Update LEDs
	gateOrLed.loop();
	noteOnLed.loop();
	modeLed[0].loop();
	modeLed[1].loop();
	for (byte i = 0; i < N; i++) gateLed[i].loop();
	
}

void setMode(byte m) {
	
	// Setup allocators
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
	modeLed[0].off();
	modeLed[1].off();
	
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
	mono.clear();
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
	dac.analogWrite(dacValues[0], dacValues[1], dacValues[2], dacValues[3]);
	
	// Update gates
	for (byte i = 0; i < N; i++) {
		bool active = voiceActive[i] && voiceRetrigTime[i] == 0;
		digitalWrite(GATES[i], active);
		gateLed[i].set(active);
	}
	
	// Update OR gate
	bool gateOrActive = false;
	byte gateOrFirstVoice = mode == MODE_MONO_POLY ? 1 : 0;
	byte gateOrLastVoice = mode == MODE_POLY_MONO ? N - 2 : N - 1;
	for (byte i = gateOrFirstVoice; i <= gateOrLastVoice; i++) {
		gateOrActive |= voiceActive[i];
	}
	digitalWrite(GATE_OR, gateOrActive);
	gateOrLed.set(gateOrActive);
	
	if (DEBUG) debugVoices();
	
}

void setModeLed() {
	modeLed[0].set(mode == MODE_POLY || mode == MODE_POLY_FIRST); // Red
	modeLed[1].set(mode == MODE_POLY_FIRST || mode == MODE_POLY_MONO || mode == MODE_MONO_POLY); // Green
	if (mode == MODE_MONO_POLY) modeLed[0].blink(20, 0.2); // Dim the red LED to make yellow (red + green)
}

void handleNoteOn(byte channel, byte note, byte velocity) {
	noteOnLed.flash();
	if (isNoteForMonophony(note)) {
		mono.noteOn(note);
		byte i = getMonophonyVoiceIndex();
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
		int newNote = mono.noteOff(note);
		byte i = getMonophonyVoiceIndex();
		if (newNote > -1) {
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

bool isNoteForMonophony(byte note) {
	bool higherKey = note >= (SPLIT_MIDI_OCTAVE + 1) * 12; // Check keyboard split
	return ( // Return TRUE if the pressed note should be used for monophony
		(mode == MODE_POLY_MONO && higherKey) ||
		(mode == MODE_MONO_POLY && !higherKey)
	);
}

byte getMonophonyVoiceIndex() {
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
			Serial.write(0xFF);
			Serial.write(0x00);
			Serial.write(0x00);
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
	gateOrLed.on();
	for (byte i = 0; i < N; i++) {
		delay(100);
		gateLed[i].on();
	}
	
	// Wait and turn them off
	delay(200);
	gateOrLed.off();
	delay(100);
	gateOrLed.loop();
	for (byte i = 0; i < N; i++) {
		gateLed[i].off();
		delay(100);
		for (byte j = 0; j < N; j++) gateLed[j].loop();
		gateOrLed.loop();
	}
	
}