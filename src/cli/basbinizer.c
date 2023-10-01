#include "basbinizer.h"

void title(void)
{
	fprintf(options.stdoutf, "_/_/_/_/_/_/_/_/  Basbinizer v0.1 _/_/_/_/_/_/_/_/\n");
	fprintf(options.stdoutf, "_/_/_/_/_/_/_/_/ 2023 MSXWiki.org _/_/_/_/_/_/_/_/\n\n\n");
}
void usage(void)
{
	fprintf(options.stdoutf, "Syntax:\n\n");
	fprintf(options.stdoutf, "basbinizer <inputfile> [-b <outputfile> [-c <CAS FILENAME>]] [-a <ASC filename>] [--fix] [--quiet]\n\n");
	fprintf(options.stdoutf, "Where\n\t<intputfile> is the path to a MSX-BASIC .BAS file in tokenized format\n\t<outputfile> is the resulting .CAS file.\n\t<CAS FILENAME> if the name of the BLOAD name (max. 6 characters)\n\t<ASC filename> to generate an ASCII file from the tokenized BASIC. If not specified, the ASCII text is written on the standard output.\n");
	fprintf(options.stdoutf, "Example:\tbasbinizer STARS.BAS STARS.CAS STARS\n\n\n");
}
bool process_params(int argc, char **argv, options_t *opt)
{
	/*
		Set default parameters
	*/
	opt->infile = NULL;
	opt->infile_s = 0;
	opt->outfile = NULL;
	opt->ascfile = NULL;
	memset(opt->casname, ' ', CAS_NAME_LEN);
	memset(opt->valerror, 0, ERR_MSG_SZ);
	opt->fix = false;
	opt->quiet = false;
	opt->stdoutf = stdout;

	/*
		Exit if no valid parameters are entered
	*/
	if (argc < 2)
	{
		title();
		usage();
		strcpy(opt->valerror, "No input file.\n");
		return (false);
	}

	/*
		Get the first argument (must be the input file) and validate it
	*/

	st_stat st_infile;

	if (!stat(argv[1], &st_infile))
	{
		// if we are here, the file exists. Good news!
		opt->infile = argv[1];
		opt->infile_s = st_infile.st_size;
	}
	else
	{
		strcpy(opt->valerror, "Invalid input file.\n");
		return (false);
	}

	/*
		Parse optional parameters
		The next parameter, if exists, is number 2
	*/
	int i = 2;
	while (i < argc)
	{
		if ((!strcmp(argv[i], "-b")) && (i < (argc - 1)))
		{
			st_stat st_outfile;
			if (!stat(argv[++i], &st_outfile))
			{
				opt->outfile = argv[i];
			}
			else
			{
				strcpy(opt->valerror, "Invalid output file.\n");
				return (false);
			}
		}
		else
		{
			if ((!strcmp(argv[i], "-c")) && (i < (argc - 1)))
			{
				int name_len = strlen(argv[++i]);
				memcpy(opt->casname, argv[i], name_len > CAS_NAME_LEN ? CAS_NAME_LEN : name_len);
			}
			else
			{
				if (!strcmp(argv[i], "--fix"))
				{
					opt->fix = true;
				}
				else
				{
					if (!strcmp(argv[i], "--quiet"))
					{
						opt->quiet = true;
#ifdef _WIN32
						opt->stdoutf = fopen("NUL", "wb");
#else
						opt->stdoutf = fopen("/dev/null", "wb");
#endif
					}
					else
					{
						if ((!strcmp(argv[i], "-a")) && (i < (argc - 1)))
						{
							st_stat st_ascfile;
							if (!stat(argv[++i], &st_ascfile))
							{
								opt->ascfile = argv[i];
							}
							else
							{
								strcpy(opt->valerror, "Invalid ASC output file.\n");
								return (false);
							}
						}
					}
				}
			}
		}
		i++;
	}
	return (true);
}
byte *get_input_file(char *infile)
{

	static byte *buffer = NULL;

	if (buffer = malloc(options.infile_s))
	{
		FILE *f = NULL;
		fprintf(options.stdoutf, "Opening input file %s...", options.infile);

		if (!(f = fopen(options.infile, "rb")))
		{
			fprintf(options.stdoutf, "FAIL\n");
			fprintf(stderr, "Error opening input file %s\n", options.infile, f);
			return NULL;
		}
		fprintf(options.stdoutf, "OK\n");
		fprintf(options.stdoutf, "Reading input file %s...", options.infile);
		if (fread(buffer, 1, options.infile_s, f) != options.infile_s)
		{
			fprintf(options.stdoutf, "FAIL\n");
			fprintf(stderr, "Unexpected end of file reading input file %s\n", options.infile);
			return NULL;
		}
		fprintf(options.stdoutf, "OK\n");

		return (buffer);
	}
	else
	{
		fprintf(stderr, "Memory error while opening input file %s\n", options.infile);
		return (NULL);
	}
}

