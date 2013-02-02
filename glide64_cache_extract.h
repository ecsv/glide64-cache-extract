#define _BSD_SOURCE
#include <endian.h>
#include <stdio.h>
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

#ifndef _GLIDE64_CACHE_EXTRACT_H_
#define _GLIDE64_CACHE_EXTRACT_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <inttypes.h>
#include <zlib.h>

#define COMPRESSION_MASK    0x0000f000
#define NO_COMPRESSION      0x00000000
#define FXT1_COMPRESSION    0x00001000
#define NCC_COMPRESSION     0x00002000
#define S3TC_COMPRESSION    0x00003000

#define HIRESTEXTURES_MASK  0x000f0000
#define NO_HIRESTEXTURES    0x00000000
#define GHQ_HIRESTEXTURES   0x00010000
#define RICE_HIRESTEXTURES  0x00020000
#define JABO_HIRESTEXTURES  0x00030000

#define COMPRESS_HIRESTEX   0x00200000
#define TILE_HIRESTEX       0x04000000
#define FORCE16BPP_HIRESTEX 0x10000000
#define GZ_HIRESTEXCACHE    0x00800000
#define LET_TEXARTISTS_FLY  0x40000000

#define FILTER_MASK         0x000000ff
#define NO_FILTER           0x00000000
#define SMOOTH_FILTER_MASK  0x0000000f
#define NO_SMOOTH_FILTER    0x00000000
#define SMOOTH_FILTER_1     0x00000001
#define SMOOTH_FILTER_2     0x00000002
#define SMOOTH_FILTER_3     0x00000003
#define SMOOTH_FILTER_4     0x00000004
#define SHARP_FILTER_MASK   0x000000f0
#define NO_SHARP_FILTER     0x00000000
#define SHARP_FILTER_1      0x00000010
#define SHARP_FILTER_2      0x00000020

#define ENHANCEMENT_MASK    0x00000f00
#define NO_ENHANCEMENT      0x00000000
#define X2_ENHANCEMENT      0x00000100
#define X2SAI_ENHANCEMENT   0x00000200
#define HQ2X_ENHANCEMENT    0x00000300
#define LQ2X_ENHANCEMENT    0x00000400
#define HQ4X_ENHANCEMENT    0x00000500
#define HQ2XS_ENHANCEMENT   0x00000600
#define LQ2XS_ENHANCEMENT   0x00000700

#define COMPRESS_TEX        0x00100000
#define FORCE16BPP_TEX      0x20000000
#define GZ_TEXCACHE         0x00400000

#define GR_TEXFMT_ALPHA_8            0x2
#define GR_TEXFMT_INTENSITY_8        0x3
#define GR_TEXFMT_ALPHA_INTENSITY_44 0x4
#define GR_TEXFMT_P_8                0x5
#define GR_TEXFMT_RGB_565            0xa
#define GR_TEXFMT_ARGB_1555          0xb
#define GR_TEXFMT_ARGB_4444          0xc
#define GR_TEXFMT_ALPHA_INTENSITY_88 0xd
#define GR_TEXFMT_ARGB_CMP_FXT1      0x11
#define GR_TEXFMT_ARGB_8888          0x12
#define GR_TEXFMT_ARGB_CMP_DXT1      0x16
#define GR_TEXFMT_ARGB_CMP_DXT3      0x18
#define GR_TEXFMT_ARGB_CMP_DXT5      0x1A
#define GR_TEXFMT_GZ                 0x8000

struct glide64_file {
	uint64_t checksum;
	uint32_t width;
	uint32_t height;
	uint16_t format;
	uint32_t smallLodLog2;
	uint32_t largeLodLog2;
	uint32_t aspectRatioLog2;
	uint32_t tiles;
	uint32_t untiled_width;
	uint32_t untiled_height;
	uint8_t is_hires_tex;
	uint32_t size;

	uint8_t *data;
};

enum verbosity_level {
	VERBOSITY_GLOBAL_HEADER = 1,
	VERBOSITY_FILE_HEADER = 2,
};

enum input_type {
	INPUT_UNKNOWN = 0,
	INPUT_HIRES,
	INPUT_TEX,
};

extern uint8_t tarblock[512];

struct _globals {
	int verbose;
	enum input_type type;
	int ignore_error;
	int bitmapv5;
	char *prefix;
};
extern struct _globals globals;

struct tar_header {
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char chksum[8];
	char link;
	char linkname[100];
};

int parse_config(uint32_t config);

int convert_file(void);
int get_buffer_endian(void *buffer, size_t size, int print_error);
#define get_item(x) get_buffer_endian(&x, sizeof(x), 1)
int prepare_file(struct glide64_file *file);
int write_tarblock(void *buffer, size_t size, size_t offset);
int write_file(struct glide64_file *file);

#endif