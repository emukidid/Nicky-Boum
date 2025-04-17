/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#define VARDECL
#include "game.h"
#include "fileio.h"
#include "globals.h"
#include "input.h"
#include "resource.h"
#include "sequence.h"
#include "sound.h"
#include "systemstub.h"

static int8 _snd_current_sfx_num, _snd_current_sfx_priority;
static tile_anim_t _screen_blinking_tile;
static uint8 _screen_cdg_offsets[256] GCC_EXT;

void game_init() {
	memset(&game_state, 0, sizeof(game_state));
	memset(&decor_state, 0, sizeof(decor_state));
	memset(&player_state, 0, sizeof(player_state));
	if (NICKY1) {
		res_init_static_data__v1();
		game_state.starshield_object_state = &objects_table[417];
		objects_table_ptr1 = &objects_table[0];
		objects_table_ptr2 = &objects_table[410]; /* bullets */
		objects_table_ptr3 = &objects_table[437]; /* bomb */
		objects_table_ptr4 = &objects_table[438];
		objects_table_ptr5 = &objects_table[446];
		objects_table_ptr6 = &objects_table[448]; /* apples */
		objects_table_ptr7 = &objects_table[452];
		objects_table_ptr8 = &objects_table[421]; /* bonuses */
		objects_table_ptr9 = &objects_table[426];
	}
	if (NICKY2) {
		res_init_static_data__v2();
		game_state.starshield_object_state = &objects_table[302];
		objects_table_ptr1 = &objects_table[0];
		objects_table_ptr2 = &objects_table[295];
		objects_table_ptr3 = &objects_table[286];
		objects_table_ptr4 = &objects_table[287];
		objects_table_ptr5 = &objects_table[288];
		objects_table_ptr6 = &objects_table[290];
		objects_table_ptr7 = &objects_table[294];
		objects_table_ptr8 = &objects_table[270];
		objects_table_ptr9 = &objects_table[275];
	}
	inventory_enabled_flag = 1;
	restart_level_flag = 0;
	quit_level_flag = 0;
	throw_apple_var = 0;
	bounding_box_x1 = bounding_box_x2 = bounding_box_y1 = bounding_box_y2 = 0;
	random_seed = (uint16)time(0);
}

static void game_draw_bonuses() {
	int i;
	object_state_t *os = objects_table_ptr8;
	bonus_count = 16;
	for (i = 0; i < 16; ++i) {
		if (os->displayed) {
			anim_data_t *ad = os->anim_data1_ptr;
			if (ad->logic_opcode == 2) {
				game_logic_op2(os, ad);
				if (os->displayed) {
					goto loc_1328B;
				}
				goto loc_132AC;
			}
			if (os->unk27 != 0 && --os->unk27 == 0) {
				if (os->anim_data4_ptr == 0) {
					goto loc_132AC;
				}
				ad = os->anim_data4_ptr;
				os->anim_data1_ptr = ad;
				os->anim_data3_ptr = ad;
				os->anim_data4_ptr = ad->anim_data2_ptr;
				os->distance_pos_x = ad->distance_dx;
				os->distance_pos_y = ad->distance_dy;
				os->move_data_ptr = ad->move_data_ptr;
				os->anim_data_cycles = ad->cycles;
				os->unk3 = ad->unk3;
				os->unk26 = ad->unk4;
				os->unk27 = ad->unk5;
				goto loc_13288;
			}
			if (ad->cycles != 0 && --os->anim_data_cycles == 0) {
				if (ad->anim_data1_ptr == 0) {
					goto loc_132AC;
				}
				if (ad->anim_data1_ptr == (anim_data_t *)0xFFFFFFFF) {
					if (ad->anim_data3_ptr == 0) {
						goto loc_132AC;
					}
					ad = ad->anim_data3_ptr;
				} else {
					ad = ad->anim_data1_ptr;
				}
				os->anim_data1_ptr = ad;
				os->anim_data_cycles = ad->cycles;
			}
loc_13288:
			game_set_object_to_initial_pos(os, ad);
loc_1328B:
			if (os->pos_x >= -280 && os->pos_x <= GAME_SCREEN_W + 280 && os->pos_y >= -140 && os->pos_y <= GAME_SCREEN_H + 150) {
				game_draw_object(os);
			} else {
loc_132AC:
				os->displayed = 0;
				--bonus_count;
			}
		}
		++os;
	}
}

static void game_handle_highscore_screen() {
	/* TODO */
}

static void game_print_level_password() {
	/* TODO */
}

static void game_handle_game_over() {
	int i;
	player_state.tilemap_offset = player_state.next_tilemap_offset - 2;
	game_adjust_player_position();
	game_disable_nicky_shield();
	player_state.pos_x = -30;
	game_draw_nicky();
	for (i = 0; i < 2; ++i) {
		anim_data_t *ad = &res_ref_ref[123 + i];
		object_state_t *os = &objects_table_ptr8[i];
		os->pos_x = (GAME_SCREEN_W - 64) / 2;
		os->pos_y = (i == 0) ? 48 : 100;
		os->anim_data1_ptr = ad;
		os->anim_data3_ptr = ad;
		os->anim_data4_ptr = ad->anim_data2_ptr;
		os->distance_pos_x = ad->distance_dx;
		os->distance_pos_y = ad->distance_dy;
		os->move_data_ptr = ad->move_data_ptr;
		os->displayed = ad->unk0;
		os->life = ad->lifes;
		os->anim_data_cycles = ad->cycles;
		os->unk3 = ad->unk3;
		++os->displayed;
		os->unk26 = ad->unk4;
		os->unk27 = ad->unk5;
		os->transparent_flag = 0;
	}
	while (1) {
		sys_update_screen();
		sys_clear_sprite_list();
		++objects_table_ptr8[0].life;
		if (objects_table_ptr8[0].life > 2) {
			break;
		}
		game_adjust_tilemap();
		game_execute_logic_op();
		game_draw_bonuses();
	}
	snd_play_song(SND_MUS_SONGDEAD);
	sys_wait_for_keys(8000, SKM_ACTION);
	game_handle_highscore_screen();
}

static void game2_handle_game_over() {
	int spr_w, spr_h;
	snd_stop();
	sys_get_sprite_dim(res_level_spr, 72, &spr_w, &spr_h);
	sys_add_to_sprite_list(res_level_spr, 72, (GAME_SCREEN_W - spr_w) / 2, (GAME_SCREEN_H - spr_h) / 2, SSF_VISIBLE, 0);
	sys_update_screen();
	sys_wait_for_keys(2000, SKM_ACTION);
	game_handle_highscore_screen();
}

void game_run(int start_level_num) {
	/* display microids logo */
	sys_set_screen_mode(SSM_BITMAP);
	seq_display_image("I00.BMP", 4000);
	/* main loop */
	while (!sys_exit_flag) {
		seq_play_intro();
		game_prepare_level(start_level_num);
		while (!sys_exit_flag) {
			game_run_cycle();
			if (quit_level_flag >= 2) {
				inp_end_level();
				if (NICKY1) {
					game_handle_game_over();
				}
				if (NICKY2) {
					game2_handle_game_over();
				}
				break;
			} else if (new_level_flag) {
				inp_end_level();
				if (current_level == 7) {
					if (NICKY1) {
						seq_play_ending();
					}
					if (NICKY2) {
						/* There's no ending sequence in Nicky2 */
					}
					game_handle_highscore_screen();
					break;
				} else {
					++current_level;
					game_print_level_password();
					game_prepare_level(current_level);
					continue;
				}
			}
			sys_update_screen();
			sys_process_events();
			sys_clear_sprite_list();
		}
	}
}

void game_destroy() {
}

uint16 game_get_random_number(uint16 mod) {
	uint16 r1 = random_seed;
	uint16 r2 = r1;
	assert(mod != 0);
	r1 <<= 1;
	r1 += r2;
	r1 <<= 1;
	r1 += r2 * 3;
	r1 <<= 2;
	r1 += r2;
	r1 <<= 2;
	r1 += r2;
	r1 <<= 2;
	r1 += r2;
	r1 <<= 1;
	r1 += r2;
	r1 <<= 2;
	r1 += r2;
	r1 <<= 2;
	r1 += r2;
	random_seed = r1 + 1;
	return random_seed % mod;
}

void game_prepare_level(int level) {
	print_debug(DBG_GAME, "game_prepare_level(%d)", level);
	sys_set_screen_mode(SSM_TILED);
	current_level = level;
	game_state.score = 0;
	game_init_player_state();
	if (NICKY1) {
		res_load_level_data__v1(current_level);
	}
	if (NICKY2) {
		res_load_level_data__v2(current_level);
	}
	sys_set_tilemap_data(res_decor_cdg, _screen_cdg_tile_map_w, _screen_cdg_tile_map_h, _screen_cdg_offsets);
	game_init_level();
	game_init_nicky();
	snd_play_song(SND_MUS_SONGLEVEL + (current_level >> 1));
	inp_start_level(current_level);
}

static void game_init_level_helper() {
	anim_data_t *ad = anim_data0;
	while (ad->unk0 >= 0) {
		if (NICKY1) {
/*			ad->bitplane_size = ((ad->anim_w + 15) >> 4) * 2 * ad->anim_h;*/
			ad->default_sprite_num = ad->sprite_num = ad->init_sprite_num;
			ad->default_sprite_flags = ad->sprite_flags = 0x8000;
		}
		if (NICKY2) {
			ad->sprite_num = ad->init_sprite_num;
			ad->sprite_flags = 0x80;
		}
		++ad;
	}
}

void game_init_level() {
	print_debug(DBG_GAME, "game_init_level() level=%d", current_level);
	new_level_flag = 0;
	_snd_current_sfx_num = -1;
	_snd_current_sfx_priority = 0;
	player_state.keys_count = 0;
	if (NICKY1) {
		player_pos_x = 96;
		player_pos_y = 99;
		_screen_tile_origin_x = _screen_cdg_tile_map_start_x;
		_screen_tile_origin_y = _screen_cdg_tile_map_start_y;
	}
	if (NICKY2) {
		player_pos_x = 160;
		player_pos_y = 104;
		_screen_tile_origin_x = _screen_cdg_tile_map_h * _screen_cdg_tile_map_start_x;
		if (_screen_tile_origin_x < 0) {
			_screen_tile_origin_x = 0;
		}
		_screen_tile_origin_y = _screen_cdg_tile_map_start_y + 2;
		if (_screen_tile_origin_y < 0) {
			_screen_tile_origin_y = 0;
		}
	}
	player_state.tilemap_offset = 0;
	player_state.next_tilemap_offset = 0;
	player_state.has_spring = 0;
	if (NICKY1) {
		nicky_move_offsets_ptr = &nicky_move_offsets_table__v1[40];
	}
	if (NICKY2) {
		nicky_move_offsets_ptr = &nicky_move_offsets_table__v2[47];
	}
	monster_fight_flag = 0;
	boss4_state = 0;
	quit_level_flag = 0;
	game_state.boss_fight_counter = 0;
	ref_ref_1 = &res_ref_ref[1];
	ref_ref_7 = &res_ref_ref[7];
	ref_ref_43 = &res_ref_ref[43];
	game_init_ref_ref(res_ref_ref, 0x8000);
	game_init_level_helper();
	memset(objects_table, 0, sizeof(objects_table));
	game_init_objects_from_positref(res_posit_ref, 0);
	game_disable_nicky_shield();
	game_init_level_start_screen();
	cycle_score = 0;
	update_nicky_anim_flag = 1;
}

void game_init_nicky() {
	player_state.pos_x = player_pos_x;
	player_state.pos_y = player_pos_y;
	player_state.unk1 = 0;
	player_state.hdir_flags = 1;
	player_state.vdir_flags = 0;
	player_state.anim_frames_count = 0;
	player_state.throwing_ball_counter = 0;
	player_state.throwing_apple_counter = 0;
	player_state.increment_life_bar = 1;
	player_state.dead_flag = 0;
	player_state.energy = 0;
	player_state.colliding = 0;
	player_state.action_code = 0;
	if (NICKY1) {
		player_state.anim_frame_ptr = &nicky_anim_frames_table__v1[19];
	}
	if (NICKY2) {
		player_state.rock_hit_counter = 0;
		player_state.bird_dir_flags = 0;
		nicky_anim_flags3 = 0;
		nicky_colliding_op39_moving_table = 0;
		player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[19];
	}
	game_enable_nicky_shield();
	player_state.shield_duration = 475;
	unk_ref_index = 0;
	execute_action_op_flag = 0;
	change_pal_flag = 0;
}

void game_enable_nicky_shield() {
	int i;
	object_state_t *os;

	os = game_state.starshield_object_state;
	for (i = 0; i < 4; ++i) {
		if (NICKY1) {
			os->anim_data1_ptr = &anim_data_starshield[i];
		}
		if (NICKY2) {
			os->anim_data1_ptr = &anim_data_starshield[(i & 1) * 2];
		}
		os->pos_x = -16;
		os->pos_y = -16;
		os->displayed = 1;
		os->distance_pos_x = 0;
		os->distance_pos_y = 0;
		os->map_pos_x = i * 256;
		os->map_pos_y = 1;
		++os;
	}
	player_state.shield_duration = 0;
}

void game_disable_nicky_shield() {
	int i;
	object_state_t *os = game_state.starshield_object_state;
	anim_data_t *ad = &anim_data_starshield[0];
	for (i = 0; i < 4; ++i) {
		os->anim_data1_ptr = ad;
		os->displayed = 0;
		os->life = 0;
		os->anim_data_cycles = 0;
		os->unk3 = 0;
		os->move_data_ptr = 0;
		os->distance_pos_x = 0;
		os->distance_pos_y = 0;
		os->transparent_flag = 0;
		os->pos_x = -16;
		os->pos_y = -16;
		++os;
		++ad;
	}
}

static void game_translate_object_table(object_state_t *os, int num, s16 dx, s16 dy) {
	int i;
	for (i = 0; i < num && (os->displayed & 0x80) == 0; ++i) {
		if (os->displayed) {
			os->pos_x += dx;
			os->pos_y += dy;
		}
		++os;
	}
}

