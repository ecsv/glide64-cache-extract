/* SPDX-License-Identifier: GPL-3.0-or-later */
/* glide64_cache_extract, Glide64 TexCache Extraction tool for debugging
 *
 * SPDX-FileCopyrightText: Sven Eckelmann <sven@narfation.org>
 */

#ifndef _GLIDE64_CACHE_EXTRACT_H_
#define _GLIDE64_CACHE_EXTRACT_H_

#if defined(__linux__) || defined(__CYGWIN__)

#define _BSD_SOURCE
#include <endian.h>

#elif defined(__WIN32__)

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

#define htole16
#define le16toh
#define htole32
#define le32toh
#define le64toh

#else /* __ORDER_LITTLE_ENDIAN__ */

#include <stddef.h>
#include <stdint.h>

static inline uint16_t htole16(uint16_t host_16bits)
{
	static const uint16_t order = 0x0001ULL;
	static const uint8_t *pos = (uint8_t *)&order;
	uint8_t *in = (uint8_t *)&host_16bits;
	uint16_t output;
	uint8_t *out = (uint8_t *)&output;
	size_t i;

	for (i = 0; i < sizeof(output); i++)
		out[sizeof(output) - 1 - i] = in[pos[i]];

	return output;
}

static inline uint16_t le16toh(uint16_t little_endian_16bits)
{
	static const uint16_t order = 0x0001ULL;
	static const uint8_t *pos = (uint8_t *)&order;
	uint8_t *in = (uint8_t *)&little_endian_16bits;
	uint16_t output;
	uint8_t *out = (uint8_t *)&output;
	size_t i;

	for (i = 0; i < sizeof(output); i++)
		out[pos[i]] = in[sizeof(output) - 1 - i];

	return output;
}

static inline uint32_t htole32(uint32_t host_32bits)
{
	static const uint32_t order = 0x00010203ULL;
	static const uint8_t *pos = (uint8_t *)&order;
	uint8_t *in = (uint8_t *)&host_32bits;
	uint32_t output;
	uint8_t *out = (uint8_t *)&output;
	size_t i;

	for (i = 0; i < sizeof(output); i++)
		out[sizeof(output) - 1 - i] = in[pos[i]];

	return output;
}

static inline uint32_t le32toh(uint32_t little_endian_32bits)
{
	static const uint32_t order = 0x00010203ULL;
	static const uint8_t *pos = (uint8_t *)&order;
	uint8_t *in = (uint8_t *)&little_endian_32bits;
	uint32_t output;
	uint8_t *out = (uint8_t *)&output;
	size_t i;

	for (i = 0; i < sizeof(output); i++)
		out[pos[i]] = in[sizeof(output) - 1 - i];

	return output;
}

static inline uint64_t le64toh(uint64_t little_endian_64bits)
{
	static const uint64_t order = 0x0001020304050607ULL;
	static const uint8_t *pos = (uint8_t *)&order;
	uint8_t *in = (uint8_t *)&little_endian_64bits;
	uint64_t output;
	uint8_t *out = (uint8_t *)&output;
	size_t i;

	for (i = 0; i < sizeof(output); i++)
		out[pos[i]] = in[sizeof(output) - 1 - i];

	return output;
}

#endif /* __ORDER_LITTLE_ENDIAN__ */

#else

#include <sys/endian.h>

#endif

#include <stddef.h>
#include <stdint.h>
#include <stdio.h> 

#define COMPRESSION_MASK    0x0000f000U
#define NO_COMPRESSION      0x00000000U
#define FXT1_COMPRESSION    0x00001000U
#define NCC_COMPRESSION     0x00002000U
#define S3TC_COMPRESSION    0x00003000U

#define HIRESTEXTURES_MASK  0x000f0000U
#define NO_HIRESTEXTURES    0x00000000U
#define GHQ_HIRESTEXTURES   0x00010000U
#define RICE_HIRESTEXTURES  0x00020000U
#define JABO_HIRESTEXTURES  0x00030000U

#define COMPRESS_HIRESTEX   0x00200000U
#define TILE_HIRESTEX       0x04000000U
#define FORCE16BPP_HIRESTEX 0x10000000U
#define GZ_HIRESTEXCACHE    0x00800000U
#define LET_TEXARTISTS_FLY  0x40000000U

#define FILTER_MASK         0x000000ffU
#define NO_FILTER           0x00000000U
#define SMOOTH_FILTER_MASK  0x0000000fU
#define NO_SMOOTH_FILTER    0x00000000U
#define SMOOTH_FILTER_1     0x00000001U
#define SMOOTH_FILTER_2     0x00000002U
#define SMOOTH_FILTER_3     0x00000003U
#define SMOOTH_FILTER_4     0x00000004U
#define SHARP_FILTER_MASK   0x000000f0U
#define NO_SHARP_FILTER     0x00000000U
#define SHARP_FILTER_1      0x00000010U
#define SHARP_FILTER_2      0x00000020U

#define ENHANCEMENT_MASK    0x00000f00U
#define NO_ENHANCEMENT      0x00000000U
#define X2_ENHANCEMENT      0x00000100U
#define X2SAI_ENHANCEMENT   0x00000200U
#define HQ2X_ENHANCEMENT    0x00000300U
#define LQ2X_ENHANCEMENT    0x00000400U
#define HQ4X_ENHANCEMENT    0x00000500U
#define HQ2XS_ENHANCEMENT   0x00000600U
#define LQ2XS_ENHANCEMENT   0x00000700U

#define COMPRESS_TEX        0x00100000U
#define FORCE16BPP_TEX      0x20000000U
#define GZ_TEXCACHE         0x00400000U

#define GR_TEXFMT_ALPHA_8            0x2U
#define GR_TEXFMT_INTENSITY_8        0x3U
#define GR_TEXFMT_ALPHA_INTENSITY_44 0x4U
#define GR_TEXFMT_P_8                0x5U
#define GR_TEXFMT_RGB_565            0xaU
#define GR_TEXFMT_ARGB_1555          0xbU
#define GR_TEXFMT_ARGB_4444          0xcU
#define GR_TEXFMT_ALPHA_INTENSITY_88 0xdU
#define GR_TEXFMT_ARGB_CMP_FXT1      0x11U
#define GR_TEXFMT_ARGB_8888          0x12U
#define GR_TEXFMT_ARGB_CMP_DXT1      0x16U
#define GR_TEXFMT_ARGB_CMP_DXT3      0x18U
#define GR_TEXFMT_ARGB_CMP_DXT5      0x1AU
#define GR_TEXFMT_GZ                 0x8000U

struct glide64_file {
	void *data;
	uint64_t checksum;
	uint32_t width;
	uint32_t height;
	uint32_t smallLodLog2;
	uint32_t largeLodLog2;
	uint32_t aspectRatioLog2;
	uint32_t tiles;
	uint32_t untiled_width;
	uint32_t untiled_height;
	uint32_t size;
	uint16_t format;
	uint8_t is_hires_tex;
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
	FILE *in;
	FILE *out;
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
