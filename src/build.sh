#!/bin/bash
echo SPELLBOUND MSX build - $(date)
echo SPELLBOUND MSX build - $(date) > version.txt
sjasm -s spellbound-msx.asm 
if [[ "$?" -eq 0 ]]
then
    echo [42mSpellbound program assembled successfully.. [40m
else
    echo [41mERROR: asm file could not be compiled successfully. [40m
    exit
fi

dd skip=0 bs=1 if=spellbound-msx.out of=\#4000 count=16384
dd skip=16384 bs=1 if=spellbound-msx.out of=\#8000 count=26880
rem	26880-16384 = 10496
dd skip=32768 bs=1 if=spellbound-msx.out of=\#c000 count=10496

zx0 -f \#4000
zx0 -f \#8000
zx0 -f scr.sc2
sjasm -s -j spellbound.asm
if [[ "$?" -eq 0 ]]
then
    echo [42mROM file assembled successfully.. [40m
    cp spellbnd.rom /media/src
else
    echo [41mERROR: ROM file could not be compiled successfully. [40m
    
fi

sjasm -s -j spellbound-cas.asm
if [[ "$?" -eq 0 ]]
then
    echo [42mCAS file assembled successfully.. [40m
    mcp spellbnd.cas -e spellbnd.wav
else
    echo [41mERROR: CAS file could not be compiled successfully. [40m
    
fi

sjasm -s -j spellbound-dsk.asm
if [[ "$?" -eq 0 ]]
then
    echo [42mDSK file assembled successfully.. [40m
else
    echo [41mERROR: DSK file could not be compiled successfully. [40m
fi



rm -f #4000
rm -f #8000
