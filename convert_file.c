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

#include "glide64_cache_extract.h"

#define DDSD_CAPS		0x00000001U
#define DDSD_HEIGHT		0x00000002U
#define DDSD_WIDTH		0x00000004U
#define DDSD_PITCH		0x00000008U
#define DDSD_PIXELFORMAT	0x00001000U
#define DDSD_LINEARSIZE		0x00080000U

#define DDPF_ALPHAPIXELS	0x00000001U
#define DDPF_FOURCC		0x00000004U
#define DDPF_RGB		0x00000040U
#define DDPF_LUMINANCE		0x00000040U

#define DDSCAPS_TEXTURE		0x00001000U

typedef struct {
	uint32_t dwSize;
	uint32_t dwFlags;
	uint32_t dwFourCC;
	uint32_t dwRGBBitCount;
	uint32_t dwRBitMask;
	uint32_t dwGBitMask;
	uint32_t dwBBitMask;
	uint32_t dwABitMask;
} DDS_PIXELFORMAT;

typedef struct {
	uint32_t           dwMagic;
	uint32_t           dwSize;
	uint32_t           dwFlags;
	uint32_t           dwHeight;
	uint32_t           dwWidth;
	uint32_t           dwPitchOrLinearSize;
	uint32_t           dwDepth;
	uint32_t           dwMipMapCount;
	uint32_t           dwReserved1[11];
	DDS_PIXELFORMAT    ddspf;
	uint32_t           dwCaps;
	uint32_t           dwCaps2;
	uint32_t           dwCaps3;
	uint32_t           dwCaps4;
	uint32_t           dwReserved2;
} DDS_HEADER;

#pragma pack(push, 1)
struct bmp_header {
	int16_t identifier;
	uint32_t filesize;
	uint32_t reserved;
	uint32_t dataofs;
	uint32_t headersize;
	uint32_t width;
	uint32_t height;
	int16_t planes;
	int16_t bitperpixel;
	uint32_t compression;
	uint32_t datasize;
	uint32_t hresolution;
	uint32_t vresolution;
	uint32_t colors;
	uint32_t importantcolors;
};

struct bmp_header_v5 {
	int16_t identifier;
	uint32_t filesize;
	uint32_t reserved;
	uint32_t dataofs;
	uint32_t headersize;
	uint32_t width;
	uint32_t height;
	uint16_t planes;
	uint16_t bitperpixel;
	uint32_t compression;
	uint32_t datasize;
	uint32_t hresolution;
	uint32_t vresolution;
	uint32_t colors;
	uint32_t importantcolors;
	uint32_t redmask;
	uint32_t greenmask;
	uint32_t bluemask;
	uint32_t alphamask;
	uint32_t colorspace;
	uint32_t ciexyz_red_x;
	uint32_t ciexyz_red_y;
	uint32_t ciexyz_red_z;
	uint32_t ciexyz_green_x;
	uint32_t ciexyz_green_y;
	uint32_t ciexyz_green_z;
	uint32_t ciexyz_blue_x;
	uint32_t ciexyz_blue_y;
	uint32_t ciexyz_blue_z;
	uint32_t gamma_red;
	uint32_t gamma_green;
	uint32_t gamma_blue;
	uint32_t intent;
	uint32_t profiledata_offset;
	uint32_t profiledata_size;
	uint32_t reserved2;
};
#pragma pack(pop)

