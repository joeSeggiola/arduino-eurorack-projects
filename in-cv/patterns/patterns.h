#ifndef patterns_h
#define patterns_h

#include "Arduino.h"
#include <avr/pgmspace.h>

#define PATTERNS_N 53
#define PATTERNS_DURATION_RESOLUTION 16
#define PATTERNS_DURATION_MAX 128
#define TUNING_CV 2000

const unsigned int PATTERNS_CV[] = {
	   0, // - (pause)
	1583, // G1
	2000, // C2
	2333, // E2
	2417, // F2
	2500, // F#2
	2583, // G2
	2750, // A2
	2833, // Bb2
	2917, // B2
	3000, // C3
	3167, // D3
	3333, // E3
	3417, // F3
	3500, // F#3
	3583, // G3
	3750, // A3
	3917, // B3
};

const byte PATTERNS_CV_INDEX_01[] PROGMEM = {  3 }; // (c2)e2/4
const byte PATTERNS_CV_INDEX_02[] PROGMEM = {  3,  4,  3 }; // (c2)e2/8 f2/8 e2/4
const byte PATTERNS_CV_INDEX_03[] PROGMEM = {  0,  3,  4,  3 }; // -/8 e2/8 f2/8 e2/8
const byte PATTERNS_CV_INDEX_04[] PROGMEM = {  0,  3,  4,  6 }; // -/8 e2/8 f2/8 g2/8
const byte PATTERNS_CV_INDEX_05[] PROGMEM = {  3,  4,  6,  0 }; // e2/8 f2/8 g2/8 -/8
const byte PATTERNS_CV_INDEX_06[] PROGMEM = { 10 }; // c3/1~c3/1
const byte PATTERNS_CV_INDEX_07[] PROGMEM = {  0,  0,  0,  0,  2,  2,  2,  0,  0,  0,  0,  0 }; // -/4 -/4 -/4 -/8 c2/16 c2/16 c2/8 -/8 -/4 -/4 -/4 -/4
const byte PATTERNS_CV_INDEX_08[] PROGMEM = {  6,  4 }; // g2/1. f2/1~f2/1
const byte PATTERNS_CV_INDEX_09[] PROGMEM = {  9,  6,  0,  0,  0,  0 }; // b2/16 g2/16 -/8 -/4 -/4 -/4
const byte PATTERNS_CV_INDEX_10[] PROGMEM = {  9,  6 }; // b2/16 g2/16
const byte PATTERNS_CV_INDEX_11[] PROGMEM = {  4,  6,  9,  6,  9,  6 }; // f2/16 g2/16 b2/16 g2/16 b2/16 g2/16
const byte PATTERNS_CV_INDEX_12[] PROGMEM = {  4,  6,  9, 10 }; // f2/8 g2/8 b2/1 c3/4
const byte PATTERNS_CV_INDEX_13[] PROGMEM = {  9,  6,  6,  4,  6,  0,  6 }; // b2/16 g2/8. g2/16 f2/16 g2/8 -/8. g2/16~g2/2.
const byte PATTERNS_CV_INDEX_14[] PROGMEM = { 10,  9,  6,  5 }; // c3/1 b2/1 g2/1 f#2/1
const byte PATTERNS_CV_INDEX_15[] PROGMEM = {  6,  0,  0,  0,  0 }; // g2/16 -/8. -/4 -/4 -/4
const byte PATTERNS_CV_INDEX_16[] PROGMEM = {  6,  9, 10,  9 }; // g2/16 b2/16 c3/16 b2/16
const byte PATTERNS_CV_INDEX_17[] PROGMEM = {  9, 10,  9, 10,  9,  0 }; // b2/16 c3/16 b2/16 c3/16 b2/16 -/16
const byte PATTERNS_CV_INDEX_18[] PROGMEM = {  3,  5,  3,  5,  3,  3 }; // e2/16 f#2/16 e2/16 f#2/16 e2/8. e2/16
const byte PATTERNS_CV_INDEX_19[] PROGMEM = {  0, 15 }; // -/4. g3/4.
const byte PATTERNS_CV_INDEX_20[] PROGMEM = {  3,  5,  3,  5,  1,  3,  5,  3,  5,  3 }; // e2/16 f#2/16 e2/16 f#2/16 g1/8. e2/16 f#2/16 e2/16 f#2/16 e2/16
const byte PATTERNS_CV_INDEX_21[] PROGMEM = {  5 }; // f#2/2.
const byte PATTERNS_CV_INDEX_22[] PROGMEM = {  3,  3,  3,  3,  3,  5,  6,  7,  9 }; // e2/4. e2/4. e2/4. e2/4. e2/4. f#2/4. g2/4. a2/4. b2/8
const byte PATTERNS_CV_INDEX_23[] PROGMEM = {  3,  5,  5,  5,  5,  5,  6,  7,  9 }; // e2/8 f#2/4. f#2/4. f#2/4. f#2/4. f#2/4. g2/4. a2/4. b2/4
const byte PATTERNS_CV_INDEX_24[] PROGMEM = {  3,  5,  6,  6,  6,  6,  6,  7,  9 }; // e2/8 f#2/8 g2/4. g2/4. g2/4. g2/4. g2/4. a2/4. b2/8
const byte PATTERNS_CV_INDEX_25[] PROGMEM = {  3,  5,  6,  7,  7,  7,  7,  7,  9 }; // e2/8 f#2/8 g2/8 a2/4. a2/4. a2/4. a2/4. a2/4. b2/4.
const byte PATTERNS_CV_INDEX_26[] PROGMEM = {  3,  5,  6,  7,  9,  9,  9,  9,  9 }; // e2/8 f#2/8 g2/8 a2/8 b2/4. b2/4. b2/4. b2/4. b2/4.
const byte PATTERNS_CV_INDEX_27[] PROGMEM = {  3,  5,  3,  5,  6,  3,  6,  5,  3,  5,  3 }; // e2/16 f#2/16 e2/16 f#2/16 g2/8 e2/16 g2/16 f#2/16 e2/16 f#2/16 e2/16
const byte PATTERNS_CV_INDEX_28[] PROGMEM = {  3,  5,  3,  5,  3,  3 }; // e2/16 f#2/16 e2/16 f#2/16 e2/8. e2/16
const byte PATTERNS_CV_INDEX_29[] PROGMEM = {  3,  6, 10 }; // e2/2. g2/2. c3/2.
const byte PATTERNS_CV_INDEX_30[] PROGMEM = { 10 }; // c3/1.
const byte PATTERNS_CV_INDEX_31[] PROGMEM = {  6,  4,  6,  9,  6,  9 }; // g2/16 f2/16 g2/16 b2/16 g2/16 b2/16
const byte PATTERNS_CV_INDEX_32[] PROGMEM = {  4,  6,  4,  6,  9,  4,  6 }; // f2/16 g2/16 f2/16 g2/16 b2/16 f2/16~f2/2. g2/4.
const byte PATTERNS_CV_INDEX_33[] PROGMEM = {  6,  4,  0 }; // g2/16 f2/16 -/8
const byte PATTERNS_CV_INDEX_34[] PROGMEM = {  6,  4 }; // g2/16 f2/16
const byte PATTERNS_CV_INDEX_35[] PROGMEM = {  4,  6,  9,  6,  9,  6,  9,  6,  9,  6,  0,  0,  0,  0,  8, 15, 16, 15, 17, 16, 15, 12, 15, 14,  0,  0,  0, 12, 13 }; // f2/16 g2/16 b2/16 g2/16 b2/16 g2/16 b2/16 g2/16 b2/16 g2/16 -/8 -/4 -/4 -/4 bb2/4 g3/2. a3/8 g3/8~g3/8 b3/8 a3/4. g3/8 e3/2. g3/8 f#3/8~f#3/2. -/4 -/4 -/8 e3/8~e3/2 f3/1.
const byte PATTERNS_CV_INDEX_36[] PROGMEM = {  4,  6,  9,  6,  9,  6 }; // f2/16 g2/16 b2/16 g2/16 b2/16 g2/16
const byte PATTERNS_CV_INDEX_37[] PROGMEM = {  4,  6 }; // f2/16 g2/16
const byte PATTERNS_CV_INDEX_38[] PROGMEM = {  4,  6,  9,  4,  6,  9 }; // f2/16 g2/16 b2/16 f2/16 g2/16 b2/16
const byte PATTERNS_CV_INDEX_39[] PROGMEM = {  9,  6,  4,  6,  9, 10 }; // b2/16 g2/16 f2/16 g2/16 b2/16 c3/16
const byte PATTERNS_CV_INDEX_40[] PROGMEM = {  9,  4 }; // b2/16 f2/16
const byte PATTERNS_CV_INDEX_41[] PROGMEM = {  9,  6 }; // b2/16 g2/16
const byte PATTERNS_CV_INDEX_42[] PROGMEM = { 10,  9,  7, 10 }; // c3/1 b2/1 a2/1 c3/1
const byte PATTERNS_CV_INDEX_43[] PROGMEM = { 13, 12, 13, 12, 12, 12, 12, 13, 12 }; // f3/16 e3/16 f3/16 e3/16 e3/8 e3/8 e3/8 f3/16 e3/16
const byte PATTERNS_CV_INDEX_44[] PROGMEM = { 13, 12, 12, 10 }; // f3/8 e3/8~e3/8 e3/8 c3/4
const byte PATTERNS_CV_INDEX_45[] PROGMEM = { 11, 11,  6 }; // d3/4 d3/4 g2/4
const byte PATTERNS_CV_INDEX_46[] PROGMEM = {  6, 11, 12, 11,  0,  6,  0,  6,  0,  6,  6, 11, 12, 11 }; // g2/16 d3/16 e3/16 d3/16 -/8 g2/8 -/8 g2/8 -/8 g2/8 g2/16 d3/16 e3/16 d3/16
const byte PATTERNS_CV_INDEX_47[] PROGMEM = { 11, 12, 11 }; // d3/16 e3/16 d3/8
const byte PATTERNS_CV_INDEX_48[] PROGMEM = {  6,  6,  4 }; // g2/1. g2/1 f2/1~f2/4
const byte PATTERNS_CV_INDEX_49[] PROGMEM = {  4,  6,  8,  6,  8,  6 }; // f2/16 g2/16 bb2/16 g2/16 bb2/16 g2/16
const byte PATTERNS_CV_INDEX_50[] PROGMEM = {  4,  6 }; // f2/16 g2/16
const byte PATTERNS_CV_INDEX_51[] PROGMEM = {  4,  6,  8,  4,  6,  8 }; // f2/16 g2/16 bb2/16 f2/16 g2/16 bb2/16
const byte PATTERNS_CV_INDEX_52[] PROGMEM = {  6,  8 }; // g2/16 bb2/16
const byte PATTERNS_CV_INDEX_53[] PROGMEM = {  8,  6 }; // bb2/16 g2/16