static void game_translate_object_tables(s16 dx, s16 dy) {
	if (NICKY1) {
		game_translate_object_table(objects_table_ptr1, 410, dx, dy);
	}
	if (NICKY2) {
		game_translate_object_table(objects_table_ptr1, 270, dx, dy);
	}
	game_translate_object_table(objects_table_ptr2, 7, dx, dy);
	game_translate_object_table(game_state.starshield_object_state, 4, dx, dy);
	game_translate_object_table(objects_table_ptr8, 16, dx, dy);
	game_translate_object_table(objects_table_ptr3, 1, dx, dy);
	if (NICKY1) {
		game_translate_object_table(objects_table_ptr4, 8, dx, dy);
	}
	if (NICKY2) {
		game_translate_object_table(objects_table_ptr4, 1, dx, dy);
	}
	game_translate_object_table(objects_table_ptr5, 2, dx, dy);
	game_translate_object_table(objects_table_ptr6, 4, dx, dy);
	game_translate_object_table(objects_table_ptr7, 1, dx, dy);
}

static void game_adjust_objects_position() {
	int _cx, _bx, _ax = 0;
	s16 x_start = 0;

	if (player_state.dead_flag) goto adjust_dy;

	/* X coord */
	if (player_state.pos_x <= 148 - _screen_cdg_tile_map_h) {
		if (monster_fight_flag) {
			if (NICKY1) {
				x_start = 19000;
			}
			if (NICKY2) {
				x_start = (_screen_cdg_tile_map_w - 20) * _screen_cdg_tile_map_h;
			}
		}
		if (decor_state.tile_map_offs_x <= x_start) goto adjust_dx;
		_ax = GAME_SCROLL_DX;
		if (NICKY2 && nicky_on_elevator_flag != 0) {
			_ax = 2;
		}
		game_translate_object_tables(_ax, 0);
		player_state.pos_x += _ax;
		decor_state.scroll_dx += _ax;
		if (decor_state.scroll_dx >= 16) {
			decor_state.tile_map_offs_x -= _screen_cdg_tile_map_h;
		}
	}
	if (player_state.pos_x >= 188 - _screen_cdg_tile_map_h) {
		if (decor_state.tile_map_offs_x >= decor_state.tile_map_end_offs_x) goto adjust_dx;
		_cx = decor_state.tile_map_offs_x + _screen_cdg_tile_map_h;
		_ax = GAME_SCROLL_DX;
		if (NICKY2 && nicky_on_elevator_flag != 0) {
			_ax = 2;
		}
		if (_cx >= decor_state.tile_map_end_offs_x && GAME_SCROLL_DX > decor_state.scroll_dx) {
			_ax = decor_state.scroll_dx;
		}
		game_translate_object_tables(-_ax, 0);
		player_state.pos_x -= _ax;
		decor_state.scroll_dx -= _ax;
		if (decor_state.scroll_dx < 0) {
			decor_state.tile_map_offs_x += _screen_cdg_tile_map_h;
		}
	}
adjust_dx:
	decor_state.scroll_dx &= 0xF;
	_ax = _bx = decor_state.scroll_dx;
	player_state.pos_dx_2 = -_bx;
	_ax = 0;

	/* Y coord */
	if (monster_fight_flag) goto adjust_dy;
	_ax = 0;
	_bx = decor_state.tile_map_offs_y;
	if (player_state.pos_y <= 64) {
		if (_bx <= 0) goto adjust_dy;
		_ax = -GAME_SCROLL_DY;
		if (NICKY2 && nicky_on_elevator_flag != 0) {
			_ax = -1;
		}
		if (decor_state.scroll_inc < 0) {
			_ax = decor_state.scroll_inc;
		}
		if (_bx <= 1) {
			_cx = _ax;
			_ax = -_ax + decor_state.scroll_dy;
			if (_ax > 16) {
				_cx = decor_state.scroll_dy - 16;
			}
			_ax = _cx;
		}
		game_translate_object_tables(0, -_ax);
		player_state.pos_y -= _ax;
		decor_state.scroll_dy -= _ax;
		if (decor_state.scroll_dy >= 16) {
			--decor_state.tile_map_offs_y;
		}
	}
	if (player_state.pos_y >= 87) {
		--_bx;
		if (_bx >= decor_state.tile_map_end_offs_y) goto adjust_dy;
		_ax = GAME_SCROLL_DY;
		if (NICKY2 && nicky_on_elevator_flag != 0) {
			_ax = (player_state.pos_y - 87) / 2; /*1;*/
		}
		_cx = decor_state.scroll_inc;
		if (_cx > 0) {
			_ax = _cx;
		}
		_cx = _bx + 1;
		if (_cx >= decor_state.tile_map_end_offs_y && _ax > decor_state.scroll_dy) {
			_ax = decor_state.scroll_dy;
		}
		game_translate_object_tables(0, -_ax);
		player_state.pos_y -= _ax;
		decor_state.scroll_dy -= _ax;
		if (decor_state.scroll_dy < 0) {
			++decor_state.tile_map_offs_y;
		}
	}

adjust_dy:
	decor_state.scroll_dy &= 0xF;
	_bx = _ax = decor_state.scroll_dy;
	player_state.pos_dy_2 = -_bx;

	decor_state.offs_x = player_state.pos_dx_2;
	decor_state.offs_y = player_state.pos_dy_2;
	decor_state.tile_block_x = decor_state.tile_map_offs_x;
	decor_state.tile_block_y = decor_state.tile_map_offs_y;
}

static void game_setup_ball_object(object_state_t *os, anim_data_t *ad) {
	if (player_state.throwing_ball_counter) {
		--player_state.throwing_ball_counter;
	} else if (os->displayed == 0 && inp_fire_button && (player_state.hdir_flags & 0x30) == 0) {
		os->anim_data1_ptr = ad;
		player_state.throwing_ball_counter = 8;
		os->displayed = ad->unk0;
		os->life = ad->lifes;
		os->anim_data_cycles = ad->cycles;
		os->unk3 = ad->unk3;
		os->unk26 = ad->unk4;
		os->unk27 = ad->unk5;
		os->pos_x = player_state.pos_x;
		os->pos_y = player_state.pos_y;
		os->distance_pos_y = ad->distance_dy;
		os->distance_pos_x = (player_state.hdir_flags & 1) ? ad->distance_dx : -ad->distance_dx;
		os->pos_x += player_state.anim_frame_ptr->pos_x;
		os->pos_y += player_state.anim_frame_ptr->pos_y;
	}
}

static void game_throw_apple_helper1() {
	object_state_t *os = objects_table_ptr5;
	if (player_state.has_blue_ball) {
		game_setup_ball_object(os, &anim_data0[3]);
	}
	if (player_state.has_red_ball) {
		++os;
		game_setup_ball_object(os, &anim_data0[4]);
	}
}

static void game_setup_object_near_player(object_state_t *os, anim_data_t *ad) {
	os->distance_pos_x = (player_state.hdir_flags & 1) ? ad->distance_dx : -ad->distance_dx;
	os->move_data_ptr = ad->move_data_ptr;
	os->anim_data1_ptr = ad;
	os->displayed = 1;
	os->pos_x = player_state.pos_x + player_state.anim_frame_ptr->pos_x;
	os->pos_y = player_state.pos_y + player_state.anim_frame_ptr->pos_y;
}

static void game_throw_apple_helper2() {
	if (player_state.has_wood > 0) {
		if (player_state.throwing_apple_counter > 0) {
			--player_state.throwing_apple_counter;
		} else if (inp_fire_button) {
			object_state_t *os = objects_table_ptr7;
			if (os->displayed == 0 && (player_state.hdir_flags & 0x30) == 0) {
				player_state.throwing_apple_counter = 5;
				game_setup_object_near_player(os, &anim_data0[2]);
			}
		}
	}
}

static void game_throw_apple() {
	if (update_nicky_anim_flag) {
		game_throw_apple_helper1();
		game_throw_apple_helper2();
		if (inp_fire_button == 0) {
			inp_fire_button_num_cycles_pressed = 0;
			goto loc_11E99;
		}
		if (inp_fire_button_num_cycles_pressed <= 20) {
			++inp_fire_button_num_cycles_pressed;
			goto loc_11E99;
		}
		if (player_state.hdir_flags & 0x40) {
			anim_data_t *ad = &anim_data0[5]; /* _bx */
			object_state_t *os = objects_table_ptr3; /* _di */
			if (player_state.bombs_count == 0 || os->displayed) goto throw_apple;
			inp_fire_button_num_cycles_pressed = 0;
			os->anim_data1_ptr = ad;
			os->displayed = ad->unk0;
			os->life = ad->lifes;
			os->anim_data_cycles = ad->cycles;
			os->unk3 = ad->unk3;
			os->unk26 = ad->unk4;
			os->unk27 = ad->unk5;
			os->pos_x = player_state.pos_x + player_state.anim_frame_ptr->pos_dx;
			os->pos_y = player_state.pos_y + player_state.anim_frame_ptr->pos_dy;
			if (NICKY2) {
				os->pos_y -= 2;
			}
			--player_state.bombs_count;
			return;
		} else {
			if ((player_state.hdir_flags & 0x7E) == 0 && player_state.unk1 == 0 && player_state.megabombs_count != 0 && player_state.action_code == 0) {
				update_nicky_anim_flag = 0;
				player_state.action_code = 1;
			}
			inp_fire_button_num_cycles_pressed = 0;
			return;
		}
	}
throw_apple:
	inp_fire_button_num_cycles_pressed = 0;
loc_11E99:
	if (player_state.fire_button_counter) {
		--player_state.fire_button_counter;
		return;
	}
	if (inp_fire_button == 0) {
		throw_apple_var = 1;
		return;
	}
	if (throw_apple_var) {
		int i;
		object_state_t *os = objects_table_ptr6;
		anim_data_t *ad = cur_anim_data_ptr;
		throw_apple_var = 0;
		for (i = 0; i < cur_anim_data_count; ++i) {
			if (os->displayed == 0 && (player_state.hdir_flags & 0x30) == 0) {
				player_state.fire_button_counter = 3;
				game_setup_object_near_player(os, ad);
				break;
			}
			++os;
		}
	}
}

static void game_update_helper7(object_state_t *os) {
	if (os->displayed) {
		int i;
		for (i = 0; i < 4; ++i) {
			s16 delta = os->anim_data3_ptr_;
			os->anim_data4_seg_ += os->map_pos_y;
			os->anim_data4_ptr_ += os->map_pos_x;
			if (delta < os->anim_data4_ptr_) {
				os->anim_data4_ptr_ -= delta;
				os->pos_x += os->distance_pos_x;
			}
			if (delta < os->anim_data4_seg_) {
				os->anim_data4_seg_ -= delta;
				os->pos_y += os->distance_pos_y;
			}
		}
		os->pos_x -= decor_state.delta_x;
		if (os->pos_x >= -280 && os->pos_x <= GAME_SCREEN_W + 280) {
			os->pos_y -= decor_state.delta_y;
			if (os->pos_y >= -140 && os->pos_y <= GAME_SCREEN_H + 150) {
				return;
			}
		}
	}
	os->displayed = 0;
	--bonus_count;
}

static void game_move_object(object_state_t *os) {
	int dy, dx;
	os->distance_pos_x = 0;
	os->distance_pos_y = 0;

	dy = player_state.pos_y + 4 + player_state.anim_frame_ptr->delta_y - os->pos_y;
	if (NICKY2) {
		dy += 2;
	}
	os->map_pos_y = (dy < 0) ? -dy : dy;

	dx = player_state.pos_x - os->pos_x;
	if (NICKY2) {
		dx += 7;
	}
	os->map_pos_x = (dx < 0) ? -dx : dx;

	os->anim_data3_ptr_ = (os->map_pos_x > os->map_pos_y) ? os->map_pos_x : os->map_pos_y;
	if (dx != 0) {
		if (dx < 0) {
			--os->distance_pos_x;
		} else {
			++os->distance_pos_x;
		}
	}
	if (dy != 0) {
		if (dy < 0) {
			--os->distance_pos_y;
		} else {
			++os->distance_pos_y;
		}
	}
	os->anim_data4_ptr_ = 0;
	os->anim_data4_seg_ = 0;
}

static void game_update_helper8() {
	int i;
	s16 _bp, _dx, _bx;
	object_state_t *os = objects_table_ptr8;
	bonus_count = 9;
	if (boss4_state == 2) {
		game_move_object(os);
		boss4_state = 3;
	} else if (boss4_state == 3) {
		game_update_helper7(os);
	}
	_bp = os->pos_x;
	_dx = os->pos_y;
	++os;
	for (i = 0; i < 8; ++i, ++os) {
		anim_data_t *ad;
		if (os->displayed == 0) {
hide_object:
			os->displayed = 0;
			--bonus_count;
			continue;
		}
		ad = os->anim_data1_ptr;
		if (ad->logic_opcode != 19) {
			ad = game_logic_op_helper1(os, ad);
			if (os->displayed == 0) {
				goto hide_object;
			}
		} else {
			/* boss level 9 */
			if (ad->cycles != 0 && --os->anim_data_cycles == 0) {
				if (ad->anim_data1_ptr == 0) {
					goto hide_object;
				}
				ad = ad->anim_data1_ptr;
				os->anim_data1_ptr = ad;
				os->anim_data_cycles = ad->cycles;
			}
			_bx = os->map_pos_x;
			if (os->distance_pos_x <= 48) {
				if (os->distance_pos_x == 0) {
					game_play_anim_data_sfx(ad);
				}
				os->distance_pos_x += 4;
				os->distance_pos_y += 4;
				if (os->distance_pos_x >= 40) {
					boss4_state = 2;
				}
			} else {
				if (os->map_pos_y <= 26) {
					os->map_pos_y += 2;
				}
				_bx -= os->map_pos_y;
				_bx &= 0x3FF;
				os->map_pos_x = _bx;
			}
			os->pos_x = ((s16)cos_table[_bx >> 1] * os->distance_pos_x) >> 8;
			os->pos_x += _bp - decor_state.delta_x;
			os->pos_y = ((s16)sin_table[_bx >> 1] * os->distance_pos_y) >> 8;
			os->pos_y += _dx - decor_state.delta_y;
		}
		if (os->pos_x >= -280 && os->pos_x <= GAME_SCREEN_W + 280 && os->pos_y >= -140 && os->pos_y <= GAME_SCREEN_H + 150) {
			game_draw_object(os);
		} else {
			goto hide_object;
		}
	}
}

static void game_update_helper3() {
	if (bonus_count) {
		if (boss4_state) {
			game_update_helper8();
		} else {
			game_draw_bonuses();
		}
	}
}

static void game_handle_objects5() {
	int i;
	object_state_t *os = objects_table_ptr5;
	for (i = 0; i < 2; ++i) {
		if (os->displayed) {
			os->pos_y += os->distance_pos_y - decor_state.delta_y;
			os->pos_x += os->distance_pos_x - decor_state.delta_x;
			if (os->pos_x >= -4 && os->pos_x <= GAME_SCREEN_W && os->pos_y >= -4 && os->pos_y <= GAME_SCREEN_H) {
				game_draw_object(os);
			} else {
				os->displayed = 0;
			}
		}
		++os;
	}
}

