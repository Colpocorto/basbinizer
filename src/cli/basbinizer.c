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

#include "basbinizer.h"

void title(void)
{
	fprintf(options.stdoutf, "_/_/_/_/_/_/_/_/  Basbinizer v%s _/_/_/_/_/_/_/_/\n", VERSION);
	fprintf(options.stdoutf, "_/_/_/_/_/_/_/_/ 2023 MSXWiki.org _/_/_/_/_/_/_/_/\n\n\n");
}
void usage(void)
{
	fprintf(options.stdoutf, "Syntax:\n\n");
	fprintf(options.stdoutf, "basbinizer <inputfile> [-i <BIN filename>] [-b <CAS filename> [-c <BLOAD \"NAME\">] [-s <SCR loadfile>] [-vm <screen mode>]] [-a <ASC filename>] [-r <ROM filename>] [--fix] [--quiet]\n\n");
	fprintf(options.stdoutf, "Where\n");
	fprintf(options.stdoutf, "\t<inputfile> is the path to an MSX-BASIC .BAS file in tokenized format\n");
	fprintf(options.stdoutf, "\t<BIN filename> is the .BIN file (loadable from disk)\n");
	fprintf(options.stdoutf, "\t<CAS filename> is the resulting .CAS file.\n");
	fprintf(options.stdoutf, "\t<BLOAD \"NAME\"> is the name of the \"FOUND\" file (max. 6 characters)\n");
	fprintf(options.stdoutf, "\t<ROM filename> is the name of the ROM file\n");
	fprintf(options.stdoutf, "\t<ASC filename> to generate an ASCII file from the tokenized BASIC. If not specified, the ASCII text is written to the standard output.\n");
	fprintf(options.stdoutf, "\t<SCR filename> is a loading screen (in BSAVE format) or a VRAM dump that would be loaded prior to the main BASIC program.\n");
	fprintf(options.stdoutf, "\t<screen mode> is the SCREEN mode in MSX-BASIC to be set before loading the screen. Default: 2\n");
	fprintf(options.stdoutf, "\nOptions:\n");
	fprintf(options.stdoutf, "\t--fix\t\tFixes certain data errors found in the source .BAS file\n");
	fprintf(options.stdoutf, "\t--quiet\t\t suppress messages on screen (except for critical errors)\n\n");

	fprintf(options.stdoutf, "ROM files must be under %d bytes and the variable area must start beyond address #C000. The program will fail if it sets the variable area to any address under #C000 (e.g. by using a CLEAR statement)\n\n", MAX_ROM_SIZE);

	fprintf(options.stdoutf, "Example:\n\tbasbinizer STARS.BAS -b STARS.CAS -c STARS\n\n\n");
}

