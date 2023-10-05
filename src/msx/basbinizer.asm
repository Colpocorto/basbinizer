
EXEC:
STARTADDR:
    di
	ld	a,#c9
	ld	(H.KEYI),a
	ld	(H.TIMI),a

    ;patch loader with size of file
    ld  hl,(VARTAB)
    ld  bc,BASINI
    sbc hl,bc       ;calculate BASIC program size
    push    hl
    
    ld  (LDR_START + #13),hl

    ld  hl,txt_name
    call    print

    ld  hl,INPSTR
    push    hl
    call    input
    pop hl

    ld  de,HDR_START+10
    ld  bc,6
    ldir

    ld  hl,txt_press
    call    print
    call    CHGET
    ld  a,1
    call    STMOTR  ;motor on
    ld  a,1         
    call    TAPOON  ;writes long header
    ld  hl,HDR_START
    ld  bc,HDR_END-HDR_START
    call    WRT_BLK ;writes header
    call    TAPOOF
    xor a
    call    TAPOON  ;writes short header
    ld  hl,LDR_START
    ld  bc,LDR_END-LDR_START
    call    WRT_BLK ;writes loader
    call    TAPOOF
    xor a
    call    TAPOON  ;writes short header
    ld  hl,BASINI
    pop bc  ;restore size of basic program
    call    WRT_BLK
    call    TAPOOF
    xor a
    jp   STMOTR
input:
    call    CHGET
    call    CHPUT
    cp  13
    ret z   ;finished
    cp  8   ;backspace
    jr  z,bs
    ld  (hl),a
    inc hl
    jr  input
bs:
    ld  (hl),' '
    dec hl
    jr  input

WRT_BLK:
    ld  a,(hl)
    push    hl
    push    bc
    call    TAPOUT
    pop bc  
    pop hl
    inc hl
    dec bc
    ld  a,b
    or  c
    jr  nz,WRT_BLK
    ret

print:
    ld  a,(hl)
    or  a
    ret z
    call    CHPUT
    inc hl
    jr  print
txt_press:
    db  "Press REC & PLAY on your cassette recorder. Press a key when ready.", 0
txt_name:
    db  12,"Name? (6 char max.)",13,10,0
LDR_ADDR
HDR_START:
    ds  10,#d0
    db  "PROG  "
HDR_END:
LDR_START:
    incbin "loader.dat"
LDR_END:
INPSTR: 
        ds  6,' '
ENDADDR: