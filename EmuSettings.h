#ifndef EMUSETTINGS_HEADER
#define EMUSETTINGS_HEADER

#ifdef __cplusplus
extern "C" {
#endif

// These are settings that are saved in the "emuSettings" variable.
#define AUTOPAUSE_EMULATION	(1<<0)
#define AUTOSAVE_SETTINGS	(1<<1)
#define AUTOLOAD_STATE		(1<<2)
#define SHOW_UI_CLOCK		(1<<3)
#define AUTOLOAD_NVRAM		(1<<4)
#define AUTOSAVE_NVRAM		(1<<5)
#define EMUSPEED_NORMAL		(0<<6)
#define EMUSPEED_2X			(1<<6)
#define EMUSPEED_MAX		(2<<6)
#define EMUSPEED_HALF		(3<<6)
#define EMUSPEED_MASK		(3<<6)
#define AUTOSLEEP_5MIN		(0<<8)
#define AUTOSLEEP_10MIN		(1<<8)
#define AUTOSLEEP_30MIN		(2<<8)
#define AUTOSLEEP_OFF		(3<<8)
#define AUTOSLEEP_MASK		(3<<8)
#define SOUND_ENABLE		(1<<10)

// Misc settings that are not saved.
#define UNSCALED		0	//display types
#define SCALED			1

#ifdef __cplusplus
} // extern "C"
#endif

#endif // EMUSETTINGS_HEADER
