/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#include "fileio.h"
#include "game.h"
#include "globals.h"
#include "resource.h"
#include "systemstub.h"

#define MAX_RES_SLOTS 11

#ifdef NICKY_GBA_VERSION
#define BLK_EXT "TIL"
#define SPR_EXT "TIL"
#define SQX_EXT "___"
#define TIL_EXT "TIL"
#else
#define BLK_EXT "BLK"
#define SPR_EXT "SPR"
#define SQX_EXT "SQX"
#define TIL_EXT "SQX"
#endif

static int res_slot_count;
static int res_slot_table[MAX_RES_SLOTS];
static int level_data_loaded = 0;

static struct anim_data_t *get_ref_ptr(s16 num_ref, int anim_count) {
	anim_data_t *ad = 0;
	if (num_ref < 0) {
		ad = (anim_data_t *)0xFFFFFFFF;
	} else if (num_ref > 0) {
		assert(num_ref < anim_count);
		ad = &res_ref_ref[num_ref];
	}
	return ad;
}

static const uint8 *get_move_delta_ptr(uint16 move) {
	return (move != 0) ? move_delta_table + move_delta_offsets_table[move - 1] : 0;
}

static const uint8 *fetch_level_data(const char *filename) {
	int slot_tmp;

	slot_tmp = fio_open(filename, FIO_READ, 1);
	assert(res_slot_count < MAX_RES_SLOTS);
	res_slot_table[res_slot_count] = slot_tmp;
	++res_slot_count;
	return fio_fetch(slot_tmp);
}

static void load_ref_ref__v1(const uint8 *data, int size) {
	int i, anim_count;

	memset(res_ref_ref, 0, sizeof(res_ref_ref));
	anim_count = size / 0x44;
	assert(anim_count <= ARRAYSIZE(res_ref_ref));
	for (i = 0; i < anim_count; ++i) {
		anim_data_t *ad = &res_ref_ref[i];
		ad->unk0 = *data++;
		ad->lifes = *data++;
		ad->cycles = *data++;
		ad->unk3 = *data++;
		ad->unk4 = *data++;
		ad->unk5 = *data++;
		ad->unk6 = *data++;
		ad->init_sprite_num = *data++;
		ad->colliding_opcode = *data++;
		ad->logic_opcode = *data++;
		ad->sound_num = *data++;
		ad->rnd = *data++;
		ad->sprite_num = read_uint16LE(data); data += 2;
		ad->sprite_flags = read_uint16LE(data); data += 2;
		ad->default_sprite_num = read_uint16LE(data); data += 2;
		ad->default_sprite_flags = read_uint16LE(data); data += 2;
		ad->anim_w = read_uint16LE(data); data += 2;
		ad->anim_h = read_uint16LE(data); data += 2;
		ad->score = read_uint16LE(data); data += 2;
		ad->bounding_box_x1 = read_uint16LE(data); data += 2;
		ad->bounding_box_x2 = read_uint16LE(data); data += 2;
		ad->bounding_box_y1 = read_uint16LE(data); data += 2;
		ad->bounding_box_y2 = read_uint16LE(data); data += 2;
		ad->move_data_ptr = get_move_delta_ptr(read_uint16LE(data)); data += 2;
		data += 2;
		ad->distance_dx = read_uint16LE(data); data += 2;
		ad->distance_dy = read_uint16LE(data); data += 2;
		ad->anim_data1_ptr = get_ref_ptr(read_uint16LE(data), anim_count); data += 2;
		data += 2;
		ad->anim_data2_ptr = get_ref_ptr(read_uint16LE(data), anim_count); data += 2;
		data += 2;
		ad->anim_data3_ptr = get_ref_ptr(read_uint16LE(data), anim_count); data += 2;
		data += 2;
		ad->dx = read_uint16LE(data); data += 2;
		ad->dy = read_uint16LE(data); data += 2;
		ad->anim_data4_ptr = get_ref_ptr(read_uint16LE(data), anim_count); data += 2;
		data += 2;
		ad->dx2 = read_uint16LE(data); data += 2;
		ad->dy2 = read_uint16LE(data); data += 2;
		data += 2;
	}
}