static size_t image_content_length(const struct glide64_file *file)
{
#define BALIGN(x, a) (((x) + (a)) & ~(a))
	size_t size;

	switch (file->format & ~GR_TEXFMT_GZ) {
	case GR_TEXFMT_ALPHA_8:
	case GR_TEXFMT_INTENSITY_8:
	case GR_TEXFMT_ALPHA_INTENSITY_44:
	case GR_TEXFMT_P_8:
		size = file->width * file->height;
		break;
	case GR_TEXFMT_RGB_565:
	case GR_TEXFMT_ARGB_1555:
	case GR_TEXFMT_ARGB_4444:
	case GR_TEXFMT_ALPHA_INTENSITY_88:
		size = file->width * file->height * 2;
		break;
	case GR_TEXFMT_ARGB_CMP_FXT1:
		size = (BALIGN(file->width, 7U) * BALIGN(file->height, 3U)) / 2;
		break;
	case GR_TEXFMT_ARGB_8888:
		size = file->width * file->height * 4;
		break;
	case GR_TEXFMT_ARGB_CMP_DXT1:
		size = BALIGN(file->width, 3U) * BALIGN(file->height, 3U);
		break;
	case GR_TEXFMT_ARGB_CMP_DXT3:
	case GR_TEXFMT_ARGB_CMP_DXT5:
		size = BALIGN(file->width, 3U) * BALIGN(file->height, 3U) * 2;
		break;
	default:
		size = 0;
		fprintf(stderr, "Unsupported format %#"PRIx16"\n", file->format);
		break;
	}

	return size;
#undef BALIGN
}

