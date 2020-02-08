
// CONFIGURATION =============================================================

// Pins A4 and A5 are reserved for the DAC I2C communication!

const bool DEBUG = false; // FALSE to disable debug messages on serial port

const byte CLOCK_INPUT = 2; // Pin for the input clock signal, must be usable for interrupts
const byte CLOCK_LED = 13; // LED pin for input clock signal indication
const unsigned int CLOCK_RESOLUTION = 8; // Resolution of the input clock, for example 8 to make clock count 8th notes (use only multiples of 2)

const byte PERFORMER_BUTTONS[] { 0, 1, 3, 4, 5, 6 }; // Button pins for performer advancing
const byte PERFORMER_GATE_LEDS[] { 7, 8, 9, 10, 11, 12 }; // LEDs pins for showing output gates
const byte PERFORMER_ALERT_BEHIND = 3; // LED will start blinking if performer is left behind by this number of patterns

const byte RESET_BUTTON = A0; // Pin for the reset button
const unsigned long RESET_BUTTON_LONG_PRESS_MS = 2000; // The reset button must be long-pressed to reset
const unsigned long DISPLAY_LATE_PERFORMERS_MS = 2000; // How long lit up the LEDs to display late performers

const byte GATES_SHIFT_REGISTER_DATA = A3; // 74HC595 serial data input (SER)
const byte GATES_SHIFT_REGISTER_CLOCK = A1; // 74HC595 shift register clock (SCK)
const byte GATES_SHIFT_REGISTER_LATCH = A2; // 74HC595 storage register clock (RCK)

const unsigned int GATE_RETRIG_MS = 70; // Time between two consecutive gates, to retrig envelopes
const float ACCIACCATURA_LENGTH = 0.4; // Length of the acciaccatura note, as a fraction of a single step

const unsigned long BUTTON_DEBOUNCE_DELAY = 50; // Debounce delay for all buttons
const unsigned long SEQUENCE_STOP_LONG_PRESS_MS = 500; // Button long-press duration to request sequence stop
const unsigned long PERFORMER_BUTTON_MULTI_ADVANCE_MS = 400; // Multi-press delay for the performer button to advance more than one pattern

const unsigned long LED_MIN_DURATION_MS = 50; // Minimum "on" duration for all LEDs visibility
const unsigned int LED_BLINK_PERIOD = 120; // Blink rate while waiting for a sequence to stop
const float LED_BLINK_DUTY = 0.1; // Blink duty cycle, to adjust overall brightness

// ===========================================================================

#include <Wire.h>
#include <avr/pgmspace.h>

#include "lib/Button.cpp"
#include "lib/Led.cpp"
#include "lib/MCP4728.cpp"
#include "lib/MM74HC595M.cpp"
#include "lib/MemoryFree.cpp"
#include "patterns/patterns.h"

// Maximum number of performers, used to allocate memory structures.
// It can't be more than 8, since there are two 4-channels DACs and a single 8-bit shift register.
// It can't be more than 6 on Arduino Nano, since performers need pins and lot of SRAM memory.
// The actual number of performers is the minimum between this and the number of buttons/LEDs declared in config above.
#define N_MAX 6

// Patterns with more steps than the max sequence size will be loaded in chunks while playing!
// Use this to adjust memory usage at expense of runtime complexity.
#define SEQUENCE_SIZE_MAX 64

// DACs and shift register, for all performers
MCP4728 dac1;
MCP4728 dac2;
MM74HC595M gates;

// Reset button
Button resetButton;
unsigned long displayLatePerformersTime = 0; // A short press make LEDs display late performers for a couple of seconds

