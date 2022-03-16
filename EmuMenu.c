#include <gba.h>

#include <string.h>

#include "EmuMenu.h"
#include "EmuSettings.h"
#include "AsmExtra.h"
#include "../Main.h"
#include "../GUI.h"
#include "../FileHandling.h"
#include "../Gfx.h"
#include "../io.h"
#include "../Sound.h"

extern const fptr fnMain[];
extern const fptr *const fnListX[];
extern const u8 menuXitems[];
extern const fptr drawuiX[];
extern const u8 menuXback[];

static void exitUI(void);

u8 autoA = 0;			// 0=off, 1=on, 2=R
u8 autoB = 0;
u8 g_debugSet = 0;		// Should we output debug text?
bool settingsChanged = false;
bool pauseEmulation = false;
bool enableExit = false;

int emuSettings = 0;
int sleepTime = 60*60*5;			// 5 min
int selected = 0;

static int selectedMenu = 0;
static int selectedMain = 0;
static int lastMainMenu = 1;
static int mainUIPosition = 0;
static int menuItemRow = 0;
static int lineRepeat = 0;

static int logBufPtr = 0;
static int logBufPtrOld = 0;
static int logTimer = 0;

static char logBuffer[8][32];

void guiRunLoop(void) {
	fnMain[selectedMenu]();
}

void uiNullDefault() {
//	drawText("       Touch screen or", 9);
//	drawText("     press L+R for menu.", 10);
}


void uiYesNo() {
	setupSubMenu("Are you sure?");
	drawSubItem("Yes ", 0);
	drawSubItem("No ", 0);
}

void uiDummy() {
}

void ui1() {
	setSelectedMenu(1);
}
void ui2() {
	setSelectedMenu(2);
}
void ui3() {
	setSelectedMenu(3);
}
void ui4() {
	setSelectedMenu(4);
}
void ui5() {
	setSelectedMenu(5);
}
void ui6() {
	setSelectedMenu(6);
}
void ui7() {
	setSelectedMenu(7);
}
void ui8() {
	setSelectedMenu(8);
}

void setSelectedMenu(int menuNr) {
	if (selectedMain == selectedMenu) {		// Are we going from a mainMenu to a subMenu?
		mainUIPosition = selected;
	}
	selectedMenu = menuNr;
	selected = 0;

	if (selectedMain == selectedMenu) {		// Are we going back to a mainMenu?
		selected = mainUIPosition;
	}
	if (selectedMenu == 0) {
		exitUI();
	}
	else if (selectedMenu < 2) {
		selectedMain = menuNr;
	}
	cls(0);
	redrawUI();
}

void openMenu() {
	enterGUI();
	setSelectedMenu(lastMainMenu);
	setupMenuPalette();
	setDarknessGs(8);
	if (emuSettings & AUTOPAUSE_EMULATION) {	// Should we pause when menu is open?
		pauseEmulation = true;
		setMuteSoundGUI();
	}
	if (emuSettings & AUTOSAVE_NVRAM) {
		saveNVRAM();
	}
}

void closeMenu() {
	setSelectedMenu(0);
}

void backOutOfMenu() {
	setSelectedMenu(menuXback[selectedMenu]);
}

void exitUI() {
	exitGUI();
	pauseEmulation = false;
	setMuteSoundGUI();
	if (settingsChanged && (emuSettings & AUTOSAVE_SETTINGS)) {
		saveSettings();
		settingsChanged = false;
	}
	setDarknessGs(0);
	paletteTxAll();

	lastMainMenu = selectedMain;
	if (lastMainMenu < 1) {
		lastMainMenu = 1;
	}
}

/// This is during emulation.
void nullUI() {
	int key = getInput();
	if ((EMUinput & (KEY_L|KEY_R)) == (KEY_L|KEY_R)) {
		openMenu();
		return;
	}

	if (g_debugSet) {
		drawText(fpsText, 0);
		nullUIDebug(key);
	}
	else {
		nullUINormal(key);
	}
	updateInfoLog();
}

/// This is during menu.
void subUI() {
	int key;

	key = getMenuInput(menuXitems[selectedMenu]);
	if (key & (KEY_A)) {
		fnListX[selectedMenu][selected]();
	}
	if (key & (KEY_B)) {
		backOutOfMenu();
	}
/*	if (key & (KEY_L)) {
		if (selectedMain > 0) {
			setSelectedMenu(selectedMain-1);
		}
	}
	if (key&(KEY_R)) {
		if (selectedMain < 3) {
			setSelectedMenu(selectedMain+1);
		}
	}*/
	if (key & (KEY_A+KEY_UP+KEY_DOWN+KEY_LEFT+KEY_RIGHT)) {
		redrawUI();
	}
	updateInfoLog();
}

int getMenuInput(int itemCount) {
	int keyHit;

	keyHit = getInput();
	selected = getMenuPos(keyHit, selected, itemCount);
	return keyHit;
}