static int resize_image_dds(struct glide64_file *file)
{
	DDS_HEADER *header;
	size_t header_size = 128;
	uint8_t *buf;

	buf = malloc(file->size + header_size);
	if (!buf) {
		fprintf(stderr, "Memory for DDS file couldn't be allocated\n");
		return -ENOMEM;
	}

	header = (DDS_HEADER *)buf;
	memset(header, 0, header_size);
	header->dwMagic = htole32(0x20534444U);
	header->dwSize = htole32(124);
	header->dwFlags = htole32(DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT);
	header->dwHeight = htole32(file->height);
	header->dwWidth = htole32(file->width);
	header->dwMipMapCount = htole32(1);
	header->dwCaps = htole32(DDSCAPS_TEXTURE);

	header->ddspf.dwSize = htole32(32);
	switch (file->format) {
	case GR_TEXFMT_ALPHA_8:
		header->dwFlags |= DDSD_PITCH;
		header->dwPitchOrLinearSize = htole32(file->width);
		header->ddspf.dwFlags = htole32(DDPF_ALPHAPIXELS);
		header->ddspf.dwRGBBitCount = htole32(8);
		header->ddspf.dwABitMask = htole32(0xffU);
		break;
	case GR_TEXFMT_INTENSITY_8:
		header->dwFlags |= DDSD_PITCH;
		header->dwPitchOrLinearSize = htole32(file->width);
		header->ddspf.dwFlags = htole32(DDPF_LUMINANCE);
		header->ddspf.dwRGBBitCount = htole32(8);
		header->ddspf.dwRBitMask = htole32(0xffU);
		break;
	case GR_TEXFMT_ALPHA_INTENSITY_44:
		header->dwFlags |= DDSD_PITCH;
		header->dwPitchOrLinearSize = htole32(file->width);
		header->ddspf.dwFlags = htole32(DDPF_ALPHAPIXELS | DDPF_LUMINANCE);
		header->ddspf.dwRGBBitCount = htole32(8);
		header->ddspf.dwRBitMask = htole32(0x0fU);
		header->ddspf.dwABitMask = htole32(0xf0U);
		break;
	case GR_TEXFMT_P_8:
		fprintf(stderr, "Unsupported format GR_TEXFMT_P_8\n");
		return -EPERM;
	case GR_TEXFMT_RGB_565:
		header->dwFlags |= DDSD_PITCH;
		header->dwPitchOrLinearSize = htole32(file->width * 2);
		header->ddspf.dwFlags = htole32(DDPF_RGB);
		header->ddspf.dwRGBBitCount = htole32(16);
		header->ddspf.dwRBitMask = htole32(0xf800U);
		header->ddspf.dwGBitMask = htole32(0x07e0U);
		header->ddspf.dwBBitMask = htole32(0x001fU);
		break;
	case GR_TEXFMT_ARGB_1555:
		header->dwFlags |= DDSD_PITCH;
		header->dwPitchOrLinearSize = htole32(file->width * 2);
		header->ddspf.dwFlags = htole32(DDPF_ALPHAPIXELS | DDPF_RGB);
		header->ddspf.dwRGBBitCount = htole32(16);
		header->ddspf.dwRBitMask = htole32(0x7c00U);
		header->ddspf.dwGBitMask = htole32(0x03e0U);
		header->ddspf.dwBBitMask = htole32(0x001fU);
		header->ddspf.dwABitMask = htole32(0x8000U);
		break;
	case GR_TEXFMT_ARGB_4444:
		header->dwFlags |= DDSD_PITCH;
		header->dwPitchOrLinearSize = htole32(file->width * 2);
		header->ddspf.dwFlags = htole32(DDPF_ALPHAPIXELS | DDPF_RGB);
		header->ddspf.dwRGBBitCount = htole32(16);
		header->ddspf.dwRBitMask = htole32(0x7c00U);
		header->ddspf.dwGBitMask = htole32(0x03e0U);
		header->ddspf.dwBBitMask = htole32(0x001fU);
		header->ddspf.dwABitMask = htole32(0x8000U);
		break;
	case GR_TEXFMT_ALPHA_INTENSITY_88:
		header->dwFlags |= DDSD_PITCH;
		header->dwPitchOrLinearSize = htole32(file->width * 2);
		header->ddspf.dwFlags = htole32(DDPF_ALPHAPIXELS | DDPF_LUMINANCE);
		header->ddspf.dwRGBBitCount = htole32(16);
		header->ddspf.dwRBitMask = htole32(0x00ffU);
		header->ddspf.dwABitMask = htole32(0xff00U);
		break;
	case GR_TEXFMT_ARGB_CMP_FXT1:
		fprintf(stderr, "Unsupported format GR_TEXFMT_ARGB_CMP_FXT1\n");
		return -EPERM;
	case GR_TEXFMT_ARGB_8888:
		header->dwFlags |= DDSD_PITCH;
		header->dwPitchOrLinearSize = htole32(file->width * 4);
		header->ddspf.dwFlags = htole32(DDPF_ALPHAPIXELS | DDPF_RGB);
		header->ddspf.dwFourCC = htole32(0);
		header->ddspf.dwRGBBitCount = htole32(32);
		header->ddspf.dwRBitMask = htole32(0x00ff0000U);
		header->ddspf.dwGBitMask = htole32(0x0000ff00U);
		header->ddspf.dwBBitMask = htole32(0x000000ffU);
		header->ddspf.dwABitMask = htole32(0xff000000U);
		break;
	case GR_TEXFMT_ARGB_CMP_DXT1:
		header->dwFlags |= DDSD_LINEARSIZE;
		header->dwPitchOrLinearSize = htole32(file->size);
		header->ddspf.dwFlags = htole32(DDPF_FOURCC);
		header->ddspf.dwFourCC = htole32(0x31545844U);
		header->ddspf.dwRGBBitCount = htole32(24);
		header->ddspf.dwRBitMask = htole32(0x00ff0000U);
		header->ddspf.dwGBitMask = htole32(0x0000ff00U);
		header->ddspf.dwBBitMask = htole32(0x000000ffU);
		break;
	case GR_TEXFMT_ARGB_CMP_DXT3:
		header->dwFlags |= DDSD_LINEARSIZE;
		header->dwPitchOrLinearSize = htole32(file->size);
		header->ddspf.dwFlags = htole32(DDPF_FOURCC);
		header->ddspf.dwFourCC = htole32(0x33545844U);
		header->ddspf.dwRGBBitCount = htole32(24);
		header->ddspf.dwRBitMask = htole32(0x00ff0000U);
		header->ddspf.dwGBitMask = htole32(0x0000ff00U);
		header->ddspf.dwBBitMask = htole32(0x000000ffU);
		break;
	case GR_TEXFMT_ARGB_CMP_DXT5:
		header->dwFlags |= DDSD_LINEARSIZE;
		header->dwPitchOrLinearSize = htole32(file->size);
		header->ddspf.dwFlags = htole32(DDPF_FOURCC);
		header->ddspf.dwFourCC = htole32(0x35545844U);
		header->ddspf.dwRGBBitCount = htole32(24);
		header->ddspf.dwRBitMask = htole32(0x00ff0000U);
		header->ddspf.dwGBitMask = htole32(0x0000ff00U);
		header->ddspf.dwBBitMask = htole32(0x000000ffU);
		break;
	default:
		fprintf(stderr, "Unsupported format %x\n", file->format);
		return -EPERM;
	}

	memcpy(buf + header_size, file->data, file->size);
	free(file->data);
	file->data = buf;
	file->size += header_size;

	return 0;
}