// Each performer is playing a pattern, pressing the button will make it advance to the next.
// At the beginning each performer is in an initial state, where it outputs a constant CV for tuning and no gate.
byte n; // Number of performers
Button performerButton[N_MAX]; // Button for advancing the performer to the next sequence
Led performerGateLed[N_MAX]; // LED for showing performer's output gates
int8_t patternCurrent[N_MAX]; // Current pattern index, -1 if in initial state 
int8_t patternNext[N_MAX]; // Pattern to load when the current one loops
int8_t patternLeader = 0; // Pattern currently played by the more advanced performer
bool performerIsBehind[N_MAX]; // TRUE of the performer is lagging too far behind the leader
unsigned long performerButtonLastTime[N_MAX]; // Last time the performer button has been pressed to advance pattern

// The duration of a clock cycle, in steps
const unsigned int CLOCK_DURATION = PATTERNS_DURATION_RESOLUTION / CLOCK_RESOLUTION; 

// When a pattern is loaded, a simpler "sequence" is created, where each note is repeated to reach its original duration.
// This is like an ordinary sequencer's setup, where each single step has the same duration, that is the smallest possible.
byte sequenceStep[N_MAX][SEQUENCE_SIZE_MAX];
unsigned int sequenceLength[N_MAX]; // Number of total steps in the currently loaded sequence
unsigned int sequenceOffset[N_MAX]; // Offset for partially loaded sequences, always zero when playing a pattern shorter than SEQUENCE_SIZE_MAX
unsigned int sequenceAcciaccatura[N_MAX]; // Note CV for the acciaccatura note at the beginning of the sequence, if any
unsigned int sequenceAcciaccaturaTo[N_MAX]; // Target note CV for the acciaccatura
unsigned int sequencePlayhead[N_MAX]; // The index of the current step in the sequence
unsigned int sequencePlayheadClocked[N_MAX]; // A playhead that is moved forward by CLOCK_DURATION steps every clock pulse
unsigned long sequenceLastStepTime[N_MAX];
unsigned int sequenceLastCV[N_MAX];
bool sequenceLastGate[N_MAX];
bool sequenceStopped[N_MAX];
bool sequenceStoppedToggleRequest[N_MAX];

// CV and gate info of each step of the sequences is packed into a single byte:
//  - first 2 bits are gate info: 0=off, 1=on, 2=ending, 3=slide
//  - last 6 bits are the index for the CV values array (max 63)
#define STEP_CV_BITS   6
#define STEP_CV_MASK   0b00111111
#define STEP_GATE_MASK 0b11000000

// The input clock makes the sequences advance by CLOCK_DURATION steps.
// Each time a clock pulse is received, the sequence's clocked playhead is moved forward.
volatile bool clockFlag = false; // Clock signal change flag, set in the clock ISR
unsigned long clockLastTime = 0; // Time of the last clock pulse
unsigned long clockCount = 0; // Clock pulses global counter
unsigned long stepTime = 0; // The duration in ms of a sequence step, it depends on clock frequency
Led clockLed;

unsigned long statusDebugLastTime = 0;

