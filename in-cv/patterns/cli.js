const fs = require('fs');
const path = require('path');
const math = require('mathjs');

// Constants
const DAC_BITS = 12; // DAC bit resolution
const DAC_VREF = 4.096; // DAC reference voltage
const DURATION_RESOLUTION = 16; // The smallest possible note, for example 32 for 32th notes (use only multiples of 2)
const SEMITONE = 1 / 12;
const TUNING_NOTE = 'C2';
const NOTES = {
	'C': SEMITONE * 0,
	'D': SEMITONE * 2,
	'E': SEMITONE * 4,
	'F': SEMITONE * 5,
	'G': SEMITONE * 7,
	'A': SEMITONE * 9,
	'B': SEMITONE * 11,
};

/** 
 * Command line script 
 */
const cli = () => {
	
	// Read patterns from TXT file
	const patternsPath = process.argv[2];
	if (!patternsPath) throw new Error("No patterns file specified!\nUsage: npm start -- patterns.txt");
	const patterns = fs.readFileSync(patternsPath).toString().trim().split("\n").filter(p => p.trim() != '');
	if (patterns.length == 0) throw new Error("No patterns found in the specified file");
	
	// Parse all patterns
	const cvs = [];
	const durations = [];
	const slides = [];
	const acciaccaturas = [];
	const names = [];
	for (const p of patterns.map(p => parsePattern(p, DURATION_RESOLUTION))) {
		cvs.push(p.cv);
		durations.push(p.duration);
		slides.push(p.slide);
		acciaccaturas.push(p.acciaccatura);
		names.push(p.name);
	}
	
	// Map of unique note CV values
	const cvsMap = {};
	const cvsUniq = [ 0 ]; // Collect unique CVs, let 0 index point to 0 value
	const cvsNamesMap = {}; // Maps note integer values to note names, for comments and report
	for (let i = 0; i < cvs.length; i++) {
		for (let j = 0; j < cvs[i].length; j++) {
			const int = cvToInt(cvs[i][j], DAC_VREF, DAC_BITS);
			if (cvsUniq.indexOf(int) == -1) {
				cvsUniq.push(int);
				cvsNamesMap[int] = names[i][j][0].toUpperCase() + names[i][j].slice(1);
			}
		}
	}
	cvsUniq.sort();
	for (let i = 0; i < cvsUniq.length; i++) {
		cvsMap[cvsUniq[i]] = i;
	}
	
	// Setup code generation
	let codeMatrices = ""; // Data arrays
	let codePointers = ""; // Pointers arrays
	const codeMatrixAndPointers = (type, name, value) => {
		
		// Lay out the matrix as many distinct arrays with different size.
		// Lay out a pointers array for pointing to those distinct arrays.
		codePointers += "const " + type + "* const " + name + "[] PROGMEM = {\n";
		for (let i = 0; i < patterns.length; i++) {
			const n = (i + 1).toString().padStart(2, "0");
			const pattern = patterns[i];
			codeMatrices += "const " + type + " " + name + "_" + n + "[] PROGMEM = { " + value(i) + " };";
			codeMatrices += " // " + (Array.isArray(pattern) ? pattern.join(" ") : pattern).trim() + "\n"; // Text notation comment
			codePointers += "\t" + name + "_" + n + ",\n";
		}
		codeMatrices += "\n";
		codePointers += "};\n\n";
		
	};
	
	// Notes CV unique values
	let codeCVs = "const unsigned int PATTERNS_CV[] = {\n";
	for (let i = 0; i < cvsUniq.length; i++) {
		const cvName = (cvsNamesMap[cvsUniq[i]] || '- (pause)');
		codeCVs += "\t" + cvsUniq[i].toString().padStart(4) + ", // " + cvName + "\n";
	}
	codeCVs += "};\n\n";
	
	// Notes CV indexes
	codeMatrixAndPointers("byte", "PATTERNS_CV_INDEX", i => {
		return cvs[i].map((v) => {
			const int = cvToInt(v, DAC_VREF, DAC_BITS);
			return (int > 0 ? cvsMap[int] : 0).toString().padStart(2);
		}).join(", ");
	});
	
	// Notes durations
	const patternsDurations = [];
	let shortestNoteDuration = Infinity;
	let longestNoteDuration = 0;
	let longestPatternDuration = 0;
	let secondLongestPatternDuration = 0;
	let longestPatternIndex = 0;
	let secondLongestPatternIndex = 0;
	codeMatrixAndPointers("byte", "PATTERNS_DURATION", i => {
		let patternDuration = 0;
		const code = durations[i].map(v => {
			patternDuration += v; // Sum up total pattern duration
			if (v < shortestNoteDuration) shortestNoteDuration = v; // Find shortest note
			if (v > longestNoteDuration) longestNoteDuration = v; // Find longest note
			return v.toString().padStart(2);
		}).join(", ");
		patternsDurations.push(patternDuration);
		if (patternDuration > longestPatternDuration) {
			secondLongestPatternDuration = longestPatternDuration;
			longestPatternDuration = patternDuration;
			secondLongestPatternIndex = longestPatternIndex;
			longestPatternIndex = i;
		} else if (patternDuration > secondLongestPatternDuration) {
			secondLongestPatternDuration = patternDuration;
			secondLongestPatternIndex = i;
		}
		return "/* Total: " + patternDuration.toString().padStart(3) + " */ " + code;
	});
	
	// Slides
	codeMatrixAndPointers("byte", "PATTERNS_SLIDE", i => {
		const binaryBytes = [];
		for (let j = 0; j < slides[i].length; j += 8) {
			let binaryByte = "B";
			for (let k = 0; k < 8; k++) binaryByte += slides[i][j + k] ? "1" : "0";
			binaryBytes.push(binaryByte);
		}
		return binaryBytes.join(", ");
	});
	
	// Acciaccatura
	let codeAcciaccatura = "const unsigned int PATTERNS_ACCIACCATURA_CV[] PROGMEM = {\n";
	for (let i = 0; i < patterns.length; i++) {
		const acciaccaturaCV = acciaccaturas[i] != null ? cvToInt(acciaccaturas[i], DAC_VREF, DAC_BITS) : 0;
		codeAcciaccatura += "\t" + acciaccaturaCV.toString().padStart(4) + ", // Pattern #" + (i + 1) + "\n";
	}
	codeAcciaccatura += "};\n\n";
	
	// Patterns size
	let codeLength = "const byte PATTERNS_SIZE[] PROGMEM = {\n";
	for (let i = 0; i < patterns.length; i++) {
		codeLength += "\t" + cvs[i].length.toString().padStart(4) + ", // Pattern #" + (i + 1) + "\n";
	}
	codeLength += "};\n\n";
	
	// Create the code of the header ".h" file
	let code = "#ifndef patterns_h\n#define patterns_h\n\n#include \"Arduino.h\"\n#include <avr/pgmspace.h>\n\n";
	code += "#define PATTERNS_N " + patterns.length + "\n"; // Patterns count
	code += "#define PATTERNS_DURATION_RESOLUTION " + DURATION_RESOLUTION + "\n"; // Duration resolution
	code += "#define PATTERNS_DURATION_MAX " + Math.max(...patternsDurations) + "\n"; // Longest pattern diration in units
	code += "#define TUNING_CV " + cvToInt(noteToCV(TUNING_NOTE), DAC_VREF, DAC_BITS) + "\n\n";
	code += codeCVs + codeMatrices + codeAcciaccatura + codeLength + codePointers;
	code += "#endif";
	
	// Save into a file
	const filename = "patterns.h";
	fs.writeFileSync(__dirname + path.sep + filename, code);
	console.log("Done: " + patterns.length + " patterns saved in " + filename);
	console.log();
	
	// Info about patterns length
	const gcd = patternsDurations.length > 1 ? math.gcd(...patternsDurations) : patternsDurations[0];
	console.log("Lower note: " + cvsNamesMap[cvsUniq[1]] + " (" + cvsUniq[1] + ")");
	console.log("Higher note: " + cvsNamesMap[cvsUniq[cvsUniq.length - 1]] + " (" + cvsUniq[cvsUniq.length - 1] + ")");
	console.log("Number of unique notes: " + cvsUniq.length + " (including pause)");
	console.log("Duration of the shortest note: " + shortestNoteDuration);
	console.log("Duration of the longest note: " + longestNoteDuration);
	console.log("Duration of the shortest pattern: " + Math.min(...patternsDurations));
	console.log("Duration of the longest pattern: " + longestPatternDuration + " (#" + (longestPatternIndex + 1) + "), followed by " + secondLongestPatternDuration + " (#" + (secondLongestPatternIndex + 1) + ")");
	console.log("Greatest possible resolution for patterns length: */" + Math.max(1, DURATION_RESOLUTION / gcd));
	console.log();
	
};