//---------------------------------------------------------------------------------
int getInput() {
	int dpad;
	int keyHit = keysDown();	// Buttons pressed this loop

	EMUinput = keysHeld();

	dpad = keysDownRepeat() & (KEY_UP+KEY_DOWN+KEY_LEFT+KEY_RIGHT);
	return dpad|(keyHit & (KEY_A+KEY_B+KEY_START+KEY_L+KEY_R));
}

//---------------------------------------------------------------------------------
int getMenuPos(int keyHit, int sel, int itemCount) {
	if (keyHit & KEY_UP) {
		sel = (sel+itemCount-1)%itemCount;
	}
	if (keyHit & KEY_DOWN) {
		sel = (sel+1)%itemCount;
	}
	if (keyHit & KEY_RIGHT) {
		sel += 10;
		if (sel > itemCount-1) {
			sel = itemCount-1;
		}
	}
	if (keyHit & KEY_LEFT) {
		sel -= 10;
		if (sel < 0) {
			sel = 0;
		}
	}
	return sel;
}

void redrawUI() {
	drawuiX[selectedMenu]();
	outputLogToScreen();
}

int drawFileList(const char *dTable, int sel, int items) {
	int i, firstItem, selected;
	const char *buf;
	lineRepeat = 0;				// Reset long filename position

	firstItem = sel-11;
	if (sel > (items-13)) {
		firstItem = items-23;
	}
	if (firstItem < 0) {
		firstItem = 0;
	}

	for (i = 0; i < (SCREEN_HEIGHT / 8 - 1); i++) {
		if ( buf == NULL ) {
			buf = "";
		}
		if ( *buf == '~' ) {
			buf++;
		}
		selected = (i == (sel-firstItem)?4:0);
		drawItem(buf, i+1, selected);
	}
	return (sel-firstItem);
}

void cls(int chrMap) {
	int i = 0;
	int len = 0x200;
	u32 *scr = (u32*)menuMap;
	if (chrMap >= 2) {
		len = 0x400;
	}
	if (chrMap == 2){
		i = 0x200;
	}
	for (;i<len;i++) {				// 512x256
		scr[i] = 0x01200120;
	}
	REG_BG3VOFS = 0;
}

void drawTextXY(const char *str, int col, int row) {
	drawItemXY(str, col, row, 0);
}

void drawText(const char *str, int row) {
	drawItemXY(str, 0, row, 0);
}

void setupSubMenu(const char *menuString) {
	int len = strlen(menuString);
	menuItemRow = 0;
	drawItemXY(menuString, (28-len)/2, 0, 0);
}

void drawMenuItem(const char *str) {
	drawItemXY(str, 0, menuItemRow+3, selected==menuItemRow);
	menuItemRow++;
}

void drawSubItem(const char *str1, const char *str2) {
	char str[32];
	if (str2) {
		strlMerge(str, str1, str2, sizeof(str));
		str1 = str;
	}
	drawMenuItem(str1);
}

void drawItemXY(const char *str, int col, int row, int hiLite) {
	u16 *here = menuMap+row*32;
	int i = col+1;

	*here = hiLite?0xF12a:0xF120;
	hiLite = 0xF100-(hiLite<<12);
	while (*str >= ' ') {
		here[i] = hiLite|*str++;
		i++;
		if (i > 31) {
			break;
		}
	}
	for (; i < 32; i++) {
		here[i] = 0x0120;
	}
}

void drawItem(const char *str, int row, int hiLite) {
	drawItemXY(str, 0, row, hiLite);
}

void setMenuItemRow(int row) {
	menuItemRow = row;
}

void strlMerge(char *dst, const char *src1, const char *src2, int dstSize) {
	if (dst != src1) {
		strlcpy(dst, src1, dstSize);
	}
	strlcat(dst, src2, dstSize);
}

void int2Str(int val, char *dst) {
	u32 j;
	int mod,k;

	j = bin2BCD(val);
	for (k = 28; k >= 0; k -= 4) {
		mod = (j>>k) & 15;
		*(dst++) = (mod+'0');
	}
	*(dst++) = 0;
}

void int2HexStr(char *dest, int val) {
	int i;
	for (i = 0; i < 8; i++) {
		dest[7-i] = (val & 0xF) + (((val & 0xF) < 10) ? '0' : '7');
		val = val>>4;
	}
	dest[8] = 0;
}

void short2HexStr(char *dest, short val) {
	int i;
	for (i = 0; i < 4; i++) {
		dest[3-i] = (val & 0xF) + (((val & 0xF) < 10) ? '0' : '7');
		val = val>>4;
	}
	dest[4] = 0;
}

void char2HexStr(char *dest, char val) {
	dest[0] = ((val>>4) & 0xF) + ((((val>>4) & 0xF) < 10) ? '0' : '7');
	dest[1] = (val & 0xF) + (((val & 0xF) < 10) ? '0' : '7');
	dest[2] = 0;
}