void setup() {
	
	// Debugging
	if (DEBUG) {
		Serial.begin(9600);
		Serial.println(F("IN CV - joeSeggiola"));
	}
	
	// Find out the actual number of performers
	n = min(N_MAX, 8);
	n = min(n, sizeof(PERFORMER_BUTTONS) / sizeof(PERFORMER_BUTTONS[0]));
	n = min(n, sizeof(PERFORMER_GATE_LEDS) / sizeof(PERFORMER_GATE_LEDS[0]));
	if (DEBUG) {
		Serial.print(F("INITIALIZATION OF "));
		Serial.print(n);
		Serial.println(F(" PERFORMERS"));
	}
	
	// Reset button
	resetButton.init(RESET_BUTTON, BUTTON_DEBOUNCE_DELAY, true, true);
	
	// Init performers and their sequences
	for (byte p = 0; p < n; p++) {
		performerButton[p].init(PERFORMER_BUTTONS[p], BUTTON_DEBOUNCE_DELAY, true, true);
		performerGateLed[p].init(PERFORMER_GATE_LEDS[p], LED_MIN_DURATION_MS);
		performerButtonLastTime[p] = 0;
		sequenceLastStepTime[p] = 0;
	}
	
	// Init shift register for gates
	gates.init(GATES_SHIFT_REGISTER_DATA, GATES_SHIFT_REGISTER_CLOCK, GATES_SHIFT_REGISTER_LATCH);
	gates.write(0);
	
	// Init I2C communication and DACs
	Wire.begin();
	Wire.setClock(400000); // Fast mode
	dac1.init(Wire, 2);
	dac1.selectVref(MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V);
	dac1.selectPowerDown(MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL);
	dac1.selectGain(MCP4728::GAIN::X2, MCP4728::GAIN::X2, MCP4728::GAIN::X2, MCP4728::GAIN::X2);
	if (n > 4) {
		dac2.init(Wire, 3);
		dac2.selectVref(MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V);
		dac2.selectPowerDown(MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL);
		dac2.selectGain(MCP4728::GAIN::X2, MCP4728::GAIN::X2, MCP4728::GAIN::X2, MCP4728::GAIN::X2);
	}
	
	// DACs calibration
	dac1.calibrate(0, -12, 0.995024876);
	dac1.calibrate(1,  -5, 0.998502247);
	dac1.calibrate(2,  -2, 0.988630746);
	dac1.calibrate(3,  -6, 0.995024876);
	if (n > 4) {
		dac2.calibrate(0, -14, 0.998003992);
		dac2.calibrate(1,  -5, 0.989609104);
		dac2.calibrate(2,  -2, 0.986923267);
		dac2.calibrate(3,  -2, 0.993295257);
	}
	
	// Init clock
	clockLed.init(CLOCK_LED, LED_MIN_DURATION_MS);
	pinMode(CLOCK_INPUT, INPUT);
	
	bootAnimation();
	
	// Start listening for input clock
	attachInterrupt(digitalPinToInterrupt(CLOCK_INPUT), clockISR, RISING);
	
	reset(0);
	
}

void reset(unsigned long t) {
	
	// Reset state
	patternLeader = 0;
	displayLatePerformersTime = 0;
	for (byte p = 0; p < n; p++) {
		performerGateLed[p].off();
		patternCurrent[p] = -1;
		patternNext[p] = -1;
		performerIsBehind[p] = false;
		sequencePlayhead[p] = 0;
		sequencePlayheadClocked[p] = 0;
		sequenceLastCV[p] = 0;
		sequenceLastGate[p] = false;
		sequenceStopped[p] = false;
		sequenceStoppedToggleRequest[p] = false;
		patternLoadInitial(p);
	}
	
	// Tuning mode, setting all DACs to a fixed reference
	gates.write(0);
	dac1.analogWrite(TUNING_CV, TUNING_CV, TUNING_CV, TUNING_CV);
	if (n > 4) dac2.analogWrite(TUNING_CV, TUNING_CV, TUNING_CV, TUNING_CV);
	
	if (DEBUG) {
		if (t > 0) Serial.println(F("RESET"));
	}
	
}

void loop() {
	
	unsigned long t = millis();
	
	if (clockFlag) {
		clockFlag = false;
		clockLoop(t);
	}
	
	if (stepTime > 0) {
		sequenceLoop(t);
	}
	
	// Sequence button: advance to next pattern, or request sequence stop on long press
	for (byte p = 0; p < n; p++) {
		byte performerButtonRead = performerButton[p].readShortOrLongPressOnce(SEQUENCE_STOP_LONG_PRESS_MS);
		if (performerButtonRead == 1 && !sequenceStopped[p]) {
			if (patternNext[p] <= patternCurrent[p] || (t - performerButtonLastTime[p]) <= PERFORMER_BUTTON_MULTI_ADVANCE_MS) {
				patternAdvance(p);
			}
			performerButtonLastTime[p] = t;
		} else if (performerButtonRead > 0) {
			if (patternCurrent[p] >= 0) { // If not in initial state
				sequenceStoppedToggle(p);
			}
		}
	}
	
	// Reset button
	byte resetButtonRead = resetButton.readShortOrLongPressOnce(RESET_BUTTON_LONG_PRESS_MS);
	if (resetButtonRead == 2) reset(t);
	displayLatePerformers(resetButtonRead == 1, t);
	
	clockLed.loop();
	for (byte p = 0; p < n; p++) performerGateLed[p].loop();
	
	if (DEBUG) {
		if (t > statusDebugLastTime + 4000) debugStatus(t);
	}
	
}

