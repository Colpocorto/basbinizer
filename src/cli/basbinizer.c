#include "basbinizer.h"

void title(void)
{
	fprintf(options.stdoutf, "_/_/_/_/_/_/_/_/  Basbinizer v1.0 _/_/_/_/_/_/_/_/\n");
	fprintf(options.stdoutf, "_/_/_/_/_/_/_/_/ 2023 MSXWiki.org _/_/_/_/_/_/_/_/\n\n\n");
}
void usage(void)
{
	fprintf(options.stdoutf, "Syntax:\n\n");
	fprintf(options.stdoutf, "basbinizer <inputfile> [-b <outputfile> [-c <CAS FILENAME>]] [-a <ASC filename>] [--fix] [--quiet]\n\n");
	fprintf(options.stdoutf, "Where\n");
	fprintf(options.stdoutf, "\t<intputfile> is the path to an MSX-BASIC .BAS file in tokenized format\n");
	fprintf(options.stdoutf, "\t<outputfile> is the resulting .CAS file.\n");
	fprintf(options.stdoutf, "\t<CAS FILENAME> if the name of the BLOAD name (max. 6 characters)\n");
	fprintf(options.stdoutf, "\t<ASC filename> to generate an ASCII file from the tokenized BASIC. If not specified, the ASCII text is written to the standard output.\n\n");
	fprintf(options.stdoutf, "Options:\n");
	fprintf(options.stdoutf, "\t--fix\t\tFixes certain data errors found in the source .BAS file\n");
	fprintf(options.stdoutf, "\t--quiet\t\t suppress messages on screen (except for critical errors\n\n");	

	fprintf(options.stdoutf, "Example:\n\tbasbinizer STARS.BAS -b STARS.CAS -c STARS\n\n\n");
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
			opt->outfile = argv[++i];
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
							opt->ascfile = argv[++i];
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

	if ((buffer = malloc(options.infile_s)) != NULL)
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
	if (!process_bas(inbuf, options.infile_s))
	{
		return (1);
	}

	if (options.outfile)
	{
		if (!write_bin(inbuf, options.infile_s, options.outfile))
		{
			return (1);
		}
	}

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
	free(inbuf);

	return (exitcode);
}
bool write_bin(byte *buffer, off_t buf_size, char *binf)
{

	// Write file size into binary loader data (loader_data is a global array)
	loader_data[PATCH_POS] = (byte)(buf_size & 0xff);
	loader_data[PATCH_POS + 1] = (byte)((buf_size & 0xff00) >> 8);

	memcpy(loader_data + PATCH_NAME_POS, options.casname, CAS_NAME_LEN);

	// Get memory for the buffer
	byte *cas_buffer = malloc(LOADER_SIZE + buf_size);

	if (!cas_buffer)
	{
		fprintf(stderr, "Memory error trying to find %lld bytes free while writing CAS file... exiting.\n\n", LOADER_SIZE + buf_size);

		return (false);
	}

	// copy data and concat .BAS file
	memcpy(cas_buffer, loader_data, LOADER_SIZE);
	memcpy(cas_buffer + LOADER_SIZE, buffer, buf_size);

	// finally, open output file and write data
	FILE *fo = fopen(binf, "wb");
	if (!fo)
	{
		fprintf(stderr, "Error opening .CAS file %s.\n", binf);
		return (false);
	}

	if (fwrite(cas_buffer, 1, LOADER_SIZE + buf_size, fo) != (LOADER_SIZE + buf_size))
	{
		fprintf(stderr, "Error writing .CAS file %s.\n", binf);
	}

	fclose(fo);

	return (true);
}
bool process_bas(byte *buffer, off_t file_size)
{
	/*
		Create the ASCII file or assign it to stdout
	*/
	FILE *asciifile = stdout;

	if (options.ascfile)
	{
		if (!(asciifile = fopen(options.ascfile, "wb")))
		{
			fprintf(stderr, "Error creating output ASCII file.\n");
			return (false);
		}
	}

	decodeBAS(buffer, file_size, asciifile);
	// EOF
	fprintf(asciifile, "%c", 0x1a);

	fclose(asciifile);

	return (true);
}

