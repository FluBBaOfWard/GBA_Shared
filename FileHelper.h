#ifndef FILEHELPER_HEADER
#define FILEHELPER_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "../Emubase.h"

#define FILENAME_MAX_LENGTH (32)

extern int romsAvailable;

extern char currentFilename[FILENAME_MAX_LENGTH];

/**
 * Tries to find appended roms through the supplied headerId.
 * @param headerId: The id to use when finding the roms.
 * @return The number of found roms.
 */
int initFileHelper(u32 headerId);

void drawSpinner(void);

const RomHeader *browseForFile(void);

/**
 * Return ptr to Nth ROM (including rominfo struct)
 * @param  n: rom number to find.
 * @return RomHeader to selected ROM or null if not found.
 */
const RomHeader *findRom(int n);

/**
 * Return ptr to Nth BIOS (including rominfo struct)
 * @param  n: Bios number to find.
 * @return RomHeader to selected BIOS or null if not found.
 */
const RomHeader *findBios(int n);

/**
 * Returns the name of the rom at the given position.
 * @param  pos: Position of the rom in the list.
 * @return The name of the rom if there is one, otherwise returns "".
 */
const char *romNameFromPos(int pos);

/**
 * Returns a pointer to the splash screen if found, else null.
 * @param headerId: The id to use when finding the roms.
 * @return A pointer to the splash screen or null.
 */
const u16 *getSplashScreen(u32 inHeaderId);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // FILEHELPER_HEADER
