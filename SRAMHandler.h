#ifndef SRAMHANDLER_HEADER
#define SRAMHANDLER_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#define STATEID 0x57A731DC

#define CONFIG_SAVE 0
#define STATE_SAVE 1
#define SRAM_SAVE 2
#define EEPROM_SAVE 4
#define FLASH_SAVE 8

#define FILENAME_MAX_LENGTH (32)

typedef struct {
	u16 size;			// header+data, needs to be first, size 0 = terminator.
	u16 type;			// STATE_SAVE, SRAM_SAVE or CONFIG_SAVE
	u32 checksum;
	u32 dataSize;
	char title[FILENAME_MAX_LENGTH];
	u32 frameCount;
	u8 data[];
} StateHeader;

void manageNVRAM(void);
void loadStateMenu(void);
void saveStateMenu(void);

int quickLoad(void);
int quickSave(void);

int saveEmuSram(const u8 *src, int size);
int loadEmuSram(u8 *dst, int size);

int writeFile(const u8 *src, int size, int checksum, const char *name);
int readFile(u8 *dst, int size, int checksum);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SRAMHANDLER_HEADER
