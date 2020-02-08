const cli = require('./cli.js');

test('Notes are correctly converted to 1V/oct values', () => {
	
	const f = cli.noteToCV;
	const precision = 5;
	
	expect(f("C0")).toBeCloseTo(0, precision);
	expect(f("D0")).toBeCloseTo(2 / 12, precision);
	expect(f("C1")).toBeCloseTo(1, precision);
	expect(f("D1")).toBeCloseTo(1 + 2 / 12, precision);
	expect(f("E1")).toBeCloseTo(1 + 4 / 12, precision);
	expect(f("B1")).toBeCloseTo(2 - 1 / 12, precision);
	expect(f("C2")).toBeCloseTo(2, precision);
	expect(f("C4")).toBeCloseTo(4, precision);
	expect(f("C5")).toBeCloseTo(5, precision);
	
	// Accidentals
	expect(f("C#1")).toBeCloseTo(1 + 1 / 12, precision);
	expect(f("D#1")).toBeCloseTo(1 + 3 / 12, precision);
	expect(f("E#1")).toBeCloseTo(1 + 5 / 12, precision);
	expect(f("Bb1")).toBeCloseTo(2 - 2 / 12, precision);
	
	// Enharmonic equivalences
	expect(f("C#1")).toBeCloseTo(f("Db1"), precision);
	expect(f("E#1")).toBeCloseTo(f("F1"), precision);
	expect(f("B#1")).toBeCloseTo(f("C2"), precision);
	
	// Rests
	expect(f("-")).toBe(0);
	
	// Whitespace/case tolerance
	expect(f(" c0")).toBeCloseTo(f("C0"), precision);
	expect(f("c# 1 ")).toBeCloseTo(f("C#1"), precision);
	expect(f("B b 1")).toBeCloseTo(f("Bb1"), precision);
	expect(f(" - ")).toBe(0);
	
	// Ignore extra
	expect(f("C#1/8")).toBeCloseTo(f("C#1"), precision);
	expect(f("C# 1 ignored")).toBeCloseTo(f("C#1"), precision);
	
});

test('Invalid notes correctly throw errors', () => {
	
	const f = cli.noteToCV;
	
	expect(() => f()).toThrow();
	expect(() => f(null)).toThrow();
	expect(() => f(undefined)).toThrow();
	expect(() => f(false)).toThrow();
	expect(() => f("")).toThrow();
	expect(() => f(" ")).toThrow();
	
	expect(() => f(0)).toThrow();
	expect(() => f(1)).toThrow();
	
	expect(() => f("H1")).toThrow();
	expect(() => f("C##1")).toThrow();
	expect(() => f("C")).toThrow();
	expect(() => f("Cb")).toThrow();
	expect(() => f("#1")).toThrow();
	
	expect(() => f("foo")).toThrow();
	expect(() => f("bar")).toThrow();
	
});

test('Notes durations are correctly converted to integers', () => {
	
	const f = cli.noteDurationToInt;
	
	expect(f("1", 16)).toBe(16);
	expect(f("1", 32)).toBe(32);
	expect(f("2", 32)).toBe(16);
	expect(f("4", 32)).toBe(8);
	expect(f("32", 32)).toBe(1);
	
	// Dotted notes
	expect(f("1.", 32)).toBe(32 + 16);
	expect(f("1..", 32)).toBe(32 + 16 + 8);
	expect(f("2.", 32)).toBe(16 + 8);
	expect(f("2..", 32)).toBe(16 + 8 + 4);
	expect(f("2...", 32)).toBe(16 + 8 + 4 + 2);
	expect(f("16.", 32)).toBe(2 + 1);
	
	// Non-integer durations
	expect(() => f("64", 32)).toThrow(); // Half the resolution
	expect(() => f("3", 32)).toThrow(); // Triplet
	
	expect(() => f()).toThrow();
	expect(() => f(null)).toThrow();
	expect(() => f("foo")).toThrow();
	expect(() => f("bar")).toThrow();
	
});

test('Pattern are parsed correctly', () => {
	
	const f = cli.parsePattern;
	const precision = 5;
	
	expect(f("C3/2 C4/2").cv).toHaveLength(2);
	expect(f("C3/2 C4/2", 32).duration).toEqual([16, 16]);
	
	// Same-note legato
	expect(f("C3/2~C3/4", 32).cv).toHaveLength(1);
	expect(f("C3/2~C3/4", 32).duration).toEqual([16 + 8]);
	expect(f("C3/2~C3/2", 32).slide).toEqual([false]);
	expect(f("C3/2~ C3/2 D4/4", 32).cv).toHaveLength(2);
	expect(f("C3/2~ C3/2 -/4", 32).duration).toEqual([16 + 16, 8]);
	expect(f("C3/2~ C3/2 D4/4", 32).slide).toEqual([false, false]);
	expect(f("C3/8~C3/8 D4/4 C3/8~C3/8", 32).duration).toEqual([4 + 4, 8, 4 + 4]);
	
	// Slide
	expect(f("C3/2~ C3/2 D4/4", 32).slide).toEqual([false, false]);
	expect(f("C3/2~ C4/2 D4/4", 32).duration).toEqual([16, 16, 8]);
	expect(f("C3/2~ C4/2 D4/4", 32).slide).toEqual([true, false, false]);
	
	// Acciaccatura at the beginning of the pattern
	expect(f("C3/4", 32).acciaccatura).toBeNull();
	expect(f("(C2)C3/4", 32).acciaccatura).toBeCloseTo(2, precision);
	expect(f("(C2)C3/4 C3/4", 32).acciaccatura).toBeCloseTo(2, precision);
	
	expect(() => f()).toThrow();
	expect(() => f(null)).toThrow();
	expect(() => f("")).toThrow();
	expect(() => f("foo bar", 32)).toThrow();
	expect(() => f("C3/4 (C2)C3/4", 32)).toThrow(); // Acciaccatura only at the beginning
	
});

test('DAC integer values for CVs are correctly computed', () => {
	
	const f = cli.cvToInt;
	
	// Vref = 5V, 8 bits
	expect(f(0, 5, 8)).toBe(0);
	expect(f(2.5, 5, 8)).toBe(128);
	expect(f(4, 5, 8)).toBe(Math.round(256 * (4 / 5)));
	expect(f(5, 5, 8)).toBe(255);
	
	// Vref = 8V, 12 bits
	expect(f(0, 8, 12)).toBe(0);
	expect(f(4, 8, 12)).toBe(4096 / 2);
	
	// Vref = 4.096, 12 bits (MCP4728 internal Vref with X2 gain)
	expect(f(0, 4.096, 12)).toBe(0);
	expect(f(1, 4.096, 12)).toBe(1000);
	expect(f(4, 4.096, 12)).toBe(4000);
	expect(f(1.234, 4.096, 12)).toBe(1234);
	
	// Bounds
	expect(f(-1, 5, 8)).toBe(0);
	expect(f(99, 5, 8)).toBe(255);
	
	expect(() => f()).toThrow();
	
});