static int resize_image_bmp(struct glide64_file *file)
{
	struct bmp_header *header;
	struct bmp_header_v5 *header_v5;
	size_t header_size;
	uint8_t *buf;
	uint8_t *imagedata;
	size_t line_size = file->width * 4;
	uint32_t i;

	if (file->format != GR_TEXFMT_ARGB_8888) {
		fprintf(stderr, "Unsupported texture format %#x for bmp export\n", file->format);
		return -EPERM;
	}

	if (globals.bitmapv5)
		header_size = sizeof(*header_v5);
	else
		header_size = sizeof(*header);

	buf = malloc(file->size + header_size);
	if (!buf) {
		fprintf(stderr, "Memory for BMP file couldn't be allocated\n");
		return -ENOMEM;
	}

	if (globals.bitmapv5) {
		header_v5 = (struct bmp_header_v5 *)buf;
		memset(header_v5, 0, header_size);
		header_v5->identifier = htole16(0x4d42U);
		header_v5->filesize = htole32(file->size + header_size);
		header_v5->dataofs = htole32(header_size);
		header_v5->headersize = htole32(header_size - 14);
		header_v5->width = htole32(file->width);
		header_v5->height = htole32(file->height);
		header_v5->planes = htole16(1);
		header_v5->bitperpixel = htole16(32);
		header_v5->compression = htole32(3);
		header_v5->datasize = htole32(file->size);
		header_v5->hresolution = htole32(2835);
		header_v5->vresolution = htole32(2835);
		header_v5->colors = htole32(0);
		header_v5->importantcolors = htole32(0);
		header_v5->redmask = htole32(0x00ff0000U);
		header_v5->greenmask = htole32(0x0000ff00U);
		header_v5->bluemask = htole32(0x000000ffU);
		header_v5->alphamask = htole32(0xff000000U);
		header_v5->colorspace = htole32(0x73524742U);
		header_v5->ciexyz_red_x = htole32(0x00000000U);
		header_v5->ciexyz_red_y = htole32(0x00000000U);
		header_v5->ciexyz_red_z = htole32(0xfc1eb854U);
		header_v5->ciexyz_green_x = htole32(0x00000000U);
		header_v5->ciexyz_green_y = htole32(0x00000000U);
		header_v5->ciexyz_green_z = htole32(0xfc666666U);
		header_v5->ciexyz_blue_x = htole32(0x00000000U);
		header_v5->ciexyz_blue_y = htole32(0x00000000U);
		header_v5->ciexyz_blue_z = htole32(0xff28f5c4U);
		header_v5->gamma_red = htole32(0);
		header_v5->gamma_green = htole32(0);
		header_v5->intent = htole32(4);
		header_v5->profiledata_offset = htole32(4);
		header_v5->profiledata_size = htole32(4);
	} else {
		header = (struct bmp_header *)buf;
		memset(header, 0, header_size);
		header->identifier = htole16(0x4d42U);
		header->filesize = htole32(file->size + header_size);
		header->dataofs = htole32(header_size);
		header->headersize = htole32(header_size - 14);
		header->width = htole32(file->width);
		header->height = htole32(file->height);
		header->planes = htole16(1);
		header->bitperpixel = htole16(32);
		header->compression = htole32(0);
		header->datasize = htole32(file->size);
		header->hresolution = htole32(2835);
		header->vresolution = htole32(2835);
		header->colors = htole32(0);
		header->importantcolors = htole32(0);
	}

	imagedata = buf + header_size;
	for (i = 0; i < file->height; i++) {
		uint32_t target_line = i;
		uint32_t source_line = file->height - i - 1;
		uint8_t *target_pos = imagedata + target_line * line_size;
		uint8_t *source_pos = file->data + source_line * line_size;

		memcpy(target_pos, source_pos, line_size);
	}
	free(file->data);
	file->data = buf;
	file->size += header_size;

	return 0;
}