void res_load_level_data__v1(int level) {
	int slot_tmp;
	char filename[10];
	const uint8 *file_data;
	static const char *suffixes_table[] = { "1", "1A", "2", "2A", "3", "3A", "4", "4A" };

	print_debug(DBG_RESOURCE, "res_load_level_data(%d)", level);

	res_unload_level_data();

	/* set level palette */
	sprintf(filename, "DECOR%s.PAL", suffixes_table[level & ~1]);
	res_decor_pal = fetch_level_data(filename);
	sys_set_palette_spr(res_decor_pal, 16, 0);

	/* set nicky palette */
	slot_tmp = fio_open("NICKY.PAL", FIO_READ, 0);
	file_data = fio_fetch(slot_tmp);
	sys_set_palette_spr(file_data, 16, 1);
	fio_close(slot_tmp);

	/* background */
	sprintf(filename, "DEGRA%s.COP", suffixes_table[level & ~1]);
	slot_tmp = fio_open(filename, FIO_READ, 1);
	file_data = fio_fetch(slot_tmp);
	sys_set_copper_pal(file_data + 8 * 2, 160);
	fio_close(slot_tmp);

	/* tiles gfx */
	sprintf(filename, "DECOR%s." BLK_EXT, suffixes_table[level & ~1]);
	res_decor_blk = fetch_level_data(filename);
	sys_set_tile_data(res_decor_blk);

	/* tile flags */
	sprintf(filename, "DECOR%s.REF", suffixes_table[level & ~1]);
	res_decor_ref = fetch_level_data(filename);

	/* tile map (will be modified, make a copy) */
	sprintf(filename, "DECOR%s.CDG", suffixes_table[level]);
	slot_tmp = fio_open(filename, FIO_READ, 1);
	file_data = fio_fetch(slot_tmp);
	memcpy(res_decor_cdg, file_data, fio_size(slot_tmp));
	fio_close(slot_tmp);

	/* tiles update data */
	res_new_new = fetch_level_data("NEW.NEW"); /* tile_map_quad_table */

	/* level sprites */
	sprintf(filename, "S0%s." SPR_EXT, suffixes_table[level & ~1]);
	res_level_spr = fetch_level_data(filename);

	/* monster sprites */
	sprintf(filename, "S1%s." SPR_EXT, suffixes_table[level & ~1]);
	res_monster_spr = fetch_level_data(filename);

	/* monster flags */
	sprintf(filename, "POSIT%s.REF", suffixes_table[level]);
	res_posit_ref = fetch_level_data(filename);

	/* monster anim_data */
	sprintf(filename, "REF%s.REF", suffixes_table[level & ~1]);
	slot_tmp = fio_open(filename, FIO_READ, 1);
	file_data = fio_fetch(slot_tmp);
	load_ref_ref__v1(file_data, fio_size(slot_tmp));
	fio_close(slot_tmp);

	/* nicky sprites */
	res_nicky_spr = fetch_level_data("NICKY." SPR_EXT);

	/* set digits palette */
	slot_tmp = fio_open("DIGITS.PAL", FIO_READ, 1);
	file_data = fio_fetch(slot_tmp);
	sys_set_palette_spr(file_data, 16, 2);
	fio_close(slot_tmp);

	/* set lifebar palette */
	slot_tmp = fio_open("LIFEBAR.PAL", FIO_READ, 1);
	file_data = fio_fetch(slot_tmp);
	sys_set_palette_spr(file_data, 16, 3);
	fio_close(slot_tmp);

	/* load bomb/flash palette */
	res_flash_pal = fetch_level_data("FLASH.PAL");

	/* digits sprites */
	res_digits_spr = fetch_level_data("DIGITS." SPR_EXT);

	/* lifebar sprites */
	res_lifebar_spr = fetch_level_data("LIFEBAR." SPR_EXT);

	_screen_cdg_tile_map_h = 50;
	_screen_cdg_tile_map_w = 400;
#if 0 /* debug, jump to the boss place directly */
	_screen_cdg_tile_map_start_x = 18400;
	_screen_cdg_tile_map_start_y = 40;
#else
	_screen_cdg_tile_map_start_x = 0;
	_screen_cdg_tile_map_start_y = 0;
#endif

	level_data_loaded = 1;
}

