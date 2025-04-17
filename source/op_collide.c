/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#include "game.h"
#include "globals.h"
#include "sound.h"

static void game_collides_play_sound(object_state_t *os) {
	anim_data_t *ad = os->anim_data1_ptr;
	os->displayed = 0;
	if (ad->sound_num >= 0) {
		snd_play_sfx(ad->sound_num);
	}
}

static void game_duplicate_object_play_sfx(object_state_t *os, anim_data_t *ad) {
	os->anim_data2_ptr = ad->anim_data2_ptr;
	if (ad->anim_data3_ptr == 0) {
		os->displayed = 0;
	} else {
		ad = ad->anim_data3_ptr;
		os->anim_data_cycles = ad->cycles;
		os->unk3 = ad->unk3;
		os->unk26 = ad->unk4;
		os->unk27 = ad->unk5;
		os->move_data_ptr = &ad->move_data_ptr[ad->init_sprite_num];
		os->distance_pos_x = ad->distance_dx;
		os->distance_pos_y = ad->distance_dy;
		os->anim_data4_ptr = ad->anim_data2_ptr;
		os->anim_data1_ptr = ad;
		os->anim_data3_ptr = ad;
		game_play_anim_data_sfx(ad);
	}
}

static void game_remove_object_play_sfx(object_state_t *os, anim_data_t *ad) { /* _di, _bx */
	if (ad->anim_data3_ptr == 0) {
		os->displayed = 0;
		if (os->visible == 0) {
			return;
		}
		++os->displayed;
		game_play_anim_data_sfx(os->anim_data1_ptr);
	} else {
		if (os->visible == 0) {
			os->anim_data2_ptr = 0;
		}
		game_play_anim_data_sfx(game_init_object_from_anim3(os, ad));
	}
}

static void game_collides_nop(object_state_t *os, anim_data_t *ad) {
}

/* pumpkin levels 5,6 */
static void game_collides_op27(object_state_t *os, anim_data_t *ad) {
	if (NICKY1) {
		os->transparent_flag ^= 2;
	}
	if (NICKY2) {
		os->transparent_flag |= 1;
	}
	player_state.colliding = 1;
}

/* snails */
static void game_collides_op0_24(object_state_t *os, anim_data_t *ad) {
	s16 y1, y2;
	if (ad->lifes == 0 || (player_state.hdir_flags & 4) == 0) {
		game_collides_op27(os, ad);
		return;
	}
	y1 = player_state.dim_h / 2 + player_state.pos_y;
	y2 = os->pos_y + ad->bounding_box_y1;
	if (y1 > y2) {
		game_collides_op27(os, ad);
		return;
	}
	if (NICKY1) {
		os->transparent_flag = 32;
	}
	if (NICKY2) {
		os->transparent_flag |= 1;
	}
	os->life -= 8;
	if (os->life > 0) {
		player_state.colliding_flag = 1;
	} else {
		int i, j;
		object_state_t *_si = objects_table_ptr9;
		os->transparent_flag = 0;
		cycle_score += ad->score;
		player_state.colliding_flag = 1;
		for (i = 0; i < 11; ++i) {
			if (_si->displayed == 0) {
				anim_data_t *_dx;
				++bonus_count;
				if (unk_ref_index >= 10) {
					unk_ref_index = 0;
				}
				++unk_ref_index;
				_dx = &ref_ref_7[unk_ref_index - 1];
				_si->anim_data1_ptr = _si->anim_data3_ptr = _dx;
				_si->displayed = _dx->unk0;
				_si->life = _dx->lifes;
				_si->anim_data_cycles = _dx->cycles;
				_si->unk3 = _dx->unk3;
				_si->displayed = 2;
				_si->anim_data4_ptr = _dx->anim_data2_ptr;
				_si->move_data_ptr = _dx->move_data_ptr;
				_si->unk26 = _dx->unk4;
				_si->unk27 = _dx->unk5;
				_si->pos_x = os->pos_x;
				_si->pos_y = os->pos_y;

				_si->pos_x += (ad->anim_w >> 1) - (_dx->anim_w >> 1);
				_si->distance_pos_x = 0;
				_si->distance_pos_y = 0;

				if (ad->colliding_opcode != 24) {
					game_remove_object_play_sfx(os, ad);
					return;
				}
				_si = os + 1;
				_dx = ad->anim_data4_ptr;
				for (j = 0; j < 2; ++j) {
					_si->anim_data1_ptr = _dx;
					_si->anim_data3_ptr = _dx;
					_si->displayed = _dx->unk0;
					_si->life = _dx->lifes;
					_si->anim_data_cycles = _dx->cycles;
					_si->unk3 = _dx->unk3;
					_si->displayed = 2;
					_si->anim_data4_ptr = _dx->anim_data2_ptr;
					_si->move_data_ptr = _dx->move_data_ptr;
					_si->distance_pos_x = _dx->distance_dx;
					_si->distance_pos_y = _dx->distance_dy;
					_si->unk26 = _dx->unk4;
					_si->unk27 = _dx->unk5;
					_si->pos_x = os->pos_x + ad->dx2;
					_si->pos_y = os->pos_y + ad->dy2;
					_si->anim_data2_ptr = _dx->anim_data2_ptr;
					++_si;
					++_dx;
				}
				game_remove_object_play_sfx(os, ad);
				return;
			}
			++_si;
		}
	}
}

