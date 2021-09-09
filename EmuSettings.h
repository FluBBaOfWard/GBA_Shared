#ifndef EMUSETTINGS_HEADER
#define EMUSETTINGS_HEADER

#ifdef __cplusplus
extern "C" {
#endif

#define AUTOPAUSE_EMULATION	(1<<0)
#define AUTOLOAD_STATE		(1<<1)
#define AUTOLOAD_NVRAM		(1<<2)
#define AUTOSAVE_NVRAM		(1<<3)
#define AUTOSAVE_SETTINGS	(1<<4)
#define EMUSPEED_NORMAL		(0<<5)
#define EMUSPEED_2X			(1<<5)
#define EMUSPEED_MAX		(2<<5)
#define EMUSPEED_HALF		(3<<5)
#define EMUSPEED_MASK		(3<<5)
#define AUTOSLEEP_5MIN		(0<<7)
#define AUTOSLEEP_10MIN		(1<<7)
#define AUTOSLEEP_30MIN		(2<<7)
#define AUTOSLEEP_OFF		(3<<7)
#define AUTOSLEEP_MASK		(3<<7)
#define MAIN_ON_BOTTOM		(1<<8)
#define POWER_SAVE_MENU		(1<<9)

#ifdef __cplusplus
} // extern "C"
#endif

#endif // EMUSETTINGS_HEADER