void patternAdvance(byte p) {
	
	if (patternNext[p] + 1 < PATTERNS_N) {
		patternNext[p]++;
		
		if (DEBUG) {
			Serial.print(F("WILL ADVANCE PERFORMER "));
			Serial.print(p + 1);
			Serial.print(F(" to PATTERN #"));
			Serial.println(patternNext[p] + 1);
		}
		
	}
	
	// Preload the acciaccatura of the next pattern, to be played (or not) at the end of the current loop before switching
	if (patternNext[p] < PATTERNS_N - 1) {
		sequenceAcciaccatura[p] = pgm_read_word(PATTERNS_ACCIACCATURA_CV + patternNext[p]);
		sequenceAcciaccaturaTo[p] = PATTERNS_CV[pgm_read_byte(pgm_read_word(PATTERNS_CV_INDEX + patternNext[p]))];
	} else {
		sequenceAcciaccatura[p] = 0;
	}
	
}

void patternLoad(byte p, byte i, int offset, unsigned long t) {
	
	// Create the sequence from the pattern
	unsigned int length = 0;
	byte size = pgm_read_byte(PATTERNS_SIZE + i);
	byte* cvs = pgm_read_word(PATTERNS_CV_INDEX + i);
	byte* durations = pgm_read_word(PATTERNS_DURATION + i);
	byte* slides = pgm_read_word(PATTERNS_SLIDE + i);
	sequenceAcciaccatura[p] = pgm_read_word(PATTERNS_ACCIACCATURA_CV + i);
	for (byte j = 0; j < size; j++) {
		byte cv = pgm_read_byte(cvs + j); // CV index
		byte duration = pgm_read_byte(durations + j);
		bool slide = bitRead(pgm_read_byte(slides + (j / 8)), 7 - (j % 8));
		bool rest = cv == 0; // If CV is zero, this represent a rest
		for (byte k = 0; k < duration; k++) {
			int l = (int)length - offset; // Offset index
			if (l >= 0 && l < SEQUENCE_SIZE_MAX) {
				bool end = k == duration - 1; // This note is ending in this step
				byte gate = rest ? 0 : (slide ? 3 : (end ? 2 : 1)); // Gate info
				sequenceStep[p][l] = ((gate << STEP_CV_BITS) & STEP_GATE_MASK) | (rest ? 0 : (cv & STEP_CV_MASK));
			}
			length++;
		}
	}
	
	sequenceLength[p] = length;
	sequenceOffset[p] = offset;
	patternCurrent[p] = i;
	
	// Check if some performer is lagging too far behind
	if (offset == 0) { // Can happen only when new patterns are loaded, offset is zero
		if (patternCurrent[p] > patternLeader) patternLeader = patternCurrent[p];
		for (byte pp = 0; pp < n; pp++) {
			if (patternCurrent[pp] >= 0) { // If not in initial state
				performerIsBehind[pp] = patternCurrent[pp] < patternLeader - PERFORMER_ALERT_BEHIND + 1;
				if (performerIsBehind[pp]) {
					if (!sequenceStopped[pp]) {
						performerGateLed[pp].blink(LED_BLINK_PERIOD, LED_BLINK_DUTY); // Start blinking to alert
					}
				} else if (!sequenceStoppedToggleRequest[pp]) {
					performerGateLed[pp].off();
				}
			}
		}
	}
	
	if (DEBUG) {
		Serial.print(F("LOADED PATTERN #"));
		Serial.print(i + 1);
		if (sequenceLength[p] > SEQUENCE_SIZE_MAX) {
			Serial.print(F("/"));
			Serial.print(offset);
		}
		Serial.print(F(" ON PERFORMER "));
		Serial.print(p + 1);
		Serial.print(F(" - Length: "));
		Serial.print(sequenceLength[p]);
		Serial.print(F(" - Free memory: "));
		Serial.print(freeMemory());
		Serial.println(F(" bytes"));
		if (i > 0) debugStatus(t);
	}
	
}