const byte PATTERNS_DURATION_01[] PROGMEM = { /* Total:   4 */  4 }; // (c2)e2/4
const byte PATTERNS_DURATION_02[] PROGMEM = { /* Total:   8 */  2,  2,  4 }; // (c2)e2/8 f2/8 e2/4
const byte PATTERNS_DURATION_03[] PROGMEM = { /* Total:   8 */  2,  2,  2,  2 }; // -/8 e2/8 f2/8 e2/8
const byte PATTERNS_DURATION_04[] PROGMEM = { /* Total:   8 */  2,  2,  2,  2 }; // -/8 e2/8 f2/8 g2/8
const byte PATTERNS_DURATION_05[] PROGMEM = { /* Total:   8 */  2,  2,  2,  2 }; // e2/8 f2/8 g2/8 -/8
const byte PATTERNS_DURATION_06[] PROGMEM = { /* Total:  32 */ 32 }; // c3/1~c3/1
const byte PATTERNS_DURATION_07[] PROGMEM = { /* Total:  36 */  4,  4,  4,  2,  1,  1,  2,  2,  4,  4,  4,  4 }; // -/4 -/4 -/4 -/8 c2/16 c2/16 c2/8 -/8 -/4 -/4 -/4 -/4
const byte PATTERNS_DURATION_08[] PROGMEM = { /* Total:  56 */ 24, 32 }; // g2/1. f2/1~f2/1
const byte PATTERNS_DURATION_09[] PROGMEM = { /* Total:  16 */  1,  1,  2,  4,  4,  4 }; // b2/16 g2/16 -/8 -/4 -/4 -/4
const byte PATTERNS_DURATION_10[] PROGMEM = { /* Total:   2 */  1,  1 }; // b2/16 g2/16
const byte PATTERNS_DURATION_11[] PROGMEM = { /* Total:   6 */  1,  1,  1,  1,  1,  1 }; // f2/16 g2/16 b2/16 g2/16 b2/16 g2/16
const byte PATTERNS_DURATION_12[] PROGMEM = { /* Total:  24 */  2,  2, 16,  4 }; // f2/8 g2/8 b2/1 c3/4
const byte PATTERNS_DURATION_13[] PROGMEM = { /* Total:  24 */  1,  3,  1,  1,  2,  3, 13 }; // b2/16 g2/8. g2/16 f2/16 g2/8 -/8. g2/16~g2/2.
const byte PATTERNS_DURATION_14[] PROGMEM = { /* Total:  64 */ 16, 16, 16, 16 }; // c3/1 b2/1 g2/1 f#2/1
const byte PATTERNS_DURATION_15[] PROGMEM = { /* Total:  16 */  1,  3,  4,  4,  4 }; // g2/16 -/8. -/4 -/4 -/4
const byte PATTERNS_DURATION_16[] PROGMEM = { /* Total:   4 */  1,  1,  1,  1 }; // g2/16 b2/16 c3/16 b2/16
const byte PATTERNS_DURATION_17[] PROGMEM = { /* Total:   6 */  1,  1,  1,  1,  1,  1 }; // b2/16 c3/16 b2/16 c3/16 b2/16 -/16
const byte PATTERNS_DURATION_18[] PROGMEM = { /* Total:   8 */  1,  1,  1,  1,  3,  1 }; // e2/16 f#2/16 e2/16 f#2/16 e2/8. e2/16
const byte PATTERNS_DURATION_19[] PROGMEM = { /* Total:  12 */  6,  6 }; // -/4. g3/4.
const byte PATTERNS_DURATION_20[] PROGMEM = { /* Total:  12 */  1,  1,  1,  1,  3,  1,  1,  1,  1,  1 }; // e2/16 f#2/16 e2/16 f#2/16 g1/8. e2/16 f#2/16 e2/16 f#2/16 e2/16
const byte PATTERNS_DURATION_21[] PROGMEM = { /* Total:  12 */ 12 }; // f#2/2.
const byte PATTERNS_DURATION_22[] PROGMEM = { /* Total:  50 */  6,  6,  6,  6,  6,  6,  6,  6,  2 }; // e2/4. e2/4. e2/4. e2/4. e2/4. f#2/4. g2/4. a2/4. b2/8
const byte PATTERNS_DURATION_23[] PROGMEM = { /* Total:  48 */  2,  6,  6,  6,  6,  6,  6,  6,  4 }; // e2/8 f#2/4. f#2/4. f#2/4. f#2/4. f#2/4. g2/4. a2/4. b2/4
const byte PATTERNS_DURATION_24[] PROGMEM = { /* Total:  42 */  2,  2,  6,  6,  6,  6,  6,  6,  2 }; // e2/8 f#2/8 g2/4. g2/4. g2/4. g2/4. g2/4. a2/4. b2/8
const byte PATTERNS_DURATION_25[] PROGMEM = { /* Total:  42 */  2,  2,  2,  6,  6,  6,  6,  6,  6 }; // e2/8 f#2/8 g2/8 a2/4. a2/4. a2/4. a2/4. a2/4. b2/4.
const byte PATTERNS_DURATION_26[] PROGMEM = { /* Total:  38 */  2,  2,  2,  2,  6,  6,  6,  6,  6 }; // e2/8 f#2/8 g2/8 a2/8 b2/4. b2/4. b2/4. b2/4. b2/4.
const byte PATTERNS_DURATION_27[] PROGMEM = { /* Total:  12 */  1,  1,  1,  1,  2,  1,  1,  1,  1,  1,  1 }; // e2/16 f#2/16 e2/16 f#2/16 g2/8 e2/16 g2/16 f#2/16 e2/16 f#2/16 e2/16
const byte PATTERNS_DURATION_28[] PROGMEM = { /* Total:   8 */  1,  1,  1,  1,  3,  1 }; // e2/16 f#2/16 e2/16 f#2/16 e2/8. e2/16
const byte PATTERNS_DURATION_29[] PROGMEM = { /* Total:  36 */ 12, 12, 12 }; // e2/2. g2/2. c3/2.
const byte PATTERNS_DURATION_30[] PROGMEM = { /* Total:  24 */ 24 }; // c3/1.
const byte PATTERNS_DURATION_31[] PROGMEM = { /* Total:   6 */  1,  1,  1,  1,  1,  1 }; // g2/16 f2/16 g2/16 b2/16 g2/16 b2/16
const byte PATTERNS_DURATION_32[] PROGMEM = { /* Total:  24 */  1,  1,  1,  1,  1, 13,  6 }; // f2/16 g2/16 f2/16 g2/16 b2/16 f2/16~f2/2. g2/4.
const byte PATTERNS_DURATION_33[] PROGMEM = { /* Total:   4 */  1,  1,  2 }; // g2/16 f2/16 -/8
const byte PATTERNS_DURATION_34[] PROGMEM = { /* Total:   2 */  1,  1 }; // g2/16 f2/16
const byte PATTERNS_DURATION_35[] PROGMEM = { /* Total: 128 */  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  4,  4,  4,  4, 12,  2,  4,  2,  6,  2, 12,  2, 14,  4,  4,  2, 10, 24 }; // f2/16 g2/16 b2/16 g2/16 b2/16 g2/16 b2/16 g2/16 b2/16 g2/16 -/8 -/4 -/4 -/4 bb2/4 g3/2. a3/8 g3/8~g3/8 b3/8 a3/4. g3/8 e3/2. g3/8 f#3/8~f#3/2. -/4 -/4 -/8 e3/8~e3/2 f3/1.
const byte PATTERNS_DURATION_36[] PROGMEM = { /* Total:   6 */  1,  1,  1,  1,  1,  1 }; // f2/16 g2/16 b2/16 g2/16 b2/16 g2/16
const byte PATTERNS_DURATION_37[] PROGMEM = { /* Total:   2 */  1,  1 }; // f2/16 g2/16
const byte PATTERNS_DURATION_38[] PROGMEM = { /* Total:   6 */  1,  1,  1,  1,  1,  1 }; // f2/16 g2/16 b2/16 f2/16 g2/16 b2/16
const byte PATTERNS_DURATION_39[] PROGMEM = { /* Total:   6 */  1,  1,  1,  1,  1,  1 }; // b2/16 g2/16 f2/16 g2/16 b2/16 c3/16
const byte PATTERNS_DURATION_40[] PROGMEM = { /* Total:   2 */  1,  1 }; // b2/16 f2/16
const byte PATTERNS_DURATION_41[] PROGMEM = { /* Total:   2 */  1,  1 }; // b2/16 g2/16
const byte PATTERNS_DURATION_42[] PROGMEM = { /* Total:  64 */ 16, 16, 16, 16 }; // c3/1 b2/1 a2/1 c3/1
const byte PATTERNS_DURATION_43[] PROGMEM = { /* Total:  12 */  1,  1,  1,  1,  2,  2,  2,  1,  1 }; // f3/16 e3/16 f3/16 e3/16 e3/8 e3/8 e3/8 f3/16 e3/16
const byte PATTERNS_DURATION_44[] PROGMEM = { /* Total:  12 */  2,  4,  2,  4 }; // f3/8 e3/8~e3/8 e3/8 c3/4
const byte PATTERNS_DURATION_45[] PROGMEM = { /* Total:  12 */  4,  4,  4 }; // d3/4 d3/4 g2/4
const byte PATTERNS_DURATION_46[] PROGMEM = { /* Total:  20 */  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  1,  1,  1,  1 }; // g2/16 d3/16 e3/16 d3/16 -/8 g2/8 -/8 g2/8 -/8 g2/8 g2/16 d3/16 e3/16 d3/16
const byte PATTERNS_DURATION_47[] PROGMEM = { /* Total:   4 */  1,  1,  2 }; // d3/16 e3/16 d3/8
const byte PATTERNS_DURATION_48[] PROGMEM = { /* Total:  60 */ 24, 16, 20 }; // g2/1. g2/1 f2/1~f2/4
const byte PATTERNS_DURATION_49[] PROGMEM = { /* Total:   6 */  1,  1,  1,  1,  1,  1 }; // f2/16 g2/16 bb2/16 g2/16 bb2/16 g2/16
const byte PATTERNS_DURATION_50[] PROGMEM = { /* Total:   2 */  1,  1 }; // f2/16 g2/16
const byte PATTERNS_DURATION_51[] PROGMEM = { /* Total:   6 */  1,  1,  1,  1,  1,  1 }; // f2/16 g2/16 bb2/16 f2/16 g2/16 bb2/16
const byte PATTERNS_DURATION_52[] PROGMEM = { /* Total:   2 */  1,  1 }; // g2/16 bb2/16
const byte PATTERNS_DURATION_53[] PROGMEM = { /* Total:   2 */  1,  1 }; // bb2/16 g2/16

