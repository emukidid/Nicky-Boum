/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#ifndef __FILEIO_H__
#define __FILEIO_H__

#include "util.h"

typedef enum {
	FIO_READ = 0,
	FIO_WRITE
} fio_open_mode_e;

extern void fio_init(const char *data_path);
extern int fio_open(const char *filename, fio_open_mode_e mode, int error_flag);
extern void fio_close(int slot);
extern int fio_size(int slot);
extern const uint8 *fio_fetch(int slot);
extern void fio_write(int slot, const uint8 *data, int len);
extern int fio_exists(const char *filename);

#endif /* __FILEIO_H__ */
