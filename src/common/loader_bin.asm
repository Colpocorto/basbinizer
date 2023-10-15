/******************************************************************************
Copyright 2023 Jose Angel Morente - MSXWiki.org

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the �Software�), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

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


LDRSTART	equ	#F975
LDREXEC		equ LDRSTART

	output	loader_bin.dat
	

	org	LDRSTART
	di
	ld	a,#c9
	ld	(H.KEYI),a
	ld	(H.TIMI),a


;	ld	hl,BASPRG+#10	;#10 is the headroom to hold the LDIR routine
;	ld	de,BASPRG
;	ld	bc,0000			;value to be overwritten externally
;	ldir
	;HL contains the top address

;	di
;	ld	h,d
;	ld	l,e
	ld	hl,0000		;to be modified
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
LDREND:
