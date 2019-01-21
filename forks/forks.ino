	
// CONFIGURATION =============================================================

const HardwareSerial *DEBUG = 0; // &Serial to debug on USB, or zero to disable debugging

const int INPUTS[] { 2, 3 }; // Input, one for each Forks channel
const int INPUT_BUTTONS[] { 4, 5 }; // Manual input buttons
const int OUTPUTS_A[] { 8, 10 }; // First outputs
const int OUTPUTS_B[] { 9, 11 }; // Second outputs

const int PROBABILITY_KNOBS[] { A1, A0 }; // Probability knobs pins
const int PROBABILITY_CV_INPUTS[] { A7, A6 }; // Probability CV inputs

const int MODE_TOGGLE_PINS[] { 6, A2 }; // Switch for enabling toggle mode
const int MODE_LATCH_PINS[] { 7, A3 }; // Switch for enabling latch mode
const unsigned long MODE_POLL_EVERY_MS = 100; // Check for mode switches periodically

const int LEDS_A[] { A4, 12 }; // LED indicators for first outputs
const int LEDS_B[] { A5, 13 }; // LED indicators for second outputs

const int PROBABILITY_KNOBS_THRESHOLD_LOW = 10; // Everything read under this value in the 0-1023 scale is considered the minimum value
const int PROBABILITY_KNOBS_THRESHOLD_HIGH = 1024 - 5; // Everything read over this value in the 0-1023 scale is considered the maximum value
const int PROBABILITY_CV_INPUTS_THRESHOLD_LOW = 10; // Everything read under this value in the 0-1023 scale is considered the minimum value
const int PROBABILITY_CV_INPUTS_THRESHOLD_HIGH = 1024 - 5; // Everything read over this value in the 0-1023 scale is considered the maximum value

const unsigned long BUTTON_DEBOUNCE_DELAY = 50; // Debounce delay for manual input buttons
const unsigned long LED_MIN_DURATION_MS = 50; // Minimum "on" duration for all LEDs visibility

// ===========================================================================

#include "lib/Led.cpp"
#include "lib/Button.cpp"
#include "lib/Knob.cpp"

unsigned int n = 0; // Number of channels

Button** buttons;
Knob** knobs;
Knob** cvs;
Led** ledsA; // LEDs for outputs A
Led** ledsB; // LEDs for outputs B

volatile bool* inputs; // Input signal digital reading, set in ISR
bool* inputsLast; // TRUE if input signal was high or manual button was pressed
bool* outcomeLast = false; // Last outcome for toggle mode

bool* modeToggle; // TRUE if toggle mode is enabled for the channel
bool* modeLatch; // TRUE if latch mode is enabled for the channel
unsigned long lastModePollMs = 0; // Modes switches are polled periodically

void setup() {
	
	// Debugging
	if (DEBUG) DEBUG->begin(9600);
	
	// Number of divisions
	n = sizeof(INPUTS) / sizeof(INPUTS[0]);
	
	// Initialize state
	inputs = new bool[n];
	inputsLast = new bool[n];
	outcomeLast = new bool[n];
	modeToggle = new bool[n];
	modeLatch = new bool[n];
	for (int i = 0; i < n; i++) {
		inputs[i] = false;
		inputsLast[i] = false;
		outcomeLast[i] = false;
		modeToggle[i] = false;
		modeLatch[i] = false;
	}
	
	// Setup I/O
	buttons = new Button*[n];
	knobs = new Knob*[n];
	cvs = new Knob*[n];
	ledsA = new Led*[n];
	ledsB = new Led*[n];
	for (int i = 0; i < n; i++) {
		buttons[i] = new Button(INPUT_BUTTONS[i], BUTTON_DEBOUNCE_DELAY);
		knobs[i] = new Knob(PROBABILITY_KNOBS[i], PROBABILITY_KNOBS_THRESHOLD_LOW, PROBABILITY_KNOBS_THRESHOLD_HIGH);
		cvs[i] = new Knob(PROBABILITY_CV_INPUTS[i], PROBABILITY_CV_INPUTS_THRESHOLD_LOW, PROBABILITY_CV_INPUTS_THRESHOLD_HIGH);
		ledsA[i] = new Led(LEDS_A[i], LED_MIN_DURATION_MS);
		ledsB[i] = new Led(LEDS_B[i], LED_MIN_DURATION_MS);
		pinMode(OUTPUTS_A[i], OUTPUT);
		pinMode(OUTPUTS_B[i], OUTPUT);
		digitalWrite(OUTPUTS_A[i], LOW);
		digitalWrite(OUTPUTS_B[i], LOW);
		pinMode(MODE_TOGGLE_PINS[i], INPUT);
		pinMode(MODE_LATCH_PINS[i], INPUT);
	}
	
	// Interrupts
	for (int i = 0; i < n; i++) {
		pinMode(INPUTS[i], INPUT);
		attachInterrupt(digitalPinToInterrupt(INPUTS[i]), isrInputs, CHANGE);
	}
	
}