/** 
 * Parses a pattern and returns "cv", "duration" and "slide" arrays in an object 
 */
const parsePattern = (pattern, durationUnits) => {
	
	if (pattern == null) throw new Error("Invalid empty pattern");
	
	// Search for the acciaccatura at the beginning of the pattern.
	// It's supported only on the first note for memory usage reasons.
	let acciaccatura = null;
	const acciaccaturaMatches = pattern.match(/^\((.+)\)/);
	if (acciaccaturaMatches) {
		acciaccatura = noteToCV(acciaccaturaMatches[1]);
		pattern = pattern.substring(acciaccaturaMatches[0].length);
	}
	
	// Split notes string on whitespace, ensure a space after legato symbol
	const notes = pattern.replace(/~/g, "~ ").trim().split(/\s+/g);
	
	// Loop notes and build the arrays
	const names = [];
	const cvs = [];
	const durations = [];
	const slides = [];
	let legatoFlag = false;
	for (const note of notes) {
		const n = note.split('/');
		if (n.length != 2) throw new Error("Invalid note: " + note);
		const cv = noteToCV(n[0]);
		const duration = noteDurationToInt(n[1], durationUnits);
		if (legatoFlag && Math.abs(cv - cvs[cvs.length - 1]) < 0.000001) { // Same-note legato?
			durations[durations.length - 1] += duration; // Simply increase last note duration
		} else {
			names.push(n[0]);
			cvs.push(cv);
			durations.push(duration);
			slides.push(false);
			if (legatoFlag) slides[slides.length - 2] = true; // Last note need to be slid
		}
		legatoFlag = note.trim().substr(-1) == "~";
	}
	
	return {
		"cv": cvs, 
		"duration": durations,
		"slide": slides,
		"acciaccatura": acciaccatura,
		"name": names,
	};
	
};