const byte PATTERNS_SLIDE_01[] PROGMEM = { B00000000 }; // (c2)e2/4
const byte PATTERNS_SLIDE_02[] PROGMEM = { B00000000 }; // (c2)e2/8 f2/8 e2/4
const byte PATTERNS_SLIDE_03[] PROGMEM = { B00000000 }; // -/8 e2/8 f2/8 e2/8
const byte PATTERNS_SLIDE_04[] PROGMEM = { B00000000 }; // -/8 e2/8 f2/8 g2/8
const byte PATTERNS_SLIDE_05[] PROGMEM = { B00000000 }; // e2/8 f2/8 g2/8 -/8
const byte PATTERNS_SLIDE_06[] PROGMEM = { B00000000 }; // c3/1~c3/1
const byte PATTERNS_SLIDE_07[] PROGMEM = { B00000000, B00000000 }; // -/4 -/4 -/4 -/8 c2/16 c2/16 c2/8 -/8 -/4 -/4 -/4 -/4
const byte PATTERNS_SLIDE_08[] PROGMEM = { B00000000 }; // g2/1. f2/1~f2/1
const byte PATTERNS_SLIDE_09[] PROGMEM = { B00000000 }; // b2/16 g2/16 -/8 -/4 -/4 -/4
const byte PATTERNS_SLIDE_10[] PROGMEM = { B00000000 }; // b2/16 g2/16
const byte PATTERNS_SLIDE_11[] PROGMEM = { B00000000 }; // f2/16 g2/16 b2/16 g2/16 b2/16 g2/16
const byte PATTERNS_SLIDE_12[] PROGMEM = { B00000000 }; // f2/8 g2/8 b2/1 c3/4
const byte PATTERNS_SLIDE_13[] PROGMEM = { B00000000 }; // b2/16 g2/8. g2/16 f2/16 g2/8 -/8. g2/16~g2/2.
const byte PATTERNS_SLIDE_14[] PROGMEM = { B00000000 }; // c3/1 b2/1 g2/1 f#2/1
const byte PATTERNS_SLIDE_15[] PROGMEM = { B00000000 }; // g2/16 -/8. -/4 -/4 -/4
const byte PATTERNS_SLIDE_16[] PROGMEM = { B00000000 }; // g2/16 b2/16 c3/16 b2/16
const byte PATTERNS_SLIDE_17[] PROGMEM = { B00000000 }; // b2/16 c3/16 b2/16 c3/16 b2/16 -/16
const byte PATTERNS_SLIDE_18[] PROGMEM = { B00000000 }; // e2/16 f#2/16 e2/16 f#2/16 e2/8. e2/16
const byte PATTERNS_SLIDE_19[] PROGMEM = { B00000000 }; // -/4. g3/4.
const byte PATTERNS_SLIDE_20[] PROGMEM = { B00000000, B00000000 }; // e2/16 f#2/16 e2/16 f#2/16 g1/8. e2/16 f#2/16 e2/16 f#2/16 e2/16
const byte PATTERNS_SLIDE_21[] PROGMEM = { B00000000 }; // f#2/2.
const byte PATTERNS_SLIDE_22[] PROGMEM = { B00000000, B00000000 }; // e2/4. e2/4. e2/4. e2/4. e2/4. f#2/4. g2/4. a2/4. b2/8
const byte PATTERNS_SLIDE_23[] PROGMEM = { B00000000, B00000000 }; // e2/8 f#2/4. f#2/4. f#2/4. f#2/4. f#2/4. g2/4. a2/4. b2/4
const byte PATTERNS_SLIDE_24[] PROGMEM = { B00000000, B00000000 }; // e2/8 f#2/8 g2/4. g2/4. g2/4. g2/4. g2/4. a2/4. b2/8
const byte PATTERNS_SLIDE_25[] PROGMEM = { B00000000, B00000000 }; // e2/8 f#2/8 g2/8 a2/4. a2/4. a2/4. a2/4. a2/4. b2/4.
const byte PATTERNS_SLIDE_26[] PROGMEM = { B00000000, B00000000 }; // e2/8 f#2/8 g2/8 a2/8 b2/4. b2/4. b2/4. b2/4. b2/4.
const byte PATTERNS_SLIDE_27[] PROGMEM = { B00000000, B00000000 }; // e2/16 f#2/16 e2/16 f#2/16 g2/8 e2/16 g2/16 f#2/16 e2/16 f#2/16 e2/16
const byte PATTERNS_SLIDE_28[] PROGMEM = { B00000000 }; // e2/16 f#2/16 e2/16 f#2/16 e2/8. e2/16
const byte PATTERNS_SLIDE_29[] PROGMEM = { B00000000 }; // e2/2. g2/2. c3/2.
const byte PATTERNS_SLIDE_30[] PROGMEM = { B00000000 }; // c3/1.
const byte PATTERNS_SLIDE_31[] PROGMEM = { B00000000 }; // g2/16 f2/16 g2/16 b2/16 g2/16 b2/16
const byte PATTERNS_SLIDE_32[] PROGMEM = { B00000000 }; // f2/16 g2/16 f2/16 g2/16 b2/16 f2/16~f2/2. g2/4.
const byte PATTERNS_SLIDE_33[] PROGMEM = { B00000000 }; // g2/16 f2/16 -/8
const byte PATTERNS_SLIDE_34[] PROGMEM = { B00000000 }; // g2/16 f2/16
const byte PATTERNS_SLIDE_35[] PROGMEM = { B00000000, B00000000, B00000000, B00000000 }; // f2/16 g2/16 b2/16 g2/16 b2/16 g2/16 b2/16 g2/16 b2/16 g2/16 -/8 -/4 -/4 -/4 bb2/4 g3/2. a3/8 g3/8~g3/8 b3/8 a3/4. g3/8 e3/2. g3/8 f#3/8~f#3/2. -/4 -/4 -/8 e3/8~e3/2 f3/1.
const byte PATTERNS_SLIDE_36[] PROGMEM = { B00000000 }; // f2/16 g2/16 b2/16 g2/16 b2/16 g2/16
const byte PATTERNS_SLIDE_37[] PROGMEM = { B00000000 }; // f2/16 g2/16
const byte PATTERNS_SLIDE_38[] PROGMEM = { B00000000 }; // f2/16 g2/16 b2/16 f2/16 g2/16 b2/16
const byte PATTERNS_SLIDE_39[] PROGMEM = { B00000000 }; // b2/16 g2/16 f2/16 g2/16 b2/16 c3/16
const byte PATTERNS_SLIDE_40[] PROGMEM = { B00000000 }; // b2/16 f2/16
const byte PATTERNS_SLIDE_41[] PROGMEM = { B00000000 }; // b2/16 g2/16
const byte PATTERNS_SLIDE_42[] PROGMEM = { B00000000 }; // c3/1 b2/1 a2/1 c3/1
const byte PATTERNS_SLIDE_43[] PROGMEM = { B00000000, B00000000 }; // f3/16 e3/16 f3/16 e3/16 e3/8 e3/8 e3/8 f3/16 e3/16
const byte PATTERNS_SLIDE_44[] PROGMEM = { B00000000 }; // f3/8 e3/8~e3/8 e3/8 c3/4
const byte PATTERNS_SLIDE_45[] PROGMEM = { B00000000 }; // d3/4 d3/4 g2/4
const byte PATTERNS_SLIDE_46[] PROGMEM = { B00000000, B00000000 }; // g2/16 d3/16 e3/16 d3/16 -/8 g2/8 -/8 g2/8 -/8 g2/8 g2/16 d3/16 e3/16 d3/16
const byte PATTERNS_SLIDE_47[] PROGMEM = { B00000000 }; // d3/16 e3/16 d3/8
const byte PATTERNS_SLIDE_48[] PROGMEM = { B00000000 }; // g2/1. g2/1 f2/1~f2/4
const byte PATTERNS_SLIDE_49[] PROGMEM = { B00000000 }; // f2/16 g2/16 bb2/16 g2/16 bb2/16 g2/16
const byte PATTERNS_SLIDE_50[] PROGMEM = { B00000000 }; // f2/16 g2/16
const byte PATTERNS_SLIDE_51[] PROGMEM = { B00000000 }; // f2/16 g2/16 bb2/16 f2/16 g2/16 bb2/16
const byte PATTERNS_SLIDE_52[] PROGMEM = { B00000000 }; // g2/16 bb2/16
const byte PATTERNS_SLIDE_53[] PROGMEM = { B00000000 }; // bb2/16 g2/16

