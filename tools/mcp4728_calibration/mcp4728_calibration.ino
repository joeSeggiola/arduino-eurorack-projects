
// CALIBRATION SKETCH for the four DACs in MCP4728 ===========================
// Set STEP to 1, 2 and the 3, following the comments in setup() and loop()
// ===========================================================================

#include <Wire.h>
#include "lib/MCP4728.cpp"

#define STEP 1

MCP4728 dac;

void setup() {
	
	Wire.begin();
	dac.init(Wire, 0);
	
	// Select internal Vref and X2 gain (4.096 V), so integer values map output mV
	dac.selectVref(MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V, MCP4728::VREF::INTERNAL_2_8V);
	dac.selectPowerDown(MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL, MCP4728::PWR_DOWN::NORMAL);
	dac.selectGain(MCP4728::GAIN::X2, MCP4728::GAIN::X2, MCP4728::GAIN::X2, MCP4728::GAIN::X2);
	
	// DACs calibration: for each DAC (0, 1, 2, 3) set offset and gain
	dac.calibrate(0, 0, 1.000000000);
	dac.calibrate(1, 0, 1.000000000);
	dac.calibrate(2, 0, 1.000000000);
	dac.calibrate(3, 0, 1.000000000);
	
	// STEP 1: Write a low (but well above zero) value, like ~100
	// Measure output in mV and use calibrate() to set an offset equal to the difference between expected and measured mV, leaving gain to 1
	if (STEP == 1) {
		dac.analogWrite(100, 100, 100, 100);
	}
	
	// STEP 2: Write an high (but well below limit) value, like 4000
	// Measure output in mV and use calibrate() to set a gain equal to the ratio between expected and measured.
	if (STEP == 2) {
		dac.analogWrite(4000, 4000, 4000, 4000);
	}
	
}

void loop() {
	
	// STEP 3: Test many values (4000, 3000, 2000, 1000, 100, 50, 0)
	// Adjust calibration for better distribution of errors
	if (STEP == 3) {
		const unsigned int values[] { 4000, 3000, 2000, 1000, 100, 50, 0 };
		int n = sizeof(values) / sizeof(values[0]);
		for (int i = 0; i < n; i++) {
			dac.analogWrite(values[i], values[i], values[i], values[i]);
			delay(4000);
		}
	}
	
}