void patternLoadInitial(byte p) {
	
	// Load an empty sequence
	unsigned int length = CLOCK_DURATION * 2;
	for (byte k = 0; k < length; k++) sequenceStep[p][k] = 0;
	sequenceAcciaccatura[p] = 0;
	
	sequenceLength[p] = length;
	sequenceOffset[p] = 0;
	patternCurrent[p] = -1;
	
}

void clockLoop(unsigned long t) {
	
	if (clockLastTime > 0) {
		
		// The whole measure duration is the clock period multiplied by the clock resolution.
		// For example, if the clock has a period of 20ms and its resolution is 16th, a measure is 320ms long
		unsigned long measureTime = (t - clockLastTime) * CLOCK_RESOLUTION;
		if (measureTime > 500) { // Avoid being too fast, this also filters clock rising bounces
			
			// Update the duration of the sequence step
			stepTime = measureTime / PATTERNS_DURATION_RESOLUTION;
			clockLastTime = t;
			clockCount++;
			
			for (byte p = 0; p < n; p++) {
				
				// Move the clocked playheads
				sequencePlayheadClocked[p] = (sequencePlayheadClocked[p] + CLOCK_DURATION) % sequenceLength[p];
				
				// Restart the sequence if requested, it will play from the first step.
				if (sequenceStopped[p] && sequenceStoppedToggleRequest[p]) {
					sequenceStoppedToggleRequest[p] = false;
					sequenceStopped[p] = false;
					sequencePlayheadClocked[p] = 0;
					if (performerIsBehind[p]) {
						performerGateLed[p].blink(LED_BLINK_PERIOD, LED_BLINK_DUTY); // Restart blinking if it's behind
					}
				}
				
			}
			
			clockLed.flash();
			
		}
		
	} else {
		clockLastTime = t;
	}
	
}

