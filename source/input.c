/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#include "fileio.h"
#include "game.h"
#include "globals.h"
#include "input.h"
#include "systemstub.h"

static int file_slot;
static const uint8 *inp_buf_ptr, *inp_buf_end;
static input_recording_state_e recording_state;

static void inp_record(int direction_mask, int fire_button) {
	if (file_slot >= 0) {
		uint8 b = direction_mask;
		if (fire_button) {
			b |= 0x80;
		}
		fio_write(file_slot, &b, 1);
	}
}

static void inp_replay() {
	if (inp_buf_ptr < inp_buf_end) {
		uint8 b = *inp_buf_ptr++;
		inp_direction_mask |= b & 0xF;
		inp_fire_button |= b >> 7;
	}
}

void inp_init(input_recording_state_e rec_state) {
	print_debug(DBG_INPUT, "inp_init(%d)", rec_state);
	file_slot = -1;
	recording_state = rec_state;
}

void inp_start_level(int level_num) {
	char filename[20];
	print_debug(DBG_INPUT, "inp_start_level(%d)", level_num);
	sprintf(filename, "NICKY-LEVEL%d.COD", level_num + 1);
	switch (recording_state) {
	case IRS_REPLAY:
		file_slot = fio_open(filename, FIO_READ, 0);
		if (file_slot >= 0) {
			inp_buf_ptr = fio_fetch(file_slot);
			inp_buf_end = inp_buf_ptr + fio_size(file_slot);
			random_seed = read_uint16BE(inp_buf_ptr); inp_buf_ptr += 2;
		} else {
			inp_buf_ptr = 0;
			inp_buf_end = 0;
		}
		break;
	case IRS_RECORD:
		file_slot = fio_open(filename, FIO_WRITE, 0);
		if (file_slot >= 0) {
			uint8 tmp[2];
			tmp[0] = random_seed >> 8;
			tmp[1] = random_seed & 0xFF;
			fio_write(file_slot, tmp, 2);
		}
		break;
	case IRS_NONE:
		break;
	}
}

void inp_end_level() {
	print_debug(DBG_INPUT, "inp_end_level()");
	if (file_slot >= 0) {
		fio_close(file_slot);
		file_slot = -1;
	}
}

void inp_copy_state() {
	int *key_mask = sys_get_key_mask();
	if (*key_mask & SKM_PAUSE) {
		*key_mask &= ~SKM_PAUSE;
		while (sys_wait_for_keys(1000, SKM_PAUSE) == 0);
		*key_mask &= ~SKM_PAUSE;
	}
	/* handle special keys */
	if (*key_mask & SKM_QUIT) {
		*key_mask &= ~SKM_QUIT;
		quit_level_flag = 2;
	}
	if (*key_mask & SKM_F1) {
		*key_mask &= ~SKM_F1;
		game_enable_nicky_shield();
		player_state.shield_duration = 1900;
		player_state.has_wood = 1600;
		player_state.bombs_count = 9;
		player_state.keys_count = 9;
		player_state.megabombs_count = 9;
		sys_print_string("Cheat mode, adding all bonuses\n");
	}
	if (*key_mask & SKM_F2) {
		*key_mask &= ~SKM_F2;
		new_level_flag = 1;
		sys_print_string("Cheat mode, jumping to next level\n");
	}
	if (*key_mask & SKM_F3) {
		*key_mask &= ~SKM_F3;
		player_state.shield_duration = 0;
		game_disable_nicky_shield();
		player_state.increment_life_bar = 0;
		player_state.colliding = 128;
	}
	if (*key_mask & SKM_INVENTORY) {
		*key_mask &= ~SKM_INVENTORY;
		inventory_enabled_flag ^= 1;
	}
	/* handle game keys */
	inp_direction_mask = 0;
	if ((*key_mask & (SKM_UP | SKM_DOWN)) != (SKM_UP | SKM_DOWN)) {
		if (*key_mask & SKM_UP) {
			inp_direction_mask |= GAME_DIR_UP;
		}
		if (*key_mask & SKM_DOWN) {
			inp_direction_mask |= GAME_DIR_DOWN;
		}
	}
	if ((*key_mask & (SKM_LEFT | SKM_RIGHT)) != (SKM_LEFT | SKM_RIGHT)) {
		if (*key_mask & SKM_LEFT) {
			inp_direction_mask |= GAME_DIR_LEFT;
		}
		if (*key_mask & SKM_RIGHT) {
			inp_direction_mask |= GAME_DIR_RIGHT;
		}
	}
	inp_fire_button = 0;
	if (*key_mask & SKM_ACTION) {
		inp_fire_button = 1;
	}
	if (recording_state == IRS_REPLAY) {
		inp_replay();
	}
	if (recording_state == IRS_RECORD) {
		inp_record(inp_direction_mask, inp_fire_button);
	}
}