void loop() {
	
	modePolling();
	
	// For each channel
	for (int i = 0; i < n; i++) {
		
		// Channel input changed?
		bool input = inputs[i] || buttons[i]->read(); // Current input
		if (input != inputsLast[i]) {
			inputsLast[i] = input; // Remeber the new input
			
			if (input) {
				
				// Flip coin
				randomSeed(micros()); // No unconnected analog pins available, seed with millis()
				float probability = min(knobs[i]->read() + cvs[i]->read(), 1.0);
				bool outcome = random(0, 1024) >= (probability * 1024); // TRUE if random is bigger than probability factor
				
				// Toggle mode?
				if (modeToggle[i]) outcome = (outcome == outcomeLast[i]);
				outcomeLast[i] = outcome;
				
				// Turn on output and LED
				if (outcome) {
					digitalWrite(OUTPUTS_A[i], HIGH);
					ledsA[i]->on();
				} else {
					digitalWrite(OUTPUTS_B[i], HIGH);
					ledsB[i]->on();
				}
				
				// If in latch mode, turn off the other output and LED
				if (modeLatch[i]) {
					if (outcome) {
						digitalWrite(OUTPUTS_B[i], LOW);
						ledsB[i]->off();
					} else {
						digitalWrite(OUTPUTS_A[i], LOW);
						ledsA[i]->off();
					}
				}
				
				if (DEBUG) {
					DEBUG->print("CH");
					DEBUG->print(i);
					DEBUG->print(" -> Gate on -> P: ");
					DEBUG->print(probability, 2);
					DEBUG->print(" -> Outcome: ");
					DEBUG->println(outcome ? 'A' : 'B');
				}
				
			} else {
				
				// If not in latch mode, turn off all outputs and LEDs
				if (!modeLatch[i]) {
					digitalWrite(OUTPUTS_A[i], LOW);
					digitalWrite(OUTPUTS_B[i], LOW);
					ledsA[i]->off();
					ledsB[i]->off();
				}
				
				if (DEBUG) {
					DEBUG->print("CH");
					DEBUG->print(i);
					DEBUG->println(" -> Gate off");
				}
				
			}
			
		}
		
		// Update LEDs
		ledsA[i]->loop();
		ledsB[i]->loop();
		
	}
	
}

void isrInputs() {
	
	// Check each channel
	for (int i = 0; i < n; i++) {
		inputs[i] = (digitalRead(INPUTS[i]) == HIGH);
	}
	
}

void modePolling() {
	
	// Poll modes switches
	unsigned long ms = millis();
	if (lastModePollMs == 0 || ms >= (lastModePollMs + MODE_POLL_EVERY_MS)) {
		lastModePollMs = ms > 0 ? ms : 1; // Avoid to re-set zero
		
		for (int i = 0; i < n; i++) {
			modeToggle[i] = (digitalRead(MODE_TOGGLE_PINS[i]) == HIGH);
			modeLatch[i] = (digitalRead(MODE_LATCH_PINS[i]) == HIGH);
		}
		
	}
	
}
