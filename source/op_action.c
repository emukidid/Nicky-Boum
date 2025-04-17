/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#include "game.h"
#include "globals.h"
#include "sound.h"
#include "systemstub.h"

static void game_action_nop() {
}

static void game_action_op_use_megabomb() {
	anim_data_t *ad = &anim_data0[21];
	object_state_t *os = objects_table_ptr4;
	player_state.action_code = 4;
	os->anim_data1_ptr = ad;
	os->displayed = ad->unk0;
	os->life = ad->lifes;
	os->anim_data_cycles = 14;
	os->unk3 = ad->unk3;
	os->move_data_ptr = ad->move_data_ptr;
	os->pos_x = player_state.pos_x - 6;
	os->pos_y = player_state.pos_y - 16;
	--player_state.megabombs_count;
}

static void game_action_op4() {
	object_state_t *os = objects_table_ptr4;
	--os->anim_data_cycles;
	if (os->anim_data_cycles <= 0) {
		player_state.action_code = 5;
	}
	os->pos_x += (int8)os->move_data_ptr[0] - decor_state.delta_x;
	os->pos_y += (int8)os->move_data_ptr[1] - decor_state.delta_y;
	os->move_data_ptr += 2;
	game_draw_object(os);
}

static void game_action_op6() {
	int i;

	object_state_t *os = objects_table_ptr4;
	if ((player_state.vdir_flags & 0x80) == 0 && player_state.vdir_flags != 0) {
		--os->unk3;
		if ((os->unk3 & 3) == 0) {
			snd_play_sfx(SND_SAMPLE_19);
		}
	}
	/* _bp = os->move_data_ptr;*/
	/* _dx = os->move_data_seg;*/
	--os->anim_data_cycles;
	if (os->anim_data_cycles <= 0) {
		update_nicky_anim_flag = 1;
		player_state.action_code = 0;
		execute_action_op_flag = 0;
		os->displayed = 0;
	} else {
		for (i = 0; i < 8; ++i) {
			os->distance_pos_x += 8;
			os->distance_pos_y += 8;
			os->map_pos_x = (os->map_pos_x + 12) & 0x3FF;
			os->pos_x = ((s16)cos_table[os->map_pos_x >> 1] * os->distance_pos_x) >> 8;
			os->pos_x += megabomb_origin_x - decor_state.delta_x;
			os->pos_y = ((s16)sin_table[os->map_pos_x >> 1] * os->distance_pos_y) >> 8;
			os->pos_y += megabomb_origin_y - decor_state.delta_y;
			game_draw_object(os);
			++os;
		}
	}
}

static void game_action_op5() {
	int i, x;
	anim_data_t *ad = &anim_data0[21];
	object_state_t *os = objects_table_ptr4;
	megabomb_origin_x = os->pos_x; /* os->move_data_ptr = os->pos_x;*/
	megabomb_origin_y = os->pos_y; /* os->move_data_seg = os->pos_y;*/
	x = 0;
	for (i = 0; i < 8; ++i) {
		os->anim_data1_ptr = ad;
		os->pos_x = megabomb_origin_x;
		os->pos_y = megabomb_origin_y;
		os->displayed = 1;
		os->anim_data_cycles = 30;
		os->unk3 = 8;
		os->distance_pos_x = 0;
		os->distance_pos_y = 0;
		os->map_pos_x = x;
		x += 128;
		x &= ~1;
		++os;
	}
	player_state.action_code = 6;
	game_shake_screen();
	game_update_decor_after_megabomb();
	game_action_op6();
}

static void game2_action_op_nop() {
}

static void game2_action_op3() {
	object_state_t *os = objects_table_ptr4;

	player_state.action_code = 4;
	os->pos_x = player_state.pos_x + 9;
	os->pos_y = player_state.pos_y + 15;
	os->pos_x &= ~7;
	os->ref_ref_index = 0;
	os->unk29 = 0;
	os->tile_num = 0;
	--player_state.megabombs_count;
	snd_play_sfx(SND_SAMPLE_19);
}

static void game2_action_op4() {
	object_state_t *os = objects_table_ptr4;

	++os->ref_ref_index;
	if (os->ref_ref_index == 6) {
		player_state.action_code = 5;
		os->ref_ref_index = 0x8000;
	}
}

static void game2_action_op5() {
	player_state.action_code = 6;
}

static void game2_action_op6() {
	player_state.action_code = 0;
	execute_action_op_flag = 0;
	update_nicky_anim_flag = 1;
	game_update_decor_after_megabomb();
}

static action_op_pf action_op_table__v1[] = {
	/* 0x00 */
	game_action_nop,
	game_action_nop,
	game_action_nop,
	game_action_op_use_megabomb,
	/* 0x04 */
	game_action_op4,
	game_action_op5,
	game_action_op6
};

static action_op_pf action_op_table__v2[] = {
	/* 0x00 */
	game2_action_op_nop,
	game2_action_op_nop,
	game2_action_op_nop,
	game2_action_op3,
	/* 0x04 */
	game2_action_op4,
	game2_action_op5,
	game2_action_op6
};

void game_execute_action_op() {
	static const int delta_pos_table[] = { 12, 40, 2, 37, 11, 26, 17, 2, 23, 2 };
	s16 x, y, index;
	object_state_t *os;

	if (execute_action_op_flag) {
		if (NICKY1) {
			assert(player_state.action_code < ARRAYSIZE(action_op_table__v1));
			(*action_op_table__v1[player_state.action_code])();
		}
		if (NICKY2) {
			assert(player_state.action_code < ARRAYSIZE(action_op_table__v2));
			(*action_op_table__v2[player_state.action_code])();
			os = objects_table_ptr4;
			if ((os->ref_ref_index & 0x8000) == 0) {
				index = os->ref_ref_index >> 1;
				assert(index < 5);
				y = os->pos_y - 27 + delta_pos_table[index * 2 + 0];
				x = os->pos_x - 42 + delta_pos_table[index * 2 + 1];
				sys_add_to_sprite_list(res_level_spr, 119 + index, x, y, SSF_HITMODE, 0);
			}
		}
	}
}
