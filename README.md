BASBINIZER v1.9.2
=================

BASBINIZER is a suite of tools for native MSX-BASIC file manipulation. It is multiplatform and several builds have been provided (Windows, Linux, Mac and MSX).
Its main features are:

* Read a tokenized .BAS file and print the contents to the standard output (screen) or a text file.
* Convert the .BAS file to a binary .CAS file that can be loaded from tape with BLOAD"CAS:",R.
* Fix some inconsistent .BAS files by rebuilding the internal link pointers.
* Create a .ROM file suitable for physical cartridges.
* Save a .BAS file directly to tape using your MSX computer (necessary for bigger .BAS programs that are incompatible with the presence of a disk drive).
* Add a loading screen using any SCREEN mode


Using BASBINIZER on your PC/Mac/Linux computer
==============================================

Syntax:

basbinizer <inputfile> [-i <BIN filename>] [-b <CAS filename> [-c <BLOAD "NAME">] [-s <SCR loadfile>] [-vm <screen mode>]] [-a <ASC filename>] [-r <ROM filename>] [--fix] [--quiet]

Where
    <inputfile> is the path to an MSX-BASIC .BAS file in tokenized format

    <BIN filename> is the .BIN file (loadable from disk)

    <CAS filename> is the resulting .CAS file.

    <BLOAD "NAME"> is the name of the "FOUND" file (max. 6 characters)

    <ROM filename> is the name of the ROM file

    <ASC filename> to generate an ASCII file from the tokenized BASIC. If not specified, the ASCII text is written to the standard output.

    <SCR filename> is a loading screen (in BSAVE format) or a VRAM dump that would be loaded prior to the main BASIC program.

    <screen mode> is the SCREEN mode in MSX-BASIC to be set before loading the screen. Default: 2


Options:

--fix       Fixes certain data errors found in the source .BAS file

--quiet     suppress messages on screen (except for critical errors)

--addr      prints memory address for each line (assumed base address #8000)


The maximum program size for ROM file conversion is 16368 bytes and the variable area must start beyond address #C000. The program will fail if it sets the variable area to any address under #C000 (e.g. by using a CLEAR statement).

Example:

    basbinizer GAME.BAS

    basbinizer STARS.BAS -b STARS.CAS -c STARS
    
    basbinizer NIBBLES.BAS -a NIBBLES.ASC -b NIBBLES.CAS -c NIBBLE --fix

    basbinizer NIBBLES.BAS -b NIBBLES.CAS -c NIBBLE -r NIBBLES.ROM --quiet

    basbinizer NIBBLES.BAS -i NIBBLES.BIN

    basbinizer NIBBLES.BAS -b NIBBLES.CAS -c NIBBLE -s GFX.SC2
   

Using BASBINIZER on your MSX computer
=====================================

Follow these easy steps:

* On your MSX, load the program you want to convert with LOAD"CAS: or LOAD"filename.bas  (DON'T run the program or add the ,R modifier).
* Load and run BASBINIZER for the MSX computer. Two flavours of the tool are provided. Use basbinizer.cas and type BLOAD"CAS:",R with your favourite CAS loading device or MSX emulator if you are using a diskless MSX computer. If you want to use a disk drive, copy the file BASBIN.BIN to the disk and load it typing BLOAD"BASBIN.BIN",R.
* Insert a blank tape (or a new .WAV file if you're using an emulator) and set it for recording. 
* When prompted, type the name for the BLOAD loading (the "found" name). It shouldn't be longer than 6 characters.
* Press a key and the recording will start. Wait until the program returns to BASIC.


Using BASBINIZER with ASCII BASIC files
=======================================

At this moment, Basbinizer does not support .ASC files as the input (only as the target file). However, as it was [suggested](https://github.com/Colpocorto/basbinizer/issues/1#issuecomment-1779204571) by user [Francesco Maida](https://github.com/fmaida), it can be used in cojunction with [basic-dignified](https://github.com/farique1/basic-dignified) in the same tool chain.


Building BASBINIZER
===================

Basbinizier has been written in plain C language and it is compatible with any C compiler that supports the ANSI-C (C89) standard. To build the program just compile the main program with the chosen C compiler:

Example:

    gcc basbinizer.c -o basbinizer

Basbinizer for the MSX is a pure Z80 assembly program provided in two versions. To build them, use SjASM assembler (SjASMPlus or other forks are NOT supported). Visit [XL2S Site](https://www.xl2s.tk) for more information on SjASM.


Contact information
===================

You can contact the author on any of his public profiles:

https://mastropiero.itch.io/

http://blog.msxwiki.org/

https://www.youtube.com/@msxwiki/


Disclaimer and License
======================

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