static void game_collides_op_potion(object_state_t *os, anim_data_t *ad) {
	player_state.increment_life_bar = 1;
	player_state.colliding = 0;
	game_collides_play_sound(os);
}

static void game_collides_op_life_bonus(object_state_t *os, anim_data_t *ad) {
	if (player_state.lifes_count < 9) {
		++player_state.lifes_count;
	}
	game_collides_play_sound(os);
}

static void game_collides_op_key_bonus(object_state_t *os, anim_data_t *ad) {
	if (player_state.keys_count < 9) {
		++player_state.keys_count;
	}
	game_collides_play_sound(os);
}

static void game_collides_op_spring(object_state_t *os, anim_data_t *ad) {
	if (NICKY1) {
		nicky_move_offsets_ptr = &nicky_move_offsets_table__v1[35];
	}
	if (NICKY2) {
		nicky_move_offsets_ptr = &nicky_move_offsets_table__v2[42];
	}
	player_state.has_spring = 2000;
	game_collides_play_sound(os);
}

static void game_collides_op_wood_bonus(object_state_t *os, anim_data_t *ad) {
	player_state.has_wood = 1600;
	os->anim_data2_ptr = os->anim_data1_ptr;
	game_remove_object_play_sfx(os, ad);
}

static void game_collides_op_shield_bonus(object_state_t *os, anim_data_t *ad) {
	game_enable_nicky_shield();
	player_state.shield_duration = 1900;
	game_collides_play_sound(os);
}

static void game_collides_op_magnifying_glass(object_state_t *os, anim_data_t *ad) {
	player_state.tile_blinking_duration = 2000;
	game_collides_play_sound(os);
}

static void game_collides_op_bomb_bonus(object_state_t *os, anim_data_t *ad) {
	if (player_state.bombs_count < 9) {
		++player_state.bombs_count;
	}
	game_collides_play_sound(os);
}

static void game_collides_op_megabomb_bonus(object_state_t *os, anim_data_t *ad) {
	if (player_state.megabombs_count < 9) {
		++player_state.megabombs_count;
	}
	game_collides_play_sound(os);
}

static void game_collides_op_red_ball(object_state_t *os, anim_data_t *ad) {
	player_state.has_red_ball = 2000;
	game_collides_play_sound(os);
}

static void game_collides_op_blue_ball(object_state_t *os, anim_data_t *ad) {
	player_state.has_blue_ball = 1;
	game_collides_play_sound(os);
}

static void game_collides_op_apple(object_state_t *os, anim_data_t *ad) {
	cur_anim_data_ptr = &anim_data0[1];
	cur_anim_data_count = 4;
	game_collides_play_sound(os);
}

static void game_collides_op_teleport(object_state_t *os, anim_data_t *ad) {
	player_state.tilemap_offset = os->ref_ref_index;
	game_adjust_player_position();
	restart_level_flag = 1;
	ad = os->anim_data1_ptr;
	if (ad->sound_num >= 0) {
		snd_play_sfx(ad->sound_num);
	}
}

static void game_collides_op_misc_bonus(object_state_t *os, anim_data_t *ad) {
	os->anim_data2_ptr = 0;
	cycle_score += ad->score;
	game_remove_object_play_sfx(os, ad);
}

static void game_collides_op_bonus_box(object_state_t *os, anim_data_t *ad) {
	int i, distance_pos;
	object_state_t *_si = objects_table_ptr8;
	anim_data_t *_dx = ad->anim_data4_ptr;
	os->anim_data2_ptr = ad->anim_data3_ptr;
	distance_pos = -2;
	for (i = 0; i < 5; ++i) {
		_si->anim_data1_ptr = _dx;
		_si->anim_data3_ptr = _dx;
		_si->displayed = _dx->unk0;
		_si->life = _dx->lifes;
		_si->anim_data4_ptr = _dx->anim_data2_ptr;
		_si->move_data_ptr = _dx->move_data_ptr;
		_si->anim_data_cycles = _dx->cycles;
		_si->unk3 = _dx->unk3;
		_si->unk26 = _dx->unk4;
		_si->unk27 = _dx->unk5;
		_si->pos_x = os->pos_x + ad->dx2 - (4 - i);
		_si->pos_y = os->pos_y + ad->dy2;
		_si->distance_pos_y = 0;
		_si->distance_pos_x = distance_pos;
		++distance_pos;
		++_si;
		++_dx;
	}
	++bonus_count;
	game_remove_object_play_sfx(os, ad);
}

static void game_collides_op_blue_box(object_state_t *os, anim_data_t *ad) {
	object_state_t *_si = os + 1;
	anim_data_t *_dx = &ref_ref_43[os->ref_ref_index];
	_si->anim_data2_ptr = _dx;
	_si->displayed = _dx->unk0;
	if (NICKY2) {
		os->map_pos_x += ad->dx2;
		os->map_pos_y += ad->dy2;
	}
	os->anim_data2_ptr = ad->anim_data3_ptr;
	game_remove_object_play_sfx(os, ad);
}

static void game_collides_op_door(object_state_t *os, anim_data_t *ad) {
	print_debug(DBG_GAME, "game_collides_op_door() keys_count=%d", player_state.keys_count);
	if (player_state.keys_count) {
		--player_state.keys_count;
		game_change_decor_tile_map_quad(os);
		game_duplicate_object_play_sfx(os, ad);
	}
}

static void game_collides_op_eye(object_state_t *os, anim_data_t *ad) {
	if ((player_state.unk1 & 2) == 0) {
		if (NICKY1) {
			player_state.move_offsets_data_ptr = &nicky_move_offsets_table__v1[50];
		}
		if (NICKY2) {
			player_state.move_offsets_data_ptr = &nicky_move_offsets_table__v2[57];
		}
		player_state.hdir_flags |= 4;
	}
	game_play_anim_data_sfx(ad);
	game_change_decor_tile_map(os);
	os->displayed = 0;
	os = &objects_list_head[os->ref_ref_index];
	if (os->displayed > 0) {
		game_change_decor_tile_map_quad(os);
		game_duplicate_object_play_sfx(os, os->anim_data1_ptr);
	}
}

/* jumpers, 'bounding plants' */
static void game_collides_op21_28(object_state_t *os, anim_data_t *ad) {
	if (player_state.hdir_flags & 4) {
		int y1 = player_state.dim_h / 2 + player_state.pos_y;
		int y2 = os->pos_y + ad->bounding_box_y1;
		if (y1 <= y2) {
			player_state.pos_y = y2 - player_state.dim_h;
			player_state.unk1 = 2;
			player_state.hdir_flags &= ~4;
			if (NICKY1) {
				player_state.move_offsets_data_ptr = &nicky_move_offsets_table__v1[0];
			}
			if (NICKY2) {
				player_state.move_offsets_data_ptr = &nicky_move_offsets_table__v2[0];
			}
			if (ad->colliding_opcode != 21) {
				if (NICKY1) {
					player_state.move_offsets_data_ptr = &nicky_move_offsets_table__v1[35];
				}
				if (NICKY2) {
					player_state.move_offsets_data_ptr = &nicky_move_offsets_table__v2[42];
				}
			}
			game_remove_object_play_sfx(os, ad);
		}
	}
}

static void game_collides_op_go_to_next_level(object_state_t *os, anim_data_t *ad) {
	new_level_flag = 1;
	ad = os->anim_data1_ptr;
	if (ad->sound_num >= 0) {
		snd_play_sfx(ad->sound_num);
	}
}

static void game_collides_op_glass_pieces(object_state_t *os, anim_data_t *ad) {
	object_state_t *_si = &objects_list_head[os->ref_ref_index];
	if (_si->displayed == 2 && _si->anim_data2_ptr == _si->anim_data1_ptr) {
		anim_data_t *_di = _si->anim_data2_ptr->anim_data3_ptr;
		_si->anim_data_cycles = _di->cycles;
		_si->unk3 = _di->unk3;
		_si->unk26 = _di->unk4;
		_si->unk27 = _di->unk5;
		_si->distance_pos_x = _di->distance_dx;
		_si->distance_pos_y = _di->distance_dy;
		_si->anim_data4_ptr = _di->anim_data2_ptr;
		_si->anim_data1_ptr = _di;
		_si->anim_data3_ptr = _di;
		_si->move_data_ptr = &_di->move_data_ptr[_di->init_sprite_num];
		game_remove_object_play_sfx(os, ad);
	}
}

/* falling platforms levels 7,8 */
static void game_collides_op29(object_state_t *os, anim_data_t *ad) {
	if (player_state.unk1 & 2) {
		game_duplicate_object_play_sfx(os, ad);
	}
}

static void game2_collides_op11(object_state_t *os, anim_data_t *ad) {
	nicky_move_offsets_ptr = &nicky_move_offsets_table__v2[65];
	player_state.has_spring = 2000;
	game_collides_play_sound(os);
}

static void game2_collides_op_teleport(object_state_t *os, anim_data_t *ad) {
	if (nicky_anim_flags3 == 0) {
		game_collides_op_teleport(os, ad);
	}
}

static void game2_collides_op_door(object_state_t *os, anim_data_t *ad) {
	if (nicky_anim_flags3 == 0) {
		game_collides_op_door(os, ad);
	}
}

static void game2_collides_op21_28(object_state_t *os, anim_data_t *ad) {
	if (nicky_anim_flags3 == 0) {
		game_collides_op21_28(os, ad);
	}
}

static void game2_collides_op20(object_state_t *os, anim_data_t *ad) {
	if (nicky_anim_flags3 == 0) {
		game_collides_op_eye(os, ad);
	}
}

static void game2_collides_op_falling_rock_block(object_state_t *os, anim_data_t *ad) {
	player_state.colliding += 4;
	if (player_state.rock_hit_counter <= 247 && nicky_anim_flags3 == 0) {
		player_state.rock_hit_counter += 16;
	}
	if ((player_state.hdir_flags & 1) == 0) {
		player_state.pos_x += 8;
	} else {
		player_state.pos_x -= 8;
	}
}

