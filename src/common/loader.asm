	include common.asm
	

BASPRG	equ #8000
buffer	equ	#8000

JMPLDR	equ	#F000

LAUNCH	equ	#4000

CRTSLT	equ	#f975
PG0SLT	equ CRTSLT+1
PG1SLT	equ PG0SLT+1
PG2SLT	equ	PG1SLT+1
DZX0	equ	PG2SLT+1

define	CASHDR	#1F,#A6,#DE,#BA,#CC,#13,#7D,#74

LDRSTART	equ	#F975
LDREXEC		equ LDRSTART

	output	loader.dat
	
	db	CASHDR
	ds	10,#D0	;binary file signature
	db	"GAME  "	;cas file name
	db	CASHDR
	dw	LOADER.LDRSTART
	dw	LOADER.LDREND - 1
	dw	LDREXEC

	module	LOADER
	org	LDRSTART
LDRSTART:
	di
	ld	a,#c9
	ld	(H.KEYI),a
	ld	(H.TIMI),a


	ld	hl,BASPRG
	ld	bc,DAT_80E-DAT_80
	call	LOAD
	;HL contains the top address

	di
	ld	(VARTAB),hl
	ld	hl,BASPRG+1
	ld	(TXTTAB),hl
	dec	hl
	ld	(BOTTOM),hl

	ld	a,15
	ld	hl,FORCLR
	ld	(hl),a
	ld	a,1
	inc	hl
	ld	(hl),a
	inc	hl
	ld	(hl),a
	call	CHGCLR

	call	INITXT
	xor	a
	ld	(BASPRG),a

	jp	BASRUN

/*
    ld (VARTAB),hl	;#f6c2	#8003 -	#8713	= #710 (BAS size - 3)
    ld  hl,BASPRG
    ld  (TXTTAB),hl	;#f676	#8001	#8001
	dec	hl
	ld	(BOTTOM),hl	;#fc48	#8000	#8000
	#8000 <- #00
*/


	jp	LAUNCH

LOAD:
	di
	push	bc	;counter
	push	hl	;destination
	;patch border routine
	ld	a,(#0007)
	inc	a
	ld	(changeBorderPatch01+1),a
	ld	(changeBorderPatch02+1),a

.loadHdr	
	call	TAPION
	jr	c,.loadHdr
.loop:	
	call	TAPIN
	jr	c,.loop
	pop	hl
	ld	(hl),a
	inc	hl
	ld	a,(COL_LOAD)
	xor	%00000110
	ld	(COL_LOAD),A
	call	changeBorder
	pop	bc
	dec	bc
	ld	a,c
	or	b
	push	bc
	push	hl
	jr	nz,.loop
	pop	hl
	pop	bc
	call	TAPIOF
	xor	a
	call	STMOTR
	ld	a,1
	jp	changeBorder
	;ret
COL_LOAD:
	db	%00000110

changeBorder:
	di
changeBorderPatch01:
	out	(#99),a

	ld	a,87h	;register 7 + 80h
changeBorderPatch02:	
	out	(#99),a
	ret
		
LDREND:

	endmodule

	;-----------------TAPE BLOCKS-----------------------
	db	CASHDR
DAT_80:	
	//incbin "#8000.zx0"
DAT_80E:

