#include <string.h>
#include "gba.h"

#include "SRAMHandler.h"
#include "EmuMenu.h"
#include "FileHelper.h"
#include "ECL/ECL_NanoLZ.h"
#include "AsmExtra.h"
#include "../Emubase.h"
#include "../Main.h"
#include "../Cart.h"
#include "../FileHandling.h"

u8 * const BUFFER1 = SCRATCH_BUFF;
u8 * const BUFFER2 = &SCRATCH_BUFF[0x10000];

int totalStateSize;			// How much SRAM is used

//-------------------
extern char isPogoShell;

extern char pogoshell_romname[32];			// FileHelper.c
//----asm stuff------
extern u32 frameTotal;						// from Gfx.s
//-------------------

int usingFlashCart() {
	return (u32)romSpacePtr & 0x8000000;
}

void getSram() {		// Copy GBA SRAM to BUFFER1
	u8 *sram = (u8 *)SRAM;
	u8 *buff1 = BUFFER1;
	u32 *p;

	p = (u32 *)buff1;
	if (*p != STATEID) {		// If sram hasn't been copied already
		bytecopy_(buff1, sram, 0x10000); // Copy everything to BUFFER1
		if (*p != STATEID) {	// Valid savestate data?
			*p = STATEID;		// Nope. Initialize
			*(p+1) = 0;
		}
	}
}

// Quick & dirty rom checksum over first 16kB
u32 checksum(const u8 *p) {
	u32 sum = 0;
	int i;
	for (i=0;i<128;i++) {
		sum += *p|(*(p+1)<<8)|(*(p+2)<<16)|(*(p+3)<<24);
		p += 128;
	}
	return sum;
}

void writeError() {
	infoOutput("Write error! Memory full.");
	infoOutput("Delete some saves.");
}

// (BUFFER1=copy of GBA SRAM, BUFFER2=new data)
// overwrite:  index=state#, erase=0
// new:  index=big number (anything >=total saves), erase=0
// erase:  index=state#, erase=1
// returns TRUE if successful
// IMPORTANT!!! totalStateSize is assumed to be current
int updateStates(int index, int erase, int type) {
	int total = totalStateSize;
	u8 *src = BUFFER1+4; // Skip STATEID
	u8 *dst = BUFFER1+4;
	StateHeader *newData = (StateHeader *)BUFFER2;

	// Skip ahead to where we want to write

	int srcSize = ((StateHeader *)dst)->size;
	int i = (type == ((StateHeader *)dst)->type)?0:-1;
	while (i < index && srcSize) {	// While (looking for state) && (not out of data)
		dst += srcSize;
		srcSize = ((StateHeader *)dst)->size;
		if (((StateHeader *)dst)->type == type)
			i++;
	}
	src = dst+srcSize;

	// Write new data

	total -= srcSize;
	if (!erase) {
		i = newData->size;
		total += i;
		if (total > 0x10000) // **OUT OF MEMORY**
			return 0;
		src = (u8 *)newData + newData->size;
		// Copy trailing data to 2nd buffer
		memcpy(src, dst+((StateHeader *)dst)->size, total-i);
		// Overwrite
		memcpy(dst, newData, i);
		dst += i;
	}

	srcSize = ((StateHeader *)src)->size;

	// Get trailing data

	while (srcSize) {
		memcpy(dst, src, srcSize);
		src += srcSize;
		dst += srcSize;
		srcSize = ((StateHeader *)src)->size;
	}
	*(u32 *)dst = 0; // Terminate
	*(u32 *)(dst+4) = 0xffffffff; // Terminate
	dst += 8;
	total += 8;

	// Copy everything to GBA sram

	totalStateSize = total;
	while (total < 0x10000)
	{
		*dst++ = 0;
		total++;
	}
	bytecopy_((u8 *)SRAM, BUFFER1, total); // Copy to sram
	return 1;
}

// More dumb stuff so we don't waste space by using sprintf
int twoDigits(int n, char *s) {
	int mod = n%10;
	n = n/10;
	*(s++) = (n+'0');
	*s = (mod+'0');
	return n;
}

void getStateTimeAndSize(char *s, int time, u32 size, u32 totalsize) {
	strlcpy(s, "00:00:00 - 00/00k", 30);
	twoDigits(time/216000, s);
	s += 3;
	twoDigits((time/3600)%60, s);
	s += 3;
	twoDigits((time/60)%60, s);
	s += 5;
	twoDigits(size/1024, s);
	s += 3;
	twoDigits(totalsize/1024, s);
}

#define LOAD_MENU 0
#define SAVE_MENU 1
#define NVRAM_MENU 2
#define FIRSTLINE 2
#define LASTLINE ((SCREEN_HEIGHT/8)-4)

