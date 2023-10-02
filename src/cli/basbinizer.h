#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

#if defined(__unix__) || defined(__APPLE__) || defined(__linux__) || defined(__gnu_linux__)
#define path_separator "/"
typedef struct dirent st_dirent;
typedef struct stat st_stat;
#else
#define DIR _DIR
#define memccpy(x) _memccpy(x)
#define opendir(x) _opendir(x)
#define path_separator "\\"
#define readdir(x) _readdir(x)
#define regfree(x) _regfree(x)
#define strdup(x) _strdup(x)
#define stat(x, y) _stat(x, y)
#define strcasecmp(x, y) _strcasecmp(x, y)
#define utimbuf _utimbuf
#define utime(x, y) _utime(x, y)
#define REG_ICASE _REG_ICASE
#define REG_NOSUB _REG_NOSUB
typedef long long int off_t;
typedef struct _dirent st_dirent;
typedef struct _stat st_stat;
#endif

#define LOADER_SIZE 174
#define PATCH_POS 51
#define PATCH_NAME_POS 0x12
#define CAS_NAME_LEN 6
#define ERR_MSG_SZ 200

typedef uint8_t byte;

byte loader_data[LOADER_SIZE] = {
    0x1F, 0xA6, 0xDE, 0xBA, 0xCC, 0x13, 0x7D, 0x74, 0xD0, 0xD0, 0xD0, 0xD0,
    0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0xD0, 0x47, 0x41, 0x4D, 0x45, 0x20, 0x20,
    0x1F, 0xA6, 0xDE, 0xBA, 0xCC, 0x13, 0x7D, 0x74, 0x75, 0xF9, 0xF4, 0xF9,
    0x75, 0xF9, 0xF3, 0x3E, 0xC9, 0x32, 0x9A, 0xFD, 0x32, 0x9F, 0xFD, 0x21,
    0x00, 0x80, 0x01, 0x00, 0x00, 0xCD, 0xB1, 0xF9, 0xF3, 0x22, 0xC2, 0xF6,
    0x21, 0x01, 0x80, 0x22, 0x76, 0xF6, 0x2B, 0x22, 0x48, 0xFC, 0x3E, 0x0F,
    0x21, 0xE9, 0xF3, 0x77, 0x3E, 0x01, 0x23, 0x77, 0x23, 0x77, 0xCD, 0x62,
    0x00, 0xCD, 0x6C, 0x00, 0xAF, 0x32, 0x00, 0x80, 0xC3, 0xAC, 0x73, 0xC3,
    0x00, 0x40, 0xF3, 0xC5, 0xE5, 0x3A, 0x07, 0x00, 0x3C, 0x32, 0xEF, 0xF9,
    0x32, 0xF3, 0xF9, 0xCD, 0xE1, 0x00, 0x38, 0xFB, 0xCD, 0xE4, 0x00, 0x38,
    0xFB, 0xE1, 0x77, 0x23, 0x3A, 0xEC, 0xF9, 0xEE, 0x06, 0x32, 0xEC, 0xF9,
    0xCD, 0xED, 0xF9, 0xC1, 0x0B, 0x79, 0xB0, 0xC5, 0xE5, 0x20, 0xE5, 0xE1,
    0xC1, 0xCD, 0xE7, 0x00, 0xAF, 0xCD, 0xF3, 0x00, 0x3E, 0x01, 0xC3, 0xED,
    0xF9, 0x06, 0xF3, 0xD3, 0x99, 0x3E, 0x87, 0xD3, 0x99, 0xC9, 0x1F, 0xA6,
    0xDE, 0xBA, 0xCC, 0x13, 0x7D, 0x74};

// Type definitions

typedef struct options
{
    bool fix;
    bool verbose;
    bool quiet;
    char *infile;
    off_t infile_s;
    char *outfile;
    char *ascfile;
    byte casname[CAS_NAME_LEN];
    char valerror[ERR_MSG_SZ];
    FILE *stdoutf;

} options_t;

// Global variables

options_t options;

// Prototypes

uint16_t get_word_value(byte *pointer);
int get_octal(byte *buffer, int pos, FILE *output);
int get_file_size(FILE *fi);
int get_float(byte *buffer, int pos, FILE *output, bool is_double);
int8_t read_mantissa(byte *buffer, char *mantissa, int8_t length);
void print_mantissa(char *mantissa, int8_t length, int8_t dot_pos, FILE *output);
int decodeLine(byte *buffer, int pos, off_t size, FILE *asciifile);
void decodeBAS(byte *buffer, off_t size, FILE *output);
int get_hex(byte *buffer, int pos, FILE *output);
int get_line_number(byte *buffer, int pos, FILE *output);
int get_single(byte *buffer, int pos, FILE *output);
int get_double(byte *buffer, int pos, FILE *output);
int get_quoted_string(byte *buffer, int pos, FILE *output);
int get_bin(byte *buffer, int pos, FILE *output);
int get_terminal_string(byte *buffer, int pos, FILE *output);
int get_colon(byte *buffer, int pos, FILE *output);
int get_string(byte *buffer, int pos, FILE *output);
bool process_bas(byte *buffer, off_t file_size);
bool write_bin(byte *buffer, off_t buf_size, char *binf);
bool process_opt(int argc, char **argv, options_t *opt);

void usage(void);
void title(void);