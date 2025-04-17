/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#include "fileio.h"
#include "sqx_decoder.h"

#define MAX_FILEIO_SLOTS 30

typedef struct {
	const char *filename;
	int size;
} pc_datafile_t;

typedef struct {
	int used;
	FILE *fp;
	int pos;
	int size;
	uint8 *data;
	fio_open_mode_e mode;
	const pc_datafile_t *pc_df;
} fileio_slot_t;

static const char *data_directory;
static fileio_slot_t fileio_slots_table[MAX_FILEIO_SLOTS];
static const pc_datafile_t pc_datafiles_table__v1[] = {
	{ "DECOR1.BLK", 32768 },
	{ "DECOR1.CDG", 20000 },
 	{ "DECOR1.REF", 2048 },
	{ "DECOR1A.CDG", 20000 },
	{ "DECOR2.BLK", 32768 },
	{ "DECOR2.CDG", 20000 },
 	{ "DECOR2.REF", 2048 },
	{ "DECOR2A.CDG", 20000 },
	{ "DECOR3.BLK", 32768 },
	{ "DECOR3.CDG", 20000 },
 	{ "DECOR3.REF", 2048 },
	{ "DECOR3A.CDG", 20000 },
	{ "DECOR4.BLK", 32768 },
	{ "DECOR4.CDG", 20000 },
 	{ "DECOR4.REF", 2048 },
	{ "DECOR4A.CDG", 20000 },
 	{ "POSIT1.REF", 4100 },
 	{ "POSIT1A.REF", 3880 },
 	{ "POSIT2.REF", 3920 },
 	{ "POSIT2A.REF", 4000 },
 	{ "POSIT3.REF", 3960 },
 	{ "POSIT3A.REF", 3960 },
 	{ "POSIT4.REF", 3820 },
 	{ "POSIT4A.REF", 4090 },
	{ "REF1.REF", 16728 },
	{ "REF2.REF", 17544 },
	{ "REF3.REF", 18428 },
	{ "REF4.REF", 19176 },
	{ "S01.SPR", 45740 },
	{ "S02.SPR", 57128 },
	{ "S03.SPR", 58958 },
	{ "S04.SPR", 59630 },
 	{ "S11.SPR", 7280 },
	{ "S12.SPR", 13824 },
	{ "S13.SPR", 39502 },
	{ "S14.SPR", 25232 }
};
static const pc_datafile_t pc_datafiles_table__v2[] = {
	{ "A11CDG.SQX", 16000 },
	{ "A11NEW.SQX", 72 },
	{ "A11POS.SQX", 2520 },
	{ "A12CDG.SQX", 10120 },
	{ "A12NEW.SQX", 68 },
	{ "A12POS.SQX", 1860 },
	{ "A21CDG.SQX", 4880 },
	{ "A21NEW.SQX", 36 },
	{ "A21POS.SQX", 680 },
	{ "A22CDG.SQX", 9752 },
	{ "A22NEW.SQX", 8 },
	{ "A22POS.SQX", 1370 },
	{ "A31CDG.SQX", 11000 },
	{ "A31NEW.SQX", 20 },
	{ "A31POS.SQX", 1230 },
	{ "A32CDG.SQX", 10400 },
	{ "A32NEW.SQX", 48 },
	{ "A32POS.SQX", 1420 },
	{ "A41CDG.SQX", 9000 },
	{ "A41NEW.SQX", 72 },
	{ "A41POS.SQX", 1810 },
	{ "A42CDG.SQX", 20240 },
	{ "A42NEW.SQX", 156 },
	{ "A42POS.SQX", 2650 },
	{ "AA1BLK.SQX", 32768 },
	{ "AA1REF.SQX", 2048 },
	{ "AA2BLK.SQX", 32768 },
	{ "AA2REF.SQX", 2048 },
	{ "AA3BLK.SQX", 32768 },
	{ "AA3REF.SQX", 2048 },
	{ "AA4BLK.SQX", 32768 },
	{ "AA4REF.SQX", 2048 },
	{ "REF1REF.SQX", 18752 },
	{ "REF2REF.SQX", 20992 },
	{ "REF3REF.SQX", 20032 },
	{ "REF4REF.SQX", 21632 },
	{ "S00.SQX", 38006 },
	{ "S01.SQX", 55122 },
	{ "S02.SQX", 65404 },
	{ "S03.SQX", 62228 },
	{ "S04.SQX", 64446 },
	{ "S11.SQX", 22704 },
	{ "S12.SQX", 21142 },
	{ "S13.SQX", 37918 },
	{ "S14.SQX", 19472 }
};

static int find_free_slot() {
	int i, slot = -1;
	for (i = 0; i < MAX_FILEIO_SLOTS; ++i) {
		if (!fileio_slots_table[i].used) {
			slot = i;
			break;
		}
	}
	return slot;
}

