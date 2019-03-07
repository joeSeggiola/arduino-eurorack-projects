	
// CONFIGURATION =============================================================

const HardwareSerial *DEBUG = 0; // &Serial to debug on USB, or zero to disable debugging

const int CLOCK_INPUT = 2; // Input signal pin, must be usable for interrupts
const int CLOCK_LED = 1; // LED pin for input signal indication
const int RESET_INPUT = 3; // Reset signal pin, must be usable for interrupts
const int RESET_BUTTON = 12; // Reset button pin

const int DIVISIONS[] { 2, 3, 4, 5, 6, 8, 16, 32 }; // Integer divisions of the input clock
const int DIVISIONS_OUTPUT[] { 4, 5, 6, 7, 8, 9, 10, 11 }; // Output pins
const int DIVISIONS_LEDS[] { 0, A5, A4, A3, A2, A1, A0, 13 }; // LEDs pins

const unsigned long MODE_SWITCH_LONG_PRESS_DURATION_MS = 3000; // Reset button long-press duration for trig/gate mode switch
const unsigned long BUTTON_DEBOUNCE_DELAY = 50; // Debounce delay for all buttons
const unsigned long LED_MIN_DURATION_MS = 50; // Minimum "on" duration for all LEDs visibility

// ===========================================================================

#include <EEPROM.h>
#include "lib/Led.cpp"
#include "lib/Button.cpp"

unsigned int n = 0; // Number of divisions
long count = -1; // Input clock counter, -1 in order to go to 0 no the first pulse
bool modeSwitchAvailable = true; // FALSE when gate/trig mode has just been engaged, but the button is still pressed
bool gateMode = false; // TRUE if gate mode is active, FALSE if standard trig mode is active

Led* clockLed; // Input LED
Led** leds; // Output LEDs
Button* resetButton;

volatile bool clock = false; // Clock signal digital reading, set in the clock ISR
volatile bool clockFlag = false; // Clock signal change flag, set in the clock ISR
volatile bool resetFlag = false; // Reset flag, set in the reset ISR

const int MODE_EEPROM_ADDRESS = 0;

void setup() {
	
	// Debugging
	if (DEBUG) DEBUG->begin(9600);
	
	// Number of divisions
	n = sizeof(DIVISIONS) / sizeof(DIVISIONS[0]);
	
	// Trig/gate mode from permanent storage
	gateMode = EEPROM.read(MODE_EEPROM_ADDRESS) == 1;
	
	// Input
	resetButton = new Button(RESET_BUTTON, BUTTON_DEBOUNCE_DELAY);
	
	// Setup outputs (divisions and LEDs)
	clockLed = new Led(CLOCK_LED, LED_MIN_DURATION_MS);
	leds = new Led*[n];
	for (int i = 0; i < n; i++) {
		leds[i] = new Led(DIVISIONS_LEDS[i], LED_MIN_DURATION_MS);
		pinMode(DIVISIONS_OUTPUT[i], OUTPUT);
		digitalWrite(DIVISIONS_OUTPUT[i], LOW);
	}
	
	// Interrupts
	pinMode(CLOCK_INPUT, INPUT);
	pinMode(RESET_INPUT, INPUT);
	attachInterrupt(digitalPinToInterrupt(CLOCK_INPUT), isrClock, CHANGE);
	attachInterrupt(digitalPinToInterrupt(RESET_INPUT), isrReset, RISING);
	
}

void loop() {
	
	// Read manual reset button and set the flag
	if (!resetFlag) {
		if (resetButton->read()) {
			resetFlag = true;
		}
	}

	// Clock signal changed
	if (clockFlag) {
		clockFlag = false;
		
		if (DEBUG) {
			DEBUG->print("Clock signal changed: ");
			DEBUG->println(clock);
		}
		
		// Input LED
		clockLed->set(clock);
		
		if (clock) {
			
			// Clock rising, update counter
			if (resetFlag) {
				resetFlag = false;
				count = 0;
			} else {
				count++;
			}
			
			if (DEBUG) {
				DEBUG->print("Counter changed: ");
				DEBUG->println(count);
			}
			
		}
		
		// Update outputs according to current trig/gate mode
		if (gateMode) {
			processGateMode();
		} else {
			processTriggerMode();
		}
		
	}
	
	// Mode switch
	if (resetButton->readLongPress(MODE_SWITCH_LONG_PRESS_DURATION_MS)) {
		if (modeSwitchAvailable) {
			modeSwitchAvailable = false; // Avoid re-triggering while the button is kept pressed
			gateMode = !gateMode;
			EEPROM.update(MODE_EEPROM_ADDRESS, gateMode ? 1 : 0); // Mode selection on permanent storage
		}
	} else {
		modeSwitchAvailable = true;
	}
	
	// Update LEDs
	clockLed->loop();
	for (int i = 0; i < n; i++) leds[i]->loop();
	
}

void processTriggerMode() {
	
	// Copy input signal on current divisions
	if (clock) {
		
		// Rising edge, go HIGH on current divisions
		for (int i = 0; i < n; i++) {
			bool v = (count % DIVISIONS[i] == 0);
			digitalWrite(DIVISIONS_OUTPUT[i], v ? HIGH : LOW);
			leds[i]->set(v);
		}
		
	} else {
		
		// Falling edge, go LOW on every output
		for (int i = 0; i < n; i++) {
			digitalWrite(DIVISIONS_OUTPUT[i], LOW);
			leds[i]->off();
		}
		
	}
	
}

void processGateMode() {
	
	// Keep outputs high for ~50% of divided time
	for (int i = 0; i < n; i++) {
		
		// Go HIGH on the rising edges that corresponds to the division
		int modulo = (count % DIVISIONS[i]);
		if (clock && modulo == 0) {
			digitalWrite(DIVISIONS_OUTPUT[i], HIGH);
			leds[i]->on();
		}
		
		// Go LOW on rising edges for even divisions and falling edges for odd divisions,
		// considering the edges that corresponds to the half value of the division
		if (modulo == (int)(floor(DIVISIONS[i] / 2.0))) {
			bool divisionIsOdd = (DIVISIONS[i] % 2 != 0);
			if ((clock && !divisionIsOdd) || (!clock && divisionIsOdd)) {
				digitalWrite(DIVISIONS_OUTPUT[i], LOW);
				leds[i]->off();
			}
		}
		
	}
	
}

void isrClock() {
	clock = (digitalRead(CLOCK_INPUT) == HIGH);
	clockFlag = true;
}

void isrReset() {
	resetFlag = true;
}