const unsigned int PATTERNS_ACCIACCATURA_CV[] PROGMEM = {
	2000, // Pattern #1
	2000, // Pattern #2
	   0, // Pattern #3
	   0, // Pattern #4
	   0, // Pattern #5
	   0, // Pattern #6
	   0, // Pattern #7
	   0, // Pattern #8
	   0, // Pattern #9
	   0, // Pattern #10
	   0, // Pattern #11
	   0, // Pattern #12
	   0, // Pattern #13
	   0, // Pattern #14
	   0, // Pattern #15
	   0, // Pattern #16
	   0, // Pattern #17
	   0, // Pattern #18
	   0, // Pattern #19
	   0, // Pattern #20
	   0, // Pattern #21
	   0, // Pattern #22
	   0, // Pattern #23
	   0, // Pattern #24
	   0, // Pattern #25
	   0, // Pattern #26
	   0, // Pattern #27
	   0, // Pattern #28
	   0, // Pattern #29
	   0, // Pattern #30
	   0, // Pattern #31
	   0, // Pattern #32
	   0, // Pattern #33
	   0, // Pattern #34
	   0, // Pattern #35
	   0, // Pattern #36
	   0, // Pattern #37
	   0, // Pattern #38
	   0, // Pattern #39
	   0, // Pattern #40
	   0, // Pattern #41
	   0, // Pattern #42
	   0, // Pattern #43
	   0, // Pattern #44
	   0, // Pattern #45
	   0, // Pattern #46
	   0, // Pattern #47
	   0, // Pattern #48
	   0, // Pattern #49
	   0, // Pattern #50
	   0, // Pattern #51
	   0, // Pattern #52
	   0, // Pattern #53
};

