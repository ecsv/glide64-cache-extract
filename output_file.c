// SPDX-License-Identifier: GPL-3.0-or-later
/* glide64_cache_extract, Glide64 TexCache Extraction tool for debugging
 *
 * SPDX-FileCopyrightText: Sven Eckelmann <sven@narfation.org>
 */

#include "glide64_cache_extract.h"
#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

uint8_t tarblock[512];

int write_tarblock(void *buffer, size_t size, size_t offset)
{
	size_t ret, padding_size;

	ret = fwrite(buffer, 1, size, globals.out);
	if (ret != size) {
		fprintf(stderr, "Could not write file content\n");
		return -EIO;
	}

	padding_size = (offset + size) % sizeof(tarblock);
	if (padding_size) {
		padding_size = sizeof(tarblock) - padding_size;

		ret = fwrite(tarblock, 1, padding_size, globals.out);
		if (ret != padding_size) {
			fprintf(stderr, "Could not write padding\n");
			return -EIO;
		}
	}

	return 0;
}

static const char *image_extension(const struct glide64_file *file)
{
	switch (file->format) {
	case GR_TEXFMT_ALPHA_8:
	case GR_TEXFMT_INTENSITY_8:
	case GR_TEXFMT_ALPHA_INTENSITY_44:
	case GR_TEXFMT_P_8:
	case GR_TEXFMT_RGB_565:
	case GR_TEXFMT_ARGB_1555:
	case GR_TEXFMT_ARGB_4444:
	case GR_TEXFMT_ALPHA_INTENSITY_88:
	case GR_TEXFMT_ARGB_8888:
		return "bmp";
	case GR_TEXFMT_ARGB_CMP_FXT1:
		fprintf(stderr, "Unsupported format GR_TEXFMT_ARGB_CMP_FXT1\n");
		return "";
	case GR_TEXFMT_ARGB_CMP_DXT1:
	case GR_TEXFMT_ARGB_CMP_DXT3:
	case GR_TEXFMT_ARGB_CMP_DXT5:
		return "dds";
	default:
		fprintf(stderr, "Unsupported format %x\n", file->format);
		return "";
	}
}

int write_file(struct glide64_file *file)
{
	struct tar_header tarheader;
	uint8_t *raw_header;
	uint32_t checksum = 0;
	size_t i;
	int ret;

	memset(&tarheader, 0, sizeof(tarheader));

	/* TODO fix this test by identifying ci mode with palette, set fmt+size in name */
	if ((uint32_t)(file->checksum >> 32) != 0)
		snprintf(tarheader.name, sizeof(tarheader.name), "%s#%08"PRIX32"#%01"PRIX32"#%01"PRIX32"#%08"PRIX32"_ciByRGBA.%s", globals.prefix, (uint32_t)file->checksum, 3 , 0, (uint32_t)(file->checksum >> 32), image_extension(file));
	else
		snprintf(tarheader.name, sizeof(tarheader.name), "%s#%08"PRIX32"#%01"PRIX32"#%01"PRIX32"_all.%s", globals.prefix, (uint32_t)file->checksum, 3 , 0, image_extension(file));

	tarheader.name[sizeof(tarheader.name) - 1] = '\0';

	strcpy(tarheader.mode, "0000644");
	strcpy(tarheader.uid, "0000000");
	strcpy(tarheader.gid, "0000000");

	snprintf(tarheader.size, sizeof(tarheader.size), "%011"PRIo32, file->size);
	tarheader.size[sizeof(tarheader.size) - 1] = '\0';

	snprintf(tarheader.mtime, sizeof(tarheader.mtime), "%011o", 1);
	tarheader.mtime[sizeof(tarheader.mtime) - 1] = '\0';
	memset(tarheader.chksum, ' ', sizeof(tarheader.chksum));
	tarheader.link = 0;

	raw_header = (void *)&tarheader;
	for (i = 0; i < sizeof(tarheader); i++)
		checksum += raw_header[i];
	checksum %= 0x40000U;

	snprintf(tarheader.chksum, sizeof(tarheader.chksum) - 1, "%06"PRIo32, checksum);

	ret = write_tarblock(&tarheader, sizeof(tarheader), 0);
	if (ret < 0) {
		fprintf(stderr, "Failed to write tar header\n");
		return ret;
	}

	ret = write_tarblock(file->data, file->size, 0);
	if (ret < 0) {
		fprintf(stderr, "Failed to write file content\n");
		return ret;
	}

	return 0;
}