static void game_handle_objects6() {
	int i;
	object_state_t *os = objects_table_ptr6;
	game_draw_bomb_object();
	game_handle_objects5();
	/* draw apples */
	for (i = 0; i < 5; ++i) {
		if (os->displayed) {
			os->pos_y += (s16)read_uint16BE(os->move_data_ptr + 2) - decor_state.delta_y;
			os->move_data_ptr += 4;
			os->pos_x += os->distance_pos_x - decor_state.delta_x;
			if (os->pos_x >= -4 && os->pos_x <= GAME_SCREEN_W && os->pos_y >= -4 && os->pos_y <= GAME_SCREEN_H) {
				game_draw_object(os);
			} else {
				os->displayed = 0;
			}
		}
		++os;
	}
}

static void game_handle_objects2() {
	int i;
	object_state_t *os = objects_table_ptr2;
	for (i = 0; i < 7; ++i, ++os) {
		if (os->displayed) {
			anim_data_t *ad = os->anim_data1_ptr;
			if (ad->cycles != 0 && --os->anim_data_cycles == 0) {
				if (ad->anim_data1_ptr == 0) {
					os->displayed = 0;
					continue;
				}
				ad = ad->anim_data1_ptr;
				os->anim_data1_ptr = ad;
				os->anim_data_cycles = ad->cycles;
				os->unk26 = ad->unk4;
			}
			if (os->unk26 & 0x10) {
				if (ad->move_data_ptr != 0) {
					if (os->move_data_ptr[0] == 0x88) {
						os->move_data_ptr = ad->move_data_ptr;
					}
					os->pos_x += (int8)os->move_data_ptr[0];
					os->pos_y += (int8)os->move_data_ptr[1];
					os->move_data_ptr += 2;
				} else {
					os->pos_x += ad->distance_dx;
					os->pos_y += ad->distance_dy;
				}
			} else {
				int j, count = 4;
				s16 delta = os->anim_data3_ptr_;
				if (NICKY2) {
					count = 3;
					if (monster_fight_flag) {
						if (current_level > 4) {
							count = 4 + ((current_level - 4) >> 1);
						} else if (current_level > 2 && cycle_counter != 0) {
							count = 4;
						}
					}
				}
				for (j = 0; j < count; ++j) {
					os->anim_data4_seg_ += os->map_pos_y;
					os->anim_data4_ptr_ += os->map_pos_x;
					if (delta < os->anim_data4_ptr_) {
						os->anim_data4_ptr_ -= delta;
						os->pos_x += os->distance_pos_x;
					}
					if (delta < os->anim_data4_seg_) {
						os->anim_data4_seg_ -= delta;
						os->pos_y += os->distance_pos_y;
					}
				}
			}
			os->pos_x -= decor_state.delta_x;
			if (os->pos_x >= -8 && os->pos_x <= GAME_SCREEN_W) {
				os->pos_y -= decor_state.delta_y;
				if (os->pos_y >= -8 && os->pos_y <= GAME_SCREEN_H) {
					game_draw_object(os);
					continue;
				}
			}
			os->displayed = 0;
		}
	}
}

/* cycle tile offsets */
static void game_update_tile_offsets() {
	int i;
	tile_anim_t *ta;

	--game_state.tile_cycling_counter;
	if (game_state.tile_cycling_counter < 0) {
		game_state.tile_cycling_counter = 3;
		for (i = 0; i < 4; ++i) {
			ta = &game_state.tile_anims_table[i];
			++ta->anim;
			if (ta->anim == 4) {
				ta->anim = 0;
			}
			_screen_cdg_offsets[ta->tile_num] = ta->tiles_table[ta->anim];
		}
	}
}

/* bomb_collision */
static void game_update_helper4() {
	object_state_t *os = objects_table_ptr3;
	if (os->displayed == 2) {
		bounding_box_y1 = os->pos_y + os->anim_data1_ptr->bounding_box_y1;
		bounding_box_x1 = os->pos_x + os->anim_data1_ptr->bounding_box_x1;
		bounding_box_y2 = os->pos_y + os->anim_data1_ptr->bounding_box_y2;
		bounding_box_x2 = os->pos_x + os->anim_data1_ptr->bounding_box_x2;
		for (os = objects_list_cur; os <= objects_list_last; ++os) {
			if (os->displayed == 2) {
				if (os->unk26 & 4) {
					anim_data_t *ad = os->anim_data1_ptr;
					if (os->pos_y + ad->bounding_box_y2 < bounding_box_y1 || os->pos_y + ad->bounding_box_y1 > bounding_box_y2) {
						continue;
					}
					if (os->pos_x + ad->bounding_box_x2 < bounding_box_x1 || os->pos_x + ad->bounding_box_x1 > bounding_box_x2) {
						continue;
					}
					if (NICKY1) {
						os->transparent_flag = 16;
					}
					if (NICKY2) {
						os->transparent_flag |= 0x80;
					}
					os->life -= 4;
					if (os->life == 0 || os->life < 0) {
						cycle_score += ad->score;
						if (ad->colliding_opcode == 18) {
							game_change_decor_tile_map_line(os, ad);
						}
						if (ad->unk4 & 8) {
							game_change_decor_tile_map_quad(os);
						}
						if (ad->unk4 & 0x40) {
							object_state_t *os2 = os + 1;
							anim_data_t *ad2 = &res_ref_ref[os->ref_ref_index];
							os2->anim_data2_ptr = ad2;
							os2->displayed = ad2->unk0;
						}
						if (ad->anim_data3_ptr == 0) {
							os->displayed = (os->visible != 0) ? 1 : 0;
						} else {
							if (os->visible == 0) {
								os->anim_data2_ptr = 0;
							}
							os->transparent_flag = 0;
							game_init_object_from_anim3(os, ad);
						}
						objects_table_ptr3->displayed = 1;
						return;
					}
				}
			}
		}
	}
}

static void game2_update_nicky_anim2_helper() {
	player_state.rock_hit_counter = 0;
	if (nicky_anim_flags3 != 0) {
		if ((player_state.bird_dir_flags & 4) == 0) {
			player_state.pos_y += 14;
		} else {
			player_state.pos_y += 10;
		}
		player_state.hdir_flags &= 1;
		player_state.pos_x += 16;
		player_state.dim_w = 16;
		player_state.dim_h = 27;
		player_state.pos_dx_1 = 10;
		player_state.pos_dy_1 = 26;
		player_state.bird_dir_flags = 0;
		nicky_anim_flags3 = 0;
		nicky_colliding_op39_moving_table = 0;
	}
}

static void game_update_life_bar() {
	if (NICKY2) {
		if (nicky_colliding_op39_moving_table || (nicky_anim_flags1 & 0xFF) != 0) {
			player_state.colliding = 0;
		}
	}
	if (player_state.increment_life_bar) {
		/* original game used 44 in Nicky1 and 48 in Nicky2 */
		if (player_state.energy >= 64) {
			player_state.increment_life_bar = 0;
			return;
		}
		++player_state.energy;
		if (player_state.pos_y >= GAME_SCREEN_H) {
			if (NICKY1) {
				player_state.pos_y = 161;
			}
			if (NICKY2) {
				player_state.pos_y = 162;
			}
		}
		return;
	}
	if (player_state.pos_y >= GAME_SCREEN_H) {
		if (NICKY1) {
			player_state.pos_y = 161;
		}
		if (NICKY2) {
			player_state.pos_y = 162;
		}
	} else {
		if (player_state.shield_duration > 0) {
			return;
		}
		if (update_nicky_anim_flag == 0) {
			return;
		}
		if (player_state.colliding == 0) {
			return;
		}
	}
	print_debug(DBG_GAME, "game_update_life_bar() energy=%d", player_state.energy);
	--player_state.colliding;
	if (player_state.energy) {
		--player_state.energy;
		return;
	}
	if (player_state.dead_flag) {
		return;
	}
	--player_state.lifes_count;
	if (player_state.lifes_count == 0) {
		quit_level_flag = 1;
	}
	if (player_state.has_blue_ball) {
		player_state.has_blue_ball = 0;
	} else {
		cur_anim_data_ptr = &anim_data0[0];
		cur_anim_data_count = 3;
	}
	player_state.tilemap_offset = player_state.next_tilemap_offset - 2;
	game_adjust_player_position();
	player_state.dead_flag = 1;
	print_debug(DBG_GAME, "game_update_life_bar() dead_flag");
	if (NICKY2) {
		game2_update_nicky_anim2_helper();
	}
	update_nicky_anim_flag = 0;
	player_state.move_offsets_data_ptr = nicky_move_offsets_table5;
	snd_play_sfx(SND_SAMPLE_8);
}

void game_run_cycle() {
	cycle_counter++;
	cycle_counter &= 1;
	inp_copy_state();
	game_adjust_objects_position();
	if (NICKY1) {
		game_update_nicky_anim();
	}
	if (NICKY2) {
		game2_update_nicky_anim1();
		game2_update_nicky_anim2();
	}
	game_handle_nicky_shield();
	game_play_sound();
	game_throw_apple();
	game_adjust_tilemap();
	game_execute_logic_op();
	game_handle_objects2();
	game_update_helper3();
	game_handle_objects6();
	game_execute_action_op();
	game_handle_projectiles_collisions();
	game_update_helper4();
	game_execute_colliding_op();
	game_update_tile_offsets();
	game_update_score();
	game_update_life_bar();
	game_handle_level_change();
	game_draw_nicky();
	if (inventory_enabled_flag) {
		game_redraw_inventory();
	}
	sys_add_to_sprite_list(res_lifebar_spr, player_state.energy * 36 >> 6, 4, 4, SSF_VISIBLE, 3);
}

void game_init_player_state() {
	memset(&player_state, 0, sizeof(player_state_t));
	if (NICKY1) {
		player_state.dim_w = 16;
		player_state.dim_h = 29;
		player_state.pos_dx_1 = 10;
		player_state.pos_dy_1 = 28;
		player_state.anim_frame_ptr = &nicky_anim_frames_table__v1[19];
	}
	if (NICKY2) {
		player_state.dim_w = 16;
		player_state.dim_h = 27;
		player_state.pos_dx_1 = 10;
		player_state.pos_dy_1 = 26;
		player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[19];
	}
	player_state.hdir_flags = 1;
	player_state.vdir_flags = 0;
	player_state.pos_x = -16;
	player_state.pos_y = -16;
	player_state.lifes_count = 5;
	cur_anim_data_ptr = anim_data0;
	cur_anim_data_count = 3;
	if (NICKY1) {
		nicky_move_offsets_ptr = &nicky_move_offsets_table__v1[40];
	}
	if (NICKY2) {
		nicky_move_offsets_ptr = &nicky_move_offsets_table__v2[47];
	}
}

void game_update_nicky_anim() {
	player_state.decor_ref_flag2 = 0;
	player_state.decor_ref_flag0 = 0;
	decor_state.scroll_inc = 0;
	if (update_nicky_anim_flag) {
		if (inp_direction_mask & GAME_DIR_UP) {
			if (player_state.vdir_flags == 3) {
				player_state.vdir_flags = 0;
			}
		} else {
			player_state.vdir_flags |= 1;
		}
		game_update_anim_helper();
		if (player_state.hdir_flags & 8) {
			game_set_next_nicky_anim_frame();
			if (player_state.anim_frame_ptr->frames_count == 0) {
				player_state.hdir_flags &= ~8;
			}
		}
		if (player_state.decor_ref_flag2) {
			if (snd_sample_priority_table[0] >= _snd_current_sfx_priority) {
				_snd_current_sfx_priority = snd_sample_priority_table[0];
				_snd_current_sfx_num = 0;
			}
		}
	} else if (player_state.dead_flag) {
		print_debug(DBG_GAME, "game_update_nicky_anim() player_state.dead_flag player_state.pos_y=%d", player_state.pos_y);
		if (player_state.pos_y >= 161) {
			if (quit_level_flag) {
				quit_level_flag = 2;
			} else {
				restart_level_flag = 1;
			}
		} else {
			uint16 dx = player_state.move_offsets_data_ptr[0];
			uint16 dy = player_state.move_offsets_data_ptr[1];
			if (dx == 0x8888) {
				player_state.pos_y = 161;
			} else {
				player_state.move_offsets_data_ptr += 2;
				player_state.pos_x += (s16)dx;
				player_state.pos_y += (s16)dy;
			}
		}
	} else if (player_state.action_code) {
		if (player_state.action_code == 1) {
			anim_frame_t *af = &nicky_anim_frames_table__v1[40];
			if (player_state.hdir_flags & 1) {
				af = &nicky_anim_frames_table__v1[46];
			}
			player_state.anim_frame_ptr = af;
			player_state.anim_frames_count = 2;
			player_state.action_code = 2;
		}
		if (player_state.action_code < 3) {
			if (player_state.anim_frame_ptr->action_op3_flag) {
				player_state.action_code = 3;
				execute_action_op_flag = 1;
			}
		}
		game_set_next_nicky_anim_frame();
	}
}

void game2_update_nicky_anim1() {
	object_state_t *os;
	anim_data_t *ad;
	nicky_on_elevator_flag = 0;
	if (player_state.dead_flag) {
		game2_update_nicky_anim2_helper();
		return;
	}
	if (nicky_anim_flags3) {
		return;
	}
	if (nicky_colliding_op39_moving_table) {
		if (read_uint16LE(nicky_colliding_op39_moving_table) != 0x8888) {
			player_state.colliding = 0;
			player_state.pos_x += nicky_colliding_op39_moving_table[0];
			player_state.pos_y += nicky_colliding_op39_moving_table[1];
			if (nicky_colliding_op39_moving_table[0] < 0) {
				player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[41];
				player_state.hdir_flags &= ~1;
			} else if (nicky_colliding_op39_moving_table[0] != 0) {
				player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[40];
				player_state.hdir_flags |= 1;
			}
			nicky_colliding_op39_moving_table += 2;
			return;
		}
		nicky_colliding_op39_moving_table = 0;
		player_state.hdir_flags &= ~0x34;
		player_state.unk1 = 0;
		player_state.move_offsets_data_ptr = &nicky_move_offsets_table__v2[57];
		if ((player_state.hdir_flags & 1) == 0) {
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[31];
		} else {
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[19];
		}
	}
	if (player_state.unk1 == 2) {
		if (player_state.move_offsets_data_ptr[0] & 0x8000) {
			return;
		}
	}
	bounding_box_y1 = player_state.pos_y + player_state.dim_h - 2;
	bounding_box_x1 = player_state.pos_x + 2;
	bounding_box_y2 = player_state.pos_y + player_state.dim_h + 2;
	bounding_box_x2 = player_state.pos_x + 14;
	os = objects_list_cur;
	do {
		if (os->displayed == 2) {
			if (os->unk26 & 0x40) {
				ad = os->anim_data1_ptr;
				if (os->pos_y + ad->bounding_box_y2 >= bounding_box_y1 && os->pos_y + ad->bounding_box_y1 <= bounding_box_y2) {
					if (os->pos_x + ad->bounding_box_x2 >= bounding_box_x1 && os->pos_x + ad->bounding_box_x1 <= bounding_box_x2) {
						nicky_on_elevator_flag = 1;
						player_state.pos_y = os->pos_y - player_state.dim_h;
						player_state.pos_y += os->distance_pos_y;
						player_state.pos_x += os->distance_pos_x;
						player_state.hdir_flags &= ~0x34;
						player_state.unk1 = 0;
						player_state.move_offsets_data_ptr = &nicky_move_offsets_table__v2[57];
						nicky_elevator_object = os;
						return;
					}
				}
			}
		}
	} while (os++ < objects_list_last);
}