// BUFFER1 holds copy of SRAM
// Draw save/loadState menu and update global totalStateSize
// Returns a pointer to current selected state
// Update *states on exit
StateHeader *drawStates(int menuType, int *menuItems) {
	int sel = selected;
	int size;
	char s[30];
	StateHeader *selectedState = NULL;
	int time = 0;
	int selectedStateSize = 0;
	StateHeader *sh = (StateHeader *)(BUFFER1+4);

	int type = (menuType == NVRAM_MENU) ? SRAM_SAVE : STATE_SAVE;

	int startLine = FIRSTLINE;
	int stateCount = 0;
	int total = 8; // Header+null terminator
	while (sh->size) {
		size = sh->size;
		if (sh->type == type) {
			if (startLine+stateCount >= FIRSTLINE && startLine+stateCount <= LASTLINE) {
				drawItem(sh->title, stateCount+FIRSTLINE, sel == stateCount);
			}
			if (sel == stateCount) { // Keep info for selected state
				time = sh->frameCount;
				selectedStateSize = size;
				selectedState = sh;
			}
			stateCount++;
		}
		total += size;
		sh = (StateHeader *)((u8 *)sh+size);
	}

	if (sel != stateCount) { // Not <NEW>
		getStateTimeAndSize(s, time, selectedStateSize, total);
		drawText(s, 18);
	}
	if (stateCount) {
		drawText("Push SELECT to delete", 19);
	}
	if (menuType == SAVE_MENU) {
		if (startLine+stateCount <= LASTLINE) {
			drawMenuItem("<NEW>");
		}
		stateCount++; // Include <NEW> as a menuitem
//		drawText("Save state:", 0);
	}
//	else if (menuType == LOAD_MENU) {
//		drawText("Load state:", 0);
//	}
	*menuItems = stateCount;
	totalStateSize = total;
	return selectedState;
}

// Compress src into BUFFER2 (adding header)
void compressState(u32 size, u16 type, const u8 *src) {

	uint8_t buf[513];
	u32 compressedSize = ECL_NanoLZ_Compress_mid2min(ECL_NANOLZ_SCHEME1, src, size, BUFFER2+sizeof(StateHeader), 0x10000, buf);

	// Setup header:
	StateHeader *sh = (StateHeader *)BUFFER2;
	sh->size = (compressedSize+sizeof(StateHeader)+3)&~3; // Size of compressed state+header, word aligned
	sh->type = type;
	sh->dataSize = compressedSize; // Size of compressed state
	sh->frameCount = frameTotal;
	sh->checksum = checksum((u8 *)romSpacePtr); // Checksum
//	if (isPogoShell) {
//		strlcpy(sh->title, pogoshell_romname, 32);
//	}
//	else {
		strlcpy(sh->title, findRom(romNum)->name, 32);
//	}
}

// Locate last save by checksum
// Returns save index (-1 if not found) and updates statePtr
// Updates totalStateSize (so quickSave can use updateStates)
int findState(u32 checksum, int type, StateHeader **statePtr) {
	getSram();
	StateHeader *sh = (StateHeader *)(BUFFER1+4);

	int state = -1;
	int foundstate = -1;
	int total = 8; // Header+null terminator
	int size = sh->size;
	while (size) {
		if (sh->type == type) {
			state++;
			if (sh->checksum == checksum) {
				foundstate = state;
				*statePtr = sh;
			}
		}
		total += size;
		sh = (StateHeader *)(((u8 *)sh)+size);
		size = sh->size;
	}
	totalStateSize = total;
	return foundstate;
}

void uncompressState(int rom, StateHeader *sh, u8 *dest) {
	ECL_usize result = ECL_NanoLZ_Decompress(ECL_NANOLZ_SCHEME1, sh->data, sh->dataSize, dest, 0x10000);
	if (result == 0) {
		unpackState(dest);
		frameTotal = sh->frameCount; // Restore global frame counter
	}
}

void saveStateMenu() {
	int menuItems;

	int i = packState(BUFFER1);
	compressState(i, STATE_SAVE, BUFFER1);

	getSram();

//	selected = 0;
	drawStates(SAVE_MENU, &menuItems);
	do {
		i = getMenuInput(menuItems);
		if (i & KEY_A) {
			if (!updateStates(selected, 0, STATE_SAVE)) {
				writeError();
			}
		}
		if (i & KEY_SELECT) {
			updateStates(selected, 1, STATE_SAVE);
			if (selected == menuItems-1) {
				selected--;	// Deleted last entry? Move up one
			}
		}
		if (i & (KEY_SELECT+KEY_UP+KEY_DOWN+KEY_LEFT+KEY_RIGHT)) {
			drawStates(SAVE_MENU, &menuItems);
		}
	} while (!(i & (KEY_A+KEY_B)));
}