void sequenceLoop(unsigned long t) {
	
	// Flags that will tell if it's necessary to update CV DACs and gates SR
	bool updateCV[] { false, false }; // On each DAC
	bool updateGates = false;
	
	// Get the sequence playhead position: start from the clock playhead (which is incremented by many steps
	// every clock cycle) and move forward depending on how much time has passed from last clock pulse.
	// Don't move past the next expected clock, i.e. don't move more than clock duration minus 1.
	unsigned int stepsAheadOfClock = min((t - clockLastTime) / stepTime, CLOCK_DURATION - 1);
	
	for (byte p = 0; p < n; p++) {
		
		// CV and gate that will be set in this step for this performer
		unsigned int cv = 0; // Zero to leave as it was
		bool gate = false;
		
		if (!sequenceStopped[p]) {
			
			unsigned int playhead = (sequencePlayheadClocked[p] + stepsAheadOfClock) % sequenceLength[p];
			unsigned int playheadOffset = playhead - sequenceOffset[p];
			
			// Playhead moved
			if (playhead != sequencePlayhead[p]) {
				
				// If playhead just rewound, sequence is restarting 
				if (playhead < sequencePlayhead[p]) {
					
					if (sequenceStoppedToggleRequest[p]) {
						
						// The sequence must be stopped
						sequenceStoppedToggleRequest[p] = false;
						sequenceStopped[p] = true;
						performerGateLed[p].off(); // Stop blinking, you're done waiting to stop
						
					} else if (patternCurrent[p] != patternNext[p]) {
						
						// There's a next pattern to load
						patternLoad(p, patternNext[p], 0, t);
						playhead = playhead % sequenceLength[p]; // Adjust playhead to new pattern
						sequencePlayheadClocked[p] = integerModulo(playhead - stepsAheadOfClock, sequenceLength[p]);
						playheadOffset = playhead;
						
					}
					
				}
				
				// If playhead moved into a non-loaded part of the sequence, load a new chunk
				if (sequenceLength[p] > SEQUENCE_SIZE_MAX) {
					if ((playhead < sequenceOffset[p]) || (playhead >= sequenceOffset[p] + SEQUENCE_SIZE_MAX)) {
						patternLoad(p, patternCurrent[p], playhead, t);
						playheadOffset = 0;
					}
				}
				
				// Play the new step
				if (!sequenceStopped[p]) {
					sequencePlayhead[p] = playhead;
					cv = PATTERNS_CV[sequenceStep[p][playheadOffset] & STEP_CV_MASK];
					sequenceLastStepTime[p] = t;
				}
				
			}
			
			if (!sequenceStopped[p]) {
				
				// Should play the acciaccatura note in this step?
				// The acciaccatura must be played at the end of the last step in the loop, to "anticipate" the first note!
				unsigned long acciaccaturaLengthMs = 0;
				if (sequenceAcciaccatura[p] > 0) {
					if (playhead == sequenceLength[p] - 1) {
						acciaccaturaLengthMs = stepTime * ACCIACCATURA_LENGTH;
					}
				}
				
				// Compute gate on/off, acciaccatura
				byte gateInfo = (sequenceStep[p][playheadOffset] & STEP_GATE_MASK) >> STEP_CV_BITS;
				if (gateInfo == 1) {
					gate = true;
				} else if (gateInfo == 2) {
					unsigned long gateRetrig = GATE_RETRIG_MS + acciaccaturaLengthMs;
					gate = t < sequenceLastStepTime[p] + max(5, (long)stepTime - (long)gateRetrig); // Cast to avoid unsigned underflow
				} else if (gateInfo == 3) {
					gate = true;
					unsigned int cvFrom = PATTERNS_CV[sequenceStep[p][playheadOffset] & STEP_CV_MASK];
					unsigned int cvTo = PATTERNS_CV[sequenceStep[p][playheadOffset + 1] & STEP_CV_MASK]; // Unsafe, won't work if the slide crosses the loaded chunk
					float interpolationFactor = (float)(t - sequenceLastStepTime[p]) / stepTime;
					cv = interpolate(cvFrom, cvTo, interpolationFactor, 4);
				}
				
				// Play the acciaccatura!
				if (acciaccaturaLengthMs > 0) {
					if (t >= sequenceLastStepTime[p] + stepTime - acciaccaturaLengthMs) {
						gate = true;
						float interpolationFactor = (float)(t - (sequenceLastStepTime[p] + stepTime - acciaccaturaLengthMs)) / acciaccaturaLengthMs;
						cv = interpolate(sequenceAcciaccatura[p], sequenceAcciaccaturaTo[p], interpolationFactor, 8);
					}
				}
				
			}
			
		}
		
		if (cv != 0 && cv != sequenceLastCV[p]) {
			updateCV[p < 4 ? 0 : 1] = true;
			sequenceLastCV[p] = cv;
		}
		
		if (gate != sequenceLastGate[p]) {
			updateGates = true;
			sequenceLastGate[p] = gate;
			
			// Flash when gate goes on, unless it's already blinking for a pending stop request, or because
			// the performer is lagging too far behind the leader, or fixed to display late performers
			if (gate && !performerIsBehind[p] && !sequenceStoppedToggleRequest[p] && displayLatePerformersTime == 0) {
				performerGateLed[p].flash();
			}
			
		}
		
	}
	
	if (updateCV[0] || updateCV[1]) {
		unsigned int dacValues[8];
		for (byte i = 0; i < 8; i++) dacValues[i] = i < n ? sequenceLastCV[i] : 0;
		if (updateCV[0]) dac1.analogWrite(dacValues[0], dacValues[1], dacValues[2], dacValues[3]);
		if (updateCV[1]) dac2.analogWrite(dacValues[4], dacValues[5], dacValues[6], dacValues[7]);
	}
	
	if (updateGates) {
		byte srValue = 0;
		for (byte i = 0; i < 8; i++) if (i < n && sequenceLastGate[i]) bitSet(srValue, i);
		gates.write(srValue);
	}
	
}

