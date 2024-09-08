#ifndef ASMEXTRA_HEADER
#define ASMEXTRA_HEADER

#ifdef __cplusplus
extern "C" {
#endif

/// The current FPS as calculated by the calculateFPS function.
extern char fpsText[8];

/// Reset Flashcart and reboot GBA.
void resetFlashCart(void);

/// Put the GBA to sleep, sets Start+Select as wakeup keys. Doesn't seem to work right now, maybe some kind of IRQ not enabled?
void suspend(void);

/**
 * Can be used to overclock EWRAM speed, doesn't work on GBA Micro.
 * @param  overclock: true to overclock EWRAM.
 */
void setEWRAMSpeed(bool overclock);

/**
 * Tries to read the Real Time Clock from the cartridge as implemented in Pokemon.
 * @return The time as BCD encoded ??SSMMHH, r1=??DDMMYY.
*/
int getTime(void);

/**
 * Converts a binary number to Binary Encoded Decimal format
 * @param  value: Value to convert.
 * @return Input value converted to BCD.
 */
int bin2BCD(int number);

/**
 * Generates a "random" number between 0 and maxValue.
 * @param  maxValue: How large the distribution of values are.
 * @return A new "random" number.
 */
int getRandomNumber(int maxValue);

/**
 * Guaranteed to only copy bytes, usefull for copying mem to GBA SRAM.
 * @param dst: The destination, this should probably be in the 0x0E000000 range.
 * @param src: The source, can be anything.
 * @param count: Number of bytes to copy.
 */
void bytecopy_(u8 *dst, const u8 *src, int count);

/// Calculates fps, needs to be called _every_ frame, eg. from the VBlank interrupt.
void calculateFPS(void);

/**
 * Converts 24bit RGB palette to 15bit BGR palette, while running gamma conversion on it.
 * @param  *dest: Destination of result, needs to be aligned to 16bit.
 * @param  *source: Source of RGB data, 8bit aligned.
 * @param  length: How many colors to convert.
 * @param  gamma: Gamma value between 1 & 4.
 */
void convertPalette(u16 *dest, const u8 *source, int length, int gamma);

/**
 * Outputs text to the console of an NDS/GBA emulator, this works at least in No$GBA.
 * @param  *text: The text to output, max 80 chars.
 */
void debugOutputToEmulator(const char *text);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // ASMEXTRA_HEADER