bool process_params(int argc, char **argv, options_t *opt)
{
	/*
		Set default parameters
	*/
	opt->infile = NULL;
	opt->infile_s = 0;
	opt->scrfile = NULL;
	opt->scrfile_s = 0;
	opt->casfile = NULL;
	opt->ascfile = NULL;
	opt->romfile = NULL;
	opt->scr_mode = 2;
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
		/*
			if we are here, the file exists. Good news!
		*/
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
		if ((!strcmp(argv[i], "-i")) && (i < (argc - 1)))
		{
			opt->binfile = argv[++i];
		}
		else
		{
			if ((!strcmp(argv[i], "-b")) && (i < (argc - 1)))
			{
				opt->casfile = argv[++i];
			}
			else
			{
				if ((!strcmp(argv[i], "-c")) && (i < (argc - 1)))
				{
					int name_len = strlen(argv[++i]);
					memcpy(opt->casname, argv[i], name_len > CAS_NAME_LEN ? CAS_NAME_LEN : name_len);
					printf("casname: %s\n", opt->casname);
				}
				else
				{
					if ((!strcmp(argv[i], "-s")) && (i < (argc - 1)))
					{
						if (!stat(argv[++i], &st_infile))
						{
							/*
								if we are here, the screen file exists.
							*/
							opt->scrfile = argv[i];
							opt->scrfile_s = st_infile.st_size;
						}
					}
					else
					{
						if ((!strcmp(argv[i], "-vm")) && (i < (argc - 1)))
						{
							opt->scr_mode = atoi(argv[++i]);
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
									else
									{
										if ((!strcmp(argv[i], "-r")) && (i < (argc - 1)))
										{
											if (opt->infile_s <= MAX_ROM_SIZE)
											{
												opt->romfile = argv[++i];
											}
											else
											{
												strcpy(opt->valerror, "BASIC program is too big to be fitted in a ROM.\n");
												return (false);
											}
										}
									}
								}
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
byte *get_input_file(char *infile, off_t infile_s)
{

	static byte *buffer = NULL;

	if ((buffer = malloc(infile_s)) != NULL)
	{
		FILE *f = NULL;
		fprintf(options.stdoutf, "Opening input file %s...", infile);

		if (!(f = fopen(infile, "rb")))
		{
			fprintf(options.stdoutf, "FAIL\n");
			fprintf(stderr, "Error opening input file %s\n", infile, f);
			return NULL;
		}
		fprintf(options.stdoutf, "OK\n");
		fprintf(options.stdoutf, "Reading input file %s...", infile);
		if (fread(buffer, 1, infile_s, f) != infile_s)
		{
			fprintf(options.stdoutf, "FAIL\n");
			fprintf(stderr, "Unexpected end of file reading input file %s\n", infile);
			return NULL;
		}
		fprintf(options.stdoutf, "OK\n");

		return (buffer);
	}
	else
	{
		fprintf(stderr, "Memory error while opening input file %s\n", infile);
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

	byte *inbuf = get_input_file(options.infile, options.infile_s);

	if (!inbuf)
	{
		return (1);
	}

	/*
		Load the VRAM file if required
	*/
	byte *scr_buf = NULL;
	if (options.scrfile)
	{
		scr_buf = get_input_file(options.scrfile, options.scrfile_s);

		if (!scr_buf)
		{
			return (1);
		}
	}

	/*
		Process the BAS file and:
		-Convert to ASCII (stdout or file)
		-Fix links if required
	*/
	if (!process_bas(inbuf, options.infile_s, BASE_BIN, false))
	{
		return (1);
	}

	/*
		Write .BIN file if requested
	*/
	if (options.binfile)
	{
		if (!write_bin(inbuf, options.infile_s, options.binfile))
		{
			return (1);
		}
	}

	/*
		Write .CAS file if requested
	*/
	if (options.casfile)
	{
		if (!write_cas(inbuf, options.infile_s, scr_buf, options.scrfile_s, options.casfile))
		{
			return (1);
		}
	}
	/*
		Write .ROM file if requested
	*/
	if (options.romfile)
	{
		/*
			Reprocess BAS file for ROM base address
		*/
		options.fix = true;
		if (!process_bas(inbuf, options.infile_s, BASE_ROM, true))
		{
			return (1);
		}
		/*
			Set first byte to 0x00 according to the Technical Handbook
		*/
		*inbuf = 0x00;
		if (!write_rom(inbuf, options.infile_s, options.romfile))
		{
			return (1);
		}
	}

	free(inbuf);

	return (exitcode);
}
bool write_bin(byte *buffer, off_t buf_size, char *binf)
{

	/*
		Write file size into binary loader data (bin_loader_data is a global array)
		and  BSAVE data on BIN header
	*/
	bin_loader_data[BIN_PATCH_POS] = (byte)((0x8000 + buf_size) & 0xff);
	bin_loader_data[BIN_PATCH_POS + 1] = (byte)((0x8000 + buf_size) >> 8);

	BIN_header[3] = (byte)((0x8000 + buf_size + BIN_LOADER_SIZE - 1) & 0xff);
	BIN_header[4] = (byte)((0x8000 + buf_size + BIN_LOADER_SIZE - 1) >> 8);

	BIN_header[5] = (byte)((0x8000 + buf_size) & 0xff);
	BIN_header[6] = (byte)((0x8000 + buf_size) >> 8);

	/*
		Get memory for the buffer
	*/
	byte *bin_buffer = malloc(sizeof(BIN_header) + buf_size + BIN_LOADER_SIZE);

	if (!bin_buffer)
	{
		fprintf(stderr, "Memory error trying to find %lld bytes free while writing .BIN File... exiting.\n\n", sizeof(BIN_header) + buf_size + BIN_LOADER_SIZE);

		return (false);
	}

	/*
		Copy data to the .BIN buffer
	*/
	memcpy(bin_buffer, BIN_header, 7);
	memcpy(bin_buffer + 7, buffer, buf_size);
	memcpy(bin_buffer + 7 + buf_size, bin_loader_data, BIN_LOADER_SIZE);

	/*
		finally, open output file and write data
	*/
	FILE *fo = fopen(binf, "wb");
	if (!fo)
	{
		fprintf(stderr, "Error opening .BIN file %s.\n", binf);
		return (false);
	}

	if (fwrite(bin_buffer, 1, sizeof(BIN_header) + buf_size + BIN_LOADER_SIZE, fo) != (sizeof(BIN_header) + buf_size + BIN_LOADER_SIZE))
	{
		fprintf(stderr, "Error writing .BIN file %s.\n", binf);
	}

	fclose(fo);
	free(bin_buffer);

	return (true);
}
bool write_cas(byte *buffer, off_t buf_size, byte *scr_buf, off_t scr_buf_size, char *casf)
{

	/*
		Write file size into cas binary loader data (cas_loader_data is a global array)
	*/
	cas_loader_data[CAS_PATCH_POS] = (byte)(buf_size & 0xff);
	cas_loader_data[CAS_PATCH_POS + 1] = (byte)((buf_size & 0xff00) >> 8);

	/*
		Patch screen size and enable routine, if needed
	*/
	if (options.scrfile)
	{
		cas_loader_data[SCR_PATCH_POS1] = (byte)((scr_buf_size - 7) & 0xff);
		cas_loader_data[SCR_PATCH_POS2] = (byte)((scr_buf_size - 7) & 0xff);
		cas_loader_data[SCR_PATCH_POS1 + 1] = (byte)(((scr_buf_size - 7) & 0xff00) >> 8);
		cas_loader_data[SCR_PATCH_POS2 + 1] = (byte)(((scr_buf_size - 7) & 0xff00) >> 8);
		cas_loader_data[SCR_PATCH_POS3] = 0;
		cas_loader_data[VM_PATCH_POS] = options.scr_mode;
	}

	memcpy(cas_loader_data + PATCH_NAME_POS, options.casname, CAS_NAME_LEN);

	/*
		Get memory for the buffers
	*/
	byte *cas_buffer = malloc(CAS_LOADER_SIZE + (options.scrfile ? sizeof(CAS_header) + scr_buf_size - 7 + (8 - (CAS_LOADER_SIZE + scr_buf_size - 7) % 8) : 0) + buf_size);

	if (!cas_buffer)
	{
		fprintf(stderr, "Memory error trying to find %lld bytes free while writing CAS file... exiting.\n\n", CAS_LOADER_SIZE + buf_size);
		return (false);
	}

	/*
		copy data and concat .BAS file
	*/
	int offset = 0;

	memcpy(cas_buffer, cas_loader_data, CAS_LOADER_SIZE);
	offset += CAS_LOADER_SIZE;

	if (options.scrfile)
	{
		memcpy(cas_buffer + offset, scr_buf + 7, scr_buf_size - 7);
		offset += scr_buf_size - 7;

		memset(cas_buffer + offset, 0, (8 - (CAS_LOADER_SIZE + scr_buf_size - 7) % 8));
		offset += (8 - (CAS_LOADER_SIZE + scr_buf_size - 7) % 8);

		memcpy(cas_buffer + offset, CAS_header, sizeof(CAS_header));
		offset += sizeof(CAS_header);

		memcpy(cas_buffer + offset, buffer, buf_size);
		offset += buf_size;
	}
	else
	{
		memcpy(cas_buffer + offset, buffer, buf_size);
		offset += buf_size;
	}

	/*
		finally, open output file and write data
	*/
	FILE *fo = fopen(casf, "wb");
	if (!fo)
	{
		fprintf(stderr, "Error opening .CAS file %s.\n", casf);
		return (false);
	}

	if (fwrite(cas_buffer, 1, offset, fo) != offset)
	{
		fprintf(stderr, "Error writing .CAS file %s.\n", casf);
	}

	fclose(fo);
	free(cas_buffer);

	return (true);
}

bool write_rom(byte *inbuf, off_t buf_size, char *romfile)
{
	/*
		Get memory for the buffer
	*/

	int rom_size = buf_size>(MAX_ROM_SIZE-ROM_SIZE/2) ? ROM_SIZE : ROM_SIZE / 2;

	byte *rom_buffer = malloc(rom_size);

	if (!rom_buffer)
	{
		fprintf(stderr, "Memory error trying to find %d bytes free while writing ROM file... exiting.\n\n", rom_size);
		return (false);
	}

	/*
		copy data and concat .BAS file
	*/
	memset(rom_buffer, 0xff, rom_size);
	memcpy(rom_buffer, ROM_header, sizeof(ROM_header));
	memcpy(rom_buffer + sizeof(ROM_header), inbuf, buf_size);

	/*
		finally, open output file and write data
	*/
	FILE *fo = fopen(romfile, "wb");
	if (!fo)
	{
		fprintf(stderr, "Error opening .ROM file %s.\n", romfile);
		return (false);
	}

	if (fwrite(rom_buffer, 1, rom_size, fo) != (rom_size))
	{
		fprintf(stderr, "Error writing .ROM file %s.\n", romfile);
	}

	fclose(fo);
	free(rom_buffer);

	return (true);
}
bool process_bas(byte *buffer, off_t file_size, uint16_t base_addr, bool force_quiet)
{
	/*
		Create the ASCII file, assign it to stdout or assign it to null
	*/

	FILE *asciifile = stdout;
	if (!force_quiet)
	{
		if (options.ascfile)
		{
			if (!(asciifile = fopen(options.ascfile, "wb")))
			{
				fprintf(stderr, "Error creating output ASCII file.\n");
				return (false);
			}
		}
	}
	else
	{
		/*
			set it to null (for example, during the .ROM creation)
		*/
#ifdef _WIN32
		asciifile = fopen("NUL", "wb");
#else
		asciifile = fopen("/dev/null", "wb");
#endif
	}

	decodeBAS(buffer, file_size, base_addr, asciifile);
	/*
		EOF
	*/
	fprintf(asciifile, "%c", 0x1a);

	fclose(asciifile);

	return (true);
}

void decodeBAS(byte *buffer, off_t size, uint16_t base_addr, FILE *output)
{

	int link_pointer;

	/*
		skip 0xff header
	*/
	int pos = 1;

	while ((link_pointer = get_word_value(buffer + pos)) != 0)
	{
		/*
			decode line
		*/
		pos = decodeLine(buffer, pos, size, base_addr, output);

		fprintf(output, "\r\n");
	}
}

int decodeLine(byte *buffer, int pos, off_t size, uint16_t base_addr, FILE *output)
{
	byte token;

	/*
		get the link pointer
	*/
	uint16_t link_ptr = get_word_value(buffer + pos);
	int initial_pos_at_line = pos;

	pos += 2;

	/*
		spit out line number
	*/
	uint16_t current_line_number = get_word_value(buffer + pos);
	fprintf(output, "%d ", current_line_number);

	pos += 2;

	while ((token = *(buffer + pos)) != 0)
	{
		/*
			process token
		*/
		switch (token)
		{
		/*
			get octal value
		*/
		case 0x0b:
			pos = get_octal(buffer, pos, output);
			break;
		/*
			get hex value
		*/
		case 0x0c:
			pos = get_hex(buffer, pos, output);
			break;
		/*
			get line number
		*/
		case 0x0e:
			pos = get_line_number(buffer, pos, output);
			break;
		/*
			numbers from 10 to 255
		*/
		case 0x0f:
			pos++;
			fprintf(output, "%d", *(buffer + pos));
			break;
		/*
			numbers from 0 to 9 are encoded without token (-0x11)
		*/
		case 0x11:
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
		/*
			numbers from 256-32767
		*/
		case 0x1c:
			fprintf(output, "%d", get_word_value(buffer + pos + 1));
			pos += 2;
			break;
		/*
			single float
		*/
		case 0x1d:
			pos = get_single(buffer, pos, output);
			break;
		/*
			double float
		*/
		case 0x1f:
			pos = get_double(buffer, pos, output);
			break;
		/*
			string literal
		*/
		case 0x22:
			pos = get_quoted_string(buffer, pos, output);
			break;
		case 0x26:
			pos = get_bin(buffer, pos, output);
			break;
		/*
			'
		*/
		case 0x27:
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
			/*
				get unquoted string
			*/
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
			/*
				extended code
			*/
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
	/*
		end of line
	*/
	pos++;

	if (link_ptr != (base_addr + (uint16_t)pos))
	{
		if (options.fix)
		{
			buffer[initial_pos_at_line] = (byte)((pos + base_addr) & 0xff);
			buffer[initial_pos_at_line + 1] = (byte)(((pos + base_addr) & 0xff00) >> 8);
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
		/*
			if terminating quote does not exist
		*/
		pos--;
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
	char *mantissa = NULL;

	int8_t mantissa_length = (is_double) ? 14 : 6;

	pos++;

	switch (*(buffer + pos))
	{
		/*
			trivial case: if first byte=0 the whole value=0
		*/

	case 0:
		fprintf(output, "0%c", qualifier);
		pos += is_double ? 7 : 3;
		break;
		/*
			process the float number
		*/
	default:

		mantissa = malloc((is_double) ? 15 : 7);
		/*
			takes absolute value, first bit is sign
		*/
		byte exponent = *(buffer + pos) & 0x7f;

		/*
			pos now points to the beginning of the mantissa
		*/
		pos++;
		int8_t length = read_mantissa(buffer + pos, mantissa, mantissa_length);

		/*
			get decimal place

			4 possible cases:

			a) actual mantissa length <= max mantissa length
			b) decimal place is > 0x40 and total length > max mantissa length -> E+xx
			c) decimal place is < 0x40 (first char) and total length <= max mantissa
			d) decimal place is < 0x40 (first char) and total length > max mantissa -> E-xx

			b  1.11111E+14
		*/
		if (exponent == 0x40)
		/*
			case: exp 0x40
		*/
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
						/*
							fill with zeroes
						*/
						int i = 0;
						for (i; i < (exponent - 0x40) - length; i++)
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
					/*
						c
					*/
					if (exponent > (0x40 - mantissa_length))
					{
						print_mantissa(mantissa, length, exponent - 0x40, output);
					}
					else
					/*
						d
					*/
					{
						print_mantissa(mantissa, length, 1, output);
						fprintf(output, "E%d", exponent - 0x40 - 1);
					}
				}
			}
		}
		/*
			add zeroes if integer ending with zero (exponent will be greater than mantissa length)
			add qualifier if integer
		*/
		if (((exponent - 0x40) >= (length)) && (exponent - 0x40 <= mantissa_length))
		{
			fprintf(output, "%c", qualifier);
		}
		pos += is_double ? 6 : 2;
		free(mantissa);
		break;
	}

	return pos;
}
void print_mantissa(char *mantissa, int8_t length, int8_t dot_pos, FILE *output)
{
	if (!mantissa)
	{
		return;
	}
	else
	{

		/*
			dot_pos = dot_pos - 0x40;
		*/
		if (dot_pos < 0)
		{
			fprintf(output, ".");
		}

		int i = (dot_pos < 0);
		for (i ? dot_pos : 0; i < length; i++)
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
	return;
}
int8_t read_mantissa(byte *buffer, char *mantissa, int8_t length)
{
	int8_t mantissa_length = 0;
	/*
		set end of string
	*/
	*(mantissa + length) = 0;

	int8_t i = length - 1;
	for (i; i >= 0; i--)
	{
		byte n = (i % 2) ? (*(buffer + (i / 2)) & 0x0f) : (*(buffer + (i / 2)) & 0xf0) >> 4;
		*(mantissa + i) = n | 0x30;
		/*
			found the first non-zero
		*/
		if (n && mantissa_length == 0)
		{
			mantissa_length = i + 1;
		}
	}

	/*
		returns actual length
	*/
	return mantissa_length;
}

/*
	used by DATA and others
*/
int get_string(byte *buffer, int pos, FILE *output)
{
	while (*(buffer + pos) != 0 && *(buffer + pos) != ':')
	{
		fprintf(output, "%c", *(buffer + pos));
		pos++;
	}

	return pos - 1;
}

/*
	used for REM
*/
int get_terminal_string(byte *buffer, int pos, FILE *output)
{
	/*
		the only condition to end string is \0
	*/
	while (*(buffer + pos))
	{
		fprintf(output, "%c", *(buffer + pos));
		pos++;
	}
	/*
		next token should point \0 (EOL)
	*/
	return pos - 1;
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