static int normalize_image_a8(struct glide64_file *file)
{
	uint32_t *buf;
	uint8_t *data, raw;
	size_t newsize;
	size_t pixels, pos;
	uint32_t p, a;

	pixels = file->width * file->height;
	newsize = pixels * 4;
	buf = malloc(newsize);
	if (!buf) {
		fprintf(stderr, "Memory for A8 image content couldn't be allocated\n");
		return -ENOMEM;
	}

	data = file->data;
	for (pos = 0; pos < pixels; pos++) {
		raw = data[pos];
		a = raw;
		p = (a << 24) | (a << 16) | (a << 8) | a;
		buf[pos] = htole32(p);
	}

	free(file->data);
	file->data = (uint8_t *)buf;
	file->size = newsize;
	file->format = GR_TEXFMT_ARGB_8888;

	return 0;
}

static int normalize_image_i8(struct glide64_file *file)
{
	uint32_t *buf;
	uint8_t *data, raw;
	size_t newsize;
	size_t pixels, pos;
	uint32_t p, i;

	pixels = file->width * file->height;
	newsize = pixels * 4;
	buf = malloc(newsize);
	if (!buf) {
		fprintf(stderr, "Memory for I8 image content couldn't be allocated\n");
		return -ENOMEM;
	}

	data = file->data;
	for (pos = 0; pos < pixels; pos++) {
		raw = data[pos];
		i = raw;
		p = (i << 24) | (i << 16) | (i << 8) | i;
		buf[pos] = htole32(p);
	}

	free(file->data);
	file->data = (uint8_t *)buf;
	file->size = newsize;
	file->format = GR_TEXFMT_ARGB_8888;

	return 0;
}

static int normalize_image_a4i4(struct glide64_file *file)
{
	uint32_t *buf;
	uint8_t *data, raw;
	size_t newsize;
	size_t pixels, pos;
	uint32_t p, a, i;

	pixels = file->width * file->height;
	newsize = pixels * 4;
	buf = malloc(newsize);
	if (!buf) {
		fprintf(stderr, "Memory for A4I4 image content couldn't be allocated\n");
		return -ENOMEM;
	}

	data = file->data;
	for (pos = 0; pos < pixels; pos++) {
		raw = data[pos];
		i = (raw & 0x0fU) << 4;
		i |= i >> 4;
		a = (raw & 0xf0U);
		a |= a >> 4;
		p = (a << 24) | (i << 16) | (i << 8) | i;
		buf[pos] = htole32(p);
	}

	free(file->data);
	file->data = (uint8_t *)buf;
	file->size = newsize;
	file->format = GR_TEXFMT_ARGB_8888;

	return 0;
}

static int normalize_image_p8(struct glide64_file *file __attribute__((unused)))
{
	fprintf(stderr, "Unsupported format GR_TEXFMT_P_8\n");
	return -EPERM;
}

static int normalize_image_r5g6b5(struct glide64_file *file)
{
	uint32_t *buf;
	uint16_t *data, raw;
	size_t newsize;
	size_t pixels, pos;
	uint32_t p, r, g, b;

	pixels = file->width * file->height;
	newsize = pixels * 4;
	buf = malloc(newsize);
	if (!buf) {
		fprintf(stderr, "Memory for R5G6B5 image content couldn't be allocated\n");
		return -ENOMEM;
	}

	data = (uint16_t *)file->data;
	for (pos = 0; pos < pixels; pos++) {
		raw = le16toh(data[pos]);
		r = (raw & 0xf800U) >> 8;
		r |= r >> 5;
		g = (raw & 0x07e0U) >> 3;
		g |= g >> 6;
		b = (raw & 0x001fU) << 3;
		b |= b >> 5;
		p = (0xffU << 24) | (r << 16) | (g << 8) | b;
		buf[pos] = htole32(p);
	}

	free(file->data);
	file->data = (uint8_t *)buf;
	file->size = newsize;
	file->format = GR_TEXFMT_ARGB_8888;

	return 0;
}

