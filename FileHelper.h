#ifndef FILEHELPER_HEADER
#define FILEHELPER_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "../EmuBase.h"

#define FILENAMEMAXLENGTH (32)

extern int romsAvailable;

extern char currentFilename[FILENAMEMAXLENGTH];

int initFileHelper(u32 headerId);
void drawSpinner(void);

const RomHeader *browseForFile(void);

const RomHeader *findRom(int n);

/**
 * Returns the name of the rom as the given position.
 * @param  pos: Position of the rom in the list.
 * @return The name of the rom if there is one, otherwise returns "".
 */
const char *romNameFromPos(int pos);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // FILEHELPER_HEADER
