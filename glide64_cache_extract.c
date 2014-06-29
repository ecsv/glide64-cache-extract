/**
 * glide64_cache_extract, Glide64 TexCache Extraction tool for debugging
 *
 * Copyright (C) 2013  Sven Eckelmann <sven@narfation.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * Example usage:
 * zcat MUPEN64PLUS.dat | ./glide64_cache_extract -vv -p MUPEN64PLUS > mupen64plus.tar
 */

#include "glide64_cache_extract.h"
#include <errno.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

struct _globals globals;

static int convert_input(void)
{
	int ret;
	uint32_t config;

	ret = get_item(config);
	if (ret < 0) {
		fprintf(stderr, "Failed to read config header\n");
		return ret;
	}

	ret = parse_config(config);
	if (ret < 0) {
		fprintf(stderr, "Failed to parse config header\n");
		return ret;
	}

	while (!feof(globals.in)) {
		ret = convert_file();
		if (ret < 0)
			return ret;
	}

	ret = write_tarblock(tarblock, sizeof(tarblock), 0);
	if (ret < 0) {
		fprintf(stderr, "Failed to write first EOF tar record\n");
		return ret;
	}

	ret = write_tarblock(tarblock, sizeof(tarblock), 0);
	if (ret < 0) {
		fprintf(stderr, "Failed to write second EOF tar record\n");
		return ret;
	}

	return 0;
}

static void usage(int argc, char *argv[])
{
	const char *cmd = "glide64_cache_extract";

	if (argc > 1)
		cmd = argv[0];

	printf("Usage: %s [options]\n\n", cmd);
	printf("options:\n");
	printf("\t -i,--input FILE                   Use FILE as uncompressed input file (default: stdin)\n");
	printf("\t -o,--output FILE                  Use FILE as output file (default: stdout)\n");
	printf("\t -p,--prefix NAME                  Add prefix to each file\n");
	printf("\t -t,--type [hires|tex]             Type of the input\n");
	printf("\t -v,--verbose                      Print extra information on stderr (repeat for more verbosity)\n");
	printf("\t -e,--ignore-error                 Skip current file when an conversion error is detected\n");
	printf("\t -b,--bitmapv5                     Use V5 Windows Bitmap files with ImageMagick compatible alpha channels\n");
	printf("\t -h,--help                         Show this message and exit\n");
}

static int init(int argc, char *argv[])
{
	int o;
	int options_index;

	memset(&globals, 0, sizeof(globals));
	memset(tarblock, 0, sizeof(tarblock));

	globals.in = stdin;
	globals.out = stdout;

	struct option long_options[] = {
		{"verbose",		no_argument,		NULL, 'v'},
		{"prefix",		required_argument,	NULL, 'p'},
		{"type",		required_argument,	NULL, 'p'},
		{"help",		no_argument,		NULL, 'h'},
		{"ignore-error",	no_argument,		NULL, 'e'},
		{"bitmapv5",		no_argument,		NULL, 'b'},
		{"input",		required_argument,	NULL, 'i'},
		{"output",		required_argument,	NULL, 'o'},
		{NULL,			0,			NULL,  0 },
	};

	while ((o = getopt_long(argc, argv, "vp:t:ebhi:o:", long_options, &options_index)) != -1) {
		switch (o) {
		case 'v':
			globals.verbose++;
			break;
		case 'p':
			globals.prefix = strdup(optarg);
			if (!globals.prefix) {
				fprintf(stderr, "Could not save prefix\n");
				return -ENOMEM;
			}
			break;
		case 'h':
			usage(argc, argv);
			exit(0);
			break;
		case 't':
			if (strcasecmp(optarg, "hires") == 0) {
				globals.type = INPUT_HIRES;
			} else if (strcasecmp(optarg, "tex") == 0) {
				globals.type = INPUT_TEX;
			} else {
				fprintf(stderr, "Invalid type %s\n", optarg);
				return -EINVAL;
			}
			break;
		case 'e':
			globals.ignore_error = 1;
			break;
		case 'b':
			globals.bitmapv5 = 1;
			break;
		case 'i':
			if (globals.in != stdin)
				fclose(globals.in);

			globals.in = fopen(optarg, "rb");
			if (!globals.in) {
				fprintf(stderr, "Could not open input file %s\n", optarg);
				return -ENOENT;
			}
			break;
		case 'o':
			if (globals.out != stdout)
				fclose(globals.in);

			globals.out = fopen(optarg, "wb");
			if (!globals.out) {
				fprintf(stderr, "Could not open output file %s\n", optarg);
				return -ENOENT;
			}
			break;
		default:
			usage(argc, argv);
			return -EINVAL;
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int ret;

	ret = init(argc, argv);
	if (ret < 0) {
		usage(argc, argv);
		return 1;
	}

	ret = convert_input();
	if (ret < 0)
		return 2;

	return 0;
}