const byte PATTERNS_SIZE[] PROGMEM = {
	   1, // Pattern #1
	   3, // Pattern #2
	   4, // Pattern #3
	   4, // Pattern #4
	   4, // Pattern #5
	   1, // Pattern #6
	  12, // Pattern #7
	   2, // Pattern #8
	   6, // Pattern #9
	   2, // Pattern #10
	   6, // Pattern #11
	   4, // Pattern #12
	   7, // Pattern #13
	   4, // Pattern #14
	   5, // Pattern #15
	   4, // Pattern #16
	   6, // Pattern #17
	   6, // Pattern #18
	   2, // Pattern #19
	  10, // Pattern #20
	   1, // Pattern #21
	   9, // Pattern #22
	   9, // Pattern #23
	   9, // Pattern #24
	   9, // Pattern #25
	   9, // Pattern #26
	  11, // Pattern #27
	   6, // Pattern #28
	   3, // Pattern #29
	   1, // Pattern #30
	   6, // Pattern #31
	   7, // Pattern #32
	   3, // Pattern #33
	   2, // Pattern #34
	  29, // Pattern #35
	   6, // Pattern #36
	   2, // Pattern #37
	   6, // Pattern #38
	   6, // Pattern #39
	   2, // Pattern #40
	   2, // Pattern #41
	   4, // Pattern #42
	   9, // Pattern #43
	   4, // Pattern #44
	   3, // Pattern #45
	  14, // Pattern #46
	   3, // Pattern #47
	   3, // Pattern #48
	   6, // Pattern #49
	   2, // Pattern #50
	   6, // Pattern #51
	   2, // Pattern #52
	   2, // Pattern #53
};

