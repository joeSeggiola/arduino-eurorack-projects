#ifndef mono_h
#define mono_h

#include "Arduino.h"

// Monophonic notes stack.

// Based on Emilie Gillet's CVpal: 
// https://github.com/pichenettes/cvpal/blob/master/cvpal/note_stack.h

#define CAPACITY 10
#define FREE 0xFF

class NoteStack {
	
	public:
		
		/**
		 * Constructor
		 */
		void init() {
			this->clear();
		}
		
		/**
		 * Handle an incoming MIDI note and add it to the stack in the front position.
		 */
		void noteOn(byte note) {
			
			// Remove the note from the list first (in case it is already here)
			this->noteOff(note);
			
			// In case of saturation, remove the least recently played note
			if (this->size == CAPACITY) {
				byte leastRecentNote;
				for (byte i = 1; i <= CAPACITY; i++) {
					if (this->next[i] == 0) {
						leastRecentNote = this->note[i];
					}
				}
				this->noteOff(leastRecentNote);
			}
			
			// Find a free slot to insert the new note
			byte freeSlot;
			for (byte i = 1; i <= CAPACITY; i++) {
				if (this->note[i] == FREE) {
					freeSlot = i;
					break;
				}
			}
			this->next[freeSlot] = this->root; // After the new note there will be the one that is currently the first
			this->note[freeSlot] = note;
			this->root = freeSlot; // The free slot now contains the first note
			this->size++;
			
		}
		
		/**
		 * Handle an outgoing MIDI note and remove it from the stack.
		 * Returns the most recent note after removing this, or -1 if there's no new note to play.
		 */ 
		int noteOff(byte note) {
			
			// Search the note to remove, and the one before it
			byte current = this->root;
			byte previous = 0;
			while (current) {
				if (this->note[current] == note) break;
				previous = current;
				current = this->next[current];
			}
			
			if (current) {
				
				// Skip the note that will be removed
				if (previous) {
					this->next[previous] = this->next[current];
				} else {
					this->root = this->next[current];
				}
				
				// Free the slot
				this->next[current] = 0;
				this->note[current] = FREE;
				this->size--;
				
			}
			
			// Return the most recent note now, or -1 if none
			return this->size > 0 ? this->note[this->root] : -1;
			
		}
		
		/**
		 * Clear the stack
		 */
		void clear() {
			this->size = 0;
			this->root = 0;
			for (byte i = 0; i < CAPACITY; i++) {
				this->note[i] = FREE;
			}
		}
		
	private:
		byte size;
		byte root; // Pointer (index) to head, base 1
		byte note[CAPACITY + 1]; // Note values, first element is a dummy note
		byte next[CAPACITY + 1]; // Pointers (index) to the next note, base 1
	
};

#endif