    include "../common/common.asm"


START   equ #f931
BASINI  equ #8000

define	CASHDR	#1F,#A6,#DE,#BA,#CC,#13,#7D,#74


    output "basbinizer.cas"

    db  CASHDR
    ds  10,#d0
    db  "BASBIN"
    db  CASHDR
    dw  STARTADDR
    dw  ENDADDR-1
    dw  EXEC

    org START
    
    include "basbinizer.asm"