static void game2_collides_op_moving_rock_block(object_state_t *os, anim_data_t *ad) {
	if (os->unk29 != 0 && (os->ref_ref_index & 0x8000) == 0 && (os->ref_ref_index >> 8) > 1) {
		++player_state.colliding;
		if (player_state.rock_hit_counter <= 247 && nicky_anim_flags3 == 0) {
			player_state.rock_hit_counter += 8;
		}
		if (os->distance_pos_x > 0) {
			player_state.pos_x += os->distance_pos_x + 7;
			player_state.pos_x &= ~7;
		} else if (os->distance_pos_x < 0) {
			player_state.pos_x = os->pos_x - 16;
		}
	}
}

static void game2_collides_op35(object_state_t *os, anim_data_t *ad) {
	if (nicky_on_elevator_flag == 0 && colliding_op35_flag == 0) {
		player_state.pos_x += os->ref_ref_index;
		colliding_op35_flag = 1;
	}
}

static void game2_collides_op38(object_state_t *os, anim_data_t *ad) {
	if (inp_direction_mask & 2) {
		if (nicky_anim_flags3 == 0 && nicky_on_elevator_flag == 0 && nicky_colliding_op39_moving_table == 0) {
/*			nicky_colliding_op39_moving_table = colliding_op39_tables[os->ref_ref_index];*/
		}
	}
}

static void game2_collides_op39(object_state_t *os, anim_data_t *ad) {
	if (inp_direction_mask & 1) {
		if (nicky_anim_flags3 == 0 && nicky_on_elevator_flag == 0 && nicky_colliding_op39_moving_table == 0) {
/*			nicky_colliding_op39_moving_table = colliding_op39_tables[os->ref_ref_index];*/
		}
	}
}

static void game2_collides_op_tree_door(object_state_t *os, anim_data_t *ad) {
	if (nicky_anim_flags3 != 0 || nicky_on_elevator_flag != 0 || (player_state.hdir_flags & 0x40) == 0) {
		return;
	}
	if (inp_fire_button == 0) {
		return;
	}
	/* TODO */
}

static void game2_collides_op_bird_egg(object_state_t *os, anim_data_t *ad) {
	if (nicky_anim_flags3 == 0) {
		if ((player_state.hdir_flags & ~1) == 0) {
			if (inp_fire_button) {
				update_nicky_anim_flag = 0;
				nicky_anim_flags3 |= 0x4000;
			}
		}
	}
}

static void game2_collides_op42(object_state_t *os, anim_data_t *ad) {
	if (nicky_anim_flags3 == 0) {
		if (cycle_counter == 0) {
			++player_state.colliding;
		}
		os->anim_data2_ptr = ad->anim_data2_ptr;
		if (ad->anim_data3_ptr != 0) {
			ad = ad->anim_data3_ptr;
			os->anim_data_cycles = ad->cycles;
			os->unk3 = ad->unk3;
			os->unk26 = ad->unk4;
			os->unk27 = ad->unk5;
			os->move_data_ptr = &ad->move_data_ptr[ad->init_sprite_num];
			os->distance_pos_x = ad->distance_dx;
			os->distance_pos_y = ad->distance_dy;
			os->anim_data4_ptr = ad->anim_data2_ptr;
			os->anim_data1_ptr = ad;
			os->anim_data3_ptr = ad;
			game_play_anim_data_sfx(ad);
		}
	}
}

static void game2_collides_op43(object_state_t *os, anim_data_t *ad) {
	os->anim_data2_ptr = ad->anim_data3_ptr;
	game_remove_object_play_sfx(os, ad);
}

static void game2_collides_op45(object_state_t *os, anim_data_t *ad) {
	os->anim_data1_ptr = ad->anim_data2_ptr;
}

