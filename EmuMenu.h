#ifndef EMUMENU_HEADER
#define EMUMENU_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#define ARRSIZE(xxxx) (sizeof((xxxx))/sizeof((xxxx)[0]))

typedef void (*fptr)(void);

extern u8 autoA;
extern u8 autoB;
extern u8 g_debugSet;

extern bool settingsChanged;
extern bool pauseEmulation;
extern bool enableExit;

extern int emuSettings;
extern int sleepTime;
/// This is the current row in the menu.
extern int selected;

void guiRunLoop(void);
int getInput(void);
int getMenuInput(int menuItems);
int getMenuPos(int keyHit, int sel, int menuItems);
void redrawUI(void);
int drawFileList(const char *dTable, int sel, int items);
void drawTextXY(const char *str, int col, int row);
void drawText(const char *str, int row);
void drawBText(const char *str, int row, int shadow);
void strlMerge(char *dst, const char *src1, const char *src2, int dstSize);
void drawSubItem(const char *str1, const char *str2);
void drawMenuItem(const char *str1);
void drawItemXY(const char *str, int col, int row, int hiLite);
void drawItem(const char *str, int row, int hiLite);
void setupSubMenu(const char *menuString);
void setMenuItemRow(int row);
void cls(int chrMap);
void int2Str(int i, char *s);
void int2HexStr(char *dest, int hexValue);
void short2HexStr(char *dest, short hexValue);
void char2HexStr(char *dest, char hexValue);
void infoOutput(const char *str);
void updateInfoLog(void);
void outputLogToScreen(void);
void debugOutput(const char *str);
void nullUI(void);
void subUI(void);
void uiDummy(void);
void setSelectedMenu(int menuNr);
void openMenu(void);
void backOutOfMenu(void);
void closeMenu(void);

void setDarknessGs(int dark);
void setBrightnessAll(int light);
void fadeToWhite();
void scrollL(int offset, int fade);
void scrollR(int offset);
void gbaSleep(void);
void exitEmulator(void);

void uiNullDefault(void);
void uiYesNo(void);

void ui1(void);
void ui2(void);
void ui3(void);
void ui4(void);
void ui5(void);
void ui6(void);
void ui7(void);
void ui8(void);

void autoPauseGameSet();
void autoStateSet();
void autoSettingsSet();
void autoNVRAMSet();
void saveNVRAMSet();
void debugTextSet();
void sleepSet();
void powerSaveSet();
void screenSwapSet();
void autoASet();
void autoBSet();
void speedSet();
void flickSet();

#ifdef __cplusplus
} // extern "C"
#endif

#endif // EMUMENU_HEADER