void decodeBAS(byte *buffer, off_t size, FILE *output)
{

	int link_pointer;

	int pos = 1; // skip 0xff header

	while ((link_pointer = get_word_value(buffer + pos)) != 0)
	{
		// decode line
		pos = decodeLine(buffer, pos, size, output);

		fprintf(output, "\r\n");
	}
}

int decodeLine(byte *buffer, int pos, off_t size, FILE *output)
{
	byte token;

	// get the link pointer
	uint16_t link_ptr = get_word_value(buffer + pos);
	int initial_pos_at_line = pos;

	pos += 2;

	// spit out line number
	uint16_t current_line_number = get_word_value(buffer + pos);
	fprintf(output, "%d ", current_line_number);

	pos += 2;

	while ((token = *(buffer + pos)) != 0)
	{
		// process token
		switch (token)
		{
		case 0x0b: // get octal value
			pos = get_octal(buffer, pos, output);
			break;
		case 0x0c: // get hex value
			pos = get_hex(buffer, pos, output);
			break;
		case 0x0e: // get line number
			pos = get_line_number(buffer, pos, output);
			break;
		case 0x0f: // numbers from 10 to 255
			pos++;
			fprintf(output, "%d", *(buffer + pos));
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
			fprintf(output, "%d", *(buffer + pos) - 0x11);
			break;
		case 0x1c: // numbers from 256-32767
			fprintf(output, "%d", get_word_value(buffer + pos + 1));
			pos += 2;
			break;
		case 0x1d: // single float
			pos = get_single(buffer, pos, output);
			break;
		case 0x1f: // double float
			pos = get_double(buffer, pos, output);
			break;
		case 0x22: // string literal
			pos = get_quoted_string(buffer, pos, output);
			break;
		case 0x26:
			pos = get_bin(buffer, pos, output);
			break;
		case 0x27: // '
			fprintf(output, "'");
			pos = get_terminal_string(buffer, pos + 1, output);
			break;
		case 0x3a:
			pos = get_colon(buffer, pos, output);
			break;
		case 0x81:
			fprintf(output, "END");
			break;
		case 0x82:
			fprintf(output, "FOR");
			break;
		case 0x83:
			fprintf(output, "NEXT");
			break;
		case 0x84:
			fprintf(output, "DATA");
			pos++;
			// get unquoted string
			pos = get_string(buffer, pos, output);
			break;
		case 0x85:
			fprintf(output, "INPUT");
			break;
		case 0x86:
			fprintf(output, "DIM");
			break;
		case 0x87:
			fprintf(output, "READ");
			break;
		case 0x88:
			fprintf(output, "LET");
			break;
		case 0x89:
			fprintf(output, "GOTO");
			break;
		case 0x8a:
			fprintf(output, "RUN");
			break;
		case 0x8b:
			fprintf(output, "IF");
			break;
		case 0x8c:
			fprintf(output, "RESTORE");
			break;
		case 0x8d:
			fprintf(output, "GOSUB");
			break;
		case 0x8e:
			fprintf(output, "RETURN");
			break;
		case 0x8f:
			fprintf(output, "REM");
			pos = get_terminal_string(buffer, pos + 1, output);
			break;
		case 0x90:
			fprintf(output, "STOP");
			break;
		case 0x91:
			fprintf(output, "PRINT");
			break;
		case 0x92:
			fprintf(output, "CLEAR");
			break;
		case 0x93:
			fprintf(output, "LIST");
			break;
		case 0x94:
			fprintf(output, "NEW");
			break;
		case 0x95:
			fprintf(output, "ON");
			break;
		case 0x96:
			fprintf(output, "WAIT");
			break;
		case 0x97:
			fprintf(output, "DEF");
			break;
		case 0x98:
			fprintf(output, "POKE");
			break;
		case 0x99:
			fprintf(output, "CONT");
			break;
		case 0x9a:
			fprintf(output, "CSAVE");
			break;
		case 0x9b:
			fprintf(output, "CLOAD");
			break;
		case 0x9c:
			fprintf(output, "OUT");
			break;
		case 0x9d:
			fprintf(output, "LPRINT");
			break;
		case 0x9e:
			fprintf(output, "LLIST");
			break;
		case 0x9f:
			fprintf(output, "CLS");
			break;
		case 0xa0:
			fprintf(output, "WIDTH");
			break;
		case 0xa1:
			fprintf(output, "ELSE");
			break;
		case 0xa2:
			fprintf(output, "TRON");
			break;
		case 0xa3:
			fprintf(output, "TROFF");
			break;
		case 0xa4:
			fprintf(output, "SWAP");
			break;
		case 0xa5:
			fprintf(output, "ERASE");
			break;
		case 0xa6:
			fprintf(output, "ERROR");
			break;
		case 0xa7:
			fprintf(output, "RESUME");
			break;
		case 0xa8:
			fprintf(output, "DELETE");
			break;
		case 0xa9:
			fprintf(output, "AUTO");
			break;
		case 0xaa:
			fprintf(output, "RENUM");
			break;
		case 0xab:
			fprintf(output, "DEFSTR");
			break;
		case 0xac:
			fprintf(output, "DEFINT");
			break;
		case 0xad:
			fprintf(output, "DEFSNG");
			break;
		case 0xae:
			fprintf(output, "DEFDBL");
			break;
		case 0xaf:
			fprintf(output, "LINE");
			break;
		case 0xb0:
			fprintf(output, "OPEN");
			break;
		case 0xb1:
			fprintf(output, "FIELD");
			break;
		case 0xb2:
			fprintf(output, "GET");
			break;
		case 0xb3:
			fprintf(output, "PUT");
			break;
		case 0xb4:
			fprintf(output, "CLOSE");
			break;
		case 0xb5:
			fprintf(output, "LOAD");
			break;
		case 0xb6:
			fprintf(output, "MERGE");
			break;
		case 0xb7:
			fprintf(output, "FILES");
			break;
		case 0xb8:
			fprintf(output, "LSET");
			break;
		case 0xb9:
			fprintf(output, "RSET");
			break;
		case 0xba:
			fprintf(output, "SAVE");
			break;
		case 0xbb:
			fprintf(output, "LFILES");
			break;
		case 0xbc:
			fprintf(output, "CIRCLE");
			break;
		case 0xbd:
			fprintf(output, "COLOR");
			break;
		case 0xbe:
			fprintf(output, "DRAW");
			break;
		case 0xbf:
			fprintf(output, "PAINT");
			break;
		case 0xc0:
			fprintf(output, "BEEP");
			break;
		case 0xc1:
			fprintf(output, "PLAY");
			break;
		case 0xc2:
			fprintf(output, "PSET");
			break;
		case 0xc3:
			fprintf(output, "PRESET");
			break;
		case 0xc4:
			fprintf(output, "SOUND");
			break;
		case 0xc5:
			fprintf(output, "SCREEN");
			break;
		case 0xc6:
			fprintf(output, "VPOKE");
			break;
		case 0xc7:
			fprintf(output, "SPRITE");
			break;
		case 0xc8:
			fprintf(output, "VDP");
			break;
		case 0xc9:
			fprintf(output, "BASE");
			break;
		case 0xca:
			fprintf(output, "CALL");
			break;
		case 0xcb:
			fprintf(output, "TIME");
			break;
		case 0xcc:
			fprintf(output, "KEY");
			break;
		case 0xcd:
			fprintf(output, "MAX");
			break;
		case 0xce:
			fprintf(output, "MOTOR");
			break;
		case 0xcf:
			fprintf(output, "BLOAD");
			break;
		case 0xd0:
			fprintf(output, "BSAVE");
			break;
		case 0xd1:
			fprintf(output, "DSKO$");
			break;
		case 0xd2:
			fprintf(output, "SET");
			break;
		case 0xd3:
			fprintf(output, "NAME");
			break;
		case 0xd4:
			fprintf(output, "KILL");
			break;
		case 0xd5:
			fprintf(output, "IPL");
			break;
		case 0xd6:
			fprintf(output, "COPY");
			break;
		case 0xd7:
			fprintf(output, "CMD");
			break;
		case 0xd8:
			fprintf(output, "LOCATE");
			break;
		case 0xd9:
			fprintf(output, "TO");
			break;
		case 0xda:
			fprintf(output, "THEN");
			break;
		case 0xdb:
			fprintf(output, "TAB(");
			break;
		case 0xdc:
			fprintf(output, "STEP");
			break;
		case 0xdd:
			fprintf(output, "USR");
			break;
		case 0xde:
			fprintf(output, "FN");
			break;
		case 0xdf:
			fprintf(output, "SPC(");
			break;
		case 0xe0:
			fprintf(output, "NOT");
			break;
		case 0xe1:
			fprintf(output, "ERL");
			break;
		case 0xe2:
			fprintf(output, "ERR");
			break;
		case 0xe3:
			fprintf(output, "STRING$");
			break;
		case 0xe4:
			fprintf(output, "USING");
			break;
		case 0xe5:
			fprintf(output, "INSTR");
			break;
		case 0xe6:
			fprintf(output, "'");
			break;
		case 0xe7:
			fprintf(output, "VARPTR");
			break;
		case 0xe8:
			fprintf(output, "CSRLIN");
			break;
		case 0xe9:
			fprintf(output, "ATTR$");
			break;
		case 0xea:
			fprintf(output, "DSKI$");
			break;
		case 0xeb:
			fprintf(output, "OFF");
			break;
		case 0xec:
			fprintf(output, "INKEY$");
			break;
		case 0xed:
			fprintf(output, "POINT");
			break;
		case 0xee:
			fprintf(output, ">");
			break;
		case 0xef:
			fprintf(output, "=");
			break;
		case 0xf0:
			fprintf(output, "<");
			break;
		case 0xf1:
			fprintf(output, "+");
			break;
		case 0xf2:
			fprintf(output, "-");
			break;
		case 0xf3:
			fprintf(output, "*");
			break;
		case 0xf4:
			fprintf(output, "/");
			break;
		case 0xf5:
			fprintf(output, "^");
			break;
		case 0xf6:
			fprintf(output, "AND");
			break;
		case 0xf7:
			fprintf(output, "OR");
			break;
		case 0xf8:
			fprintf(output, "XOR");
			break;
		case 0xf9:
			fprintf(output, "EQV");
			break;
		case 0xfa:
			fprintf(output, "IMP");
			break;
		case 0xfb:
			fprintf(output, "MOD");
			break;
		case 0xfc:
			fprintf(output, "\\");
			break;
		case 0xff:
			// extended code
			pos++;
			switch (*(buffer + pos))
			{
			case 0x81:
				fprintf(output, "LEFT$");
				break;
			case 0x82:
				fprintf(output, "RIGHT$");
				break;
			case 0x83:
				fprintf(output, "MID$");
				break;
			case 0x84:
				fprintf(output, "SGN");
				break;
			case 0x85:
				fprintf(output, "INT");
				break;
			case 0x86:
				fprintf(output, "ABS");
				break;
			case 0x87:
				fprintf(output, "SQR");
				break;
			case 0x88:
				fprintf(output, "RND");
				break;
			case 0x89:
				fprintf(output, "SIN");
				break;
			case 0x8a:
				fprintf(output, "LOG");
				break;
			case 0x8b:
				fprintf(output, "EXP");
				break;
			case 0x8c:
				fprintf(output, "COS");
				break;
			case 0x8d:
				fprintf(output, "TAN");
				break;
			case 0x8e:
				fprintf(output, "ATN");
				break;
			case 0x8f:
				fprintf(output, "FRE");
				break;
			case 0x90:
				fprintf(output, "INP");
				break;
			case 0x91:
				fprintf(output, "POS");
				break;
			case 0x92:
				fprintf(output, "LEN");
				break;
			case 0x93:
				fprintf(output, "STR$");
				break;
			case 0x94:
				fprintf(output, "VAL");
				break;
			case 0x95:
				fprintf(output, "ASC");
				break;
			case 0x96:
				fprintf(output, "CHR$");
				break;
			case 0x97:
				fprintf(output, "PEEK");
				break;
			case 0x98:
				fprintf(output, "VPEEK");
				break;
			case 0x99:
				fprintf(output, "SPACE$");
				break;
			case 0x9a:
				fprintf(output, "OCT$");
				break;
			case 0x9b:
				fprintf(output, "HEX$");
				break;
			case 0x9c:
				fprintf(output, "LPOS");
				break;
			case 0x9d:
				fprintf(output, "BIN$");
				break;
			case 0x9e:
				fprintf(output, "CINT");
				break;
			case 0x9f:
				fprintf(output, "CSNG");
				break;
			case 0xa0:
				fprintf(output, "CDBL");
				break;
			case 0xa1:
				fprintf(output, "FIX");
				break;
			case 0xa2:
				fprintf(output, "STICK");
				break;
			case 0xa3:
				fprintf(output, "STRIG");
				break;
			case 0xa4:
				fprintf(output, "PDL");
				break;
			case 0xa5:
				fprintf(output, "PAD");
				break;
			case 0xa6:
				fprintf(output, "DSKF");
				break;
			case 0xa7:
				fprintf(output, "FPOS");
				break;
			case 0xa8:
				fprintf(output, "CVI");
				break;
			case 0xa9:
				fprintf(output, "CVS");
				break;
			case 0xaa:
				fprintf(output, "CVD");
				break;
			case 0xab:
				fprintf(output, "EOF");
				break;
			case 0xac:
				fprintf(output, "LOC");
				break;
			case 0xad:
				fprintf(output, "LOF");
				break;
			case 0xae:
				fprintf(output, "MKI$");
				break;
			case 0xaf:
				fprintf(output, "MKS$");
				break;
			case 0xb0:
				fprintf(output, "MKD$");
				break;
			}

			break;
		default:
			fprintf(output, "%c", *(buffer + pos));

			break;
		}
		pos++;
	}
	pos++; // end of line

	if (link_ptr != (0x8000 + (uint16_t)pos))
	{
		if (options.fix)
		{
			buffer[initial_pos_at_line] = (byte)((pos + 0x8000) & 0xff);
			buffer[initial_pos_at_line + 1] = (byte)(((pos + 0x8000) & 0xff00) >> 8);
			if (!options.quiet)
				fprintf(stderr, "Wrong address link fixed at line %d.\n", current_line_number);
		}
		else
		{
			if (!options.quiet)
				fprintf(stderr, "Wrong address link detected at line %d. Consider using --fix option.\n", current_line_number);
		}
	}

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

	int8_t mantissa_length = (is_double) ? 14 : 6;

	pos++;

	switch (*(buffer + pos))
	{
		// trivial case: if first byte=0 the whole value=0

	case 0:
		fprintf(output, "0%c", qualifier);
		pos += is_double ? 7 : 3;
		break;

	// process the float number
	default:

		mantissa = malloc((is_double) ? 15 : 7);
		byte exponent = *(buffer + pos) & 0x7f; // takes absolute value, first bit is sign

		pos++; // pos now points to the beginning of the mantissa
		int8_t length = read_mantissa(buffer + pos, mantissa, mantissa_length);

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
					if (is_double)
						fprintf(output, "%c", qualifier);
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
						// fprintf(output, "%c", qualifier);
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

		pos += is_double ? 6 : 2;
		break;
	}

	return pos;
}
void print_mantissa(char *mantissa, int8_t length, int8_t dot_pos, FILE *output)
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
int8_t read_mantissa(byte *buffer, char *mantissa, int8_t length)
{
	int8_t mantissa_length = 0;
	// set end of string
	*(mantissa + length) = 0;

	for (int8_t i = length - 1; i >= 0; i--)
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
	   0x3a:    plain colon (:)
	   0x3a 0x8f:   REM
	   0x3a 0xa1:	ELSE
	   0x3a 0x8f 0xe6: rem mark (')
	 */

	if (*(buffer + pos + 1) == 0x8f && *(buffer + pos + 2) == 0xe6)
	{
		fprintf(output, "'");
		pos += 2;
	}
	else
	{
		if (*(buffer + pos + 1) == 0xa1)
		{
			fprintf(output, "ELSE");
			pos++;
		}
		else
		{
			fprintf(output, ":");
		}
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