static int normalize_image_a1r5g5b5(struct glide64_file *file)
{
	uint32_t *buf;
	uint16_t *data, raw;
	size_t newsize;
	size_t pixels, pos;
	uint32_t p, a, r, g, b;

	pixels = file->width * file->height;
	newsize = pixels * 4;
	buf = malloc(newsize);
	if (!buf) {
		fprintf(stderr, "Memory for A1R5G5B5 image content couldn't be allocated\n");
		return -ENOMEM;
	}

	data = (uint16_t *)file->data;
	for (pos = 0; pos < pixels; pos++) {
		raw = le16toh(data[pos]);
		a = (raw & 0x8000U) >> 15;
		a *= 0xffU;
		r = (raw & 0x7c00U) >> 7;
		r |= r >> 5;
		g = (raw & 0x03e0U) >> 2;
		g |= g >> 5;
		b = (raw & 0x001fU) << 3;
		b |= b >> 5;
		p = (a << 24) | (r << 16) | (g << 8) | b;
		buf[pos] = htole32(p);
	}

	free(file->data);
	file->data = (uint8_t *)buf;
	file->size = newsize;
	file->format = GR_TEXFMT_ARGB_8888;

	return 0;
}

static int normalize_image_a4r4g4b4(struct glide64_file *file)
{
	uint32_t *buf;
	uint16_t *data, raw;
	size_t newsize;
	size_t pixels, pos;
	uint32_t p, a, r, g, b;

	pixels = file->width * file->height;
	newsize = pixels * 4;
	buf = malloc(newsize);
	if (!buf) {
		fprintf(stderr, "Memory for A4R4G4B4 image content couldn't be allocated\n");
		return -ENOMEM;
	}

	data = (uint16_t *)file->data;
	for (pos = 0; pos < pixels; pos++) {
		raw = le16toh(data[pos]);
		a = (raw & 0xf000U) >> 4;
		a |= a >> 4;
		r = (raw & 0x0f00U) >> 4;
		r |= r >> 4;
		g = (raw & 0x00f0U);
		g |= g >> 4;
		b = (raw & 0x000fU) << 4;
		b |= b >> 4;
		p = (a << 24) | (r << 16) | (g << 8) | b;
		buf[pos] = htole32(p);
	}

	free(file->data);
	file->data = (uint8_t *)buf;
	file->size = newsize;
	file->format = GR_TEXFMT_ARGB_8888;

	return 0;
}

static int normalize_image_a8i8(struct glide64_file *file)
{
	uint32_t *buf;
	uint16_t *data, raw;
	size_t newsize;
	size_t pixels, pos;
	uint32_t p, a, i;

	pixels = file->width * file->height;
	newsize = pixels * 4;
	buf = malloc(newsize);
	if (!buf) {
		fprintf(stderr, "Memory for A4R4G4B4 image content couldn't be allocated\n");
		return -ENOMEM;
	}

	data = (uint16_t *)file->data;
	for (pos = 0; pos < pixels; pos++) {
		raw = le16toh(data[pos]);
		a = (raw & 0xff00U) >> 8;
		i = (raw & 0x00ffU);
		p = (a << 24) | (i << 16) | (i << 8) | i;
		buf[pos] = htole32(p);
	}

	free(file->data);
	file->data = (uint8_t *)buf;
	file->size = newsize;
	file->format = GR_TEXFMT_ARGB_8888;

	return 0;
}