static void game2_update_nicky_anim3() {
	if (nicky_anim_flags1 != 0) {
		inp_direction_mask = 8;
		if (player_state.hdir_flags & 1) {
			inp_direction_mask |= 1;
		} else {
			inp_direction_mask |= 2;
		}
	}
	nicky_anim_flags1 = 0;
	player_state.decor_ref_flag2 = 0;
	player_state.decor_ref_flag0 = 0;
	decor_state.scroll_inc = 0;
	if (update_nicky_anim_flag) {
		if (player_state.rock_hit_counter) {
			inp_direction_mask &= ~0xF;
			inp_fire_button = 0;
			--player_state.rock_hit_counter;
			if (player_state.rock_hit_counter == 0) {
				player_state.hdir_flags &= ~0x80;
			}
		}
		if ((inp_direction_mask & 4) == 0) {
			player_state.vdir_flags |= 1;
		} else {
			if (player_state.vdir_flags == 3) {
				player_state.vdir_flags = 0;
			}
		}
		game_update_anim_helper();
		if (player_state.hdir_flags & 8) {
			game_set_next_nicky_anim_frame();
			if (player_state.anim_frame_ptr->frames_count == 0) {
				player_state.hdir_flags &= ~8;
			}
		}
		if (player_state.decor_ref_flag2) {
			if (snd_sample_priority_table[0] >= _snd_current_sfx_priority) {
				_snd_current_sfx_priority = snd_sample_priority_table[0];
				_snd_current_sfx_num = 0;
			}
		}
		return;
	}
	if (player_state.dead_flag) {
		game2_update_nicky_anim2_helper();
		if (player_state.pos_y >= 162) {
			if (quit_level_flag) {
				quit_level_flag = 2;
			} else {
				restart_level_flag = 1;
			}
		} else {
			uint16 dx = player_state.move_offsets_data_ptr[0];
			uint16 dy = player_state.move_offsets_data_ptr[1];
			if (dx == 0x8888) {
				player_state.pos_y = 162;
			} else {
				player_state.move_offsets_data_ptr += 2;
				player_state.pos_x += (s16)dx;
				player_state.pos_y += (s16)dy;
			}
		}
		return;
	}
	if (player_state.action_code) {
		anim_frame_t *af;
		if (player_state.action_code == 1) {
			af = &nicky_anim_frames_table__v2[60];
			if (player_state.hdir_flags & 1) {
				af = &nicky_anim_frames_table__v2[66];
			}
			player_state.anim_frame_ptr = af;
			player_state.anim_frames_count = 2;
			player_state.action_code = 2;
		}
		if (player_state.action_code < 3) {
			if (player_state.anim_frame_ptr->action_op3_flag) {
				player_state.action_code = 3;
				execute_action_op_flag = 1;
			}
		}
		game_set_next_nicky_anim_frame();
	}
}

static void game2_update_nicky_bird_anim_helper1() {
	game2_update_nicky_anim2_helper();
	game2_update_nicky_anim3();
}

static void game2_update_nicky_bird_anim_helper2() {
	anim_frame_t *af = player_state.anim_frame_ptr;
	if (af != &nicky_anim_frames_table__v2[125] && af != &nicky_anim_frames_table__v2[118]) {
	    if ((nicky_anim_flags3 & 0x100) == 0) {
	    	nicky_anim_flags3 |= 0x100;
	    	af = &nicky_anim_frames_table__v2[124];
	    	if (player_state.hdir_flags & 1) {
	    		af = &nicky_anim_frames_table__v2[117];
	    	}
			player_state.pos_y -= 14;
			player_state.pos_x -= 16;
			player_state.dim_w = 48;
			player_state.dim_h = 41;
			player_state.pos_dx_1 = 44;
			player_state.pos_dy_1 = 41;
	    	player_state.anim_frame_ptr = af;
	    	player_state.anim_frames_count = af->frames_count;
	    }
		game_set_next_nicky_anim_frame();
	} else {
		nicky_anim_flags3 &= ~0x4100;
		nicky_anim_flags3 |= 0x8000;
		update_nicky_anim_flag = 0xFF;
		player_state.bird_dir_flags = 0;
/*		player_state.unk42 = &logic_op24_table[0];*/
		if ((player_state.hdir_flags & 1) == 0) {
			af = &nicky_anim_frames_table__v2[134];
		} else {
			af = &nicky_anim_frames_table__v2[133];
		}
    	player_state.anim_frame_ptr = af;
    	player_state.anim_frames_count = af->frames_count;
		game_set_next_nicky_anim_frame();
	}
}

static void game2_update_nicky_bird_anim_dir_helper() {
	s16 _cx, _dx;
	anim_frame_t *af;

	player_state.decor_ref_flag4 = 0;
	if (nicky_on_elevator_flag == 0) {
		_cx = 0;
		_dx = 0;
		if ((player_state.hdir_flags & 1) == 0) {
			_cx += 12;
		} else {
			_cx += 8;
		}
		_cx += player_state.pos_x + player_state.pos_dx_2;
		_cx >>= 4;
		_cx = _screen_cdg_tile_map_h * _cx;
		_cx += decor_state.tile_map_offs_x;
		_dx += player_state.pos_y + player_state.pos_dy_2;
		_dx += 41;
		if (_dx <= 160) {
			_dx >>= 4;
			_dx += decor_state.tile_map_offs_y;
			_cx += _dx;
			nicky_cur_pos_x1 = _cx;
			player_state.decor_ref_flag0 = 0;
			player_state.decor_ref_flag3 = 0;
			game_set_decor_tile_flags4_3_2_0(1, _cx);
			if (player_state.decor_ref_flag3 == 0 && (player_state.decor_ref_flag0 & 1) == 0) {
				_cx = nicky_cur_pos_x1 + _screen_cdg_tile_map_h;
				game_set_decor_tile_flags4_3_2_0(1, _cx);
				if (player_state.decor_ref_flag3 == 0 && (player_state.decor_ref_flag0 & 1) == 0) {
					if (player_state.pos_y <= 147) {
						player_state.pos_y += nicky_cur_pos_y;
						if (nicky_cur_pos_x2 == 0) {
							if (player_state.bird_dir_flags & 1) {
								player_state.bird_dir_flags &= ~5;
							}
						}
						if ((player_state.bird_dir_flags & 4) == 0) {
							player_state.bird_dir_flags |= 4;
							if ((player_state.hdir_flags & 1) == 0) {
								player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[80];
							} else {
								player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[72];
							}
					    	player_state.anim_frames_count = 2;
						}
						goto loc_19DA;
					}
				}
			}
		}
	}
	player_state.bird_counter &= ~1;
	if (player_state.bird_dir_flags & 4) {
		player_state.bird_dir_flags &= ~2;
		af = &nicky_anim_frames_table__v2[134];
		if (inp_direction_mask & 2) {
			af = &nicky_anim_frames_table__v2[125];
			player_state.bird_dir_flags |= 2;
		}
		if (player_state.hdir_flags & 1) {
			player_state.bird_dir_flags &= ~2;
			af = &nicky_anim_frames_table__v2[133];
			if ((inp_direction_mask & 1) == 0) {
				af = &nicky_anim_frames_table__v2[118];
				player_state.bird_dir_flags |= 2;
			}
		}
		player_state.anim_frame_ptr = af;
		player_state.anim_frames_count = 2;
		player_state.bird_dir_flags &= ~4;
	}
loc_19DA:
	if (player_state.decor_ref_flag4 && cycle_counter == 0) {
		++player_state.colliding;
	}
}

static void game2_update_nicky_bird_anim_dir_right() {
	s16 _cx, _dx;

	_cx = _dx = 0;
	if ((player_state.bird_dir_flags & 4) == 0) {
		if ((player_state.hdir_flags & 1) == 0) {
			_cx += 31;
		} else {
			_cx += 32;
		}
	}
	_cx += player_state.pos_x + player_state.pos_dx_2;
	_cx >>= 4;
	_cx *= _screen_cdg_tile_map_h;
	_cx += decor_state.tile_map_offs_x;
	_dx += player_state.pos_y + player_state.pos_dy_2 + 40;
	if (_dx > 160) {
		return;
	}
	_dx >>= 4;
	_dx += decor_state.tile_map_offs_y;
	_cx += _dx;
	nicky_cur_pos_x1 = _cx;
	player_state.decor_ref_flag0 = 0;
	player_state.decor_ref_flag3 = 0;
	if ((player_state.bird_dir_flags & 4) == 0) {
		_cx -= 2;
		game_set_decor_tile_flags4_3_2_0(4, _cx);
		if (player_state.decor_ref_flag0 & 4) {
			goto loc_1BC7;
		}
	}
	game_set_decor_tile_flags4_3_2_0(4, nicky_cur_pos_x1 - 1);
	if ((player_state.decor_ref_flag0 & 4) == 0) {
		game_set_decor_tile_flags4_3_2_0(4, nicky_cur_pos_x1);
		if ((player_state.decor_ref_flag0 & 4) == 0) {
			if (player_state.pos_x < GAME_SCREEN_W - 32) {
				player_state.pos_x += GAME_SCROLL_DX;
			}
		}
	}
loc_1BC7:
	if ((player_state.bird_dir_flags & 4) == 0) {
		player_state.pos_y = (player_state.pos_y + player_state.pos_dy_2 + 52) & ~15;
		player_state.pos_y -= 41;
		player_state.pos_y -= player_state.pos_dy_2;
	}
	if ((player_state.hdir_flags & 1) != 0) {
		if (player_state.bird_dir_flags & 2) {
			return;
		}
		player_state.bird_dir_flags |= 2;
		player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[118];
		if (player_state.bird_dir_flags & 4) {
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[72];
			if ((player_state.bird_dir_flags & 1) != 0 && player_state.bird_counter <= 192) {
				player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[88];
			}
		}
		player_state.anim_frames_count = 2;
	} else {
		player_state.bird_dir_flags |= 8;
		player_state.bird_dir_flags &= ~1;
		player_state.bird_counter = 0;
		player_state.bird_dir_flags &= ~2;
		player_state.bird_move_speed = 0;
		if ((player_state.bird_dir_flags & 4) == 0) {
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[144];
			if (player_state.hdir_flags & 1) {
				player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[141];
			}
		} else {
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[138];
			if (player_state.hdir_flags & 1) {
				player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[135];
			}
		}
		if (player_state.hdir_flags & 1) {
			player_state.hdir_flags &= ~1;
		} else {
			player_state.hdir_flags |= 1;
		}
		player_state.anim_frames_count = 2;
	}
}

static void game2_update_nicky_bird_anim_helper5() {
	nicky_cur_pos_y = 0;
	nicky_cur_pos_x2 = -1;
	game2_update_nicky_bird_anim_dir_helper();
}

static void game2_update_nicky_bird_anim_dir_left() {
	s16 _cx, _dx;

	_cx = _dx = 0;
	if ((player_state.bird_dir_flags & 4) == 0) {
		if ((player_state.hdir_flags & 1) == 0) {
			_cx += 5;
		} else {
			_cx += 2;
		}
	}
	_cx += player_state.pos_x + player_state.pos_dx_2;
	_cx >>= 4;
	_cx *= _screen_cdg_tile_map_h;
	_cx += decor_state.tile_map_offs_x;
	_dx += player_state.pos_y + player_state.pos_dy_2 + 40;
	if (_dx > 160) {
		return;
	}
	_dx >>= 4;
	_dx += decor_state.tile_map_offs_y;
	_cx += _dx;
	nicky_cur_pos_x1 = _cx;
	player_state.decor_ref_flag0 = 0;
	player_state.decor_ref_flag3 = 0;
	if ((player_state.bird_dir_flags & 4) == 0) {
		_cx -= 2;
		game_set_decor_tile_flags4_3_2_0(8, _cx);
		if (player_state.decor_ref_flag0 & 8) {
			goto loc_1A86;
		}
	}
	game_set_decor_tile_flags4_3_2_0(8, nicky_cur_pos_x1 - 1);
	if ((player_state.decor_ref_flag0 & 8) == 0) {
		game_set_decor_tile_flags4_3_2_0(8, nicky_cur_pos_x1);
		if ((player_state.decor_ref_flag0 & 8) == 0) {
			if (player_state.pos_x > 0) {
				player_state.pos_x -= GAME_SCROLL_DX;
			}
		}
	}
loc_1A86:
	if ((player_state.bird_dir_flags & 4) == 0) {
		player_state.pos_y = (player_state.pos_y + player_state.pos_dy_2 + 52) & ~15;
		player_state.pos_y -= 41;
		player_state.pos_y -= player_state.pos_dy_2;
	}
	if ((player_state.hdir_flags & 1) == 0) {
		if (player_state.bird_dir_flags & 2) {
			return;
		}
		player_state.bird_dir_flags |= 2;
		player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[125];
		if (player_state.bird_dir_flags & 4) {
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[80];
			if ((player_state.bird_dir_flags & 1) != 0 && player_state.bird_counter <= 192) {
				player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[96];
			}
		}
		player_state.anim_frames_count = 2;
	} else {
		player_state.bird_dir_flags |= 8;
		player_state.bird_dir_flags &= ~1;
		player_state.bird_counter = 0;
		player_state.bird_dir_flags &= ~2;
		player_state.bird_move_speed = 0;
		if ((player_state.bird_dir_flags & 4) == 0) {
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[144];
			if (player_state.hdir_flags & 1) {
				player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[141];
			}
		} else {
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[138];
			if (player_state.hdir_flags & 1) {
				player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[135];
			}
		}
		if (player_state.hdir_flags & 1) {
			player_state.hdir_flags &= ~1;
		} else {
			player_state.hdir_flags |= 1;
		}
		player_state.anim_frames_count = 2;
	}
}

static void game2_update_nicky_bird_anim_dir_up() {
	s16 _cx, _dx;

	_cx = _dx = 0;
	if ((player_state.bird_dir_flags & 4) == 0) {
		if ((player_state.hdir_flags & 1) == 0) {
			_cx += 6;
		} else {
			_cx += 2;
		}
	}
	_cx += player_state.pos_x + player_state.pos_dx_2 + 8;
	_cx >>= 4;
	_cx *= _screen_cdg_tile_map_h;
	_cx += decor_state.tile_map_offs_x;

	_dx += player_state.pos_y + player_state.pos_dy_2 + 40;
	if (_dx <= 0 || _dx > 160) {
		return;
	}
	_dx >>= 4;
	_dx += decor_state.tile_map_offs_y;
	_cx += _dx;
	nicky_cur_pos_x1 = _cx;
	player_state.decor_ref_flag0 = 0;
	player_state.decor_ref_flag3 = 0;
	game_set_decor_tile_flags4_3_2_0(2, nicky_cur_pos_x1);
	if (player_state.decor_ref_flag3 || (player_state.decor_ref_flag0 & 2) != 0) {
		return;
	}
	game_set_decor_tile_flags4_3_2_0(2, nicky_cur_pos_x1 + _screen_cdg_tile_map_h);
	if (player_state.decor_ref_flag3 || (player_state.decor_ref_flag0 & 2) != 0) {
		return;
	}
	if (player_state.pos_y <= 0) {
		return;
	}
	player_state.pos_y -= GAME_SCROLL_DY;
	if (player_state.bird_dir_flags & 1) {
		player_state.bird_dir_flags &= ~3;
	}
	if ((player_state.bird_dir_flags & 4) == 0) {
		player_state.bird_dir_flags &= ~2;
		player_state.bird_dir_flags |= 4;
	}
	if ((player_state.bird_dir_flags & 2) == 0) {
		player_state.bird_dir_flags |= 2;
		player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[80];
		if (player_state.hdir_flags & 1) {
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[72];
		}
		player_state.anim_frames_count = 2;
	}
}

static void game2_update_nicky_bird_anim_dir_down() {
	nicky_cur_pos_y = GAME_SCROLL_DY;
	nicky_cur_pos_x2 = 0;
	game2_update_nicky_bird_anim_dir_helper();
}

static void game2_update_nicky_bird_anim_shoot() {
	++nicky_anim_flags3;
	if ((nicky_anim_flags3 & 0xFF) > 30) {
		nicky_anim_flags3 = 0x2000;
	}
}

static void game2_update_nicky_bird_anim_helper3() {
	s16 _cx, _dx;
	const uint16 *_bx;

	if ((player_state.bird_dir_flags & 8) == 0) {
		if (inp_direction_mask & GAME_DIR_RIGHT) {
			game2_update_nicky_bird_anim_dir_right();
			game2_update_nicky_bird_anim_helper5();
		}
		if (inp_direction_mask & GAME_DIR_LEFT) {
			game2_update_nicky_bird_anim_dir_left();
			game2_update_nicky_bird_anim_helper5();
		}
		if (inp_direction_mask & GAME_DIR_UP) {
			game2_update_nicky_bird_anim_dir_up();
		}
		if (inp_direction_mask & GAME_DIR_DOWN) {
			game2_update_nicky_bird_anim_dir_down();
		}
		if ((inp_direction_mask & 0xF) == 0) {
			game2_update_nicky_bird_anim_helper5();
		}
		if (inp_fire_button) {
			game2_update_nicky_bird_anim_shoot();
		} else {
			nicky_anim_flags3 &= 0xFF00;
		}
	}
	if (player_state.bird_dir_flags & 8) {
		++player_state.bird_move_speed;
		if (player_state.bird_move_speed >= 6) {
			player_state.bird_dir_flags &= ~8;
		}
	}
	if ((player_state.bird_dir_flags & 4) == 0 && (inp_direction_mask & 3) == 0) {
		player_state.bird_dir_flags &= ~2;
		if (player_state.hdir_flags & 1) {
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[133];
		} else {
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[134];
		}
		player_state.anim_frames_count = 2;
	}
	if ((inp_direction_mask & 0xC) == 0 && (player_state.bird_dir_flags & 4) != 0) {
		if ((player_state.bird_dir_flags & 1) == 0) {
			player_state.bird_dir_flags |= 1;
			player_state.bird_counter = 0;
			player_state.bird_move_dy_ptr = &bird_move_dy_table[0];
		}
		if (player_state.bird_counter == 192) {
			if ((player_state.bird_dir_flags & 8) == 0) {
				if (player_state.hdir_flags & 1) {
					player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[72];
				} else {
					player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[80];
				}
			}
			player_state.anim_frame_ptr += 16;
		}
		++player_state.bird_counter;
		if (player_state.bird_counter != 0) {
			if (player_state.bird_counter <= 240) {
				return;
			}
		}
		_dx = 0;
		if ((player_state.bird_counter & 0xF) == 0) {
			player_state.bird_counter = 240;
			_dx = 1;
		}
		_cx = 4;
		_bx = player_state.bird_move_dy_ptr;
		if (_bx[0] != 0x7777 && _bx[1] != 0x7777) {
			_cx = _bx[player_state.bird_counter & 1];
			player_state.bird_move_dy_ptr = _bx + _dx;
		}
/*		if (_cx > 2) {
			nicky_anim_flags2 = _cx;
		}*/
		nicky_cur_pos_y = _cx;
		nicky_cur_pos_x2 = -1;
		game2_update_nicky_bird_anim_dir_helper();
	} else {
		player_state.bird_counter = 0;
		player_state.bird_dir_flags &= ~1;
	}
}

static void game2_update_nicky_bird_anim() {
	if (nicky_anim_flags3 & 0x2000) {
		game2_update_nicky_bird_anim_helper1();
	} else if (nicky_anim_flags3 & 0x4000) {
		game2_update_nicky_bird_anim_helper2();
	} else if (nicky_anim_flags3 & 0x8000) {
		game2_update_nicky_bird_anim_helper3();
		game_set_next_nicky_anim_frame();
		if (nicky_anim_flags3 & 0x2000) {
			game2_update_nicky_bird_anim_helper1();
		}
	}
}

void game2_update_nicky_anim2() {
/*	nicky_anim_flags2 = 0;*/
	if (nicky_colliding_op39_moving_table) {
		return;
	}
	if (nicky_anim_flags3 != 0) {
		game2_update_nicky_bird_anim();
		return;
	}
	game2_update_nicky_anim3();
}

void game_adjust_player_position() {
	int x, y, x_max, y_max;
	if (NICKY1) {
		x_max = 20000 - (GAME_SCREEN_W / 16) * 50; /* 19000 */
		y_max = 50 - 10;
		x = player_state.tilemap_offset / 50;
		y = player_state.tilemap_offset % 50;
		player_pos_x = x * 16;
		player_pos_y = y * 16 + 3;
		y -= 8;
		x = x * 50 - 500;
	}
	if (NICKY2) {
		x_max = (_screen_cdg_tile_map_w - 20) * _screen_cdg_tile_map_h;
		y_max = _screen_cdg_tile_map_h - 10;
		x = player_state.tilemap_offset / _screen_cdg_tile_map_h;
		y = player_state.tilemap_offset % _screen_cdg_tile_map_h;
		player_pos_x = x * 16;
		player_pos_y = y * 16 + 1;
		y -= 7;
		x = x * _screen_cdg_tile_map_h - _screen_cdg_tile_map_h * 10;
	}
	if (y < 0) {
		y = 0;
	} else if (y > y_max) {
		y = y_max;
	}
	if (x < 0) {
		x = 0;
	} else if (x > x_max) {
		x = x_max;
	}
	if (monster_fight_flag) {
		y = y_max;
		x = x_max;
	}
	_screen_tile_origin_x = x;
	_screen_tile_origin_y = y;
	print_debug(DBG_GAME, "game_adjust_player_position() offs=%d origin=%d,%d", player_state.tilemap_offset, _screen_tile_origin_x, _screen_tile_origin_y);
}

void game_handle_nicky_shield() {
	if (player_state.shield_duration > 0) {
		int i;
		s16 delta = 0;
		object_state_t *os = game_state.starshield_object_state;
		if (player_state.shield_duration <= 50) {
			os->map_pos_y = -1;
		}
		delta = os->map_pos_y;
		if (delta == 0) {
			goto next_shield_step;
		} else if (delta > 0) {
			if (os->distance_pos_x >= 30) {
				os->map_pos_y = 0;
			}
			goto next_shield_step;
		} else if (os->distance_pos_x > -1) {
			player_state.shield_duration = 24;
			goto next_shield_step;
		}

		player_state.shield_duration = 0;
		for (i = 0; i < 4; ++i) {
			os->displayed = 0;
			++os;
		}
		player_state.colliding = 0;
		return;

next_shield_step:
		--player_state.shield_duration;
		for (i = 0; i < 4; ++i) {
			os->distance_pos_x += delta;
			os->distance_pos_y += delta;
			if (NICKY1) {
				os->map_pos_x = (os->map_pos_x + 24) & 0x3FF;
			}
			if (NICKY2) {
				if ((player_state.hdir_flags & 1) == 0) {
					os->map_pos_x += 24;
				} else {
					os->map_pos_x -= 24;
				}
				os->map_pos_x &= 0x3FF;
			}
			os->pos_x = ((s16)cos_table[os->map_pos_x >> 1] * os->distance_pos_x) >> 8;
			os->pos_x += player_state.pos_x + 4;
			if (NICKY2) {
				if (nicky_anim_flags3 != 0 && (nicky_anim_flags3 & 0x2000) == 0) {
					if ((player_state.hdir_flags & 1) == 0) {
						if ((player_state.bird_dir_flags & 4) == 0) {
							os->pos_x += 10;
						} else {
							os->pos_x += 2;
						}
					} else {
						if ((player_state.bird_dir_flags & 4) == 0) {
							os->pos_x += 8;
						} else {
							os->pos_x += 14;
						}
					}
				}
			}
			os->pos_y = ((s16)sin_table[os->map_pos_x >> 1] * os->distance_pos_y) >> 8;
			os->pos_y += player_state.pos_y;
			if (NICKY2) {
				++os->pos_y;
			}
			--os->anim_data_cycles;
			if (os->anim_data_cycles <= 0) {
				anim_data_t *ad = os->anim_data1_ptr;
				os->anim_data_cycles = 5;
				os->anim_data_cycles = ad->cycles;
				os->anim_data1_ptr = ad->anim_data1_ptr;
			}
			++os;
		}
	}
}

void game_play_sound() {
	if (_snd_current_sfx_num >= 0) {
		snd_play_sfx(_snd_current_sfx_num);
	}
	_snd_current_sfx_num = -1;
	_snd_current_sfx_priority = 0;
}

void game_draw_bomb_object() {
	object_state_t *os = objects_table_ptr3;
	if (os->displayed) {
		anim_data_t *ad = os->anim_data1_ptr;
		if (ad->cycles != 0 && --os->anim_data_cycles == 0) {
			if (ad->anim_data1_ptr == 0) {
				os->displayed = 0;
				return;
			}
			ad = ad->anim_data1_ptr;
			os->anim_data1_ptr = ad;
			os->anim_data_cycles = ad->cycles;
			if (ad->colliding_opcode == 2) {
				os->pos_x -= 8;
				os->pos_y -= 8;
				os->displayed = 2;
				snd_play_sfx(SND_SAMPLE_1);
			}
		}
		os->pos_x -= decor_state.delta_x;
		os->pos_y -= decor_state.delta_y;
		if (os->pos_x >= 0 && os->pos_x <= GAME_SCREEN_W && os->pos_y >= 0 && os->pos_y <= GAME_SCREEN_H) {
			game_draw_object(os);
		} else {
			os->displayed = 0;
		}
	}
}

void game_draw_object(object_state_t *os) {
	anim_data_t *ad = os->anim_data1_ptr;
	if (ad->anim_h != 0) {
		const uint8 *spr = res_level_spr;
		if (NICKY1) {
			if ((ad->sprite_flags & 0xF000) == 0x9000) {
				spr = res_monster_spr;
			}
			if (os->transparent_flag) {
				os->transparent_flag = 0;
				sys_add_to_sprite_list(spr, ad->sprite_num, os->pos_x, os->pos_y, SSF_HITMODE, 0);
			} else {
				sys_add_to_sprite_list(spr, ad->sprite_num, os->pos_x, os->pos_y, SSF_VISIBLE, 0);
			}
		}
		if (NICKY2) {
			int x, y;
			x = os->pos_x + ad->draw_dx;
			y = os->pos_y + ad->draw_dy;
			if (ad->sprite_flags != 0x80) {
				spr = res_monster_spr;
			}
			if ((os->transparent_flag & 0x80) != 0 || (os->transparent_flag != 0 && cycle_counter == 0)) {
				sys_add_to_sprite_list(spr, ad->sprite_num, x, y, SSF_HITMODE, 0);
			} else {
				sys_add_to_sprite_list(spr, ad->sprite_num, x, y, SSF_VISIBLE, 0);
			}
			os->transparent_flag = 0;
		}
	}
}

void game_shake_screen() {
	/* disabled in PC versions, present for the Amiga in the original */
	/* TODO */
}

void game_update_decor_after_megabomb() {
	int j, offs;
	object_state_t *os;

	offs = decor_state.tile_block_x + decor_state.tile_block_y;
	for (j = 0; j < GAME_SCREEN_W / 16; ++j) {
		int i;
		for (i = 0; i < GAME_SCREEN_H / 16; ++i) {
			uint8 tile_num = res_decor_cdg[offs + i];
			if (res_decor_ref[tile_num * 8] & 0x10) {
				uint8 new_tile_num = res_decor_ref[tile_num * 8 + 6];
				if (new_tile_num != 0) {
					res_decor_cdg[offs + i] = new_tile_num;
				}
			}
		}
		offs += _screen_cdg_tile_map_h;
	}
	for (os = objects_list_cur; os <= objects_list_last; ++os) {
		anim_data_t *_bx = os->anim_data1_ptr;
		if (os->displayed != 2 || (os->unk26 & 4) == 0 || (os->unk26 & 8)) {
			continue;
		}
		if (os->pos_y >= GAME_SCREEN_H || os->pos_y + _bx->bounding_box_y2 < 0) {
			continue;
		}
		if (os->pos_x >= GAME_SCREEN_W || os->pos_x + _bx->bounding_box_x2 < 0) {
			continue;
		}
		if (_bx->lifes == 0) {
			continue;
		}
		if (NICKY1) {
			os->transparent_flag = 32;
			os->life -= 8;
		}
		if (NICKY2) {
			os->transparent_flag = 1;
			os->life -= 12;
		}
		if (os->life > 0) {
			continue;
		}
		if (_bx->score != 0) {
			cycle_score += _bx->score;
			unk_ref_index = 0;
		}
		if (_bx->colliding_opcode == 18) {
			game_change_decor_tile_map_line(os, _bx);
		}
		if (_bx->anim_data3_ptr == 0) {
			os->displayed = (os->visible != 0) ? 1 : 0;
		} else {
			if (os->visible == 0) {
				os->anim_data2_ptr = 0;
			}
			os->transparent_flag = 0;
			game_init_object_from_anim3(os, _bx);
		}
	}
}

void game_draw_nicky() {
	int i, x, y;
	anim_frame_t *af;
	object_state_t *os;

	/* add nicky sprite */
	af = player_state.anim_frame_ptr;
	x = player_state.pos_x;
	y = player_state.pos_y;
	if (NICKY2) {
		/* the original inverts for the delta when drawing nicky sprite (fixes bird drawing), weird... */
		x += af->draw_dy;
		y += af->draw_dx;
	}
	sys_add_to_sprite_list(res_nicky_spr, af->sprite_num, x, y, SSF_VISIBLE, 1);
	/* add starshield */
	os = game_state.starshield_object_state;
	for (i = 0; i < 4; ++i) {
		if (os->displayed) {
			anim_data_t *ad = os->anim_data1_ptr;
			sys_add_to_sprite_list(res_nicky_spr, ad->sprite_num, os->pos_x, os->pos_y, SSF_VISIBLE, 1);
		}
		++os;
	}
}

static void game_draw_inventory_item(int item, int count) {
	static const int icon_map_table[] = { 34, 33, 28, 27 };
	const int y = 13;
	const int x = 5 + item * 18;
	sys_add_to_sprite_list(res_level_spr, icon_map_table[item], x, y, SSF_VISIBLE, 0);
	sys_add_to_sprite_list(res_digits_spr, count, x + 10, y + 8, SSF_VISIBLE, 2);
}

void game_redraw_inventory() {
	game_draw_inventory_item(GAME_INV_MEGABOMBS, player_state.megabombs_count);
	game_draw_inventory_item(GAME_INV_BOMBS, player_state.bombs_count);
	game_draw_inventory_item(GAME_INV_KEYS, player_state.keys_count);
	game_draw_inventory_item(GAME_INV_LIFES, player_state.lifes_count);
}

uint8 game_set_decor_tile_flags4_3_2_0(uint8 b, int offset) {
	uint8 m;
	int ref_offset = res_decor_cdg[offset] * 8;
	if (res_decor_ref[ref_offset + 4] & 0xFF) {
		++player_state.decor_ref_flag4;
	}
	if (res_decor_ref[ref_offset + 3] & 0xFF) {
		++player_state.decor_ref_flag3;
	}
	if (res_decor_ref[ref_offset + 2] & 0xFF) {
		player_state.decor_ref_flag2 = 1;
		res_decor_cdg[offset] = res_decor_ref[ref_offset + 2];
	}
	m = res_decor_ref[ref_offset];
	if (b & m) {
		player_state.decor_ref_flag0 |= b;
	}
	return m;
}

void game_set_decor_tile_flag0(uint8 b, int offset) {
	int ref_offset = res_decor_cdg[offset] * 8;
	if (b & res_decor_ref[ref_offset]) {
		player_state.decor_ref_flag0 |= b;
	}
}

void game_set_next_nicky_anim_frame() {
	anim_frame_t *af = player_state.anim_frame_ptr;
	if (af->frames_count) {
		--player_state.anim_frames_count;
		if (player_state.anim_frames_count <= 0) {
			af = af->next_anim_frame;
			player_state.anim_frame_ptr = af;
			player_state.anim_frames_count = af->frames_count;
		}
	}
}

static void game_reset_objects8() {
	int i;
	object_state_t *os = objects_table_ptr8;
	player_state.fire_button_counter = 0;
	bonus_count = 0;
	execute_action_op_flag = 0;
	for (i = 0; i < 32; ++i) {
		os->displayed = 0;
		os->life = 0;
		os->distance_pos_x = 0;
		os->move_data_ptr = 0;
		os->anim_data_cycles = 0;
		os->unk3 = 0;
		os->distance_pos_y = 0;
		os->transparent_flag = 0;
		++os;
	}
}

static void game_set_tile_offsets() {
	static const uint8 game_state_unk16__v1[8] = { 149, 129, 149, 129, 0, 0, 165, 166 };
	static const uint8 game_state_unk16__v2[8] = { 252, 125, 0, 0, 129, 205, 0, 0 };
	const uint8 *p;
	int i, j, tile_num;

	for (i = 0; i < 256; ++i) {
		_screen_cdg_offsets[i] = i;
	}
	if (NICKY1) {
		tile_num = 240;
		p = game_state_unk16__v1;
	}
	if (NICKY2) {
		tile_num = 4;
		p = game_state_unk16__v2;
	}
	for (i = 0; i < 4; ++i) {
		tile_anim_t *ta = &game_state.tile_anims_table[i];
		ta->tile_num = tile_num;
		ta->anim = 0;
		for (j = 0; j < 4; ++j) {
			ta->tiles_table[j] = tile_num;
			++tile_num;
		}
	}
	game_state.tile_cycling_counter = 0;

	p += (current_level & ~1);
	_screen_blinking_tile.tile_num = p[0];
	_screen_blinking_tile.anim = 0;
	_screen_blinking_tile.tiles_table[0] = _screen_cdg_offsets[p[0]];
	_screen_blinking_tile.tiles_table[1] = _screen_cdg_offsets[p[1]];
}

static void game_update_cur_objects_ptr() {
	int x1, x2;
	object_state_t *os;

	objects_list_cur = objects_list_head;
	objects_list_last = objects_list_head;
	x1 = _screen_tile_map_offs_x_shl4 - 280;
	x2 = _screen_tile_map_offs_x_shl4 + GAME_SCREEN_W + 280;

	os = objects_list_cur;
	while ((os->displayed & 0x80) == 0) {
		if (NICKY1) {
			if (os->displayed) {
				os->displayed = 1;
			}
		}
		if (NICKY2) {
			if (os->displayed) {
				anim_data_t *ad = os->anim_data2_ptr;
				if (ad->logic_opcode >= 21 && ad->logic_opcode <= 24) {
					os->tile_num = 0;
				} else if (ad->logic_opcode >= 26 && ad->logic_opcode <= 27) {
					os->tile_num = 0;
					os->ref_ref_index = 0;
				}
				os->displayed = 1;
			}
		}
		++os;
	}

	os = objects_list_cur;
	while ((os->displayed & 0x80) == 0 && x1 > os->map_pos_x) {
		++objects_list_cur;
		os = objects_list_cur;
	}

	os = objects_list_cur;
	while ((os->displayed & 0x80) == 0 && x2 >= os->map_pos_x) {
		if (os->anim_data2_ptr == 0) {
			os->displayed = 0;
		}
		if (os->displayed != 0) {
			anim_data_t *_si = os->anim_data2_ptr;
			os->displayed = 2;
			os->pos_x = os->map_pos_x - _screen_tile_map_offs_x_shl4;
			os->pos_y = os->map_pos_y - _screen_tile_map_offs_y_shl4;
			os->anim_data1_ptr = _si;
			os->anim_data3_ptr = _si;
			os->anim_data4_ptr = _si->anim_data2_ptr;
			os->distance_pos_x = _si->distance_dx;
			os->distance_pos_y = _si->distance_dy;
			os->move_data_ptr = _si->move_data_ptr;
			os->anim_data_cycles = _si->cycles;
			os->unk3 = _si->unk3;
			os->life = _si->lifes;
			os->unk26 = _si->unk4;
			os->unk27 = _si->unk5;
			os->transparent_flag = 0;
		}
		objects_list_last = os;
		++os;
	}
}

static void game_init_level_helper2() {
	if (monster_fight_flag == 0) {
		update_nicky_anim_flag = 0;
		memset(&decor_state, 0, sizeof(decor_state));
		decor_state.tile_map_offs_x = _screen_tile_origin_x;
		_screen_tile_map_offs_x = _screen_tile_origin_x / _screen_cdg_tile_map_h;
		_screen_tile_map_offs_x_shl4 = _screen_tile_map_offs_x << 4;
		decor_state.tile_map_offs_y = _screen_tile_origin_y;
		_screen_tile_map_offs_y = _screen_tile_origin_y;
		_screen_tile_map_offs_y_shl4 = _screen_tile_map_offs_y << 4;
		if (NICKY1) {
			decor_state.tile_map_end_offs_x = 20000 - (GAME_SCREEN_W / 16) * 50 + 50; /* 19050 */
			decor_state.tile_map_end_offs_y = 40;
		}
		if (NICKY2) {
			decor_state.tile_map_end_offs_x = (_screen_cdg_tile_map_w - 19) * _screen_cdg_tile_map_h;
			decor_state.tile_map_end_offs_y = _screen_cdg_tile_map_h - 10;
		}
		decor_state.tile_block_x = decor_state.tile_map_offs_x;
		decor_state.tile_block_y = decor_state.tile_map_offs_y;
		game_set_tile_offsets();
		game_update_cur_objects_ptr();
		game_reset_objects8();
	}
}

void game_init_level_start_screen() {
	game_init_level_helper2();
	if (player_state.dead_flag) {
		game_init_nicky();
	}
	player_state.pos_x = player_pos_x - _screen_tile_map_offs_x_shl4;
	player_state.pos_y = player_pos_y - _screen_tile_map_offs_y_shl4;
	player_state.pos_dx_2 = 0;
	player_state.pos_dy_2 = 0;
	if (NICKY1) {
		if ((player_state.hdir_flags & 1) == 0) {
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v1[23];
		} else {
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v1[19];
		}
	}
	if (NICKY2) {
		if ((player_state.hdir_flags & 1) == 0) {
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[31];
		} else {
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[19];
		}
	}
	player_state.hdir_flags &= 1;
	if (monster_fight_flag == 0) {
		int i;
		sys_clear_sprite_list();
		game_handle_nicky_shield();
		for (i = 0; i < 2; ++i) {
			game_adjust_tilemap();
			game_execute_logic_op();
		}
	}
	++update_nicky_anim_flag;
}

void game_init_ref_ref(anim_data_t *ad, uint16 flags) {
	if (NICKY1) {
		if ((ad->sprite_flags & 0xF000) == 0) {
			while (ad->unk0 >= 0) {
				ad->default_sprite_num = ad->sprite_num;
				ad->sprite_flags |= flags;
				ad->default_sprite_flags = ad->sprite_flags;
				++ad;
			}
		}
	}
	if (NICKY2) {
		flags >>= 8;
		while (ad->unk0 >= 0) {
			ad->sprite_flags |= flags;
			++ad;
		}
	}
	/* pointer conversions handled in load_ref_ref() */
}

void game_init_objects_from_positref(const uint8 *posit_ref, uint16 objects_offset) {
	object_state_t *os;
	objects_list_head = objects_table_ptr1 + objects_offset;
	os = objects_list_head;
	while (1) {
		anim_data_t *ad;
		uint16 num = read_uint16LE(posit_ref);
		if (num & 0x8000) {
			break;
		}
		ad = &res_ref_ref[num];
		assert(os < objects_table_ptr2);
		os->anim_data1_ptr = ad;
		os->anim_data2_ptr = ad;
		os->anim_data3_ptr = ad;
		os->displayed = ad->unk0;
		os->life = ad->lifes;
		os->anim_data_cycles = ad->cycles;
		os->unk3 = ad->unk3;
		os->move_data_ptr = ad->move_data_ptr;
		os->distance_pos_x = ad->distance_dx;
		os->distance_pos_y = ad->distance_dy;
		os->anim_data4_ptr = ad->anim_data2_ptr;
		os->unk26 = ad->unk4;
		os->unk27 = ad->unk5;
		os->map_pos_x = read_uint16LE(posit_ref + 2);
		os->map_pos_y = read_uint16LE(posit_ref + 4);
		os->ref_ref_index = read_uint16LE(posit_ref + 6);
		os->tile_num = posit_ref[9];
		os->visible = posit_ref[8];
		os->transparent_flag = 0;
		if (NICKY2) {
			os->unk29 = 0;
			os->map_pos_x *= 16;
			++os->map_pos_y;
			os->map_pos_y *= 16;
			if (ad->sprite_num != 0 || ad->sprite_flags != 0) {
				int spr_w, spr_h;
				sys_get_sprite_dim((ad->sprite_flags == 0x80) ? res_level_spr : res_monster_spr, ad->sprite_num, &spr_w, &spr_h);
				os->map_pos_y -= spr_h;
			}
		}
		++os;
		posit_ref += 10;
	}
	os->displayed = 0xFF;
	os->map_pos_x = 30000;
	assert(os - objects_table <= 417);
}

/* used to display stairs */
void game_change_decor_tile_map_line(const object_state_t *os, const anim_data_t *ad) {
	int offs, i, count;

	offs = ((os->map_pos_x + ad->anim_w / 2) >> 4);
	if (NICKY2) {
		--offs;
	}
	offs *= _screen_cdg_tile_map_h;
	offs += ((os->map_pos_y + ad->anim_h - 8) >> 4);

	count = 10;
	if (NICKY2) {
		count = os->tile_num;
	}
	for (i = 0; i < count; ++i) {
		res_decor_cdg[offs - i] = os->ref_ref_index;
	}
}

/* used to restore the decor after using a bomb / a door */
void game_change_decor_tile_map_quad(const object_state_t *os) {
	int offs = (os->map_pos_x >> 4) * _screen_cdg_tile_map_h + (os->map_pos_y >> 4);
	if ((os->tile_num & 0x80) == 0) {
		res_decor_cdg[offs] = res_new_new[os->tile_num * 4 + 0];
		res_decor_cdg[offs + 1] = res_new_new[os->tile_num * 4 + 1];
		res_decor_cdg[offs + _screen_cdg_tile_map_h] = res_new_new[os->tile_num * 4 + 2];
		res_decor_cdg[offs + _screen_cdg_tile_map_h + 1] = res_new_new[os->tile_num * 4 + 3];
	}
}

void game_change_decor_tile_map(const object_state_t *os) {
	int offs = (os->map_pos_x >> 4) * _screen_cdg_tile_map_h + (os->map_pos_y >> 4);
	res_decor_cdg[offs] = os->tile_num;
}

void game_update_score() {
	if (cycle_score != 0) {
		game_state.extra_life_score += cycle_score;
		if (game_state.extra_life_score > 200000) {
			if (player_state.lifes_count < 9) {
				++player_state.lifes_count;
			}
			game_state.extra_life_score -= 200000;
			snd_play_sfx(SND_SAMPLE_15);
		}
		game_state.score += cycle_score;
		if (game_state.score > 9999999) {
			game_state.score = 9999999;
		}
	}
	cycle_score = 0;
}

void game_adjust_tilemap() {
	_screen_tile_map_offs_y_shl4 = (decor_state.tile_map_offs_y << 4) - decor_state.scroll_dy;
	_screen_tile_map_offs_x_shl4 = ((decor_state.tile_map_offs_x / _screen_cdg_tile_map_h) << 4) - decor_state.scroll_dx;
	sys_set_tilemap_origin(_screen_tile_map_offs_x_shl4, _screen_tile_map_offs_y_shl4);
	if (monster_fight_flag < 2 && (current_level & 1)) {
		if (NICKY1) {
			if (_screen_tile_map_offs_x_shl4 >= (380 << 4)) {
				if (_screen_tile_map_offs_y_shl4 >= (40 << 4)) {
					++monster_fight_flag;
				}
			}
		}
		if (NICKY2) {
			if (_screen_tile_map_offs_x_shl4 >= ((_screen_cdg_tile_map_w - 20) << 4)) {
				if (_screen_tile_map_offs_y_shl4 >= ((_screen_cdg_tile_map_h - 10 - 1) << 4)) {
					++monster_fight_flag;
				}
			}
		}
		if (monster_fight_flag >= 2) {
			game_setup_monster();
		}
	}
}

void game_handle_level_change() {
	if (restart_level_flag) {
		print_debug(DBG_GAME, "game_handle_level_change() restart_level_flag=%d", restart_level_flag);
		restart_level_flag = 0;
		game_init_level_start_screen();
	} else {
		/* tiles blinking */
		if (player_state.tile_blinking_duration > 0) {
			uint16 offs, tile;
			--player_state.tile_blinking_duration;
			++_screen_blinking_tile.anim;
			if (_screen_blinking_tile.anim > 1) {
				_screen_blinking_tile.anim = 0;
			}
			offs = _screen_blinking_tile.tile_num;
			tile = _screen_blinking_tile.anim;
			_screen_cdg_offsets[offs] = _screen_blinking_tile.tiles_table[tile];
			if (player_state.tile_blinking_duration == 0) {
				_screen_cdg_offsets[offs] = _screen_blinking_tile.tiles_table[0];
			}
		}
		if (player_state.has_spring > 0) {
			--player_state.has_spring;
			if (player_state.has_spring == 0) {
				nicky_move_offsets_ptr = &nicky_move_offsets_table__v1[40];
			}
		}
		if (player_state.has_wood > 0) {
			--player_state.has_wood;
		}
		if (player_state.has_red_ball > 0) {
			--player_state.has_red_ball;
		}
		if ((NICKY1 && player_state.action_code >= 5) || change_pal_flag) {
			change_pal_flag ^= 1;
			if (change_pal_flag) {
				sys_set_palette_spr(res_flash_pal, 16, 0);
			} else {
				sys_set_palette_spr(res_decor_pal, 16, 0);
			}
		}
		if (game_state.boss_fight_counter >= 60) {
			++game_state.boss_fight_counter;
			if (game_state.boss_fight_counter >= 200) {
				if (NICKY2 && jump_to_new_level_after_boss == 0) {
					return;
				}
				new_level_flag = 1;
			}
		}
	}
}

static uint8 game_get_decor_tile_ref(int x, int y) {
	int offs = (x >> 4) * _screen_cdg_tile_map_h + decor_state.tile_block_x + (y >> 4) + decor_state.tile_block_y;
	uint8 tile_num = res_decor_cdg[offs];
	return res_decor_ref[tile_num * 8];
}

static void game_update_decor_helper(object_state_t *os) {
	if (os->life <= 0) {
		os->displayed = 0;
		--update_decor_counter;
	} else {
		int pos_dx, pos_dy;
		anim_data_t *ad = os->anim_data1_ptr;
		nicky_cur_pos_y = (ad->anim_h >> 1) + os->pos_y + decor_state.offs_y;
		nicky_cur_pos_x1  = (ad->anim_w >> 1) + os->pos_x + decor_state.offs_x;
		if (NICKY2) {
			pos_dx = 8;
			pos_dy = 8;
		} else {
			pos_dx = 0;
			pos_dy = 0;
		}
		if (os->distance_pos_y >= 0) {
			if (os->distance_pos_x >= 0) {
				if (game_get_decor_tile_ref(nicky_cur_pos_x1 + pos_dx, nicky_cur_pos_y + pos_dy) & 0x10) {
					if (game_get_decor_tile_ref(nicky_cur_pos_x1 + 6 + pos_dx, nicky_cur_pos_y - 5 - pos_dy) & 0x10) {
						if (game_get_decor_tile_ref(nicky_cur_pos_x1 - 6 - pos_dx, nicky_cur_pos_y + 5 + pos_dy) & 0x10) {
							os->distance_pos_y = -os->distance_pos_y;
						}
						os->distance_pos_x = -os->distance_pos_x;
						--os->life;
					} else {
						os->distance_pos_y = -os->distance_pos_y;
						--os->life;
					}
				}
			} else {
				if (game_get_decor_tile_ref(nicky_cur_pos_x1 - pos_dx, nicky_cur_pos_y + pos_dy) & 0x10) {
					if (game_get_decor_tile_ref(nicky_cur_pos_x1 - 6 - pos_dx, nicky_cur_pos_y - 5 - pos_dy) & 0x10) {
						if (game_get_decor_tile_ref(nicky_cur_pos_x1 + 6 + pos_dx, nicky_cur_pos_y + 5 + pos_dy) & 0x10) {
							os->distance_pos_y = -os->distance_pos_y;
						}
						os->distance_pos_x = -os->distance_pos_x;
						--os->life;
					} else {
						os->distance_pos_x = -os->distance_pos_x;
						--os->life;
					}
				}
			}
		} else {
			if (os->distance_pos_x >= 0) {
				if (game_get_decor_tile_ref(nicky_cur_pos_x1 + pos_dx, nicky_cur_pos_y - pos_dy) & 0x10) {
					if (game_get_decor_tile_ref(nicky_cur_pos_x1 + 6 + pos_dx, nicky_cur_pos_y + 5 + pos_dy) & 0x10) {
						if (game_get_decor_tile_ref(nicky_cur_pos_x1 - 6 - pos_dx, nicky_cur_pos_y - 5 - pos_dy) & 0x10) {
							os->distance_pos_y = -os->distance_pos_y;
						}
						os->distance_pos_x = -os->distance_pos_x;
						--os->life;
					} else {
						os->distance_pos_y = -os->distance_pos_y;
						--os->life;
					}

				}
			} else {
				if (game_get_decor_tile_ref(nicky_cur_pos_x1 - pos_dx, nicky_cur_pos_y - pos_dy) & 0x10) {
					if (game_get_decor_tile_ref(nicky_cur_pos_x1 - 6 - pos_dx, nicky_cur_pos_y + 5 + pos_dy) & 0x10) {
						if (game_get_decor_tile_ref(nicky_cur_pos_x1 + 6 + pos_dx, nicky_cur_pos_y - 5 - pos_dy) & 4) {
							os->distance_pos_y = -os->distance_pos_y;
						}
						os->distance_pos_x = -os->distance_pos_x;
						--os->life;
					} else {
						os->distance_pos_y = -os->distance_pos_y;
						--os->life;
					}
				}
			}
		}
	}
}

int game_projectile_collides(object_state_t *os) {
	int i;
	anim_data_t *ad;
	object_state_t *_si;
	if ((os->unk26 & 1) == 0) return 1;
	ad = os->anim_data1_ptr;
	if (os->pos_y >= GAME_SCREEN_H || os->pos_y + ad->bounding_box_y2 < 0) return 1;
	if (os->pos_x >= GAME_SCREEN_W || os->pos_x + ad->bounding_box_x2 < 0) return 1;
	bounding_box_x1 = os->pos_x + ad->bounding_box_x1;
	bounding_box_x2 = os->pos_x + ad->bounding_box_x2;
	bounding_box_y1 = os->pos_y + ad->bounding_box_y1;
	bounding_box_y2 = os->pos_y + ad->bounding_box_y2;
	update_decor_counter = 7;
	_si = objects_table_ptr5;
	for (i = 0; i < 7; ++i, ++_si) {
		if (_si->displayed == 0) {
			--update_decor_counter;
			if (update_decor_counter == 0) {
				return 0;
			}
		} else {
			int _bp = _si->pos_x;
			int _dx = _si->pos_y;
			anim_data_t *ad2 = os->anim_data1_ptr;
			if (_dx + ad2->bounding_box_y1 > bounding_box_y2 || _dx + ad2->bounding_box_y2 < bounding_box_y1) {
				continue;
			}
			if (_bp + ad2->bounding_box_x1 > bounding_box_x2 || _bp + ad2->bounding_box_x2 < bounding_box_x1) {
				continue;
			}
			_si->displayed = 0;
			if (ad->lifes == 0) {
				return 0;
			}
			if (ad->colliding_opcode == 17) {
				game_handle_projectiles_collisions_helper3(os, ad);
			}
			os->transparent_flag ^= 1;
			--os->life;
			print_debug(DBG_GAME, "life counter = %d", os->life);
			if (os->life > 0) {
				return 0;
			}
			if (ad->colliding_opcode != 17) {
				unk_ref_index = 0;
			}
			cycle_score += ad->score;
			if (ad->anim_data3_ptr == 0) {
				os->displayed = (os->visible != 0) ? 1 : 0;
				return 0;
			} else {
				if (os->visible == 0) {
					os->anim_data2_ptr = 0;
				}
				os->transparent_flag = 0;
				game_play_anim_data_sfx(game_init_object_from_anim3(os, ad));
			}
		}
	}
	return 1;
}

static void game_update_decor() {
	int i;
	object_state_t *os = objects_table_ptr5;
	for (i = 0; i < 2; ++i) {
		if (os->displayed) {
			game_update_decor_helper(os);
		} else {
			--update_decor_counter;
		}
		++os;
	}
}

void game_handle_projectiles_collisions() {
	object_state_t *os;
	update_decor_counter = 7;
	game_update_decor();
	game_handle_projectiles_collisions_helper2(objects_table_ptr6, 1, 4);
	game_handle_projectiles_collisions_helper1(objects_table_ptr7);
	if (update_decor_counter == 0) {
		return;
	}
	for (os = objects_list_cur; os <= objects_list_last; ++os) {
		if (os->displayed == 2) {
			if (!game_projectile_collides(os)) {
				break;
			}
		}
	}
}

void game_execute_colliding_op() {
	colliding_op35_flag = 0;
	if (!player_state.dead_flag) {
		object_state_t *os = objects_list_cur;
		anim_frame_t *af = player_state.anim_frame_ptr;
		player_state.colliding_flag = 0;
		if (NICKY2 && nicky_anim_flags3 != 0) {
			bounding_box_y1 = bounding_box_y2 = player_state.pos_y;
			bounding_box_x1 = bounding_box_x2 = player_state.pos_x;
			if ((player_state.bird_dir_flags & 4) == 0) {
				bounding_box_x2 += 32;
				bounding_box_y2 += 41;
				if ((player_state.hdir_flags & 1) == 0) {
					bounding_box_x1 += 2;
				}
			} else {
				bounding_box_y2 += 33;
				if ((player_state.hdir_flags & 1) == 0) {
					bounding_box_x2 += 38;
				} else {
					bounding_box_x2 += 32;
				}
			}
		} else {
			bounding_box_y1 = player_state.pos_y + af->delta_y;
			bounding_box_y2 = player_state.pos_y + player_state.dim_h;
			bounding_box_x1 = player_state.pos_x + 2;
			bounding_box_x2 = player_state.pos_x + 14;
		}
		game_execute_colliding_op_helper1();
		game_execute_colliding_op_helper2();
		while (1) {
			if (os->displayed == 2) {
				if (os->unk26 & 2) {
					anim_data_t *ad = os->anim_data1_ptr;
					if (os->pos_y + ad->bounding_box_y2 >= bounding_box_y1 && os->pos_y + ad->bounding_box_y1 <= bounding_box_y2) {
						if (os->pos_x + ad->bounding_box_x2 >= bounding_box_x1 && os->pos_x + ad->bounding_box_x1 <= bounding_box_x2) {
							game_collides_op(os, ad);
						}
					}
				}
			}
			if (os < objects_list_last) {
				++os;
			} else {
				if (player_state.colliding_flag != 0) {
					player_state.unk1 = 2;
					player_state.hdir_flags &= ~4;
					player_state.move_offsets_data_ptr = nicky_move_offsets_ptr + 3;
				}
				break;
			}
		}
	}
}

/* bullets collision */
void game_execute_colliding_op_helper1() {
	int i;
	object_state_t *os = objects_table_ptr2;
	update_decor_counter = 7;
	/* collision with decor */
	game_handle_projectiles_collisions_helper2(objects_table_ptr2, 0, 7);
	if (update_decor_counter == 0) {
		return;
	}
	/* collision with nicky */
	for (i = 0; i < 7; ++i, ++os) {
		if (os->displayed) {
			anim_data_t *ad = os->anim_data1_ptr;
			if (ad->unk4 & 1) {
				if (os->pos_y + ad->bounding_box_y2 < bounding_box_y1 || os->pos_y + ad->bounding_box_y1 > bounding_box_y2) {
					continue;
				}
				if (os->pos_x + ad->bounding_box_x2 < bounding_box_x1 || os->pos_x + ad->bounding_box_x1 > bounding_box_x2) {
					continue;
				}
				player_state.colliding = 1;
				if (ad->anim_data3_ptr != 0) {
					game_init_object_from_anim3(os, ad);
				} else {
					os->displayed = 0;
				}
				break;
			}
		}
	}
}

/* bonus collision */
void game_execute_colliding_op_helper2() {
	if (bonus_count != 0) {
		int i;
		object_state_t *os = objects_table_ptr8;
		for (i = 0; i < 16; ++i, ++os) {
			if (os->displayed) {
				if (os->unk26 & 2) {
					anim_data_t *ad = os->anim_data1_ptr;
					if (os->pos_y + ad->bounding_box_y2 < bounding_box_y1 || os->pos_y + ad->bounding_box_y1 > bounding_box_y2) {
						continue;
					}
					if (os->pos_x + ad->bounding_box_x2 < bounding_box_x1 || os->pos_x + ad->bounding_box_x1 > bounding_box_x2) {
						continue;
					}
					if (ad->colliding_opcode != 14) {
						if (NICKY1) {
							os->transparent_flag ^= 2;
						}
						if (NICKY2) {
							os->transparent_flag |= 1;
						}
						player_state.colliding = 1;
					} else {
						cycle_score += ad->score;
						if (ad->anim_data3_ptr != 0) {
							game_play_anim_data_sfx(game_init_object_from_anim3(os, ad));
						} else {
							os->displayed = 0;
						}
					}
					break;
				}
			}
		}
	}
}

void game_play_anim_data_sfx(const anim_data_t *ad) {
	if (ad->sound_num >= 0) {
		uint8 priority = snd_sample_priority_table[ad->sound_num];
		if (priority >= _snd_current_sfx_priority) {
			_snd_current_sfx_priority = priority;
			_snd_current_sfx_num = ad->sound_num;
		}
	}
}

void game_set_object_to_initial_pos(object_state_t *os, anim_data_t *ad) {
	int x, y;
	if (ad->move_data_ptr) {
		if (os->move_data_ptr[0] == 0x88) {
			os->move_data_ptr = ad->move_data_ptr;
		}
		x = (int8)os->move_data_ptr[0];
		y = (int8)os->move_data_ptr[1];
		os->move_data_ptr += 2;
	} else {
		x = os->distance_pos_x;
		y = os->distance_pos_y;
	}
	os->pos_x += x - decor_state.delta_x;
	os->pos_y += y - decor_state.delta_y;
}

void game_setup_monster() {
	const uint8 *p, *boss_posit_ref;
	anim_data_t *ad, *boss_ref_ref;

	ad = &res_ref_ref[0];
	while (ad->unk0 != -1) {
		++ad;
	}
	++ad;
	boss_ref_ref = ad;

	p = res_posit_ref;
	while (read_uint16LE(p) != 0xFFFF) {
		p += 10;
	}
	p += 10;
	boss_posit_ref = p;
	if (NICKY1) {
		snd_play_song(SND_MUS_SONGMON);
	}
	game_init_ref_ref(boss_ref_ref, 0x9000);
	if (NICKY1) {
		memset(objects_table, 0, sizeof(object_state_t) * 417);
		/* XXX original offset was 10000 / 0x2C, weird... */
		game_init_objects_from_positref(boss_posit_ref, 227);
	}
	if (NICKY2) {
		memset(objects_table, 0, sizeof(object_state_t) * 36);
		game_init_objects_from_positref(boss_posit_ref, 0);
	}

	game_update_cur_objects_ptr();
	game_state.boss_fight_counter = 0;
	game_state.boss_explosion_counter = 0;
	if (NICKY2) {
		s16 shield_duration = player_state.shield_duration;
		if (shield_duration != 0) {
			game_enable_nicky_shield();
			player_state.shield_duration = shield_duration;
		}
		update_nicky_anim_flag = 1;
	}
}

anim_data_t *game_init_object_from_anim3(object_state_t *os, anim_data_t *ad) {
	anim_data_t *next = ad->anim_data3_ptr;
	os->pos_x += ad->dx;
	os->pos_y += ad->dy;
	os->anim_data_cycles = next->cycles;
	os->unk3 = ad->unk3;
	os->unk26 = next->unk4;
	os->unk27 = next->unk5;
	os->life = next->lifes;
	os->distance_pos_x = next->distance_dx;
	os->move_data_ptr = next->move_data_ptr;
	os->anim_data4_ptr = next->anim_data2_ptr;
	os->distance_pos_y = next->distance_dy;
	os->anim_data1_ptr = next;
	os->anim_data3_ptr = next;
	return next;
}

/* construction du pont */
void game_handle_projectiles_collisions_helper1(object_state_t *os) {
	if (os->displayed) {
		anim_data_t *ad = os->anim_data1_ptr;
		int y = os->pos_y + (ad->anim_h >> 1) + decor_state.offs_y;
		int x = os->pos_x + (ad->anim_w >> 1) + decor_state.offs_x;
		int offs = (x >> 4) * _screen_cdg_tile_map_h + decor_state.tile_block_x + (y >> 4) + decor_state.tile_block_y;
		int tile_offs = res_decor_cdg[offs] * 8;
		if (res_decor_ref[tile_offs] & 0x20) {
			if (res_decor_ref[tile_offs + 5]) {
				res_decor_cdg[offs] = res_decor_ref[tile_offs + 5];
				snd_play_sfx(SND_SAMPLE_9);
				--update_decor_counter;
				os->displayed = 0;
			}
		}
	}
}

void game_handle_projectiles_collisions_helper2(object_state_t *os, int _bp, int count) {
	while (count--) {
		anim_data_t *ad = os->anim_data1_ptr;
		if (os->displayed && (ad->unk4 & 1)) {
			int y = os->pos_y + decor_state.offs_y + (ad->anim_h >> 1);
			int x = os->pos_x + decor_state.offs_x + (ad->anim_w >> 1);
			int offs = (x >> 4) * _screen_cdg_tile_map_h + decor_state.tile_block_x + (y >> 4) + decor_state.tile_block_y;
			int tile_offs = res_decor_cdg[offs] * 8;
			if (res_decor_ref[tile_offs] & 0x10) {
				if (res_decor_ref[tile_offs + 5] != 0 && _bp != 0) {
					res_decor_cdg[offs] = res_decor_ref[tile_offs + 5];
					if (snd_sample_priority_table[0] >= _snd_current_sfx_priority) {
						_snd_current_sfx_priority = snd_sample_priority_table[0];
						_snd_current_sfx_num = 0;
					}
				}
				if (ad->anim_data3_ptr != 0) {
					game_init_object_from_anim3(os, ad);
				} else {
					--update_decor_counter;
					os->displayed = 0;
				}
				return;
			}
		} else {
			--update_decor_counter;
		}
		++os;
	}
}

static void game_execute_logic_op_helper1(int x) {
	if (NICKY1) {
		if (nicky_cur_pos_x2 <= objects_list_cur->map_pos_x && objects_list_head != objects_list_cur) {
			--objects_list_cur;
		}
	}
	if (NICKY2) {
		while (nicky_cur_pos_x2 <= objects_list_cur->map_pos_x && objects_list_head != objects_list_cur) {
			--objects_list_cur;
		}
	}
}

anim_data_t *game_logic_op_helper1(object_state_t *os, anim_data_t *ad) {
	if (ad->cycles != 0 && --os->anim_data_cycles == 0) {
		anim_data_t *_si = ad->anim_data1_ptr;
		if (_si == (anim_data_t *)0xFFFFFFFF) {
			_si = os->anim_data3_ptr;
		}
		if (_si == 0) {
			os->displayed = 0;
			return ad;
		}
		ad = _si;
		os->anim_data1_ptr = ad;
		os->distance_pos_x = ad->distance_dx;
		os->distance_pos_y = ad->distance_dy;
		os->anim_data_cycles = ad->cycles;
	}
	game_set_object_to_initial_pos(os, ad);
	return ad;
}

void game_logic_op_helper2(object_state_t *_di, anim_data_t *_si, anim_data_t *ad, uint16 move) {
	int i;
	anim_data_t *_game_logic_op_helper2_temp_var = ad;
	object_state_t *bx = objects_table_ptr8;
	for (i = 0; i < 16; ++i) {
		if (bx->displayed == 0) {
			bx->anim_data1_ptr = _si;
			bx->anim_data3_ptr = _si;
			bx->displayed = _si->unk0;
			bx->life = _si->lifes;
			bx->anim_data_cycles = _si->cycles;
			bx->unk3 = _si->unk3;
			bx->displayed = 2;
			bx->anim_data4_ptr = _si->anim_data2_ptr;
			bx->move_data_ptr = _si->move_data_ptr;
			bx->unk26 = _si->unk4;
			bx->unk27 = _si->unk5;
			bx->pos_x = _di->pos_x + _game_logic_op_helper2_temp_var->dx2;
			bx->pos_y = _di->pos_y + _game_logic_op_helper2_temp_var->dy2;
			bx->move_data_ptr += game_get_random_number(move) * 2;
			bx->distance_pos_y = 0;
			bx->distance_pos_x = game_get_random_number(5) - 2;
			bonus_count = 1;
			++_di->ref_ref_index;
			game_play_anim_data_sfx(_si);
			return;
		}
		++bx;
	}
	++_di->ref_ref_index;
}

/* called when hitting bonus cone */
void game_handle_projectiles_collisions_helper3(object_state_t *os, anim_data_t *ad) {
	if (os->ref_ref_index == 0) {
		os->life += unk_ref_index & 0xFF;
	}
	game_logic_op_helper2(os, &ad->anim_data4_ptr[os->ref_ref_index], ad, 6);
}

static void game_execute_logic_op_helper2(object_state_t *_di, anim_data_t *ad) {
	if (ad->anim_data4_ptr) {
		int i;
		anim_data_t *_bx = ad->anim_data4_ptr;
		anim_data_t *_bp = ad;
		object_state_t *os = objects_table_ptr2;
		for (i = 0; i < 7; ++i) {
			if (os->displayed == 0) {
				_di->unk3 = _bp->unk3;
				os->anim_data1_ptr = _bx;
				os->pos_x = _di->pos_x + _bp->dx2;
				os->pos_y = _di->pos_y + _bp->dy2;
				os->displayed = _bx->unk0;
				os->life = _bx->lifes;
				os->distance_pos_x = _bx->distance_dx;
				os->distance_pos_y = _bx->distance_dy;
				os->move_data_ptr = _bx->move_data_ptr;
				os->anim_data_cycles = _bx->cycles;
				os->unk3 = _bx->unk3;
				os->unk26 = _bx->unk4;
				os->unk27 = _bx->unk5;
				game_play_anim_data_sfx(_bx);
				if ((_bx->unk4 & 0x10) == 0) {
					game_move_object(os);
				}
				return;
			}
			++os;
		}
	}
}

void game_execute_logic_op() {
	s16 _bp, _cx, _dx, _ax;
	object_state_t *os;

	nicky_cur_pos_x2 = _screen_tile_map_offs_x_shl4 - 280;
	_bp = _screen_tile_map_offs_x_shl4 + GAME_SCREEN_W + 280;
	_cx = _screen_tile_map_offs_y_shl4 - 140;
	_dx = _screen_tile_map_offs_y_shl4 + GAME_SCREEN_H + 150;
	game_execute_logic_op_helper1(_bp);
	os = objects_list_cur;
loop:
	if (os->displayed & 0x80) goto loc_1269A;
	if (os->displayed == 2) goto loc_12617;
	if (os >= objects_list_last) {
		if (_bp < os->map_pos_x) goto loc_1269A;
	} else {
		if (_bp < os->map_pos_x) goto next;
	}
	_ax = nicky_cur_pos_x2;
	if (_ax <= os->map_pos_x && _dx >= os->map_pos_y && _cx <= os->map_pos_y && os->displayed) {
		if (os->anim_data2_ptr == 0) {
			os->displayed = 0;
		} else {
			anim_data_t *ad = os->anim_data2_ptr;
			if (os > objects_list_last) {
				objects_list_last = os;
			}
			os->pos_y = os->map_pos_y - _screen_tile_map_offs_y_shl4 + decor_state.delta_y;
			_ax = os->map_pos_x - _screen_tile_map_offs_x_shl4 + decor_state.delta_x;
			if ((ad->unk4 & 0x80) || _ax > GAME_SCREEN_W || _ax + ad->anim_w <= 0 || os->pos_y > GAME_SCREEN_H || os->pos_y + ad->anim_h <= 0) {
				anim_data_t *_si;
				os->pos_x = _ax;
				os->anim_data1_ptr = ad;
				os->anim_data3_ptr = ad;
				os->anim_data4_ptr = ad->anim_data2_ptr;
				os->distance_pos_x = ad->distance_dx;
				os->move_data_ptr = ad->move_data_ptr;
				os->displayed = ad->unk0;
				os->life = ad->lifes;
				os->distance_pos_y = ad->distance_dy;
				os->anim_data_cycles = ad->cycles;
				os->unk3 = ad->unk3;
				os->displayed = 2;
				os->unk26 = ad->unk4;
				os->unk27 = ad->unk5;
				os->transparent_flag = 0;
				if (NICKY2) {
					if (ad->logic_opcode >= 21 && ad->logic_opcode <= 24) {
						os->tile_num = 0;
					} else if (ad->logic_opcode >= 26 && ad->logic_opcode <= 27) {
						os->tile_num = 0;
						os->ref_ref_index = 0;
					}
					if (os->displayed == 3) goto loc_12665;
				}
loc_12617:
				_si = os->anim_data1_ptr;
				_si = game_logic_op(os, _si);
				if (os->displayed == 0) {
					os->displayed = (os->visible != 0) ? 1 : 0;
				} else {
					if (_si->unk3 != 0 && --os->unk3 <= 0) {
						os->unk3 = 1;
						if ((os->unk26 & 0x20) || (os->pos_x <= GAME_SCREEN_W && os->pos_x >= 0 && os->pos_y <= GAME_SCREEN_H && os->pos_y >= 0)) {
							game_execute_logic_op_helper2(os, _si);
						}
					}
loc_12665:
					if (NICKY1) {
						if (os->pos_x >= -280 && os->pos_x <= GAME_SCREEN_W + 280 && os->pos_y >= -140 && os->pos_y <= GAME_SCREEN_H + 150) {
							game_draw_object(os);
						} else {
							os->displayed = 1;
						}
					}
					if (NICKY2) {
						if (os->pos_x >= -280 && os->pos_x <= GAME_SCREEN_W + 280 && os->pos_y >= -140 && os->pos_y <= GAME_SCREEN_H + 150) {
							os->displayed = 2;
							game_draw_object(os);
						} else {
							os->displayed = 3;
						}
					}
				}
			}
		}
	}
next:
	++os;
	goto loop;

loc_1269A:
	if (os <= objects_list_last && os != objects_list_head) {
		--objects_list_last;
	}
}
