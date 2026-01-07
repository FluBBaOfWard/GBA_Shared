#ifdef __arm__

#include "gba_asm.h"

	.global fpsValue
	.global fpsText
	.global fpsTarget

	.global getTime
	.global bin2BCD
	.global getRandomNumber
	.global setupSpriteScaling
	.global calculateFPS
	.global setTargetFPS
	.global convertPalette
	.global debugOutputToEmulator
	.global r0OutputToEmulator
	.global debugOutput_asm
	.global resetFlashCart
	.global suspend
	.global setEWRAMSpeed
	.global bytecopy_
	.global memclr_
	.global memset_
	.global memorr_

	.syntax unified
	.arm

	.section .ewram, "ax", %progbits
	.align 2

;@----------------------------------------------------------------------------
resetFlashCart:
	.type   doReset STT_FUNC
;@----------------------------------------------------------------------------
	mov r1,#REG_BASE
	mov r0,#0
	strh r0,[r1,#REG_DMA0CNT_H]	;@ Stop all DMA
	strh r0,[r1,#REG_DMA1CNT_H]
	strh r0,[r1,#REG_DMA2CNT_H]
	strh r0,[r1,#REG_DMA3CNT_H]
	add r1,r1,#0x200
	str r0,[r1,#8]				;@ Interrupts off

	;@ Copy code to EWRAM and execute it
	adr r0,suspend				;@ Temporary buffer for reset code
	ldr r1,=VISOLY_START		;@ Source address
	ldr r2,=VISOLY_END			;@ End
	sub r2,r2,r1				;@ Subtract to get size
	mov lr,r0
	b memcpy	  				;@ and jump to code too

;@----------------------------------------------------------------------------
suspend:					;@ Called from ui.c and cpu.s
	.type   suspend STT_FUNC
;@----------------------------------------------------------------------------
	mov r3,#REG_BASE

	ldr r1,=REG_P1CNT
	ldr r0,=0xc00c				;@ Interrupt on start+sel
	strh r0,[r3,r1]

	ldrh r1,[r3,#REG_SGCNT_L]
	strh r3,[r3,#REG_SGCNT_L]	;@ Sound off

	ldrh r0,[r3,#REG_DISPCNT]
	orr r0,r0,#0x80
	strh r0,[r3,#REG_DISPCNT]	;@ LCD off

	swi 0x030000

	ldrh r0,[r3,#REG_DISPCNT]
	bic r0,r0,#0x80
	strh r0,[r3,#REG_DISPCNT]	;@ LCD on

	strh r1,[r3,#REG_SGCNT_L]	;@ Sound on

	ldr r1,=REG_P1
susLoop:
	ldrh r0,[r1]
	and r0,r0,#0xc
	eors r0,r0,#0xc
	bne susLoop

	bx lr
;@----------------------------------------------------------------------------
setEWRAMSpeed:				;@ in r0 = 0 normal, != 0 overclocked.
	.type   setEWRAMSpeed STT_FUNC
;@----------------------------------------------------------------------------
	cmp r0,#0
	ldr r2,=REG_WRWAITCTL
	ldr r1,[r2]
	bic r1,r1,#0x0F000000
	orrne r1,r1,#0x0E000000		// 1 waitstate, overclocked
	orreq r1,r1,#0x0D000000		// 2 waitstates, normal
	str r1,[r2]
	bx lr
;@----------------------------------------------------------------------------
getTime:					;@ Out r0 = ??ssMMHH, r1 = ??DDMMYY
	.type   getTime STT_FUNC
;@----------------------------------------------------------------------------
	ldr r3,=0x080000c4			;@ Base address for RTC
	mov r2,#1
	strh r2,[r3,#4]				;@ Enable RTC
	mov r2,#7
	strh r2,[r3,#2]				;@ Enable write

	mov r2,#1
	strh r2,[r3]
	mov r2,#5
	strh r2,[r3]				;@ State=Command

	mov r1,#0x65				;@ r1=Command, YY:MM:DD 00 hh:mm:ss
	mov r12,#8
RTCLoop1:
	mov r2,#2
	and r2,r2,r1,lsr#6
	orr r2,r2,#4
	strh r2,[r3]
	mov r2,r1,lsr#6
	orr r2,r2,#5
	strh r2,[r3]
	mov r1,r1,lsl#1
	subs r12,r12,#1
	bne RTCLoop1

	mov r2,#5
	strh r2,[r3,#2]				;@ Enable read
	mov r1,#0
	mov r12,#32
RTCLoop2:
	mov r2,#4
	strh r2,[r3]
	mov r2,#5
	strh r2,[r3]
	ldrh r2,[r3]
	and r2,r2,#2
	mov r1,r1,lsr#1
	orr r1,r1,r2,lsl#30
	subs r12,r12,#1
	bne RTCLoop2

	mov r0,#0
	mov r12,#24
RTCLoop3:
	mov r2,#4
	strh r2,[r3]
	mov r2,#5
	strh r2,[r3]
	ldrh r2,[r3]
	and r2,r2,#2
	mov r0,r0,lsr#1
	orr r0,r0,r2,lsl#22
	subs r12,r12,#1
	bne RTCLoop3

	bx lr
;@----------------------------------------------------------------------------
bin2BCD:		;@ Transform value to BCD
	.type   bin2BCD STT_FUNC
;@----------------------------------------------------------------------------
	stmfd sp!,{r4}

	mov r1,#0
	adr r3,bcdTable				;@ BCD
	ldr r12,=0x11111110			;@ Carry count mask value

bin2BCDLoop:
	ldr r2,[r3],#4				;@ BCD (read BCD table)
	movs r0,r0,lsr#1			;@ Test bit

	bcc bin2BCDLoopCheck
bin2BCDCalc:
	add r4,r2,r1				;@ r4 = r2 + r1
	eor r2,r1,r2				;@ r2 = r1 XOR r2
	eor r2,r4,r2				;@ r2 = r4 XOR r2
	bic r2,r12,r2				;@ r2 = 0x11111110 AND NOT r2
	orr r2,r2,r2,lsr#1			;@ r2 = r2 OR (r2 LSR 1)
	sub r1,r4,r2,lsr#2			;@ r1 = r4 -  (r2 LSR 2)

bin2BCDLoopCheck:
	bne bin2BCDLoop
	mov r0,r1

	ldmfd sp!,{r4}
	bx lr

bcdTable:
	.long 0x06666667
	.long 0x06666668
	.long 0x0666666a
	.long 0x0666666e
	.long 0x0666667c
	.long 0x06666698
	.long 0x066666ca
	.long 0x0666678e
	.long 0x066668bc
	.long 0x06666b78
	.long 0x0666768a
	.long 0x066686ae
	.long 0x0666a6fc
	.long 0x0666e7f8
	.long 0x0667c9ea
	.long 0x06698dce
	.long 0x066cbb9c
	.long 0x067976d8
	.long 0x068c87aa
	.long 0x06b8a8ee
	.long 0x076aebdc
	.long 0x086fd7b8
	.long 0x0a7fa96a
	.long 0x0e9eec6e
	.long 0x1cddd87c
	.long 0x39bbaa98
	.long 0x6d76eeca

;@----------------------------------------------------------------------------
getRandomNumber:			;@ r0 = max value
	.type   getRandomNumber STT_FUNC
;@----------------------------------------------------------------------------
	ldr r2,rndSeed0
	ldr r3,rndSeed1
	tst r3,r3,lsr#1				;@ Top bit into Carry
	movs r1,r2,rrx				;@ 33 bit rotate right
	adc r3,r3,r3				;@ Carry into lsb of r1
	eor r1,r1,r2,lsl#12			;@ (involved!)
	eor r2,r1,r1,lsr#20			;@ (similarly involved!)

	str r2,rndSeed0
	str r3,rndSeed1
	umull r3,r0,r2,r0
	bx lr
rndSeed0:
	.long 0x39bbaa98
rndSeed1:
	.long 0x00000000

;@----------------------------------------------------------------------------
setupSpriteScaling:
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r6}
	ldmia r0,{r1-r6}			;@ Get sprite scaling params
	add r0,r1,#6

	mov r12,#2
scaleLoop:
	strh r3,[r0],#8				;@ Buffer1, buffer2
	strh r2,[r0],#8
	strh r2,[r0],#8
	strh r5,[r0],#232
		strh r4,[r0],#8
		strh r2,[r0],#8
		strh r2,[r0],#8
		strh r5,[r0],#232
			strh r3,[r0],#8
			strh r2,[r0],#8
			strh r2,[r0],#8
			strh r6,[r0],#232
		strh r4,[r0],#8
		strh r2,[r0],#8
		strh r2,[r0],#8
		strh r6,[r0],#232
	subs r12,r12,#1
	bne scaleLoop

	ldmfd sp!,{r4-r6}
	mov r0,#OAM
	mov r2,#0x400
	b memcpy
;@----------------------------------------------------------------------------
calculateFPS:		;@ Fps output, r0-r3=used.
	.type   calculateFPS STT_FUNC
;@----------------------------------------------------------------------------
	ldrb r0,fpsCheck
	subs r0,r0,#1
	movmi r0,#59
	strb r0,fpsCheck
	bxpl lr						;@ End if not 60 frames has passed

	ldr r0,fpsValue
	mov r1,#0
	str r1,fpsValue

	mov r1,#100
	swi 0x060000				;@ Division r0/r1, r0=result, r1=remainder.
	cmp r0,#0
	addeq r0,r0,#0x20
	addne r0,r0,#0x30
	strb r0,fpsText+4
	mov r0,r1
	mov r1,#10
	swi 0x060000				;@ Division r0/r1, r0=result, r1=remainder.
	add r1,r1,#0x30
	strb r1,fpsText+6
	ldrb r1,fpsText+4
	cmp r0,#0
	cmpeq r1,#0
	addeq r0,r0,#0x20
	addne r0,r0,#0x30
	strb r0,fpsText+5

	bx lr
;@----------------------------------------------------------------------------
setTargetFPS:				;@ Write target FPS, r0=in fps, r0-r3=used.
	.type setTargetFPS STT_FUNC
;@----------------------------------------------------------------------------
	strb r0,fpsTarget
	mov r1,#10
	swi 0x060000				;@ Division r0/r1, r0=result, r1=remainder.
	add r0,r0,#0x30
	strb r0,fpsText+8
	add r1,r1,#0x30
	strb r1,fpsText+9
	bx lr
;@----------------------------------------------------------------------------
fpsValue:	.long 0
fpsText:	.string "FPS:   /60"
fpsCheck:	.byte 0
fpsTarget:	.byte 60
	.align 3

;@----------------------------------------------------------------------------
convertPalette:			;@ r0 = destination, r1 = source, r2 = length. r3 = gamma (0 -> 4), r12 modified.
	.type convertPalette STT_FUNC
;@ Called by main.c:  void convertPalette(u16 *destination, const u8 *source, int length, int gamma);
;@----------------------------------------------------------------------------
	stmfd sp!,{r4-r6,lr}
convLoop:						;@ Map rrrrrrrrggggggggbbbbbbbb  ->  0bbbbbgggggrrrrr
	ldrb r4,[r1],#1				;@ Red
	bl gammaConvert
	mov r6,r4

	ldrb r4,[r1],#1				;@ Green
	bl gammaConvert
	orr r6,r6,r4,lsl#5

	ldrb r4,[r1],#1				;@ Blue
	bl gammaConvert
	orr r6,r6,r4,lsl#10

	strh r6,[r0],#2
	subs r2,r2,#1
	bhi convLoop

	ldmfd sp!,{r4-r6,lr}
	bx lr
;@----------------------------------------------------------------------------
gammaConvert:	;@ Takes value in r4(0-0xFF), gamma in r3(0-4),returns new value in r4=0x1F
;@----------------------------------------------------------------------------
	rsb r5,r4,#0x100
	mul r12,r5,r5
	rsbs r5,r12,#0x10000
	rsb r12,r3,#4
	orr r4,r4,r4,lsl#8
	mul r5,r3,r5
	mla r4,r12,r4,r5
	mov r4,r4,lsr#13

	bx lr
;@----------------------------------------------------------------------------
debugOutputToEmulator:		;@ r0 ptr to string
	.type debugOutputToEmulator STT_FUNC
;@----------------------------------------------------------------------------
	mov r12,#80					;@ Max length.
	adr r2,debugString
debugLoop:
	ldrb r1,[r0],#1
	strb r1,[r2],#1
	cmp r1,#0
	subsne r12,r12,#1
	bne debugLoop
dStringEnd:
	mov r12,r12
	b debugContinue
	.short 0x6464,0x0000
debugString:
	.space 84
debugContinue:
	bx lr
;@----------------------------------------------------------------------------
r0OutputToEmulator:			;@ r0 = value
	.type   r0OutputToEmulator STT_FUNC
;@----------------------------------------------------------------------------
	mov r12,r12
	b r0Continue
	.short 0x6464,0x0000
	.string "r0=%r0%\n"
	.align 2
r0Continue:
	bx lr
;@----------------------------------------------------------------------------
debugOutput_asm:			;@ Input = r1. ptr to str.
;@----------------------------------------------------------------------------
	stmfd sp!,{r0-r3,lr}
	ldr r0,=gDebugSet
	ldrb r0,[r0]
	cmp r0,#0
	ldmfdeq sp!,{r0-r3,pc}
	mov r0,r1
	bl debugOutput
	ldmfd sp!,{r0-r3,pc}

;@----------------------------------------------------------------------------
bytecopy_:					;@ void bytecopy(u8 *dst, u8 *src, int count)
;@----------------------------------------------------------------------------
	subs r2,r2,#1
	ldrbpl r3,[r1,r2]
	strbpl r3,[r0,r2]
	bhi bytecopy_
	bx lr
;@----------------------------------------------------------------------------

	.section .iwram, "ax", %progbits	;@ For the GBA
	.align 2
;@----------------------------------------------------------------------------
memclr_:					;@ r0=dest r1=word count
;@	exit with r0 unchanged, r2=r1, r1=0
;@----------------------------------------------------------------------------
	mov r2,r1
	mov r1,#0
;@----------------------------------------------------------------------------
memset_:					;@ r0=dest r1=data r2=word count
;@	exit with r0 & r1 unchanged, r2=0
;@----------------------------------------------------------------------------
	subs r2,r2,#1
	strpl r1,[r0,r2,lsl#2]
	bhi memset_
	bx lr
;@----------------------------------------------------------------------------
memorr_:					;@ r0=dest r1=data r2=word count
;@	exit with r0 & r1 unchanged, r2=0, r3 trashed
;@----------------------------------------------------------------------------
	subs r2,r2,#1
	ldrpl r3,[r0,r2,lsl#2]
	orrpl r3,r3,r1
	strpl r3,[r0,r2,lsl#2]
	bhi memorr_
	bx lr
;@----------------------------------------------------------------------------
	.end
#endif // #ifdef __arm__