void sequenceStoppedToggle(byte p) {
	if (sequenceStoppedToggleRequest[p]) {
		if (!sequenceStopped[p]) {
			sequenceStoppedToggleRequest[p] = false; // Undo the pending request
			if (!performerIsBehind[p]) {
				performerGateLed[p].off(); // Stop blinking
			}
		}
	} else {
		sequenceStoppedToggleRequest[p] = true; // Request sequence stop when loop ends
		if (!sequenceStopped[p] && !performerIsBehind[p]) {
			performerGateLed[p].blink(LED_BLINK_PERIOD, LED_BLINK_DUTY); // Blink while waiting to stop
		}
	}
}

void displayLatePerformers(bool display, unsigned long t) {
	
	if (display) {
		
		// Light up the LEDs of late performers
		displayLatePerformersTime = t;
		int8_t patternTail = patternLeader - 1;
		for (byte p = 0; p < n; p++) {
			if (patternCurrent[p] >= 0 && patternCurrent[p] < patternTail) {
				patternTail = patternCurrent[p];
			}
		}
		for (byte p = 0; p < n; p++) {
			if (patternCurrent[p] >= 0 && patternCurrent[p] <= patternTail) {
				performerGateLed[p].on();
			} else {
				performerGateLed[p].off();
			}
		}
		
	} else if (displayLatePerformersTime > 0) {
		
		// Turn display off
		if (t - displayLatePerformersTime > DISPLAY_LATE_PERFORMERS_MS) {
			displayLatePerformersTime = 0;
			for (byte p = 0; p < n; p++) {
				if (performerIsBehind[p] && !sequenceStopped[p]) {
					performerGateLed[p].blink(LED_BLINK_PERIOD, LED_BLINK_DUTY); // Restart blinking if it's behind
				} else {
					performerGateLed[p].off();
				}
			}
		}
		
	}
	
}

void clockISR() {
	clockFlag = true;
}

int integerModulo(int a, int b) {
	return (((a % b) + b) % b); // http://yourdailygeekery.com/2011/06/28/modulo-of-negative-numbers.html
}

unsigned int interpolate(unsigned int a, unsigned int b, float factor, float exp) {
	float f = max(0, min(1, pow(factor, exp)));
	return (1 - f) * a + f * b;
}

void debugStatus(unsigned long t) {
	if (DEBUG) {
		statusDebugLastTime = t;
		Serial.print(F("STATUS - Step time: "));
		Serial.print(stepTime);
		Serial.print(F(" ms - Clock time: "));
		Serial.print(stepTime * CLOCK_DURATION);
		Serial.print(F(" ms - Patterns"));
		for (byte p = 0; p < n; p++) {
			Serial.print(F(" #"));
			if (patternCurrent[p] >= 0) {
				Serial.print(patternCurrent[p] + 1);
				if (sequenceLength[p] > SEQUENCE_SIZE_MAX) {
					Serial.print(F("/"));
					Serial.print(sequenceOffset[p]);
				}
				if (sequenceStopped[p]) {
					Serial.print(F("/P"));
				} else if (performerIsBehind[p]) {
					Serial.print(F("/B"));
				}
			} else {
				Serial.print(F("-"));
			}
		}
		Serial.print(F("\r\n"));
	}
}

void bootAnimation() {
	
	// Turn on all LEDs
	clockLed.on();
	for (byte p = 0; p < n; p++) {
		delay(100);
		performerGateLed[p].on();
	}
	
	// Wait and turn them off
	delay(200);
	clockLed.off();
	delay(100);
	clockLed.loop();
	for (byte p = 0; p < n; p++) {
		performerGateLed[p].off();
		delay(100);
		for (byte p = 0; p < n; p++) performerGateLed[p].loop();
		clockLed.loop();
	}
	
}
