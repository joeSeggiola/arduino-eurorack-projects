	
// CONFIGURATION =============================================================

const HardwareSerial *DEBUG = 0; // &Serial to debug on USB, or zero to disable debugging

const int CLOCK_INPUT = 2; // Input signal pin, must be usable for interrupts
const int CLOCK_LED = 1; // LED pin for input signal indication
const int RESET_INPUT = 3; // Reset signal pin, must be usable for interrupts
const int RESET_BUTTON = 12; // Reset button pin

const int DIVISIONS[] { 2, 3, 4, 5, 6, 8, 16, 32 }; // Integer divisions of the input clock
const int DIVISIONS_OUTPUT[] { 4, 5, 6, 7, 8, 9, 10, 11 }; // Output pins
const int DIVISIONS_LEDS[] { 0, A5, A4, A3, A2, A1, A0, 13 }; // LEDs pins

const unsigned long BUTTON_DEBOUNCE_DELAY = 50; // Debounce delay for all buttons
const unsigned long LED_MIN_DURATION_MS = 50; // Minimum "on" duration for all LEDs visibility

// ===========================================================================

#include "lib/Led.cpp"
#include "lib/Button.cpp"

unsigned int n = 0; // Number of divisions
long count = -1; // Input clock counter, -1 in order to go to 0 no the first pulse

Led* clockLed; // Input LED
Led** leds; // Output LEDs
Button* resetButton;

volatile bool clock = false; // Clock signal digital reading, set in the clock ISR
volatile bool clockFlag = false; // Clock signal change flag, set in the clock ISR
volatile bool resetFlag = false; // Reset flag, set in the reset ISR

void setup() {
	
	// Debugging
	if (DEBUG) DEBUG->begin(9600);
	
	// Number of divisions
	n = sizeof(DIVISIONS) / sizeof(DIVISIONS[0]);
	
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
			
			// Update outputs and LEDs
			clockLed->on();
			for (int i = 0; i < n; i++) {
				bool v = (count % DIVISIONS[i] == 0);
				digitalWrite(DIVISIONS_OUTPUT[i], v ? HIGH : LOW);
				leds[i]->set(v);
			}
			
		} else {
			
			// Clock falling, turn off everything
			clockLed->off();
			for (int i = 0; i < n; i++) {
				digitalWrite(DIVISIONS_OUTPUT[i], LOW);
				leds[i]->off();
			}
			
		}
		
	}
	
	// Update LEDs
	clockLed->loop();
	for (int i = 0; i < n; i++) leds[i]->loop();
	
}

void isrClock() {
	clock = (digitalRead(CLOCK_INPUT) == HIGH);
	clockFlag = true;
}

void isrReset() {
	resetFlag = true;
}