int main(int argc, char **argv)
{

	int exitcode = 0;

	if (!process_params(argc, argv, &options))
	{

		fprintf(stderr, options.valerror);
		return (exitcode);
	}

	title();

	/*
		Load the input binary file into the buffer
	*/

	byte *inbuf = get_input_file(options.infile);

	if (!inbuf)
	{
		return (1);
	}

	/*
		Process the BAS file and:
		-Convert to ASCII (stdout or file)
		-Fix links if required
	*/
	if (!process_bas(inbuf, options.infile_s)) {
		return (1);
	}
	

	return (0); // TEST

	if (options.outfile)
	{
		FILE *fo = NULL;
	}

	// Write file size into binary loader data
	loader_data[PATCH_POS] = (unsigned char)(options.infile_s & 0xff);
	loader_data[PATCH_POS + 1] = (unsigned char)((options.infile_s & 0xff00) >> 8);

	int name_len = strlen(argv[3]);

	memset(loader_data + PATCH_NAME_POS, ' ', CAS_NAME_LEN);
	memcpy(loader_data + PATCH_NAME_POS, argv[3], name_len > CAS_NAME_LEN ? CAS_NAME_LEN : name_len);

	// Get memory for the buffer
	unsigned char *buffer = malloc(LOADER_SIZE + options.infile_s);

	if (!buffer)
	{
		fprintf(stdout, "Memory error trying to find %d bytes free... exiting.\n\n", LOADER_SIZE + options.infile_s);
		exitcode = 1;

		return (exitcode);
	}

	// copy data and concat .BAS file
	memcpy(buffer, loader_data, LOADER_SIZE);
	// int read_data = fread(buffer + LOADER_SIZE, 1, options.infile_s, options);
	/*if (read_data < (options.infile_s))
	{
		fprintf(stdout, "Memory error while reading input file... Exiting\n\n");
		exitcode = 1;

		return (exitcode);
	}*/

	// try to open outputs file
	//	fo_path = argv[2];

	//	fprintf(stdout, "Opening file %s...\t", fo_path);
	//	fo = fopen(fo_path, "wb");
	/*
		if (!fo)
		{
			fprintf(stdout, "FAIL\n\n");
			fprintf(stdout, "Could not open input file. Exiting...\n\n");
			exitcode = 2;

			return (exitcode);
		}
		fprintf(stdout, "OK\n");

		// write data to output file
		int written = fwrite(buffer, 1, file_size + LOADER_SIZE, fo);
		if (written < file_size + LOADER_SIZE)
		{
			fprintf(stdout, "Memory error while writing output file... Exiting\n\n");
			exitcode = 1;

			return (exitcode);
		}

		// close input file
		fprintf(stdout, "Closing file %s...\t", fi_path);
		fclose(fi);
		fprintf(stdout, "OK\n");

		// close output file
		fprintf(stdout, "Closing file %s...\t", fo_path);
		fclose(fo);
		fprintf(stdout, "OK\n");
		*/

	// free resources
	free(buffer);

	return (exitcode);
}

bool process_bas(byte *buffer, int file_size)
{
	/*
		Create the ASCII file or assign it to stdout
	*/
	FILE *asciifile = stdout;

	if (options.ascfile)
	{
		if (!(asciifile = fopen(options.ascfile, "w")))
		{
			fprintf(stderr, "Error creating output ASCII file.\n");
			return(false);
		}
	}

	decodeBAS(buffer, file_size, asciifile);

	return (true);
}

void check_links(char *buf)
{
	bool broken = false;
}

void decodeBAS(byte *buffer, int size, FILE *output)
{

	int link_pointer;

	int pos = 1; // skip 0xff header

	while ((link_pointer = get_word_value(buffer + pos)) != 0)
	{
		// decode line
		pos = decodeLine(buffer, pos, size, output);
		fprintf(output, "\n");
	}
}

