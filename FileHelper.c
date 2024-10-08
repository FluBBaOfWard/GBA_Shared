#include <gba.h>

#include <string.h>

#include "FileHelper.h"
#include "EmuMenu.h"
#include "../Main.h"
#include "../FileHandling.h"


EWRAM_BSS static u32 headerId = 0;
/** Pointer to roms including bioses */
EWRAM_BSS static const RomHeader *romData;
/** Pointer to games */
EWRAM_BSS static const RomHeader *romGames;
/** Total number of roms found */
EWRAM_BSS int romCount = 0;

// Set text_start
extern u8 __rom_end__[];

EWRAM_BSS int romsAvailable = 0;

EWRAM_BSS char currentFilename[FILENAME_MAX_LENGTH];

EWRAM_DATA static const char *const spinner[4]={"\\","|","/","-"};
EWRAM_BSS static char spinnerCount = 0;
//---------------------------------------------------------------------------------

const RomHeader *findRomHeader(const RomHeader *base, u32 headerId)
{
	// Look up to 256 bytes later for a ROM header
	const u32 *p=(u32*)base;
	
	int i;
	for (i=0; i<64; i++) {
		if (*p == headerId) {
			return ((RomHeader *)p);
		}
		else {
			p++;
		}
	}
	return NULL;
}

static const RomHeader *getNextRom(const RomHeader *rh) {
	const u8 *p = (const u8 *)rh;
	return (const RomHeader *)(p + rh->filesize + sizeof(RomHeader));
}

const u16 *getSplashScreen(u32 inHeaderId) {
	const u16 *sp = (const u16 *)__rom_end__;
	const RomHeader *rh = (const RomHeader *)(sp + 240*160);
	if (rh->identifier == inHeaderId) {
		return sp;
	}
	return NULL;
}

int initFileHelper(u32 inHeaderId) {
	const RomHeader *rh = (const RomHeader *)__rom_end__;
	// Check for splash screen
	if (rh->identifier != inHeaderId) {
		rh = (const RomHeader *)((const u8 *)rh + 240*160*2);
	}
	romData = rh;
	romGames = romData;
	headerId = inHeaderId;
	romCount = 0;
	while (rh && rh->identifier == inHeaderId) {
		char isBios = rh->bios;
		// Count roms
		rh = getNextRom(rh);
		rh = findRomHeader(rh, inHeaderId);
		if (isBios & 1) {
			romGames = rh;
		}
		else {
			romCount++;
		}
	}
	romsAvailable = romCount;
	return romCount;
}

const RomHeader *findRom(int n) {
	const RomHeader *rh = romGames;
	while (n--) {
		rh = getNextRom(rh);
	}
	if (rh->identifier == headerId) {
		return rh;
	}
	return NULL;
}

const RomHeader *findBios(int n) {
	const RomHeader *rh = romData;
	while (n--) {
		rh = getNextRom(rh);
	}
	if (rh->identifier == headerId && rh->bios & 1) {
		return rh;
	}
	return NULL;
}

const char *romNameFromPos(int pos) {
	if (pos < romCount) {
		return findRom(pos)->name;
	}
	return "";
}

void drawSpinner() {
	drawTextXY(spinner[spinnerCount&3], 15, 10);
	spinnerCount++;
}

//---------------------------------------------------------------------------------
const RomHeader *browseForFile(void) {
	static int pos = 0;
	int oldPos = -1;
	const RomHeader *rh;
	int pressed = 0;

	if (romCount > 0) {
		cls(0);
		if (pos >= romCount) {
			pos = romCount-1;
		}
		while (1) {
			waitVBlank();
			pressed = getInput();
			pos = getMenuPos(pressed, pos, romCount);
			if (pressed & (KEY_A)) {
				rh = findRom(pos);
				strlcpy(currentFilename, rh->name, sizeof(currentFilename));
				return rh;
			}
			if (oldPos != pos) {
				oldPos = pos;
				drawFileList(pos, romCount);
				outputLogToScreen();
			}
			if (pressed & (KEY_B)) {
				break;
			}
			updateInfoLog();
		}
	}
	return NULL;
}