const byte* const PATTERNS_CV_INDEX[] PROGMEM = {
	PATTERNS_CV_INDEX_01,
	PATTERNS_CV_INDEX_02,
	PATTERNS_CV_INDEX_03,
	PATTERNS_CV_INDEX_04,
	PATTERNS_CV_INDEX_05,
	PATTERNS_CV_INDEX_06,
	PATTERNS_CV_INDEX_07,
	PATTERNS_CV_INDEX_08,
	PATTERNS_CV_INDEX_09,
	PATTERNS_CV_INDEX_10,
	PATTERNS_CV_INDEX_11,
	PATTERNS_CV_INDEX_12,
	PATTERNS_CV_INDEX_13,
	PATTERNS_CV_INDEX_14,
	PATTERNS_CV_INDEX_15,
	PATTERNS_CV_INDEX_16,
	PATTERNS_CV_INDEX_17,
	PATTERNS_CV_INDEX_18,
	PATTERNS_CV_INDEX_19,
	PATTERNS_CV_INDEX_20,
	PATTERNS_CV_INDEX_21,
	PATTERNS_CV_INDEX_22,
	PATTERNS_CV_INDEX_23,
	PATTERNS_CV_INDEX_24,
	PATTERNS_CV_INDEX_25,
	PATTERNS_CV_INDEX_26,
	PATTERNS_CV_INDEX_27,
	PATTERNS_CV_INDEX_28,
	PATTERNS_CV_INDEX_29,
	PATTERNS_CV_INDEX_30,
	PATTERNS_CV_INDEX_31,
	PATTERNS_CV_INDEX_32,
	PATTERNS_CV_INDEX_33,
	PATTERNS_CV_INDEX_34,
	PATTERNS_CV_INDEX_35,
	PATTERNS_CV_INDEX_36,
	PATTERNS_CV_INDEX_37,
	PATTERNS_CV_INDEX_38,
	PATTERNS_CV_INDEX_39,
	PATTERNS_CV_INDEX_40,
	PATTERNS_CV_INDEX_41,
	PATTERNS_CV_INDEX_42,
	PATTERNS_CV_INDEX_43,
	PATTERNS_CV_INDEX_44,
	PATTERNS_CV_INDEX_45,
	PATTERNS_CV_INDEX_46,
	PATTERNS_CV_INDEX_47,
	PATTERNS_CV_INDEX_48,
	PATTERNS_CV_INDEX_49,
	PATTERNS_CV_INDEX_50,
	PATTERNS_CV_INDEX_51,
	PATTERNS_CV_INDEX_52,
	PATTERNS_CV_INDEX_53,
};

