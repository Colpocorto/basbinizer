/******************************************************************************
Copyright 2023 Jose Angel Morente - MSXWiki.org

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

	include common.asm
	

BASPRG	equ	#8000
SCRBUF	equ	#8000
JMPLDR	equ	#F000

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
	org	#0000	;used to calculate block size
	dw	LOADER.LDRSTART
	dw	LOADER.LDREND - 1
	dw	LDREXEC

LDRBLCK:
	module	LOADER
	org	LDRSTART
LDRSTART:
	di
	ld	a,#c9
	ld	(H.KEYI),a
	ld	(H.TIMI),a

	ld	a,1
	ld	hl,FORCLR
	ld	(hl),a
	ld	a,1
	inc	hl
	ld	(hl),a
	inc	hl
	ld	(hl),a
	call	CHGCLR

	;------ this routine to be patched to allow SCREEN loading
	jr	PRG_LOAD
	ld	hl,SCRBUF
	ld	bc,#4000	;to be patched with actual SCR size minus bsave header
	call	LOAD
	ld	a,2		;screen mode, 2 by default, patch the actual one
	call	CHGMOD
	call	DISSCR
	ld	hl,SCRBUF
	ld	de,#0000	;start of VRAM
	ld	bc,#4000	;to be patched with actual SCR size minus bsave header
	call	LDIRVM
	call	ENASCR

PRG_LOAD:
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

	xor	a
	ld	(BASPRG),a

	jp	BASRUN

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

	org	LOADER.LDREND-LOADER.LDRSTART + LDRBLCK
	ds	8- $ % 8,0	
	;-----------------TAPE BLOCKS-----------------------
	db	CASHDR
DAT_80:	
	//incbin "#8000.zx0"
DAT_80E:

