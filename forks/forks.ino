	
// CONFIGURATION =============================================================

const bool DEBUG = false; // FALSE to disable debug messages on serial port

const int INPUTS[] { 2, 3 }; // Input, one for each Forks channel
const int INPUT_BUTTONS[] { 4, 5 }; // Manual input buttons
const int OUTPUTS_A[] { 6, 8 }; // First outputs
const int OUTPUTS_B[] { 7, 9 }; // Second outputs

const int PROBABILITY_KNOBS[] { A4, A5 }; // Probability knobs pins
const int PROBABILITY_CV_INPUTS[] { A6, A7 }; // Probability CV

const int MODE_TOGGLE_PINS[] { A0, A1 }; // Switch for enabling toggle mode
const int MODE_LATCH_PINS[] { A2, A3 }; // Switch for enabling latch mode
const unsigned long MODE_POLL_EVERY_MS = 100; // Check for mode switches periodically

const int LEDS_A[] { 10, 12 }; // LED indicators for first outputs (red)
const int LEDS_B[] { 11, 13 }; // LED indicators for second outputs (green)

const int PROBABILITY_KNOBS_THRESHOLD_LOW = 0; // Everything read under this value in the 0-1023 scale is considered the minimum value
const int PROBABILITY_KNOBS_THRESHOLD_HIGH = 1023; // Everything read over this value in the 0-1023 scale is considered the maximum value
const int PROBABILITY_CV_INPUTS_THRESHOLD_LOW = 6; // Everything read under this value in the 0-1023 scale is considered the minimum value
const int PROBABILITY_CV_INPUTS_THRESHOLD_HIGH = 670; // Everything read over this value in the 0-1023 scale is considered the maximum value

const unsigned long BUTTON_DEBOUNCE_DELAY = 50; // Debounce delay for manual input buttons
const unsigned long LED_MIN_DURATION_MS = 50; // Minimum "on" duration for all LEDs visibility

// ===========================================================================

#include "lib/Button.cpp"
#include "lib/CV.cpp"
#include "lib/Led.cpp"

unsigned int n = 0; // Number of channels

Button buttons[8];
CV knobs[8];
CV cvs[8];
Led ledsA[8]; // LEDs for outputs A
Led ledsB[8]; // LEDs for outputs B

volatile bool inputs[8]; // Input signal digital reading, set in ISR
bool inputsLast[8]; // TRUE if input signal was high or manual button was pressed
bool outcomeLast[8]; // Last outcome for toggle mode

bool modeToggle[8]; // TRUE if toggle mode is enabled for the channel
bool modeLatch[8]; // TRUE if latch mode is enabled for the channel
unsigned long lastModePollMs = 0; // Modes switches are polled periodically

void setup() {
	
	// Debugging
	if (DEBUG) {
		Serial.begin(9600);
		Serial.println("FORKS - joeSeggiola");
	}
	
	// Number of divisions
	n = sizeof(INPUTS) / sizeof(INPUTS[0]);
	
	// Initialize state
	for (int i = 0; i < n; i++) {
		inputs[i] = false;
		inputsLast[i] = false;
		outcomeLast[i] = false;
		modeToggle[i] = false;
		modeLatch[i] = false;
	}
	
	// Setup I/O
	for (int i = 0; i < n; i++) {
		buttons[i].init(INPUT_BUTTONS[i], BUTTON_DEBOUNCE_DELAY, true, true);
		knobs[i].init(PROBABILITY_KNOBS[i], PROBABILITY_KNOBS_THRESHOLD_LOW, PROBABILITY_KNOBS_THRESHOLD_HIGH);
		cvs[i].init(PROBABILITY_CV_INPUTS[i], PROBABILITY_CV_INPUTS_THRESHOLD_LOW, PROBABILITY_CV_INPUTS_THRESHOLD_HIGH, true);
		ledsA[i].init(LEDS_A[i], LED_MIN_DURATION_MS);
		ledsB[i].init(LEDS_B[i], LED_MIN_DURATION_MS);
		pinMode(OUTPUTS_A[i], OUTPUT);
		pinMode(OUTPUTS_B[i], OUTPUT);
		digitalWrite(OUTPUTS_A[i], LOW);
		digitalWrite(OUTPUTS_B[i], LOW);
		pinMode(MODE_TOGGLE_PINS[i], INPUT_PULLUP);
		pinMode(MODE_LATCH_PINS[i], INPUT_PULLUP);
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
		bool input = inputs[i] || buttons[i].read(); // Current input
		if (input != inputsLast[i]) {
			inputsLast[i] = input; // Remember the new input
			
			if (input) {
				
				// Calculate probability combining knob and CV
				float probability;
				float probabilityKnob = knobs[i].read() * 1.1 - 0.05; // Let the knob push more toward the edges, to compensate for CV values near zero
				float probabilityCV = cvs[i].read() - 0.5; // Use CV as an offset
				probability = constrain(probabilityKnob + probabilityCV, 0.0, 1.0);
				
				// Flip coin
				randomSeed(micros()); // No unconnected analog pins available, seed with millis()
				bool outcome = random(0, 1024) >= (probability * 1024); // TRUE if random is bigger than probability factor
				
				// Toggle mode?
				if (modeToggle[i]) outcome = (outcome == outcomeLast[i]);
				outcomeLast[i] = outcome;
				
				// Turn on output and LED
				if (outcome) {
					digitalWrite(OUTPUTS_A[i], HIGH);
					ledsA[i].on();
				} else {
					digitalWrite(OUTPUTS_B[i], HIGH);
					ledsB[i].on();
				}
				
				// If in latch mode, turn off the other output and LED
				if (modeLatch[i]) {
					if (outcome) {
						digitalWrite(OUTPUTS_B[i], LOW);
						ledsB[i].off();
					} else {
						digitalWrite(OUTPUTS_A[i], LOW);
						ledsA[i].off();
					}
				}
				
				if (DEBUG) {
					Serial.print("CH");
					Serial.print(i);
					Serial.print(" -> Gate on -> P: ");
					Serial.print(probability, 3);
					Serial.print(" (knob: ");
					Serial.print(probabilityKnob, 3);
					Serial.print(", CV: ");
					Serial.print(probabilityCV, 3);
					Serial.print(") -> Outcome: ");
					Serial.println(outcome ? 'A' : 'B');
				}
				
			} else {
				
				// If not in latch mode, turn off all outputs and LEDs
				if (!modeLatch[i]) {
					digitalWrite(OUTPUTS_A[i], LOW);
					digitalWrite(OUTPUTS_B[i], LOW);
					ledsA[i].off();
					ledsB[i].off();
				}
				
				if (DEBUG) {
					Serial.print("CH");
					Serial.print(i);
					Serial.println(" -> Gate off");
				}
				
			}
			
		}
		
		// Update LEDs
		ledsA[i].loop();
		ledsB[i].loop();
		
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
