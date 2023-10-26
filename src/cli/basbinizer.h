/******************************************************************************
Copyright (c) 2023 Jose Angel Morente - MSXWiki.org

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(__unix__) || defined(__APPLE__) || defined(__linux__) || defined(__gnu_linux__)
typedef struct stat st_stat;
#else
#define stat(x, y) _stat(x, y)
typedef long long int off_t;
typedef struct _stat st_stat;
#endif

#define VERSION "1.9"
#define BIN_LOADER_SIZE 50
#define CAS_LOADER_SIZE 208
#define BIN_PATCH_POS   10
#define CAS_PATCH_POS   0x64
#define SCR_PATCH_POS1  0x44
#define SCR_PATCH_POS2  0x58
#define SCR_PATCH_POS3  0x3f
#define VM_PATCH_POS    0x4a
#define PATCH_NAME_POS  0x12
#define CAS_NAME_LEN    6
#define ERR_MSG_SZ      200
#define ROM_SIZE        16384
#define MAX_ROM_SIZE    16384 - 0x10
#define BASE_BIN        0x8000
#define BASE_ROM        0x8010
#define REL_PROG        0xf975

typedef uint8_t byte;

byte ROM_header[16] = {
    0x41, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x80, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00
};
byte CAS_header[8] = {
    0x1F, 0xA6, 0xDE, 0xBA, 0xCC, 0x13, 0x7D, 0x74
};
byte BIN_header[7] = {
    0xfe, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00
};
byte bin_loader_data[BIN_LOADER_SIZE] = {
	0xF3, 0x3E, 0xC9, 0x32, 0x9A, 0xFD, 0x32, 0x9F, 0xFD, 0x21, 0x00, 0x00,
	0x22, 0xC2, 0xF6, 0x21, 0x01, 0x80, 0x22, 0x76, 0xF6, 0x2B, 0x22, 0x48,
	0xFC, 0x3E, 0x0F, 0x21, 0xE9, 0xF3, 0x77, 0x3E, 0x01, 0x23, 0x77, 0x23,
	0x77, 0xCD, 0x62, 0x00, 0xCD, 0x6C, 0x00, 0xAF, 0x32, 0x00, 0x80, 0xC3,
	0xAC, 0x73

};

byte cas_loader_data[CAS_LOADER_SIZE] = {
	0x1F, 0xA6, 0xDE, 0xBA, 0xCC, 0x13, 0x7D, 0x74,   0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0,     //#00
	0xD0, 0xD0, 0x47, 0x41, 0x4D, 0x45, 0x20, 0x20,   0x1F, 0xA6, 0xDE, 0xBA, 0xCC, 0x13, 0x7D, 0x74,     //#10
	0x75, 0xF9, 0x10, 0xFA, 0x75, 0xF9, 0xF3, 0x3E,   0xC9, 0x32, 0x9A, 0xFD, 0x32, 0x9F, 0xFD, 0x3E,     //#20
	0x01, 0x21, 0xE9, 0xF3, 0x77, 0x3E, 0x01, 0x23,   0x77, 0x23, 0x77, 0xCD, 0x62, 0x00, 0x18, 0x20,     //#30
	0x21, 0x00, 0x80, 0x01, 0x00, 0x40, 0xCD, 0xCD,   0xF9, 0x3E, 0x02, 0xCD, 0x5F, 0x00, 0xCD, 0x41,     //#40
	0x00, 0x21, 0x00, 0x80, 0x11, 0x00, 0x00, 0x01,   0x00, 0x40, 0xCD, 0x5C, 0x00, 0xCD, 0x44, 0x00,     //#50
	0x21, 0x00, 0x80, 0x01, 0x00, 0x00, 0xCD, 0xCD,   0xF9, 0xF3, 0x22, 0xC2, 0xF6, 0x21, 0x01, 0x80,     //#60
	0x22, 0x76, 0xF6, 0x2B, 0x22, 0x48, 0xFC, 0xAF,   0x32, 0x00, 0x80, 0xC3, 0xAC, 0x73, 0xF3, 0xC5, 
	0xE5, 0x3A, 0x07, 0x00, 0x3C, 0x32, 0x0B, 0xFA,   0x32, 0x0F, 0xFA, 0xCD, 0xE1, 0x00, 0x38, 0xFB, 
	0xCD, 0xE4, 0x00, 0x38, 0xFB, 0xE1, 0x77, 0x23,   0x3A, 0x08, 0xFA, 0xEE, 0x06, 0x32, 0x08, 0xFA, 
	0xCD, 0x09, 0xFA, 0xC1, 0x0B, 0x79, 0xB0, 0xC5,   0xE5, 0x20, 0xE5, 0xE1, 0xC1, 0xCD, 0xE7, 0x00, 
	0xAF, 0xCD, 0xF3, 0x00, 0x3E, 0x01, 0xC3, 0x09,   0xFA, 0x06, 0xF3, 0xD3, 0x99, 0x3E, 0x87, 0xD3, 
	0x99, 0xC9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   0x1F, 0xA6, 0xDE, 0xBA, 0xCC, 0x13, 0x7D, 0x74
};


/*
    Type definitions
*/

typedef struct options
{
    bool fix;
    bool verbose;
    bool quiet;
    char *infile;
    off_t infile_s;
    char *scrfile;
    off_t scrfile_s;
    char *casfile;
    char *binfile;
    char *ascfile;
    char *romfile;
    byte scr_mode;
    byte casname[CAS_NAME_LEN];
    char valerror[ERR_MSG_SZ];
    FILE *stdoutf;

} options_t;

/*
    Global variables
*/

options_t options;

/*
    Prototypes
*/
uint16_t get_word_value(byte *pointer);
int get_octal(byte *buffer, int pos, FILE *output);
//int get_file_size(FILE *fi);
byte *get_input_file(char *infile, off_t infile_s);
int get_float(byte *buffer, int pos, FILE *output, bool is_double);
int8_t read_mantissa(byte *buffer, char *mantissa, int8_t length);
void print_mantissa(char *mantissa, int8_t length, int8_t dot_pos, FILE *output);
int decodeLine(byte *buffer, int pos, off_t size, uint16_t base_addr, FILE *output);
void decodeBAS(byte *buffer, off_t size, uint16_t base_addr, FILE *output);
int get_hex(byte *buffer, int pos, FILE *output);
int get_line_number(byte *buffer, int pos, FILE *output);
int get_single(byte *buffer, int pos, FILE *output);
int get_double(byte *buffer, int pos, FILE *output);
int get_quoted_string(byte *buffer, int pos, FILE *output);
int get_bin(byte *buffer, int pos, FILE *output);
int get_terminal_string(byte *buffer, int pos, FILE *output);
int get_colon(byte *buffer, int pos, FILE *output);
int get_string(byte *buffer, int pos, FILE *output);
bool process_bas(byte *buffer, off_t file_size, uint16_t base_addr, bool force_quiet);
bool write_bin(byte *buffer, off_t buf_size, char *binf);
bool write_cas(byte *buffer, off_t buf_size, byte *scr_buf, off_t scr_buf_size, char *casf);
bool write_rom(byte *inbuf, off_t buf_size, char *romfile);
bool process_opt(int argc, char **argv, options_t *opt);

void usage(void);
void title(void);
