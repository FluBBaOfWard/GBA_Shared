#include <gba.h>

#include <string.h>

#include "FileHelper.h"
#include "EmuMenu.h"
#include "../Main.h"
#include "../FileHandling.h"


static u32 headerId = 0;
static const u8 *romData;
/** Total number of roms found */
int romCount = 0;

// Set text_start (before moving the rom)
extern u8 __rom_end__[];

int romsAvailable = 0;

char currentFilename[FILENAMEMAXLENGTH];

static const char *const spinner[4]={"\\","|","/","-"};
static char spinnerCount = 0;
//---------------------------------------------------------------------------------

const u8 *findRomHeader(const u8 *base, u32 headerId)
{
	// Look up to 256 bytes later for a ROM header
	const u32 *p=(u32*)base;
	
	int i;
	for (i=0; i<64; i++) {
		if (*p == headerId) {
			return ((u8*)p);
		}
		else {
			p++;
		}
	}
	return NULL;
}

int initFileHelper(u32 inHeaderId) {
	romData = __rom_end__;
	headerId = inHeaderId;
	const u8 *p = romData;
	romCount = 0;
	while (p && *(u32*)(p) == inHeaderId) {
		// Count roms
		const romheader *rh = (romheader *)p;
		p += rh->filesize + sizeof(romheader);
		p = findRomHeader(p, inHeaderId);
		romCount++;
	}
	romsAvailable = romCount;
	return romCount;
}

static int offsetOfNextRom(romheader *rh) {
	return rh->filesize + sizeof(romheader);
}

// Return ptr to Nth ROM (including rominfo struct)
const u8 *findRom(int n) {
	const u8 *p = romData;
	while (n--) {
		p += offsetOfNextRom((romheader *)p);
	}
	return p;
}

const char *romNameFromPos(int pos) {
	if (pos < romCount) {
		romheader *rh = (romheader *)findRom(pos);
		return rh->name;
	}
	return NULL;
}

void drawSpinner() {
	drawTextXY(spinner[spinnerCount&3], 15, 10);
	spinnerCount++;
}

//---------------------------------------------------------------------------------
const romheader *browseForFile(void) {
	static int pos = 0;
	int oldPos = -1;
	const romheader *rh;
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
				rh = (romheader *)findRom(pos);
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