static int resize_image_content(struct glide64_file *file)
{
	int ret;

	switch (file->format) {
	case GR_TEXFMT_ALPHA_8:
		ret = normalize_image_a8(file);
		if (ret < 0) {
			fprintf(stderr, "Error during conversion from ALPHA_8 to ARGB_8888\n");
			return ret;
		}

		return resize_image_bmp(file);
	case GR_TEXFMT_INTENSITY_8:
		ret = normalize_image_i8(file);
		if (ret < 0) {
			fprintf(stderr, "Error during conversion from INTENSITY_8 to ARGB_8888\n");
			return ret;
		}

		return resize_image_bmp(file);
	case GR_TEXFMT_ALPHA_INTENSITY_44:
		ret = normalize_image_a4i4(file);
		if (ret < 0) {
			fprintf(stderr, "Error during conversion from INTENSITY_44 to ARGB_8888\n");
			return ret;
		}

		return resize_image_bmp(file);
	case GR_TEXFMT_P_8:
		ret = normalize_image_p8(file);
		if (ret < 0) {
			fprintf(stderr, "Error during conversion from P_8 to ARGB_8888\n");
			return ret;
		}

		return resize_image_bmp(file);
	case GR_TEXFMT_RGB_565:
		ret = normalize_image_r5g6b5(file);
		if (ret < 0) {
			fprintf(stderr, "Error during conversion from RGB_565 to ARGB_8888\n");
			return ret;
		}

		return resize_image_bmp(file);
	case GR_TEXFMT_ARGB_1555:
		ret = normalize_image_a1r5g5b5(file);
		if (ret < 0) {
			fprintf(stderr, "Error during conversion from ARGB_1555 to ARGB_8888\n");
			return ret;
		}

		return resize_image_bmp(file);
	case GR_TEXFMT_ARGB_4444:
		ret = normalize_image_a4r4g4b4(file);
		if (ret < 0) {
			fprintf(stderr, "Error during conversion from ARGB_4444 to ARGB_8888\n");
			return ret;
		}

		return resize_image_bmp(file);
	case GR_TEXFMT_ALPHA_INTENSITY_88:
		ret = normalize_image_a8i8(file);
		if (ret < 0) {
			fprintf(stderr, "Error during conversion from ALPHA_INTENSITY_88 to ARGB_8888\n");
			return ret;
		}

		return resize_image_bmp(file);
	case GR_TEXFMT_ARGB_CMP_FXT1:
		fprintf(stderr, "Unsupported format GR_TEXFMT_ARGB_CMP_FXT1\n");
		return -EPERM;
	case GR_TEXFMT_ARGB_8888:
		return resize_image_bmp(file);
	case GR_TEXFMT_ARGB_CMP_DXT1:
	case GR_TEXFMT_ARGB_CMP_DXT3:
	case GR_TEXFMT_ARGB_CMP_DXT5:
		return resize_image_dds(file);
	default:
		fprintf(stderr, "Unsupported format %x\n", file->format);
		return -EPERM;
	}

	return 0;
}

int prepare_file(struct glide64_file *file)
{
	size_t expected_size;
	uint8_t *buf;
	uLongf destLen;
	int ret;

	expected_size = image_content_length(file);
	if (file->format & GR_TEXFMT_GZ) {
		destLen = expected_size + 4096;
		buf = malloc(destLen);
		if (!buf) {
			fprintf(stderr, "Memory for uncompressing the file couldn't be allocated\n");
			return -ENOMEM;
		}

		ret = uncompress(buf, &destLen, file->data, file->size);
		if (ret != Z_OK) {
			free(buf);
			fprintf(stderr, "Failure during decompressing\n");
			return -EINVAL;
		}

		if (expected_size != destLen) {
			free(buf);
			fprintf(stderr, "Decompressed file has wrong filesize\n");
			return -EINVAL;
		}

		file->format &= ~GR_TEXFMT_GZ;
		free(file->data);
		file->data = buf;
		file->size = expected_size;
	} else {
		if (expected_size != file->size) {
			fprintf(stderr, "Expected size of file is not the actual file size\n");
			return -EINVAL;
		}
	}

	ret = resize_image_content(file);
	if (ret < 0) {
		fprintf(stderr, "Failed to prepare image content\n");
		return ret;
	}

	return 0;
}