int decodeLine(byte *buffer, int pos, int size, FILE *asciifile)
{
	byte token;

	// skip link pointer
	pos += 2;

	fprintf(asciifile, "%d ", get_word_value(buffer + pos));

	pos += 2;

	while ((token = *(buffer + pos)) != 0)
	{
		// process token
		switch (token)
		{
		case 0x0b: // get octal value
			pos = get_octal(buffer, pos, asciifile);
			break;
		case 0x0c: // get hex value
			pos = get_hex(buffer, pos, asciifile);
			break;
		case 0x0e: // get line number
			pos = get_line_number(buffer, pos, asciifile);
			break;
		case 0x0f: // numbers from 10 to 255
			pos++;
			fprintf(asciifile, "%d", *(buffer + pos));
			break;
		case 0x11: // numbers from 0 to 9 are encoded without token (-0x11)
		case 0x12:
		case 0x13:
		case 0x14:
		case 0x15:
		case 0x16:
		case 0x17:
		case 0x18:
		case 0x19:
		case 0x1a:
			fprintf(asciifile, "%d", *(buffer + pos) - 0x11);
			break;
		case 0x1c: // numbers from 256-32767
			fprintf(asciifile, "%d", get_word_value(buffer + pos + 1));
			pos += 2;
			break;
		case 0x1d: // single float
			pos = get_single(buffer, pos, asciifile);
			break;
		case 0x1f: // double float
			pos = get_double(buffer, pos, asciifile);
			break;
		case 0x22: // string literal
			pos = get_quoted_string(buffer, pos, asciifile);
			break;
		case 0x26:
			pos = get_bin(buffer, pos, asciifile);
			break;
		case 0x27: // '
			pos = get_terminal_string(buffer, pos + 1, asciifile);
			break;
		case 0x3a:
			pos = get_colon(buffer, pos, asciifile);
			break;
		case 0x81:
			fprintf(asciifile, "END");
			break;
		case 0x82:
			fprintf(asciifile, "FOR");
			break;
		case 0x83:
			fprintf(asciifile, "NEXT");
			break;
		case 0x84:
			fprintf(asciifile, "DATA");
			pos++;
			// get unquoted string
			pos = get_string(buffer, pos, asciifile);
			break;
		case 0x85:
			fprintf(asciifile, "INPUT");
			break;
		case 0x86:
			fprintf(asciifile, "DIM");
			break;
		case 0x87:
			fprintf(asciifile, "READ");
			break;
		case 0x88:
			fprintf(asciifile, "LET");
			break;
		case 0x89:
			fprintf(asciifile, "GOTO");
			break;
		case 0x8a:
			fprintf(asciifile, "RUN");
			break;
		case 0x8b:
			fprintf(asciifile, "IF");
			break;
		case 0x8c:
			fprintf(asciifile, "RESTORE");
			break;
		case 0x8d:
			fprintf(asciifile, "GOSUB");
			break;
		case 0x8e:
			fprintf(asciifile, "RETURN");
			break;
		case 0x8f:
			fprintf(asciifile, "REM");
			pos = get_terminal_string(buffer, pos + 1, asciifile);
			break;
		case 0x90:
			fprintf(asciifile, "STOP");
			break;
		case 0x91:
			fprintf(asciifile, "PRINT");
			break;
		case 0x92:
			fprintf(asciifile, "CLEAR");
			break;
		case 0x93:
			fprintf(asciifile, "LIST");
			break;
		case 0x94:
			fprintf(asciifile, "NEW");
			break;
		case 0x95:
			fprintf(asciifile, "ON");
			break;
		case 0x96:
			fprintf(asciifile, "WAIT");
			break;
		case 0x97:
			fprintf(asciifile, "DEF");
			break;
		case 0x98:
			fprintf(asciifile, "POKE");
			break;
		case 0x99:
			fprintf(asciifile, "CONT");
			break;
		case 0x9a:
			fprintf(asciifile, "CSAVE");
			break;
		case 0x9b:
			fprintf(asciifile, "CLOAD");
			break;
		case 0x9c:
			fprintf(asciifile, "OUT");
			break;
		case 0x9d:
			fprintf(asciifile, "LPRINT");
			break;
		case 0x9e:
			fprintf(asciifile, "LLIST");
			break;
		case 0x9f:
			fprintf(asciifile, "CLS");
			break;
		case 0xa0:
			fprintf(asciifile, "WIDTH");
			break;
		case 0xa1:
			fprintf(asciifile, "ELSE");
			break;
		case 0xa2:
			fprintf(asciifile, "TRON");
			break;
		case 0xa3:
			fprintf(asciifile, "TROFF");
			break;
		case 0xa4:
			fprintf(asciifile, "SWAP");
			break;
		case 0xa5:
			fprintf(asciifile, "ERASE");
			break;
		case 0xa6:
			fprintf(asciifile, "ERROR");
			break;
		case 0xa7:
			fprintf(asciifile, "RESUME");
			break;
		case 0xa8:
			fprintf(asciifile, "DELETE");
			break;
		case 0xa9:
			fprintf(asciifile, "AUTO");
			break;
		case 0xaa:
			fprintf(asciifile, "RENUM");
			break;
		case 0xab:
			fprintf(asciifile, "DEFSTR");
			break;
		case 0xac:
			fprintf(asciifile, "DEFINT");
			break;
		case 0xad:
			fprintf(asciifile, "DEFSNG");
			break;
		case 0xae:
			fprintf(asciifile, "DEFDBL");
			break;
		case 0xaf:
			fprintf(asciifile, "LINE");
			break;
		case 0xb0:
			fprintf(asciifile, "OPEN");
			break;
		case 0xb1:
			fprintf(asciifile, "FIELD");
			break;
		case 0xb2:
			fprintf(asciifile, "GET");
			break;
		case 0xb3:
			fprintf(asciifile, "PUT");
			break;
		case 0xb4:
			fprintf(asciifile, "CLOSE");
			break;
		case 0xb5:
			fprintf(asciifile, "LOAD");
			break;
		case 0xb6:
			fprintf(asciifile, "MERGE");
			break;
		case 0xb7:
			fprintf(asciifile, "FILES");
			break;
		case 0xb8:
			fprintf(asciifile, "LSET");
			break;
		case 0xb9:
			fprintf(asciifile, "RSET");
			break;
		case 0xba:
			fprintf(asciifile, "SAVE");
			break;
		case 0xbb:
			fprintf(asciifile, "LFILES");
			break;
		case 0xbc:
			fprintf(asciifile, "CIRCLE");
			break;
		case 0xbd:
			fprintf(asciifile, "COLOR");
			break;
		case 0xbe:
			fprintf(asciifile, "DRAW");
			break;
		case 0xbf:
			fprintf(asciifile, "PAINT");
			break;
		case 0xc0:
			fprintf(asciifile, "BEEP");
			break;
		case 0xc1:
			fprintf(asciifile, "PLAY");
			break;
		case 0xc2:
			fprintf(asciifile, "PSET");
			break;
		case 0xc3:
			fprintf(asciifile, "PRESET");
			break;
		case 0xc4:
			fprintf(asciifile, "SOUND");
			break;
		case 0xc5:
			fprintf(asciifile, "SCREEN");
			break;
		case 0xc6:
			fprintf(asciifile, "VPOKE");
			break;
		case 0xc7:
			fprintf(asciifile, "SPRITE");
			break;
		case 0xc8:
			fprintf(asciifile, "VDP");
			break;
		case 0xc9:
			fprintf(asciifile, "BASE");
			break;
		case 0xca:
			fprintf(asciifile, "CALL");
			break;
		case 0xcb:
			fprintf(asciifile, "TIME");
			break;
		case 0xcc:
			fprintf(asciifile, "KEY");
			break;
		case 0xcd:
			fprintf(asciifile, "MAX");
			break;
		case 0xce:
			fprintf(asciifile, "MOTOR");
			break;
		case 0xcf:
			fprintf(asciifile, "BLOAD");
			break;
		case 0xd0:
			fprintf(asciifile, "BSAVE");
			break;
		case 0xd1:
			fprintf(asciifile, "DSKO$");
			break;
		case 0xd2:
			fprintf(asciifile, "SET");
			break;
		case 0xd3:
			fprintf(asciifile, "NAME");
			break;
		case 0xd4:
			fprintf(asciifile, "KILL");
			break;
		case 0xd5:
			fprintf(asciifile, "IPL");
			break;
		case 0xd6:
			fprintf(asciifile, "COPY");
			break;
		case 0xd7:
			fprintf(asciifile, "CMD");
			break;
		case 0xd8:
			fprintf(asciifile, "LOCATE");
			break;
		case 0xd9:
			fprintf(asciifile, "TO");
			break;
		case 0xda:
			fprintf(asciifile, "THEN");
			break;
		case 0xdb:
			fprintf(asciifile, "TAB(");
			break;
		case 0xdc:
			fprintf(asciifile, "STEP");
			break;
		case 0xdd:
			fprintf(asciifile, "USR");
			break;
		case 0xde:
			fprintf(asciifile, "FN");
			break;
		case 0xdf:
			fprintf(asciifile, "SPC(");
			break;
		case 0xe0:
			fprintf(asciifile, "NOT");
			break;
		case 0xe1:
			fprintf(asciifile, "ERL");
			break;
		case 0xe2:
			fprintf(asciifile, "ERR");
			break;
		case 0xe3:
			fprintf(asciifile, "STRING$");
			break;
		case 0xe4:
			fprintf(asciifile, "USING");
			break;
		case 0xe5:
			fprintf(asciifile, "INSTR");
			break;
		case 0xe7:
			fprintf(asciifile, "VARPTR");
			break;
		case 0xe8:
			fprintf(asciifile, "CSRLIN");
			break;
		case 0xe9:
			fprintf(asciifile, "ATTR$");
			break;
		case 0xea:
			fprintf(asciifile, "DSKI$");
			break;
		case 0xeb:
			fprintf(asciifile, "OFF");
			break;
		case 0xec:
			fprintf(asciifile, "INKEY$");
			break;
		case 0xed:
			fprintf(asciifile, "POINT");
			break;
		case 0xee:
			fprintf(asciifile, ">");
			break;
		case 0xef:
			fprintf(asciifile, "=");
			break;
		case 0xf0:
			fprintf(asciifile, "<");
			break;
		case 0xf1:
			fprintf(asciifile, "+");
			break;
		case 0xf2:
			fprintf(asciifile, "-");
			break;
		case 0xf3:
			fprintf(asciifile, "*");
			break;
		case 0xf4:
			fprintf(asciifile, "/");
			break;
		case 0xf5:
			fprintf(asciifile, "^");
			break;
		case 0xf6:
			fprintf(asciifile, "AND");
			break;
		case 0xf7:
			fprintf(asciifile, "OR");
			break;
		case 0xf8:
			fprintf(asciifile, "XOR");
			break;
		case 0xf9:
			fprintf(asciifile, "EQV");
			break;
		case 0xfa:
			fprintf(asciifile, "IMP");
			break;
		case 0xfb:
			fprintf(asciifile, "MOD");
			break;
		case 0xfc:
			fprintf(asciifile, "\\");
			break;
		case 0xff:
			// extended code
			pos++;
			switch (*(buffer + pos))
			{
			case 0x81:
				fprintf(asciifile, "LEFT$");
				break;
			case 0x82:
				fprintf(asciifile, "RIGHT$");
				break;
			case 0x83:
				fprintf(asciifile, "MID$");
				break;
			case 0x84:
				fprintf(asciifile, "SGN");
				break;
			case 0x85:
				fprintf(asciifile, "INT");
				break;
			case 0x86:
				fprintf(asciifile, "ABS");
				break;
			case 0x87:
				fprintf(asciifile, "SQR");
				break;
			case 0x88:
				fprintf(asciifile, "RND");
				break;
			case 0x89:
				fprintf(asciifile, "SIN");
				break;
			case 0x8a:
				fprintf(asciifile, "LOG");
				break;
			case 0x8b:
				fprintf(asciifile, "EXP");
				break;
			case 0x8c:
				fprintf(asciifile, "COS");
				break;
			case 0x8d:
				fprintf(asciifile, "TAN");
				break;
			case 0x8e:
				fprintf(asciifile, "ATN");
				break;
			case 0x8f:
				fprintf(asciifile, "FRE");
				break;
			case 0x90:
				fprintf(asciifile, "INP");
				break;
			case 0x91:
				fprintf(asciifile, "POS");
				break;
			case 0x92:
				fprintf(asciifile, "LEN");
				break;
			case 0x93:
				fprintf(asciifile, "STR$");
				break;
			case 0x94:
				fprintf(asciifile, "VAL");
				break;
			case 0x95:
				fprintf(asciifile, "ASC");
				break;
			case 0x96:
				fprintf(asciifile, "CHR$");
				break;
			case 0x97:
				fprintf(asciifile, "PEEK");
				break;
			case 0x98:
				fprintf(asciifile, "VPEEK");
				break;
			case 0x99:
				fprintf(asciifile, "SPACE$");
				break;
			case 0x9a:
				fprintf(asciifile, "OCT$");
				break;
			case 0x9b:
				fprintf(asciifile, "HEX$");
				break;
			case 0x9c:
				fprintf(asciifile, "LPOS");
				break;
			case 0x9d:
				fprintf(asciifile, "BIN$");
				break;
			case 0x9e:
				fprintf(asciifile, "CINT");
				break;
			case 0x9f:
				fprintf(asciifile, "CSNG");
				break;
			case 0xa0:
				fprintf(asciifile, "CDBL");
				break;
			case 0xa1:
				fprintf(asciifile, "FIX");
				break;
			case 0xa2:
				fprintf(asciifile, "STICK");
				break;
			case 0xa3:
				fprintf(asciifile, "STRIG");
				break;
			case 0xa4:
				fprintf(asciifile, "PDL");
				break;
			case 0xa5:
				fprintf(asciifile, "PAD");
				break;
			case 0xa6:
				fprintf(asciifile, "DSKF");
				break;
			case 0xa7:
				fprintf(asciifile, "FPOS");
				break;
			case 0xa8:
				fprintf(asciifile, "CVI");
				break;
			case 0xa9:
				fprintf(asciifile, "CVS");
				break;
			case 0xaa:
				fprintf(asciifile, "CVD");
				break;
			case 0xab:
				fprintf(asciifile, "EOF");
				break;
			case 0xac:
				fprintf(asciifile, "LOC");
				break;
			case 0xad:
				fprintf(asciifile, "LOF");
				break;
			case 0xae:
				fprintf(asciifile, "MKI$");
				break;
			case 0xaf:
				fprintf(asciifile, "MKS$");
				break;
			case 0xb0:
				fprintf(asciifile, "MKD$");
				break;
			}

			break;
		default:
			fprintf(asciifile, "%c", *(buffer + pos));
			break;
		}
		pos++;
	}
	pos++; // end of line

	return pos;
}
int get_octal(byte *buffer, int pos, FILE *output)
{
	pos++;
	fprintf(output, "&O%o", get_word_value(buffer + pos));

	pos++;
	return pos;
}