static void load_ref_ref__v2(const uint8 *data, int size) {
	int i, anim_count;

	anim_count = size / 0x40;
	print_debug(DBG_RESOURCE, "load_ref_ref() anim_count %d size %d", anim_count, size);
	assert(anim_count <= ARRAYSIZE(res_ref_ref));
	for (i = 0; i < anim_count; ++i) {
		anim_data_t *ad = &res_ref_ref[i];
		ad->unk0 = *data++;
		ad->lifes = *data++;
		ad->cycles = *data++;
		ad->unk3 = *data++;
		ad->unk4 = *data++;
		ad->unk5 = *data++;
		ad->unk6 = *data++;
		ad->init_sprite_num = *data++;
		ad->colliding_opcode = *data++;
		ad->logic_opcode = *data++;
		ad->sound_num = *data++;
		ad->rnd = *data++;
		ad->sprite_num = *data++;
		ad->sprite_flags = *data++;
		ad->anim_w = read_uint16LE(data); data += 2;
		ad->anim_h = read_uint16LE(data); data += 2;
		ad->score = read_uint16LE(data); data += 2;
		ad->bounding_box_x1 = read_uint16LE(data); data += 2;
		ad->bounding_box_x2 = read_uint16LE(data); data += 2;
		ad->bounding_box_y1 = read_uint16LE(data); data += 2;
		ad->bounding_box_y2 = read_uint16LE(data); data += 2;
		ad->move_data_ptr = get_move_delta_ptr(read_uint16LE(data)); data += 2;
		data += 2;
		ad->distance_dx = read_uint16LE(data); data += 2;
		ad->distance_dy = read_uint16LE(data); data += 2;
		ad->anim_data1_ptr = get_ref_ptr(read_uint16LE(data), anim_count); data += 2;
		data += 2;
		ad->anim_data2_ptr = get_ref_ptr(read_uint16LE(data), anim_count); data += 2;
		data += 2;
		ad->anim_data3_ptr = get_ref_ptr(read_uint16LE(data), anim_count); data += 2;
		data += 2;
		ad->dx = read_uint16LE(data); data += 2;
		ad->dy = read_uint16LE(data); data += 2;
		ad->anim_data4_ptr = get_ref_ptr(read_uint16LE(data), anim_count); data += 2;
		data += 2;
		ad->dx2 = read_uint16LE(data); data += 2;
		ad->dy2 = read_uint16LE(data); data += 2;
		ad->draw_dx = read_uint16LE(data); data += 2;
		ad->draw_dy = read_uint16LE(data); data += 2;
	}
}

void res_load_level_data__v2(int level) {
	int slot_tmp;
	char filename[10];
	const uint8 *file_data;
	char levelnum1, levelnum2;

	levelnum1 = '1' + (level >> 1);
	levelnum2 = '1' + (level & 1);

	print_debug(DBG_RESOURCE, "res_load_level_data(%d)", level);

	res_unload_level_data();

	/* set level palette */
	sprintf(filename, "DECOR%c.PAL", levelnum1);
	res_decor_pal = fetch_level_data(filename);
	sys_set_palette_spr(res_decor_pal, 32, 0);

	/* background */
	sprintf(filename, "DEGRA%c.COP", levelnum1);
	slot_tmp = fio_open(filename, FIO_READ, 1);
	file_data = fio_fetch(slot_tmp);
	sys_set_copper_pal(file_data + 8 * 2, 160);
	fio_close(slot_tmp);

	/* tiles gfx */
	sprintf(filename, "AA%cBLK." TIL_EXT, levelnum1);
	res_decor_blk = fetch_level_data(filename);
	sys_set_tile_data(res_decor_blk);

	/* tiles flags */
	sprintf(filename, "AA%cREF." SQX_EXT, levelnum1);
	res_decor_ref = fetch_level_data(filename);

	/* tile map (will be modified, make a copy) */
	sprintf(filename, "A%c%cCDG." SQX_EXT, levelnum1, levelnum2);
	slot_tmp = fio_open(filename, FIO_READ, 1);
	file_data = fio_fetch(slot_tmp);
	memcpy(res_decor_cdg, file_data, fio_size(slot_tmp));
	fio_close(slot_tmp);

	/* tiles update data */
	sprintf(filename, "A%c%cNEW." SQX_EXT, levelnum1, levelnum2);
	res_new_new = fetch_level_data(filename);

	/* level sprites */
	sprintf(filename, "S0%c." TIL_EXT, levelnum1);
	res_level_spr = fetch_level_data(filename);

	/* monster sprites */
	sprintf(filename, "S1%c." TIL_EXT, levelnum1);
	res_monster_spr = fetch_level_data(filename);

	/* monster flags */
	sprintf(filename, "A%c%cPOS." SQX_EXT, levelnum1, levelnum2);
	res_posit_ref = fetch_level_data(filename);

	/* monster anim_data */
	sprintf(filename, "REF%cREF." SQX_EXT, levelnum1);
	slot_tmp = fio_open(filename, FIO_READ, 1);
	file_data = fio_fetch(slot_tmp);
	load_ref_ref__v2(file_data, fio_size(slot_tmp));
	fio_close(slot_tmp);

	/* nicky sprites */
	res_nicky_spr = fetch_level_data("S00." TIL_EXT);

	/* set digits palette */
	slot_tmp = fio_open("DIGITS.PAL", FIO_READ, 1);
	file_data = fio_fetch(slot_tmp);
	sys_set_palette_spr(file_data, 16, 2);
	fio_close(slot_tmp);

	/* set lifebar palette */
	slot_tmp = fio_open("LIFEBAR.PAL", FIO_READ, 1);
	file_data = fio_fetch(slot_tmp);
	sys_set_palette_spr(file_data, 16, 3);
	fio_close(slot_tmp);

	/* load bomb/flash palette */
	res_flash_pal = fetch_level_data("FLASH.PAL");

	/* digits sprites */
	res_digits_spr = fetch_level_data("DIGITS." SPR_EXT);

	/* lifebar sprites */
	res_lifebar_spr = fetch_level_data("LIFEBAR." SPR_EXT);

	if (read_uint16LE(res_posit_ref) == 0xFFFE) {
		_screen_cdg_tile_map_h = read_uint16LE(res_posit_ref + 2);
		_screen_cdg_tile_map_w = read_uint16LE(res_posit_ref + 4);
		_screen_cdg_tile_map_start_y = read_uint16LE(res_posit_ref + 6);
		_screen_cdg_tile_map_start_x = read_uint16LE(res_posit_ref + 8);
		res_posit_ref += 10;
	} else {
		_screen_cdg_tile_map_h = 50;
		_screen_cdg_tile_map_w = 400;
		_screen_cdg_tile_map_start_y = 9;
		_screen_cdg_tile_map_start_x = 10;
	}
#if 0
	if (_game_jump_to_end_of_level) {
		_screen_cdg_tile_map_start_y = _screen_cdg_tile_map_h - 9;
		_screen_cdg_tile_map_start_x = _screen_cdg_tile_map_w - 23;
		_game_jump_to_end_of_level = 0;
	}
#endif
	_screen_cdg_tile_map_start_y -= 9;
	_screen_cdg_tile_map_start_x -= 10;

	print_debug(DBG_RESOURCE, "tile_map %d,%d %d,%d", _screen_cdg_tile_map_start_x, _screen_cdg_tile_map_start_y, _screen_cdg_tile_map_w, _screen_cdg_tile_map_h);

	level_data_loaded = 1;
}

