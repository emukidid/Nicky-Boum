/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#include "../source/fileio.h"

#define FILENAME_LENGTH 16

typedef struct {
	char filename[FILENAME_LENGTH];
	uint32 size;
	uint32 offset;
} GCC_PACK fileio_slot_t;

static int file_slot_count;
static const uint8 *file_slot_data;
static const fileio_slot_t *file_slot_table;

void fio_init(const char *data_path) {
	static const uint8 *search_addr_start = (const uint8 *)0x08028000;
	static const uint8 *search_addr_limit = (const uint8 *)0x0A000000;
	const uint8 *p;
	file_slot_count = 0;
	for (p = search_addr_start; p < search_addr_limit; p += 256) {
		if (p[0] == 'B' && p[1] == 'O' && p[2] == 'U' && p[3] == 'M') {
			/* hdr+4 (BE32) : version */
			/* hdr+8 (BE32) : timestamp */
			file_slot_count = read_uint32BE(p + 12);
			file_slot_table = (const fileio_slot_t *)(p + 16);
			file_slot_data = (const uint8 *)(p + 16 + file_slot_count * sizeof(fileio_slot_t));
			break;
		}
	}
	print_debug(DBG_FILEIO, "fio_init() file_slot_count=%d", file_slot_count);
	if (file_slot_count == 0) {
		print_error("No files found");
	}
}

static int file_slot_compare(const void *a, const void *b) {
	const char *key = (const char *)a;
	const fileio_slot_t *file_slot = (const fileio_slot_t *)b;
	return strcmp(key, file_slot->filename);
}

int fio_open(const char *filename, fio_open_mode_e mode, int error_flag) {
	int slot = -1;
	if (mode == FIO_READ) {
		fileio_slot_t *file_slot = bsearch(filename, file_slot_table, file_slot_count, sizeof(fileio_slot_t), file_slot_compare);
		if (!file_slot) {
			if (error_flag) {
				print_error("Unable to open file '%s'", filename);
			}
			print_warning("Unable to open file '%s'", filename);
		} else {
			slot = file_slot - file_slot_table;
		}
	} else {
		/* write to SRAM ? */
		print_error("Unhandled mode %d", mode);
	}
	return slot;
}

void fio_close(int slot) {
	/* nothing to do */
}

int fio_size(int slot) {
	int size = 0;
	if (slot >= 0 && slot < file_slot_count) {
		size = read_uint32BE(&file_slot_table[slot].size);
	}
	return size;
}

const uint8 *fio_fetch(int slot) {
	const uint8 *data = 0;
	if (slot >= 0 && slot < file_slot_count) {
		data = file_slot_data + read_uint32BE(&file_slot_table[slot].offset);
	}
	return data;
}

void fio_write(int slot, const uint8 *data, int len) {
	/* XXX */
}

int fio_exists(const char *filename) {
	/* XXX */
	return 0;
}