const byte* const PATTERNS_DURATION[] PROGMEM = {
	PATTERNS_DURATION_01,
	PATTERNS_DURATION_02,
	PATTERNS_DURATION_03,
	PATTERNS_DURATION_04,
	PATTERNS_DURATION_05,
	PATTERNS_DURATION_06,
	PATTERNS_DURATION_07,
	PATTERNS_DURATION_08,
	PATTERNS_DURATION_09,
	PATTERNS_DURATION_10,
	PATTERNS_DURATION_11,
	PATTERNS_DURATION_12,
	PATTERNS_DURATION_13,
	PATTERNS_DURATION_14,
	PATTERNS_DURATION_15,
	PATTERNS_DURATION_16,
	PATTERNS_DURATION_17,
	PATTERNS_DURATION_18,
	PATTERNS_DURATION_19,
	PATTERNS_DURATION_20,
	PATTERNS_DURATION_21,
	PATTERNS_DURATION_22,
	PATTERNS_DURATION_23,
	PATTERNS_DURATION_24,
	PATTERNS_DURATION_25,
	PATTERNS_DURATION_26,
	PATTERNS_DURATION_27,
	PATTERNS_DURATION_28,
	PATTERNS_DURATION_29,
	PATTERNS_DURATION_30,
	PATTERNS_DURATION_31,
	PATTERNS_DURATION_32,
	PATTERNS_DURATION_33,
	PATTERNS_DURATION_34,
	PATTERNS_DURATION_35,
	PATTERNS_DURATION_36,
	PATTERNS_DURATION_37,
	PATTERNS_DURATION_38,
	PATTERNS_DURATION_39,
	PATTERNS_DURATION_40,
	PATTERNS_DURATION_41,
	PATTERNS_DURATION_42,
	PATTERNS_DURATION_43,
	PATTERNS_DURATION_44,
	PATTERNS_DURATION_45,
	PATTERNS_DURATION_46,
	PATTERNS_DURATION_47,
	PATTERNS_DURATION_48,
	PATTERNS_DURATION_49,
	PATTERNS_DURATION_50,
	PATTERNS_DURATION_51,
	PATTERNS_DURATION_52,
	PATTERNS_DURATION_53,
};