uint16_t get_word_value(byte *pointer)
{
	return (byte)(*pointer) + ((byte) * (pointer + 1)) * 256;
}

/* int get_file_size(FILE *fi)
{
	fseek(fi, 0L, SEEK_END);
	int file_size = ftell(fi);
	rewind(fi);

	return (file_size);
}
*/

int get_quoted_string(byte *buffer, int pos, FILE *output)
{
	fprintf(output, "\"");
	pos++;
	while (*(buffer + pos) != 0x22 && *(buffer + pos) != 0)
	{
		fprintf(output, "%c", *(buffer + pos));
		pos++;
	}
	if (*(buffer + pos) == 0)
	{
		pos--; // if terminating quote does not exist
	}
	else
	{
		fprintf(output, "%c", *(buffer + pos));
	}
	return pos;
}
int get_single(byte *buffer, int pos, FILE *output)
{
	return get_float(buffer, pos, output, false);
}
int get_double(byte *buffer, int pos, FILE *output)
{
	return get_float(buffer, pos, output, true);
}
int get_float(byte *buffer, int pos, FILE *output, bool is_double)
{
	char qualifier = (is_double) ? '#' : '!';
	char *mantissa;

	uint8_t mantissa_length = (is_double) ? 14 : 6;

	pos++;

	switch (*(buffer + pos))
	{
		// trivial case: if first byte=0 the whole value=0

	case 0:
		fprintf(output, "0%c", qualifier);
		pos += 3;
		break;

	// process the float number
	default:
		mantissa = malloc((is_double) ? 15 : 7);
		byte exponent = *(buffer + pos) & 0x7f; // takes absolute value, first bit is sign

		pos++; // pos now points to the beginning of the mantissa

		uint8_t length = read_mantissa(buffer + pos, mantissa, mantissa_length);

		// get decimal place

		// 4 possible cases:

		// a) actual mantissa length <= max mantissa length
		// b) decimal place is > 0x40 and total length > max mantissa length -> E+xx
		// c) decimal place is < 0x40 (first char) and total length <= max mantissa
		// d) decimal place is < 0x40 (first char) and total length > max mantissa -> E-xx

		// b  1.11111E+14
		if (exponent == 0x40)
		// case: exp 0x40
		{
			print_mantissa(mantissa, length, 0, output);
		}
		else
		{

			if (exponent > 0x40)
			{

				if ((exponent - 0x40) > mantissa_length)
				{
					print_mantissa(mantissa, length, 1, output);
					fprintf(output, "E+%d", exponent - 0x40 - 1);
				}
				else
				{
					print_mantissa(mantissa, length, exponent - 0x40, output);
					if ((exponent - 0x40) > length)
					{
						// fill with zeroes
						for (int i = 0; i < (exponent - 0x40) - length; i++)
						{
							fprintf(output, "0");
						}
					}
				}
			}
			else
			{

				if (exponent < 0x40)
				{
					// c
					if (exponent > (0x40 - mantissa_length))
					{
						print_mantissa(mantissa, length, exponent - 0x40, output);
					}
					else
					// d
					{
						print_mantissa(mantissa, length, 1, output);
						fprintf(output, "E%d", exponent - 0x40 - 1);
					}
				}
			}
		}
		// add zeroes if integer ending with zero (exponent will be greater than mantissa length)
		// add qualifier if integer
		if (((exponent - 0x40) >= (length)) && (exponent - 0x40 <= mantissa_length))
		{
			fprintf(output, "%c", qualifier);
		}

		pos += 2;
		break;
	}

	return pos;
}
void print_mantissa(char *mantissa, uint8_t length, uint8_t dot_pos, FILE *output)
{
	// dot_pos = dot_pos - 0x40;
	if (dot_pos < 0)
	{
		fprintf(output, ".");
	}
	for (int i = (dot_pos < 0) ? dot_pos : 0; i < length; i++)
	{
		if (i < 0)
		{
			fprintf(output, "0");
		}
		else
		{
			if (dot_pos == i)
			{
				fprintf(output, ".");
			}
			fprintf(output, "%c", *(mantissa + i));
		}
	}
}
uint8_t read_mantissa(byte *buffer, char *mantissa, uint8_t length)
{
	uint8_t mantissa_length = 0;
	// set end of string
	*(mantissa + length) = 0;

	for (uint8_t i = length - 1; i >= 0; i--)
	{
		byte n = (i % 2) ? (*(buffer + (i / 2)) & 0x0f) : (*(buffer + (i / 2)) & 0xf0) >> 4;
		*(mantissa + i) = n | 0x30;

		if (n && mantissa_length == 0) // found the first non-zero
		{
			mantissa_length = i + 1;
		}
	}

	// returns actual length
	return mantissa_length;
}