void res_unload_level_data() {
	print_debug(DBG_RESOURCE, "res_unload_level_data()");

	if (level_data_loaded) {
		int i;
		for (i = 0; i < res_slot_count; ++i) {
			fio_close(res_slot_table[i]);
		}
		memset(res_slot_table, 0, sizeof(res_slot_table));
		res_slot_count = 0;
		level_data_loaded = 0;
	}
}

void res_init_static_data__v1() {
	int i;
	anim_data_t *ad;
	static const uint16 starshield_sprite_num_table[] = { 35, 39, 36, 40, 37, 41, 38, 42 };
	static const uint16 anim_dim_table[] = { 26, 28, 30, 32, 32, 31, 29, 29, 27, 24 };
	static const uint16 death_anim_data[] = { 82, 8, 9, 5, 84, 9, 9, 7, 30, 16, 16, 6 };

	memset(anim_data_starshield, 0, sizeof(anim_data_starshield));
	for (i = 0; i < 8; ++i) {
		ad = &anim_data_starshield[i];
		ad->unk0 = 1;
		ad->cycles = 4;
		ad->sprite_num = starshield_sprite_num_table[i];
		ad->anim_w = 15;
		ad->anim_h = 15;
		ad->anim_data1_ptr = &anim_data_starshield[(6 + i) & 7];
	}

	memset(anim_data0, 0, sizeof(anim_data0));

/*	anim_data0[0].unk0 = 1;
	anim_data0[0].unk4 = 1;
	anim_data0[0].init_sprite_num = 82;
	anim_data0[0].anim_w = 8;
	anim_data0[0].anim_h = 9;
	anim_data0[0].bounding_box_x2 = 8;
	anim_data0[0].bounding_box_y2 = 9;
	anim_data0[0].move_data_ptr = &move_delta_table2[0];
	anim_data0[0].distance_dx = 5;
	anim_data0[1].unk0 = 1;
	anim_data0[1].unk4 = 1;
	anim_data0[1].init_sprite_num = 84;
	anim_data0[1].logic_opcode = 1;
	anim_data0[1].anim_w = 9;
	anim_data0[1].anim_h = 9;
	anim_data0[1].bounding_box_x2 = 9;
	anim_data0[1].bounding_box_y2 = 9;
	anim_data0[1].move_data_ptr = &move_delta_table2[0];
	anim_data0[1].distance_dx = 7;
	anim_data0[2].unk0 = 1;
	anim_data0[2].unk4 = 1;
	anim_data0[2].init_sprite_num = 30;
	anim_data0[2].logic_opcode = 1;
	anim_data0[2].anim_w = 16;
	anim_data0[2].anim_h = 16;
	anim_data0[2].bounding_box_x1 = 1;
	anim_data0[2].bounding_box_x2 = 15;
	anim_data0[2].bounding_box_y1 = 1;
	anim_data0[2].bounding_box_y2 = 15;
	anim_data0[2].move_data_ptr = &move_delta_table2[0];
	anim_data0[2].distance_dx = 6;*/
	/* death anim data */
	for (i = 0; i < 3; ++i) {
		ad = &anim_data0[i];
		ad->unk0 = 1;
		ad->unk4 = 1;
		ad->init_sprite_num = death_anim_data[i * 4];
		if (i != 0) {
			ad->logic_opcode = 1;
		}
		ad->bounding_box_x2 = ad->anim_w = death_anim_data[i * 4 + 1];
		ad->bounding_box_y2 = ad->anim_h = death_anim_data[i * 4 + 2];
		if (i == 2) {
			ad->bounding_box_x1 = 1;
			ad->bounding_box_y1 = 1;
			ad->bounding_box_x2 = 15;
			ad->bounding_box_y2 = 15;
		}
		ad->move_data_ptr = &move_delta_table2[0];
		ad->distance_dx = ad->anim_h = death_anim_data[i * 4 + 3];
	}

	/* blue ball data */
	ad = &anim_data0[3];
	ad->unk0 = 1;
	ad->lifes = 20;
	ad->unk4 = 1;
	ad->init_sprite_num = 83;
	ad->anim_w = 8;
	ad->anim_h = 8;
	ad->bounding_box_x2 = 8;
	ad->bounding_box_y2 = 8;
	ad->distance_dx = 6;
	ad->distance_dy = 3;

	/* red ball data */
	ad = &anim_data0[4];
	ad->unk0 = 1;
	ad->lifes = 20;
	ad->unk4 = 1;
	ad->init_sprite_num = 35;
	ad->anim_w = 14;
	ad->anim_h = 14;
	ad->bounding_box_x1 = 2;
	ad->bounding_box_x2 = 12;
	ad->bounding_box_y1 = 2;
	ad->bounding_box_y2 = 12;
	ad->distance_dx = 5;
	ad->distance_dy = 3;

/*	anim_data0[5].unk0 = 1;
	anim_data0[5].cycles = 4;
	anim_data0[5].init_sprite_num = 90;
	anim_data0[5].anim_w = 16;
	anim_data0[5].anim_h = 16;
	anim_data0[5].anim_data1_ptr = &anim_data0[6];
	anim_data0[6].unk0 = 1;
	anim_data0[6].cycles = 4;
	anim_data0[6].init_sprite_num = 91;
	anim_data0[6].anim_w = 16;
	anim_data0[6].anim_h = 16;
	anim_data0[6].anim_data1_ptr = &anim_data0[7];
	anim_data0[7].unk0 = 1;
	anim_data0[7].cycles = 4;
	anim_data0[7].init_sprite_num = 92;
	anim_data0[7].anim_w = 16;
	anim_data0[7].anim_h = 16;
	anim_data0[7].anim_data1_ptr = &anim_data0[8];
	anim_data0[8].unk0 = 1;
	anim_data0[8].cycles = 5;
	anim_data0[8].init_sprite_num = 93;
	anim_data0[8].anim_w = 16;
	anim_data0[8].anim_h = 16;
	anim_data0[8].anim_data1_ptr = &anim_data0[9];
	anim_data0[9].unk0 = 1;
	anim_data0[9].cycles = 5;
	anim_data0[9].init_sprite_num = 94;
	anim_data0[9].anim_w = 16;
	anim_data0[9].anim_h = 16;
	anim_data0[9].anim_data1_ptr = &anim_data0[10];
	anim_data0[10].unk0 = 1;
	anim_data0[10].cycles = 5;
	anim_data0[10].init_sprite_num = 95;
	anim_data0[10].anim_w = 16;
	anim_data0[10].anim_h = 16;
	anim_data0[10].anim_data1_ptr = &anim_data0[11];*/
	/* apple anim data */
	for (i = 0; i < 6; ++i) {
		ad = &anim_data0[5 + i];
		ad->unk0 = 1;
		ad->cycles = (i <= 7) ? 4 : 5;
		ad->init_sprite_num = 90 + i;
		ad->anim_w = 16;
		ad->anim_h = 16;
		ad->anim_data1_ptr = &anim_data0[6 + i];
	}
/*	anim_data0[11].unk0 = 1;
	anim_data0[11].cycles = 1;
	anim_data0[11].init_sprite_num = 64;
	anim_data0[11].colliding_opcode = 2;
	anim_data0[11].sound_num = -1;
	anim_data0[11].anim_w = 26;
	anim_data0[11].anim_h = 26;
	anim_data0[11].bounding_box_x1 = -8;
	anim_data0[11].bounding_box_x2 = 34;
	anim_data0[11].bounding_box_y1 = -8;
	anim_data0[11].bounding_box_y2 = 34;
	anim_data0[11].anim_data1_ptr = &anim_data0[12];
	anim_data0[12].unk0 = 1;
	anim_data0[12].cycles = 2;
	anim_data0[12].init_sprite_num = 65;
	anim_data0[12].sound_num = -1;
	anim_data0[12].anim_w = 28;
	anim_data0[12].anim_h = 28;
	anim_data0[12].anim_data1_ptr = &anim_data0[13];
	anim_data0[13].unk0 = 1;
	anim_data0[13].cycles = 2;
	anim_data0[13].init_sprite_num = 66;
	anim_data0[13].sound_num = -1;
	anim_data0[13].anim_w = 30;
	anim_data0[13].anim_h = 30;
	anim_data0[13].anim_data1_ptr = &anim_data0[14];
	anim_data0[14].unk0 = 1;
	anim_data0[14].cycles = 2;
	anim_data0[14].init_sprite_num = 67;
	anim_data0[14].sound_num = -1;
	anim_data0[14].anim_w = 32;
	anim_data0[14].anim_h = 32;
	anim_data0[14].anim_data1_ptr = &anim_data0[15];
	anim_data0[15].unk0 = 1;
	anim_data0[15].cycles = 2;
	anim_data0[15].init_sprite_num = 68;
	anim_data0[15].sound_num = -1;
	anim_data0[15].anim_w = 32;
	anim_data0[15].anim_h = 32;
	anim_data0[15].anim_data1_ptr = &anim_data0[16];
	anim_data0[16].unk0 = 1;
	anim_data0[16].cycles = 2;
	anim_data0[16].init_sprite_num = 69;
	anim_data0[16].sound_num = -1;
	anim_data0[16].anim_w = 31;
	anim_data0[16].anim_h = 31;
	anim_data0[16].anim_data1_ptr = &anim_data0[17];
	anim_data0[17].unk0 = 1;
	anim_data0[17].cycles = 2;
	anim_data0[17].init_sprite_num = 70;
	anim_data0[17].sound_num = -1;
	anim_data0[17].anim_w = 29;
	anim_data0[17].anim_h = 29;
	anim_data0[17].anim_data1_ptr = &anim_data0[18];
	anim_data0[18].unk0 = 1;
	anim_data0[18].cycles = 2;
	anim_data0[18].init_sprite_num = 71;
	anim_data0[18].sound_num = -1;
	anim_data0[18].anim_w = 29;
	anim_data0[18].anim_h = 29;
	anim_data0[18].anim_data1_ptr = &anim_data0[19];
	anim_data0[19].unk0 = 1;
	anim_data0[19].cycles = 2;
	anim_data0[19].init_sprite_num = 72;
	anim_data0[19].sound_num = -1;
	anim_data0[19].anim_w = 27;
	anim_data0[19].anim_h = 27;
	anim_data0[19].anim_data1_ptr = &anim_data0[20];
	anim_data0[20].unk0 = 1;
	anim_data0[20].cycles = 2;
	anim_data0[20].init_sprite_num = 73;
	anim_data0[20].sound_num = -1;
	anim_data0[20].anim_w = 24;
	anim_data0[20].anim_h = 24;*/
	/* apple/bomb anim data */
	for (i = 0; i < 10; ++i) {
		ad = &anim_data0[11 + i];
		ad->unk0 = 1;
		ad->cycles = 2;
		ad->init_sprite_num = 64 + i;
		if (i == 0) {
			ad->colliding_opcode = 2;
		}
		ad->sound_num = -1;
		ad->anim_w = anim_dim_table[i];
		ad->anim_h = anim_dim_table[i];
		if (i == 0) {
			ad->bounding_box_x1 = -8;
			ad->bounding_box_x2 = 34;
			ad->bounding_box_y1 = -8;
			ad->bounding_box_y2 = 34;
		}
		if (i != 9) {
			ad->anim_data1_ptr = &anim_data0[12 + i];
		}
	}

	/* megabomb anim data */
	ad = &anim_data0[21];
	ad->unk0 = 1;
	ad->init_sprite_num = 34;
	ad->sound_num = -1;
	ad->anim_w = 16;
	ad->anim_h = 16;
	ad->move_data_ptr = &move_delta_table[0];

	anim_data0[22].unk0 = -1;
	anim_data0[22].sound_num = -1;
}