void updateInfoLog() {
	if (logBufPtrOld != logBufPtr) {
		logBufPtrOld = logBufPtr;
		outputLogToScreen();
		logTimer = 120;
	}
	if (logTimer >= 0) {
		if (logTimer == 0) {
			int i;
			for (i = 0; i < 8; i++) {
				char *str = logBuffer[((logBufPtr-7+i)&7)];
				if (str[0] != 0) {
					drawText("", i+12);
					str[0] = 0;
				}
			}
			redrawUI();
		}
		logTimer--;
	}
}

void outputLogToScreen() {
	int i;
	for (i = 0; i < 8; i++) {
		char *str = logBuffer[((logBufPtr-7+i)&7)];
		if (str[0] != 0) {
			drawText(str, i+12);
		}
	}
}

void infoOutput(const char *str) {
	logBufPtr++;
	strlcpy(logBuffer[logBufPtr&7], str, 32);
}

void debugOutput(const char *str) {
	if (g_debugSet) {
		infoOutput(str);
	}
	debugOutputToEmulator(str);
}

void exitEmulator() {
	if (enableExit) {
	}
}

void setDarknessGs(int dark) {
	REG_BLDCNT = 0x01f7;			// Darken game screen
	REG_BLDY = dark;				// Darken screen
	REG_BLDALPHA = (0x10-dark)<<8;	// Set blending for OBJ affected BG0
}

void setBrightnessAll(int light) {
	REG_BLDCNT = 0x00bf;			// Brightness increase all
	REG_BLDY = light;
}

void fadeToWhite() {
	int i;
	for (i=7;i>=0;i--) {
		setDarknessGs(i);			// Go from dark to normal
		waitVBlank();
	}
	for (i=0;i<17;i++) {			// Fade to white
		setBrightnessAll(i);		// Go from normal to white
		waitVBlank();
	}
}

void scrollL(int offset, int fade) {
	int i;
	for (i=0;i<9;i++) {
		if (fade) setDarknessGs(8+i);	// Darken screen
		REG_BG3HOFS = i*32 + offset;	// Move screen left
		waitVBlank();
	}
}

void scrollR(int offset) {
	int i;
	for (i=8;i>=0;i--) {
		waitVBlank();
		REG_BG3HOFS = i*32 + offset;	// Move screen right
	}
}

void gbaSleep() {
	fadeToWhite();
	suspend();
	setDarknessGs(7);				// Restore screen
	while ((~REG_KEYINPUT) & 0x3ff) {
		waitVBlank();				// (polling REG_P1 too fast seems to cause problems)
	}
}

//---------------------------------------------

void autoPauseGameSet() {
	emuSettings ^= AUTOPAUSE_EMULATION;
	settingsChanged = true;
	pauseEmulation = (emuSettings & AUTOPAUSE_EMULATION);
	setMuteSoundGUI();
}

void autoStateSet() {
	emuSettings ^= AUTOLOAD_STATE;
	settingsChanged = true;
}

void autoSettingsSet() {
	emuSettings ^= AUTOSAVE_SETTINGS;
	settingsChanged = true;
}

void autoNVRAMSet() {
	emuSettings ^= AUTOLOAD_NVRAM;
	settingsChanged = true;
}

void saveNVRAMSet() {
	emuSettings ^= AUTOSAVE_NVRAM;
	settingsChanged = true;
}

void debugTextSet() {
	g_debugSet ^= 1;
}

void sleepSet() {
	int i = (emuSettings+0x100) & AUTOSLEEP_MASK;
	emuSettings = (emuSettings & ~AUTOSLEEP_MASK) | i;
	if (i == AUTOSLEEP_5MIN) {
		sleepTime = 60*60*5;		// 5 min
	}
	else if (i == AUTOSLEEP_10MIN) {
		sleepTime = 60*60*10;		// 10 min
	}
	else if (i == AUTOSLEEP_30MIN) {
		sleepTime = 60*60*30;		// 30 min
	}
	else if (i == AUTOSLEEP_OFF) {
		sleepTime = 0x7F000000;		// 360 days...
	}
	settingsChanged = true;
}

void autoASet() {
	autoA++;
	joyCfg |= KEY_A+(KEY_A<<16);
	if (autoA == 1) {
		joyCfg &= ~KEY_A;
	}
	else if (autoA == 2) {
		joyCfg &= ~(KEY_A<<16);
	}
	else {
		autoA = 0;
	}
}

void autoBSet() {
	autoB++;
	joyCfg |= KEY_B+(KEY_B<<16);
	if (autoB == 1) {
		joyCfg &= ~KEY_B;
	}
	else if (autoB == 2) {
		joyCfg &= ~(KEY_B<<16);
	}
	else {
		autoB = 0;
	}
}

void speedSet() {
	int i = (emuSettings+0x40) & EMUSPEED_MASK;
	emuSettings = (emuSettings & ~EMUSPEED_MASK) | i;
	setEmuSpeed(i>>5);
}

void flickSet() {
	gFlicker++;
	if (gFlicker > 1) {
		gFlicker = 0;
		gTwitch = 0;
	}
}