void loadStateMenu() {
	int menuItems;

	getSram();

//	selected = 0;
	StateHeader *sh = drawStates(LOAD_MENU, &menuItems);
	if (!menuItems) {
		return; // Nothing to load!
	}
	u32 key;
	int i;
	do {
		key = getMenuInput(menuItems);
		if (key & KEY_A) {
			u32 sum = sh->checksum;
			i = 0;
			do {
				if (sum == checksum(findRom(i)->romData)) { // Find rom with matching checksum
					uncompressState(i, sh, BUFFER2);
					i = 8192;
				}
				i++;
			} while (i < romsAvailable);
			if (i < 8192) {
				infoOutput("ROM not found!");
			}
		}
		else if (key & KEY_SELECT) {
			updateStates(selected, 1, STATE_SAVE);
			if (selected == menuItems-1) {
				selected--;	// Deleted last entry? Move up one
			}
		}
		if (key & (KEY_SELECT+KEY_UP+KEY_DOWN+KEY_LEFT+KEY_RIGHT)) {
			sh = drawStates(LOAD_MENU, &menuItems);
		}
	} while (menuItems && !(key & (KEY_A+KEY_B)));
}

int quickSave() {
	if (usingFlashCart()) {
		infoOutput("Saving State.");

		int i = packState(BUFFER1);
		compressState(i, STATE_SAVE, BUFFER1);
		StateHeader *sh;
		i = findState(checksum((u8 *)romSpacePtr), STATE_SAVE, &sh);
		if (i < 0) i = 65536; // Make new save if one doesn't exist
		if (updateStates(i, 0, STATE_SAVE)) {
			return 1;
		}
		writeError();
	}
	return 0;
}

int quickLoad() {
	if (usingFlashCart()) {
		StateHeader *sh;
		int i = findState(checksum((u8 *)romSpacePtr), STATE_SAVE, &sh);
		if (i >= 0) {
			infoOutput("Loading State.");
			uncompressState(romNum, sh, BUFFER2);
			return 1;
		}
	}
	return 0;
}

void manageNVRAM() {
	int menuItems;

	getSram();

	drawStates(NVRAM_MENU, &menuItems);
	if (!menuItems) {
		return; // Nothing to do!
	}
	int key;
	do {
		key = getMenuInput(menuItems);
		if (key & KEY_SELECT) {
			updateStates(selected, 1, SRAM_SAVE);
			if (selected == menuItems-1) {
				selected--; // Deleted last entry.. move up one
			}
		}
		if (key & (KEY_SELECT+KEY_UP+KEY_DOWN+KEY_LEFT+KEY_RIGHT)) {
			drawStates(NVRAM_MENU, &menuItems);
		}
	} while (menuItems && !(key & KEY_B));
}

// Check if there allready is a SRAM save for the current game.
int checkForEmuSram() {
	if (!usingFlashCart()) {
		return -1;
	}
	StateHeader *sh;
	return findState(checksum((u8 *)romSpacePtr), SRAM_SAVE, &sh);	// See if packed SRAM exists
}

// Make new saved sram (using EMU_SRAM contents)
// This is to ensure that we have all info for this rom and can save it even after this rom is removed
int saveEmuSram(const u8 *src, int size) {
	if (usingFlashCart()) {
		infoOutput("Saving NVRAM.");
		compressState(size, SRAM_SAVE, src);
		int i = checkForEmuSram();	// See if packed SRAM exists
		if (i < 0) i = 65536;		// Make new save if one doesn't exist
		if (updateStates(i, 0, SRAM_SAVE)) {
			return 1;
		}
		writeError(1);
	}
	return 0;
}

int loadEmuSram(u8 *dst, int size) {
	if (usingFlashCart()) {
		u32 chk = checksum(romSpacePtr);
		StateHeader *sh;
		int i = findState(chk, SRAM_SAVE, &sh); // See if packed SRAM exists

		if (i >= 0) { // Packed SRAM exists: unpack into EMU_SRAM
			infoOutput("Loading NVRAM.");
			ECL_usize result = ECL_NanoLZ_Decompress(ECL_NANOLZ_SCHEME1, sh->data, sh->dataSize, dst, size);
			if (result == size) {
				return 1;
			}
		}
	}
	return 0;
}

int writeConfig(ConfigHeader *cfgDat) {
	if (!usingFlashCart()) {
		return 0;
	}
	ConfigData *cfg;
	int i = findState(0, CONFIG_SAVE, (StateHeader **)&cfg);

	if (i < 0) { // Create new config
		memcpy(BUFFER2, cfgDat, sizeof(ConfigData));
		cfg = (ConfigData *)BUFFER2;
		updateStates(0, 0, CONFIG_SAVE);
	}
	else {		// Config already exists, update SRAM directly (faster)
		bytecopy_((u8 *)cfg-BUFFER1+(u8 *)SRAM, (u8 *)cfgDat, sizeof(ConfigData));
	}
	return 1;
}

int readConfig(ConfigHeader *cfgHeader) {
	if (!usingFlashCart()) {
		return 0;
	}
	const ConfigData *cfg;
	int i = findState(0, CONFIG_SAVE, (StateHeader **)&cfg);
	if (i < 0) {
		return 0;
	}
	ConfigData *cfgDat = (ConfigData *)cfgHeader;
	*cfgDat = *cfg;
	return 1;
}