static colliding_op_pf colliding_op_table__v1[] = {
	/* 0x00 */
	game_collides_op0_24,
	game_collides_op_potion,
	game_collides_op_life_bonus,
	game_collides_op_key_bonus,
	/* 0x04 */
	game_collides_op_spring,
	game_collides_op_wood_bonus,
	game_collides_op_shield_bonus,
	game_collides_op_magnifying_glass,
	/* 0x08 */
	game_collides_op_bomb_bonus,
	game_collides_op_megabomb_bonus,
	game_collides_op_red_ball,
	game_collides_op_blue_ball,
	/* 0x0C */
	game_collides_op_apple,
	game_collides_op_teleport,
	game_collides_op_misc_bonus,
	game_collides_op_bonus_box,
	/* 0x10 */
	game_collides_op_blue_box,
	game_collides_nop,
	game_collides_nop,
	game_collides_op_door,
	/* 0x14 */
	game_collides_op_eye,
	game_collides_op21_28,
	game_collides_nop,
	game_collides_nop,
	/* 0x18 */
	game_collides_op0_24,
	game_collides_op_go_to_next_level,
	game_collides_op_glass_pieces,
	game_collides_op27,
	/* 0x1C */
	game_collides_op21_28,
	game_collides_op29,
	game_collides_nop
};

static colliding_op_pf colliding_op_table__v2[] = {
	/* 0x00 */
	game_collides_op0_24,
	game_collides_op_potion,
	game_collides_op_life_bonus,
	game_collides_op_key_bonus,
	/* 0x04 */
	0, /* game_collides_op_spring */
	game_collides_op_wood_bonus,
	game_collides_op_shield_bonus,
	game_collides_op_magnifying_glass,
	/* 0x08 */
	game_collides_op_bomb_bonus,
	game_collides_op_megabomb_bonus,
	game_collides_op_red_ball,
	0, /* game2_collides_op11 */
	/* 0x0C */
	game_collides_op_apple,
	game2_collides_op_teleport,
	game_collides_op_misc_bonus,
	game_collides_op_bonus_box,
	/* 0x10 */
	game_collides_op_blue_box,
	0, /* game_collides_nop */
	0, /* game_collides_nop */
	game2_collides_op_door,
	/* 0x14 */
	game2_collides_op20,
	game2_collides_op21_28,
	0, /* game_collides_nop */
	0, /* game_collides_nop */
	/* 0x18 */
	game_collides_op0_24,
	game_collides_op_go_to_next_level,
	0, /* game_collides_op_glass_pieces */
	game_collides_op27,
	/* 0x1C */
	0, /* game2_collides_op21_28 */
	0, /* game_collides_op29 */
	game2_collides_op_falling_rock_block,
	game2_collides_op_moving_rock_block,
	/* 0x20 */
	game_collides_nop,
	0, /* game_collides_nop */
	0, /* game_collides_nop */
	game2_collides_op35,
	/* 0x24 */
	0, /* game_collides_nop */
	0, /* game_collides_nop */
	0, /* game2_collides_op38 */
	0, /* game2_collides_op39 */
	/* 0x28 */
	game2_collides_op_tree_door,
	game2_collides_op_bird_egg,
	0, /* game2_collides_op42 */
	game2_collides_op43,
	/* 0x2C */
	0, /* game_collides_nop */
	game2_collides_op45
};

void game_collides_op(object_state_t *os, anim_data_t *ad) {
	if (NICKY1) {
		assert(ad->colliding_opcode < ARRAYSIZE(colliding_op_table__v1));
		(*colliding_op_table__v1[ad->colliding_opcode])(os, ad);
	}
	if (NICKY2) {
		assert(ad->colliding_opcode < ARRAYSIZE(colliding_op_table__v2));
		if (!colliding_op_table__v2[ad->colliding_opcode]) {
			print_warning("game_collides_op() unhandled colliding opcode %d", ad->colliding_opcode);
		} else {
			(*colliding_op_table__v2[ad->colliding_opcode])(os, ad);
		}
	}
}
