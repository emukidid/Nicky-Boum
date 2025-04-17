/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#include "sound.h"
#include "fileio.h"
#include "systemstub.h"

static int sfx_file_slot;
static const uint8 *sfx_file_data;
static int mod_file_slot;
static int ins_file_slot;
static const uint8 *ins_file_data;

void snd_init() {
	sfx_file_slot = -1;
	sfx_file_data = 0;
	mod_file_slot = -1;
	ins_file_slot = -1;
	ins_file_data = 0;
}

void snd_stop() {
	sys_stop_module();
	if (sfx_file_slot >= 0) {
		fio_close(sfx_file_slot);
		sfx_file_slot = -1;
	}
	if (mod_file_slot >= 0) {
		fio_close(mod_file_slot);
		mod_file_slot = -1;
	}
	if (ins_file_slot >= 0) {
		fio_close(ins_file_slot);
		ins_file_slot = -1;
	}
}

void snd_play_song(int song) {
	sys_stop_module();
	if (mod_file_slot >= 0) {
		fio_close(mod_file_slot);
		mod_file_slot = -1;
	}
#ifndef NICKY_DEBUG
	if (song >= 0 && song < MAX_SND_MODULES) {
		if (ins_file_slot < 0) {
			ins_file_slot = fio_open("INSTRUMENTS.RAW", FIO_READ, 1);
			ins_file_data = fio_fetch(ins_file_slot);
		}
		if (ins_file_data != 0) {
			const char *module_name = 0;
			if (NICKY1) {
				module_name = snd_module_table__v1[song];
			}
			if (NICKY2) {
				module_name = snd_module_table__v2[song];
			}
			if (module_name) {
				const uint8 *module_data = 0;
				print_debug(DBG_SOUND, "snd_play_song(%d, '%s')", song, module_name);
				mod_file_slot = fio_open(module_name, FIO_READ, 1);
				module_data = fio_fetch(mod_file_slot);
				sys_play_module(module_data, ins_file_data);
			}
		}
	}
#endif
}

void snd_play_sfx(int sfx) {
	print_debug(DBG_SOUND, "snd_play_sfx(%d)", sfx);
	if (sfx >= 0 && sfx < MAX_SND_SAMPLES) {
		const sample_data_t *sd = 0;
		if (NICKY1) {
			sd = &snd_sample_data_table__v1[sfx];
		}
		if (NICKY2) {
			sd = &snd_sample_data_table__v2[sfx];
		}
		if (sfx_file_slot < 0) {
			sfx_file_slot = fio_open("SFXS.RAW", FIO_READ, 1);
			sfx_file_data = fio_fetch(sfx_file_slot);
		}
		if (sd && sfx_file_data) {
			sys_play_sfx((const int8 *)sfx_file_data + sd->offs, sd->size, PAULA_FREQ / sd->period);
		}
	}
}
