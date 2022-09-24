#ifndef FILEHELPER_HEADER
#define FILEHELPER_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#include "../RomHeader.h"

#define FILENAMEMAXLENGTH (32)

extern int romsAvailable;

extern char currentFilename[FILENAMEMAXLENGTH];

int initFileHelper(u32 headerId);
void drawSpinner(void);

const romheader *browseForFile(void);

const char *romNameFromPos(int pos);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // FILEHELPER_HEADER