/** 
 * Converts notes written like "C4", "D#5" to 1V/oct CV voltage value 
 */
const noteToCV = (noteString) => {
	const s = (noteString && noteString.toString().replace(/\s+/g, '')) || '(blank)';
	if (s[0] == '-') return 0; // Rest
	if (s[0] == null || NOTES[s[0].toUpperCase()] == null) throw new Error("Invalid note name: " + s);
	let value = NOTES[s[0].toUpperCase()];
	let octavePosition = 1;
	if (s[1] && (s[1] == '#' || s[1] == 'b')) {
		octavePosition++;
		if (s[1] == '#') value += SEMITONE;
		if (s[1] == 'b') value -= SEMITONE;
	}
	if (s[octavePosition] == null) throw new Error("Invalid note octave: " + s);
	const octave = parseInt(s[octavePosition], 10);
	if (isNaN(octave)) throw new Error("Invalid note octave: " + s);
	value += octave;
	return value;
};

/** 
 * Returns the numbers of "units" of time to use to represente the note "duration",
 * for example, assuming "units" is 32 (i.e. a resolution of 32th note):
 *   - a whole note written as "1" will return 32
 *   - a quarter note written as "4" will return 8
 *   - a half dotted note written as "2." will return 24 when "units" is 32 
 */
const noteDurationToInt = (duration, units) => {
	const d = parseInt(duration, 10);
	if (isNaN(d) || d <= 0) throw new Error("Invalid note duration: " + (duration || (typeof duration)));
	let v = units / d;
	let dotValue = v / 2;
	for (let i = 0; i < duration.length; i++) {
		if (duration[i] == ".") {
			v += dotValue;
			dotValue /= 2;
		}
	}
	if (Math.abs(v - Math.round(v)) > 0.0001) throw new Error("Given duration cannot be expressed in integers: " + duration);
	return Math.round(v);
};

/** 
 * Returns the DAC integer value to push in order to get the given "cv" voltage value,
 * when "vref" is the maximum voltage and "bits" is the DAC resolution 
 */
const cvToInt = (cv, vref, bits) => {
	if (typeof cv !== 'number') throw new Error("Invalid CV value type: " + (typeof cv));
	if (typeof vref !== 'number') throw new Error("Invalid CV value type: " + (typeof cv));
	if (typeof bits !== 'number') throw new Error("Invalid CV value type: " + (typeof cv));
	const max = Math.pow(2, bits);
	const v = (cv / vref) * max;
	return Math.round(Math.max(0, Math.min(max - 1, v)));
};

// Run if called directly, not for tests
// https://stackoverflow.com/q/6398196/995958
// https://nodejs.org/docs/latest/api/modules.html#modules_accessing_the_main_module
if (require.main === module) {
	cli();
} else {
	module.exports = {
		parsePattern,
		noteToCV,
		noteDurationToInt,
		cvToInt
	};
}