const byte* const PATTERNS_SLIDE[] PROGMEM = {
	PATTERNS_SLIDE_01,
	PATTERNS_SLIDE_02,
	PATTERNS_SLIDE_03,
	PATTERNS_SLIDE_04,
	PATTERNS_SLIDE_05,
	PATTERNS_SLIDE_06,
	PATTERNS_SLIDE_07,
	PATTERNS_SLIDE_08,
	PATTERNS_SLIDE_09,
	PATTERNS_SLIDE_10,
	PATTERNS_SLIDE_11,
	PATTERNS_SLIDE_12,
	PATTERNS_SLIDE_13,
	PATTERNS_SLIDE_14,
	PATTERNS_SLIDE_15,
	PATTERNS_SLIDE_16,
	PATTERNS_SLIDE_17,
	PATTERNS_SLIDE_18,
	PATTERNS_SLIDE_19,
	PATTERNS_SLIDE_20,
	PATTERNS_SLIDE_21,
	PATTERNS_SLIDE_22,
	PATTERNS_SLIDE_23,
	PATTERNS_SLIDE_24,
	PATTERNS_SLIDE_25,
	PATTERNS_SLIDE_26,
	PATTERNS_SLIDE_27,
	PATTERNS_SLIDE_28,
	PATTERNS_SLIDE_29,
	PATTERNS_SLIDE_30,
	PATTERNS_SLIDE_31,
	PATTERNS_SLIDE_32,
	PATTERNS_SLIDE_33,
	PATTERNS_SLIDE_34,
	PATTERNS_SLIDE_35,
	PATTERNS_SLIDE_36,
	PATTERNS_SLIDE_37,
	PATTERNS_SLIDE_38,
	PATTERNS_SLIDE_39,
	PATTERNS_SLIDE_40,
	PATTERNS_SLIDE_41,
	PATTERNS_SLIDE_42,
	PATTERNS_SLIDE_43,
	PATTERNS_SLIDE_44,
	PATTERNS_SLIDE_45,
	PATTERNS_SLIDE_46,
	PATTERNS_SLIDE_47,
	PATTERNS_SLIDE_48,
	PATTERNS_SLIDE_49,
	PATTERNS_SLIDE_50,
	PATTERNS_SLIDE_51,
	PATTERNS_SLIDE_52,
	PATTERNS_SLIDE_53,
};

#endif