static int pc_datafile_compare(const void *a, const void *b) {
	const char *key = (const char *)a;
	const pc_datafile_t *pc_df = (const pc_datafile_t *)b;
	return strcmp(key, pc_df->filename);
}

static const pc_datafile_t *pc_find_datafile(const char *filename) {
	if (NICKY1) {
		return bsearch(filename, pc_datafiles_table__v1, ARRAYSIZE(pc_datafiles_table__v1), sizeof(pc_datafile_t), pc_datafile_compare);
	}
	if (NICKY2) {
		return bsearch(filename, pc_datafiles_table__v2, ARRAYSIZE(pc_datafiles_table__v2), sizeof(pc_datafile_t), pc_datafile_compare);
	}
	return 0;
}

void fio_init(const char *data_path) {
	data_directory = data_path;
	memset(fileio_slots_table, 0, sizeof(fileio_slots_table));
}

static FILE *fio_open_no_case(const char *filename, fio_open_mode_e mode) {
	static const char *modes_table[] = { "rb", "wb" };
	char *p;
	char buf[512];

	sprintf(buf, "%s/%s", data_directory, filename);
	p = buf + strlen(data_directory) + 1;
	string_upper(p);
	//printf("!opening: [%s/%s] [%s]\n", data_directory, filename, modes_table[mode]);
	return fopen(buf, modes_table[mode]);
}

int fio_open(const char *filename, fio_open_mode_e mode, int error_flag) {
	int slot = find_free_slot();
	if (slot < 0) {
		print_error("Unable to find free slot for '%s'", filename);
	} else {
		fileio_slot_t *file_slot = &fileio_slots_table[slot];
		memset(file_slot, 0, sizeof(fileio_slot_t));
		file_slot->fp = fio_open_no_case(filename, mode);
		if (file_slot->fp==NULL) {
			if (error_flag) {
				print_error("!Unable to open file '%s'", filename);
			}
			print_warning("Unable to open file '%s'", filename);
			slot = -1;
		} else {
			file_slot->mode = mode;
			fseek(file_slot->fp, 0, SEEK_END);
			file_slot->size = ftell(file_slot->fp);
			fseek(file_slot->fp, 0, SEEK_SET);
			file_slot->used = 1;
			file_slot->pc_df = pc_find_datafile(filename);
		}
	}
	return slot;
}

void fio_close(int slot) {
	if (slot >= 0) {
		fileio_slot_t *file_slot = &fileio_slots_table[slot];
		assert(file_slot->used);
		free(file_slot->data);
		//printf("closing\n");
		fclose(file_slot->fp);
		memset(file_slot, 0, sizeof(fileio_slot_t));
	}
}

int fio_size(int slot) {
	int size = 0;
	if (slot >= 0) {
		fileio_slot_t *file_slot = &fileio_slots_table[slot];
		assert(file_slot->used);
		if (file_slot->pc_df) {
			size = file_slot->pc_df->size;
		} else {
			size = file_slot->size;
		}
	}
	return size;
}

const uint8 *fio_fetch(int slot) {
	uint8 *data = 0;
	if (slot >= 0) {
		fileio_slot_t *file_slot = &fileio_slots_table[slot];
		assert(file_slot->used && file_slot->mode == FIO_READ);
		if (!file_slot->data) {
			file_slot->data = (uint8 *)malloc(file_slot->size);
			if (!file_slot->data) {
				print_error("Unable to allocate %d bytes", file_slot->size);
			} else {
				const pc_datafile_t *df = file_slot->pc_df;
				int cur_pos = ftell(file_slot->fp);
				fseek(file_slot->fp, 0, SEEK_SET);
				fread(file_slot->data, 1, file_slot->size, file_slot->fp);
				fseek(file_slot->fp, cur_pos, SEEK_SET);
				if (df) {
					uint8 *dst = (uint8 *)malloc(df->size);
					if (!dst) {
						print_error("Unable to allocate %d bytes", df->size);
						free(file_slot->data);
						file_slot->data = 0;
					} else {
						int decoded_size = sqx_decode(file_slot->data + 2, dst);
						if (decoded_size != df->size) {
							print_error("Failed to decode file_slot %d", slot);
							free(dst);
							dst = 0;
						}
						free(file_slot->data);
						file_slot->data = dst;
					}
				}
			}
		}
		data = file_slot->data;
		if (file_slot->fp!=NULL) //deniska
			fclose(file_slot->fp);
	}
	return data;
}

void fio_write(int slot, const uint8 *data, int len) {
	if (slot >= 0) {
		fileio_slot_t *file_slot = &fileio_slots_table[slot];
		assert(file_slot->used && file_slot->mode == FIO_WRITE);
		fwrite(data, len, 1, file_slot->fp);
	}
}

int fio_exists(const char *filename) {
	FILE *fp = fio_open_no_case(filename, FIO_READ);
	if (fp) {
		//printf("!closing\n");
		fclose(fp);
		return 1;
	}
	return 0;
}
