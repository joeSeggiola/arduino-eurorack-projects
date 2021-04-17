
// CALIBRATION SKETCH for the four DACs in MCP4728 ===========================
// Set STEP to 1 and then 2, following the comments in loop()
// ===========================================================================

#include <Wire.h>
#include "lib/MCP4728.cpp"

#define STEP 1
#define DAC_ADDR 0
#define LDAC_PIN 4

MCP4728 dac;

void setup() {
	
	pinMode(LDAC_PIN, OUTPUT);
	digitalWrite(LDAC_PIN, LOW);
	
	Wire.begin();
	dac.init(Wire, DAC_ADDR);
	
	// Select internal Vref and X2 gain (4.096 V), so integer values map output mV
	dac.selectVref(MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V);
	dac.selectPowerDown(MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL);
	dac.selectGain(MCP4728::GAIN::X2, MCP4728::GAIN::X2, MCP4728::GAIN::X2, MCP4728::GAIN::X2);
	
	// DACs calibration
	if (STEP > 1) {
		dac.calibrate(0,  0, 400, 1000, 2000, 3000, 4000);
		dac.calibrate(1,  0, 400, 1000, 2000, 3000, 4000);
		dac.calibrate(2,  0, 400, 1000, 2000, 3000, 4000);
		dac.calibrate(3,  0, 400, 1000, 2000, 3000, 4000);
	}
	
}

void loop() {
	
	// STEP 1: Wait 15 minutes, then measure actual output in mV for the following 6 calibration 
	// points, and pass the results to the calibration methods in setup().
	// Do this individually for each one of the 4 DACs
	if (STEP == 1) {
		unsigned int N = 6;
		unsigned int points[N] = { 0, 400, 1000, 2000, 3000, 4000 };
		for (unsigned int i = 0; i < N; i++) {
			dac.analogWrite(points[i], points[i], points[i], points[i]);
			delay(4000);
		}
	}
	
	// STEP 2: After setting up the calibration, cycle many values for testing
	if (STEP == 2) {
		for (unsigned int v = 0; v < 4096; v += v < 250 ? 50 : 250) {
			dac.analogWrite(v, v, v, v);
			delay(4000);
		}
	}
	
}
