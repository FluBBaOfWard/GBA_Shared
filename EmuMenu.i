
;@----------------------------------------------------------------------------
	.equ KEY_A,         (1<<0)  ;@ Keypad A button.
	.equ KEY_B,         (1<<1)  ;@ Keypad B button.
	.equ KEY_SELECT,    (1<<2)  ;@ Keypad SELECT button.
	.equ KEY_START,     (1<<3)  ;@ Keypad START button.
	.equ KEY_RIGHT,     (1<<4)  ;@ Keypad RIGHT button.
	.equ KEY_LEFT,      (1<<5)  ;@ Keypad LEFT button.
	.equ KEY_UP,        (1<<6)  ;@ Keypad UP button.
	.equ KEY_DOWN,      (1<<7)  ;@ Keypad DOWN button.
	.equ KEY_R,         (1<<8)  ;@ Right shoulder button.
	.equ KEY_L,         (1<<9)  ;@ Left shoulder button.

	.equ ACT_COIN,      1<<23   ;@ Action to insert coin
	.equ ACT_POWER,     1<<24   ;@ Action to press power button
	.equ ACT_SCRL_UP,   1<<25   ;@ Action to scroll unscaled screen up
	.equ ACT_SCRL_DOWN, 1<<26   ;@ Action to scroll unscaled screen down
	.equ ACT_FAST_FORW, 1<<27   ;@ Action to fast forward
	.equ ACT_LOAD_STAT, 1<<28   ;@ Action to load state
	.equ ACT_SAVE_STAT, 1<<29   ;@ Action to save state
	.equ ACT_OPEN_MENU, 1<<30   ;@ Action to open menu
;@----------------------------------------------------------------------------
