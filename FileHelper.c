#include <gba.h>

#include <string.h>

#include "FileHelper.h"
#include "EmuMenu.h"
#include "../Main.h"
#include "../FileHandling.h"


static u32 headerId = 0;
/** Pointer to roms including bioses */
static const RomHeader *romData;
/** Pointer to games */
static const RomHeader *romGames;
/** Total number of roms found */
int romCount = 0;

// Set text_start
extern u8 __rom_end__[];

int romsAvailable = 0;

char currentFilename[FILENAMEMAXLENGTH];

static const char *const spinner[4]={"\\","|","/","-"};
static char spinnerCount = 0;
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

int initFileHelper(u32 inHeaderId) {
	romData = (const RomHeader *)__rom_end__;
	romGames = romData;
	headerId = inHeaderId;
	const RomHeader *p = romData;
	romCount = 0;
	char isBios;
	while (p && p->identifier == inHeaderId) {
		isBios = p->bios;
		// Count roms
		p = getNextRom(p);
		p = findRomHeader(p, inHeaderId);
		if (isBios & 1) {
			romGames = p;
		} else {
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
