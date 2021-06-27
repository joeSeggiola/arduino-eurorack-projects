#ifndef MultiPointMap_h
#define MultiPointMap_h

#include "Arduino.h"
#include <EEPROM.h>

class MultiPointMap {
	
	public:
		
		/** 
		 * Initialize a function that maps values using a multi-linear scale defined by equidistant
		 * fixed points along the specified range. This is used to implement DACs calibration, and
		 * it's adapted from Befaco MIDI Thing and Emilie Gillet's CVpal.
		 */
		void init(uint16_t range = 4000) {
			this->step = range / N; // Distance between two consecutive fixed points
			this->reset();
		}
		
		/**
		 * Map the given value to another value, interpolating between a pair of fixed points
		 */
		uint16_t map(uint16_t value) {
			uint8_t interval = value / this->step; // Index of the interval in which the given value falls
			if (interval > N - 1) interval = N - 1;
			int16_t a = interval == 0 ? 0 : this->points[interval - 1]; // Low interpolation point
			int16_t b = this->points[interval]; // High interpolation point
			return a + ((int32_t)(value - interval * step) * (b - a)) / step; // Linear interpolation
		}
		
		/**
		 * Get the value of a fixed point
		 */
		uint16_t get(uint8_t i) {
			return this->points[i];
		}
		
		/**
		 * Set the value of a fixed point
		 */
		uint16_t set(uint8_t i, uint16_t value) {
			this->points[i] = value;
		}
		
		/**
		 * Returns the distance between two consecutive points of the multi-linear scale
		 */
		uint16_t getStep() {
			return this->step;
		}
		
		/**
		 * Return the number of points in the multi-linear scale
		 */
		uint8_t size() {
			return N;
		}
		
		/**
		 * Load the map from the EEPROM memory starting from the given address.
		 * If the loaded data is invalid, the points are initialized linearly.
		 * Return the number of bytes read.
		 */
		int load(int address) {
			int size = 0;
			uint16_t checksum = 0, checksumLoaded = 0;
			for (uint8_t i = 0; i < N; i++) {
				EEPROM.get(address + size, this->points[i]);
				checksum += this->points[i];
				size += sizeof(this->points[i]);
			}
			EEPROM.get(address + size, checksumLoaded);
			size += sizeof(checksumLoaded);
			if (checksum != checksumLoaded) {
				this->reset();
			}
			return size;
		}
		
		/**
		 * Write the map to the EEPROM memory starting from the given address.
		 * Return the number of bytes written.
		 */
		int save(int address) {
			int size = 0;
			uint16_t checksum = 0;
			for (uint8_t i = 0; i < N; i++) {
				EEPROM.put(address + size, this->points[i]);
				checksum += this->points[i];
				size += sizeof(this->points[i]);
			}
			EEPROM.put(address + size, checksum);
			size += sizeof(checksum);
			return size;
		}
		
		/**
		 * Initialize the points of the multi-linear scale linearly.
		 * The first point is assumed to be zero.
		 */
		void reset() {
			for (uint8_t i = 0; i < N; i++) {
				this->points[i] = (i + 1) * this->getStep();
			}
		}
		
	private:
		
		static const uint8_t N = 8;
		
		uint16_t step;
		uint16_t points[N];
		
};

#endif