void res_init_static_data__v2() {
	int i;
	anim_data_t *ad;
	static const uint16 death_anim_data[] = { 82, 7, 7, 5, 84, 9, 11, 7, 30, 16, 16, 6 };

	memset(anim_data_starshield, 0, sizeof(anim_data_starshield));
	for (i = 0; i < 3; ++i) {
		ad = &anim_data_starshield[i];
		ad->unk0 = 1;
		ad->cycles = 3;
		ad->sprite_num = 35 + i;
		ad->anim_w = 16;
		ad->anim_h = 10;
		ad->anim_data1_ptr = &anim_data_starshield[(2 + i) % 3];
		ad->draw_dy = 3;
	}

	memset(anim_data0, 0, sizeof(anim_data0));

/*	anim_data0[0].unk0 = 1;
	anim_data0[0].unk4 = 1;
	anim_data0[0].init_sprite_num = 82;
	anim_data0[0].anim_w = 7;
	anim_data0[0].anim_h = 7;
	anim_data0[0].bounding_box_x2 = 7;
	anim_data0[0].bounding_box_y2 = 7;
	anim_data0[0].move_data_ptr = &move_delta_table2[0];
	anim_data0[0].distance_dx = 5;
	anim_data0[1].unk0 = 1;
	anim_data0[1].unk4 = 1;
	anim_data0[1].init_sprite_num = 84;
	anim_data0[1].logic_opcode = 1;
	anim_data0[1].anim_w = 9;
	anim_data0[1].anim_h = 11;
	anim_data0[1].bounding_box_x2 = 9;
	anim_data0[1].bounding_box_y2 = 11;
	anim_data0[1].move_data_ptr = &move_delta_table2[0];
	anim_data0[1].distance_dx = 7;
	anim_data0[2].unk0 = 1;
	anim_data0[2].unk4 = 1;
	anim_data0[2].init_sprite_num = 30;
	anim_data0[2].logic_opcode = 1;
	anim_data0[2].anim_w = 16;
	anim_data0[2].anim_h = 16;
	anim_data0[2].bounding_box_x1 = 1;
	anim_data0[2].bounding_box_x2 = 15;
	anim_data0[2].bounding_box_y1 = 1;
	anim_data0[2].bounding_box_y2 = 15;
	anim_data0[2].move_data_ptr = &move_delta_table2[0];
	anim_data0[2].distance_dx = 6;*/
	/* death anim data */
	for (i = 0; i < 3; ++i) {
		ad = &anim_data0[i];
		ad->unk0 = 1;
		ad->unk4 = 1;
		ad->init_sprite_num = death_anim_data[i * 4];
		if (i != 0) {
			ad->logic_opcode = 1;
		}
		ad->bounding_box_y1 = ad->anim_w = death_anim_data[i * 4 + 1];
		ad->bounding_box_y2 = ad->anim_h = death_anim_data[i * 4 + 2];
		if (i == 2) {
			ad->bounding_box_x1 = 1;
			ad->bounding_box_y1 = 1;
			ad->bounding_box_x2 = 15;
			ad->bounding_box_y2 = 15;
		}
		ad->move_data_ptr = &move_delta_table2[0];
		ad->distance_dx = death_anim_data[i * 4 + 3];
	}

	/* blue ball anim data */
	ad = &anim_data0[3];
	ad->unk0 = 1;
	ad->lifes = 20;
	ad->unk4 = 1;
	ad->init_sprite_num = 83;
	ad->anim_w = 8;
	ad->anim_h = 8;
	ad->bounding_box_x2 = 8;
	ad->bounding_box_y2 = 8;
	ad->distance_dx = 6;
	ad->distance_dy = 3;

	/* red ball anim data */
	ad = &anim_data0[4];
	ad->unk0 = 1;
	ad->lifes = 20;
	ad->unk4 = 1;
	ad->init_sprite_num = 35;
	ad->anim_w = 18;
	ad->anim_h = 16;
	ad->bounding_box_x1 = 2;
	ad->bounding_box_x2 = 12;
	ad->bounding_box_y1 = 2;
	ad->bounding_box_y2 = 12;
	ad->distance_dx = 5;
	ad->distance_dy = 3;

	anim_data0[5].unk0 = 1;
	anim_data0[5].cycles = 4;
	anim_data0[5].init_sprite_num = 90;
	anim_data0[5].anim_w = 16;
	anim_data0[5].anim_h = 16;
	anim_data0[5].anim_data1_ptr = &anim_data0[6];
	anim_data0[6].unk0 = 1;
	anim_data0[6].cycles = 4;
	anim_data0[6].init_sprite_num = 91;
	anim_data0[6].anim_w = 16;
	anim_data0[6].anim_h = 16;
	anim_data0[6].anim_data1_ptr = &anim_data0[7];
	anim_data0[7].unk0 = 1;
	anim_data0[7].cycles = 5;
	anim_data0[7].init_sprite_num = 92;
	anim_data0[7].anim_w = 16;
	anim_data0[7].anim_h = 16;
	anim_data0[7].anim_data1_ptr = &anim_data0[8];
	anim_data0[8].unk0 = 1;
	anim_data0[8].cycles = 1;
	anim_data0[8].init_sprite_num = 64;
	anim_data0[8].colliding_opcode = 2;
	anim_data0[8].sound_num = -1;
	anim_data0[8].anim_w = 32;
	anim_data0[8].anim_h = 12;
	anim_data0[8].bounding_box_x1 = -8;
	anim_data0[8].bounding_box_x2 = 34;
	anim_data0[8].bounding_box_y1 = -8;
	anim_data0[8].bounding_box_y2 = 34;
	anim_data0[8].anim_data1_ptr = &anim_data0[9];
	anim_data0[8].draw_dx = -8;
	anim_data0[8].draw_dy = 13;
	anim_data0[9].unk0 = 1;
	anim_data0[9].cycles = 2;
	anim_data0[9].init_sprite_num = 65;
	anim_data0[9].sound_num = -1;
	anim_data0[9].anim_w = 48;
	anim_data0[9].anim_h = 17;
	anim_data0[9].anim_data1_ptr = &anim_data0[10];
	anim_data0[9].draw_dx = -8;
	anim_data0[9].draw_dy = 10;
	anim_data0[10].unk0 = 1;
	anim_data0[10].cycles = 2;
	anim_data0[10].init_sprite_num = 66;
	anim_data0[10].sound_num = -1;
	anim_data0[10].anim_w = 48;
	anim_data0[10].anim_h = 23;
	anim_data0[10].anim_data1_ptr = &anim_data0[11];
	anim_data0[10].draw_dx = -8;
	anim_data0[10].draw_dy = 7;
	anim_data0[11].unk0 = 1;
	anim_data0[11].cycles = 2;
	anim_data0[11].init_sprite_num = 67;
	anim_data0[11].sound_num = -1;
	anim_data0[11].anim_w = 48;
	anim_data0[11].anim_h = 27;
	anim_data0[11].anim_data1_ptr = &anim_data0[12];
	anim_data0[11].draw_dx = -8;
	anim_data0[11].draw_dy = 4;
	anim_data0[12].unk0 = 1;
	anim_data0[12].cycles = 2;
	anim_data0[12].init_sprite_num = 68;
	anim_data0[12].sound_num = -1;
	anim_data0[12].anim_w = 48;
	anim_data0[12].anim_h = 36;
	anim_data0[12].anim_data1_ptr = &anim_data0[13];
	anim_data0[12].draw_dx = -8;
	anim_data0[12].draw_dy = -1;
	anim_data0[13].unk0 = 1;
	anim_data0[13].cycles = 2;
	anim_data0[13].init_sprite_num = 69;
	anim_data0[13].sound_num = -1;
	anim_data0[13].anim_w = 48;
	anim_data0[13].anim_h = 37;
	anim_data0[13].anim_data1_ptr = &anim_data0[14];
	anim_data0[13].draw_dx = -8;
	anim_data0[13].draw_dy = -2;
	anim_data0[14].unk0 = 1;
	anim_data0[14].cycles = 2;
	anim_data0[14].init_sprite_num = 70;
	anim_data0[14].sound_num = -1;
	anim_data0[14].anim_w = 48;
	anim_data0[14].anim_h = 40;
	anim_data0[14].anim_data1_ptr = &anim_data0[15];
	anim_data0[14].draw_dx = -8;
	anim_data0[14].draw_dy = -3;
	anim_data0[15].unk0 = 1;
	anim_data0[15].cycles = 2;
	anim_data0[15].init_sprite_num = 71;
	anim_data0[15].sound_num = -1;
	anim_data0[15].anim_w = 48;
	anim_data0[15].anim_h = 39;
	anim_data0[15].draw_dx = -8;
	anim_data0[15].draw_dy = -3;

	anim_data0[16].unk0 = 1;
	anim_data0[16].sound_num = -1;
	anim_data0[17].unk0 = 1;
	anim_data0[17].sound_num = -1;
	anim_data0[18].unk0 = 1;
	anim_data0[18].sound_num = -1;

	anim_data0[19].unk0 = -1;
	anim_data0[19].sound_num = -1;

/*	anim_data0[20].unk0 = 1;
	anim_data0[20].cycles = 3;
	anim_data0[20].sprite_num = 35;
	anim_data0[20].anim_w = 16;
	anim_data0[20].anim_h = 10;
	anim_data0[20].anim_data1_ptr = &anim_data0[22];
	anim_data0[20].draw_dy = 3;
	anim_data0[21].unk0 = 1;
	anim_data0[21].cycles = 3;
	anim_data0[21].sprite_num = 36;
	anim_data0[21].anim_w = 16;
	anim_data0[21].anim_h = 10;
	anim_data0[21].anim_data1_ptr = &anim_data0[20];
	anim_data0[21].draw_dy = 3;
	anim_data0[22].unk0 = 1;
	anim_data0[22].cycles = 3;
	anim_data0[22].sprite_num = 37;
	anim_data0[22].anim_w = 16;
	anim_data0[22].anim_h = 10;
	anim_data0[22].anim_data1_ptr = &anim_data0[21];
	anim_data0[22].draw_dy = 3;*/
	for (i = 0; i < 3; ++i) {
		ad = &anim_data0[20 + i];
		ad->unk0 = 1;
		ad->cycles = 3;
		ad->sprite_num = 35 + i;
		ad->anim_w = 16;
		ad->anim_h = 10;
		if (i == 0) {
			ad->anim_data1_ptr = &anim_data0[20];
		} else {
			ad->anim_data1_ptr = &anim_data0[19 + i];
		}
		ad->draw_dy = 3;
	}
}
