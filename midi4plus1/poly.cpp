#ifndef poly_h
#define poly_h

#include "Arduino.h"

// Polyphonic voice allocator with two modes:
//  - LRU strategy and voice stealing, i.e. priority to last
//  - First-available strategy and priority to first

// Based on Emilie Gillet's CVpal: 
// https://github.com/pichenettes/cvpal/blob/master/cvpal/voice_allocator.cc

#define MAX 4

class VoiceAllocator {
	
	public:
		
		enum class Mode { LAST, FIRST };
		
		/**
		 * Constructor
		 */
		void init() {
			this->setMode(Mode::LAST);
			this->setSize(0);
			this->clear();
			for (byte i = 0; i < MAX; i++) {
				this->note[i] = 12; // C0
			}
		}
		
		/**
		 * Set the allocation mode.
		 * - Mode::LAST for LRU strategy and voice stealing, i.e. priority to last
		 * - Mode::FIRST for first-available strategy and priority to first
		 */
		void setMode(Mode mode) {
			this->mode = mode;
		}
		
		/**
		 * Set the polyphony size, i.e. the total number of available voices
		 */
		void setSize(byte size) {
			this->size = min(MAX, size);
		}
		
		/**
		 * Handle an incoming MIDI note and returns the index of the allocated voice.
		 * Returns -1 if no voice has been allocated.
		 */
		int noteOn(byte note) {
			
			if (this->size == 0) return -1;
			
			int voice = -1;
			
			if (this->mode == Mode::LAST) {
				
				// Check if there is a voice currently playing this note
				voice = this->find(note);
				
				// Try to find the least recently touched, currently inactive voice
				if (voice == -1) {
					for (byte i = 0; i < MAX; i++) {
						if (this->lru[i] < this->size && !this->active[this->lru[i]]) {
							voice = this->lru[i];
						}
					}
				}
				
				// If all voices are active, use the least recently played note
				if (voice == -1) {
					for (byte i = 0; i < MAX; i++) {
						if (this->lru[i] < this->size) {
							voice = this->lru[i];
						}
					}
				}
				
				// Mark the chosen voice as recently used
				this->touch(voice);
				
			} else if (this->mode == Mode::FIRST) {
				
				// Try to find the first currently inactive voice
				for (byte i = 0; i < this->size; i++) {
					if (!this->active[i]) {
						voice = i;
						break;
					}
				}
				
				// In case all voices are active, the new note will not be played
				if (voice == -1) {
					return -1;
				}
				
			}
			
			// Allocate the note
			this->note[voice] = note;
			this->active[voice] = true;
			
			return voice;
			
		}
		
		/**
		 * Handle an outgoing MIDI note and returns the index of the freed voice.
		 * Returns -1 if no voice has been freed.
		 */ 
		int noteOff(byte note) {
			int voice = this->find(note);
			if (voice != -1) {
				this->active[voice] = false;
				
				// Mark the freed voice as recently used
				if (this->mode == Mode::LAST) {
					this->touch(voice);
				}
				
			}
			return voice;
		}
		
		/**
		 * Clear allocation state, i.e. sets all voices inactive and resets LRU order.
		 */
		void clear() {
			for (byte i = 0; i < MAX; i++) {
				this->active[i] = false;
				this->lru[i] = MAX - i - 1;
			}
		}
		
	private:
		
		int find(byte note) {
			for (byte i = 0; i < this->size; i++) {
				if (this->note[i] == note) return i;
			}
			return -1;
		}
		
		void touch(byte voice) {
			int s = MAX - 1;
			int d = MAX - 1;
			while (s >= 0) {
				if (this->lru[s] != voice) this->lru[d--] = this->lru[s];
				s--;
			}
			this->lru[0] = voice;
		}
		
	private:
		Mode mode;
		byte size; // Number of available voices
		byte note[MAX]; // Note values for each voice
		bool active[MAX]; // Active state for each voice
		byte lru[MAX]; // Indexes of voices sorted by most recent usage
	
};

#endif