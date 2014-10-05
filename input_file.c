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
#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static int get_buffer(void *buffer, size_t size, int print_error)
{
	size_t ret;

	ret = fread(buffer, 1, size, globals.in);

	if (ret == size)
		return 0;

	if (ferror(globals.in) && print_error)
		fprintf(stderr, "Error while reading input\n");

	if (feof(globals.in) && print_error)
		fprintf(stderr, "File stream ended to early\n");

	return -EIO;
}

int get_buffer_endian(void *buffer, size_t size, int print_error)
{
	int ret;

	ret = get_buffer(buffer, size, print_error);
	if (ret < 0)
		return ret;


	switch (size) {
	case 1:
		/* no endian problems */
		break;
	case 2:
		*(uint16_t *)buffer = le16toh(*(uint16_t *)buffer);
		break;
	case 4:
		*(uint32_t *)buffer = le32toh(*(uint32_t *)buffer);
		break;
	case 8:
		*(uint64_t *)buffer = le64toh(*(uint64_t *)buffer);
		break;
	default:
		fprintf(stderr, "Invalid item size %u for endianness conversion\n", (unsigned int)size);
		return -EINVAL;
	}

	return 0;
}

int convert_file(void)
{
	struct glide64_file file;
	int ret;
	long pos = ftell(globals.in);

	ret = get_buffer_endian(&file.checksum, sizeof(file.checksum), 0);
	if (ret < 0)
		return 0;

	ret = get_item(file.width);
	if (ret < 0) {
		fprintf(stderr, "Failed to read file width\n");
		return ret;
	}

	ret = get_item(file.height);
	if (ret < 0) {
		fprintf(stderr, "Failed to read file height\n");
		return ret;
	}

	ret = get_item(file.format);
	if (ret < 0) {
		fprintf(stderr, "Failed to read file format\n");
		return ret;
	}

	ret = get_item(file.smallLodLog2);
	if (ret < 0) {
		fprintf(stderr, "Failed to read file smallLodLog2\n");
		return ret;
	}

	ret = get_item(file.largeLodLog2);
	if (ret < 0) {
		fprintf(stderr, "Failed to read file largeLodLog2\n");
		return ret;
	}

	ret = get_item(file.aspectRatioLog2);
	if (ret < 0) {
		fprintf(stderr, "Failed to read file aspectRatioLog2\n");
		return ret;
	}

	ret = get_item(file.tiles);
	if (ret < 0) {
		fprintf(stderr, "Failed to read file tiles\n");
		return ret;
	}

	ret = get_item(file.untiled_width);
	if (ret < 0) {
		fprintf(stderr, "Failed to read file untiled_width\n");
		return ret;
	}

	ret = get_item(file.untiled_height);
	if (ret < 0) {
		fprintf(stderr, "Failed to read file untiled_height\n");
		return ret;
	}

	ret = get_item(file.is_hires_tex);
	if (ret < 0) {
		fprintf(stderr, "Failed to read file is_hires_tex\n");
		return ret;
	}

	ret = get_item(file.size);
	if (ret < 0) {
		fprintf(stderr, "Failed to read filesize\n");
		return ret;
	}

	if (globals.verbose >= VERBOSITY_FILE_HEADER) {
		if (pos >= 0 && globals.in != stdin)
			fprintf(stderr, "Offset: %#lx\n", pos);

		fprintf(stderr, "File header:\n");
		fprintf(stderr, "\tchecksum: 0x%016"PRIX64"\n", file.checksum);
		fprintf(stderr, "\twidth: %"PRIu32"\n", file.width);
		fprintf(stderr, "\theight: %"PRIu32"\n", file.height);
		fprintf(stderr, "\tformat: %#"PRIx16"\n", file.format);
		fprintf(stderr, "\tsmallLodLog2: %"PRIu32"\n", file.smallLodLog2);
		fprintf(stderr, "\tlargeLodLog2: %"PRIu32"\n", file.largeLodLog2);
		fprintf(stderr, "\taspectRatioLog2: %"PRIu32"\n", file.aspectRatioLog2);
		fprintf(stderr, "\ttiles: %"PRIu32"\n", file.tiles);
		fprintf(stderr, "\tuntiled_width: %"PRIu32"\n", file.untiled_width);
		fprintf(stderr, "\tuntiled_height: %"PRIu32"\n", file.untiled_height);
		fprintf(stderr, "\tis_hires_tex: %"PRIu8"\n", file.is_hires_tex);
		fprintf(stderr, "\tsize: %"PRIu32"\n", file.size);
		fprintf(stderr, "\n");
	}

	file.data = malloc(file.size);
	if (!file.data) {
		fprintf(stderr, "Could not allocate memory for file content\n");
		return -ENOMEM;
	}
	ret = get_buffer(file.data, file.size, 1);
	if (ret < 0) {
		free(file.data);
		fprintf(stderr, "Failed to read file content\n");
		return ret;
	}

	ret = prepare_file(&file);
	if (ret < 0) {
		free(file.data);
		fprintf(stderr, "Failed to prepare file for export\n");
		if (globals.ignore_error)
			return 0;
		else
			return ret;
	}

	ret = write_file(&file);
	free(file.data);
	if (ret < 0) {
		fprintf(stderr, "Could not write file content\n");
		return ret;
	}

	return 0;
}