// used by DATA and others
int get_string(byte *buffer, int pos, FILE *output)
{
	while (*(buffer + pos) != 0 && *(buffer + pos) != ':')
	{
		fprintf(output, "%c", *(buffer + pos));
		pos++;
	}

	/*
	   if (*(buffer+pos)) {
	   //the string ended up with a 0x3a (:)
	   //otherwise the ending is the line's ending so no need to increase pointer
	   pos++;
	   } */

	return pos - 1;
}

// used for REM
int get_terminal_string(byte *buffer, int pos, FILE *output)
{
	while (*(buffer + pos)) // the only condition to end string is \0
	{
		fprintf(output, "%c", *(buffer + pos));
		pos++;
	}

	return pos - 1; // next token should point \0 (EOL)
}

int get_colon(byte *buffer, int pos, FILE *output)
{
	/*
	   Possible cases:
	   0x3a:    plain colon
	   0x3a 0x8f:   REM
	   0x3a 0x8f 0xe6: rem mark (')
	 */

	if (*(buffer + pos + 1) == 0x8f)
	{
		// process REM and '
		pos++;

		if (*(buffer + pos + 1) == 0xe6)
		{
			fprintf(output, "'");
			pos += 2;
		}
		else
		{
			fprintf(output, "REM");
		}
		pos = get_terminal_string(buffer, pos, output);
	}
	else
	{
		fprintf(output, ":");
	}

	return pos;
}
int get_bin(byte *buffer, int pos, FILE *output)
{
	pos = get_string(buffer, pos, output);
	return pos;
}
int get_hex(byte *buffer, int pos, FILE *output)
{
	pos++;
	fprintf(output, "&H%X", get_word_value(buffer + pos));

	pos++;
	return pos;
}
int get_line_number(byte *buffer, int pos, FILE *output)
{
	pos++;
	fprintf(output, "%d", get_word_value(buffer + pos));

	pos++;
	return pos;
}
