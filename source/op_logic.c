/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#include "game.h"
#include "globals.h"
#include "resource.h"
#include "sound.h"

static void game_logic_op_helper6(object_state_t *os, anim_data_t *ad) {
	os->anim_data1_ptr = ad;
	os->anim_data3_ptr = ad;
	os->anim_data4_ptr = ad->anim_data2_ptr;
	os->move_data_ptr = ad->move_data_ptr;
	os->distance_pos_x = ad->distance_dx;
	os->distance_pos_y = ad->distance_dy;
	os->anim_data_cycles = ad->cycles;
	os->unk3 = ad->unk3;
	os->unk26 = ad->unk4;
	os->unk27 = ad->unk5;
	game_set_object_to_initial_pos(os, ad);
}

static anim_data_t *game_logic_op_helper7(object_state_t *os, anim_data_t *ad) {
	int x, y, offs;
	x = os->pos_x - 2;
	if (os->distance_pos_x != 0) {
		uint8 _al;
		if (os->distance_pos_x > 0) {
			x += ad->anim_w + 4;
		}
		x += decor_state.offs_x - decor_state.delta_x;
		x = (x >> 4) * _screen_cdg_tile_map_h + decor_state.tile_block_x;
		offs = x;
		offs += (os->pos_y + decor_state.offs_y - decor_state.delta_y) >> 4;
		offs += decor_state.tile_block_y;
		_al = res_decor_cdg[offs];
		if (res_decor_ref[_al * 8] & 1) {
			os->distance_pos_x = 0;
		} else {
			offs = x;
			offs += (os->pos_y + ad->anim_h + decor_state.offs_y - decor_state.delta_y) >> 4;
			offs += decor_state.tile_block_y;
			_al = res_decor_cdg[offs];
			if (res_decor_ref[_al * 8] & 1) {
				os->distance_pos_x = 0;
			}
		}
	}
	y = os->pos_y - 2;
	if (os->distance_pos_y != 0) {
		uint8 _al;
		if (os->distance_pos_y > 0) {
			y += ad->anim_h + 4;
		}
		y += decor_state.offs_y - decor_state.delta_y;
		y = (y >> 4) + decor_state.tile_block_y;
		offs = os->pos_x + decor_state.offs_x - decor_state.delta_x;
		offs = (offs >> 4) * _screen_cdg_tile_map_h + decor_state.tile_block_x;
		offs += y;
		_al = res_decor_cdg[offs];
		if (res_decor_ref[_al * 8] & 1) {
			os->distance_pos_y = 0;
		} else {
			offs = os->pos_x + ad->anim_w + decor_state.offs_x - decor_state.delta_x;
			offs = (offs >> 4) * _screen_cdg_tile_map_h + decor_state.tile_block_x;
			offs += y;
			_al = res_decor_cdg[offs];
			if (res_decor_ref[_al * 8] & 1) {
				os->distance_pos_y = 0;
			}
		}
	}
	if (ad->cycles != 0 && --os->anim_data_cycles == 0) {
		anim_data_t *_si = ad->anim_data1_ptr;
		if (_si == (anim_data_t *)0xFFFFFFFF) {
			_si = os->anim_data3_ptr;
		}
		if (_si == 0) {
			os->displayed = 0;
		} else {
			ad = _si;
			os->anim_data1_ptr = _si;
			os->anim_data_cycles = _si->cycles;
			game_set_object_to_initial_pos(os, _si);
		}
	} else {
		game_set_object_to_initial_pos(os, ad);
	}
	return ad;
}

/* setup_objects with 'os' as origin */
static void loc_13132(object_state_t *os, anim_data_t *ad, object_state_t *_bx, int count) {
	int i;
	for (i = 0; i < count; ++i, ++_bx) {
		if (_bx->displayed != 2) {
			int dy;
			anim_data_t *_di;
			_bx->pos_x = os->pos_x + ad->dx2;
			_bx->pos_y = os->pos_y + ad->dy2;
			_di = ad->anim_data4_ptr;
			_bx->anim_data1_ptr = _di;
			_bx->anim_data3_ptr = _di;
			_bx->anim_data4_ptr = _di->anim_data2_ptr;
			_bx->distance_pos_x = _di->distance_dx;
			_bx->distance_pos_y = _di->distance_dy;
			_bx->displayed = _di->unk0;
			_bx->life = _di->lifes;
			_bx->anim_data_cycles = _di->cycles;
			_bx->unk3 = _di->unk3;
			_bx->displayed = 2;
			_bx->unk26 = _di->unk4;
			_bx->unk27 = _di->unk5;
			if (_di->rnd != 0) {
				_bx->distance_pos_x = -(game_get_random_number(_di->rnd) + 1);
			}
			dy = ad->init_sprite_num;
			if ((_di->unk6 & 4) && (dy >= 2)) {
				dy = game_get_random_number(dy >> 1) * 2;
			}
			_bx->move_data_ptr = &_di->move_data_ptr[dy];
			game_play_anim_data_sfx(_di);
			break;
		}
	}
}

static void game_logic_op_helper3(object_state_t *os, anim_data_t *ad, object_state_t *_bx, int count) {
	if (os->pos_x <= GAME_SCREEN_W && os->pos_x >= 0 && os->pos_y <= GAME_SCREEN_H && os->pos_y >= -28) {
		loc_13132(os, ad, _bx, count);
	}
}

static void game_logic_op_helper4(object_state_t *os, anim_data_t *ad) {
	os->anim_data1_ptr = ad;
	os->anim_data3_ptr = ad;
	os->anim_data4_ptr = ad->anim_data2_ptr;
	os->distance_pos_x = ad->distance_dx;
	os->distance_pos_y = ad->distance_dy;
	os->move_data_ptr = &ad->move_data_ptr[ad->init_sprite_num];
	os->displayed = ad->unk0;
	os->life = ad->lifes;
	os->anim_data_cycles = ad->cycles;
	os->unk3 = ad->unk3;
	os->displayed = 2;
	os->unk26 = ad->unk4;
	os->unk27 = ad->unk5;
	game_change_decor_tile_map_quad(os);
}

static anim_data_t *game_logic_op_helper5(object_state_t *os, anim_data_t *ad) {
	--game_state.boss_explosion_counter;
	if (game_state.boss_explosion_counter <= 0) {
		int i;
		object_state_t *_bx = os + 1;
		game_state.boss_explosion_counter = game_get_random_number(5) + 1;
		for (i = 0; i < 9; ++i, ++_bx) {
			if (_bx->displayed != 2) {
				if (ad->anim_data3_ptr) {
					anim_data_t *_dx = ad->anim_data3_ptr;
					_bx->pos_x = os->pos_x + game_get_random_number(ad->anim_w) - 10;
					_bx->pos_y = os->pos_y + game_get_random_number(ad->anim_h) - 10;
					_bx->anim_data1_ptr = _dx;
					_bx->anim_data3_ptr = _dx;
					_bx->anim_data4_ptr = _dx->anim_data2_ptr;
					_bx->distance_pos_x = _dx->distance_dx;
					_bx->distance_pos_y = _dx->distance_dy;
					_bx->move_data_ptr = _dx->move_data_ptr;
					_bx->displayed = _dx->unk0;
					_bx->life = _dx->lifes;
					_bx->anim_data_cycles = _dx->cycles;
					_bx->unk3 = _dx->unk3;
					_bx->displayed = 2;
					_bx->unk26 = _dx->unk4;
					_bx->unk27 = _dx->unk5;
					_bx->transparent_flag = 0;
					if (_dx->sound_num >= 0) {
						snd_play_sfx(_dx->sound_num);
					}
				}
				if (game_state.boss_fight_counter < 60) {
					++game_state.boss_fight_counter;
					break;
				}
				os->displayed = 0;
				if ((NICKY1 && current_level == 7) || NICKY2) {
					anim_data_t *__bx;
					ad = os->anim_data1_ptr; /* _si */
					__bx = ad->anim_data4_ptr;
					if (!__bx) {
						print_warning("game_logic_op_helper5() ad->anim_data4_ptr is 0");
						break;
					}
					os->anim_data1_ptr = __bx;
					os->anim_data3_ptr = __bx;
					os->anim_data4_ptr = __bx->anim_data2_ptr;
					os->distance_pos_x = __bx->distance_dx;
					os->distance_pos_y = __bx->distance_dy;
					os->move_data_ptr = &__bx->move_data_ptr[__bx->init_sprite_num];
					os->displayed = __bx->unk0;
					os->life = __bx->lifes;
					os->anim_data_cycles = __bx->cycles;
					os->unk3 = __bx->unk3;
					os->displayed = 2;
					os->unk26 = __bx->unk4;
					os->unk27 = __bx->unk5;
					os->pos_y += ad->dy2;
					os->pos_x += ad->dx2;
					break;
				}
			}
		}
	}
	return ad;
}

static anim_data_t *game_logic_op0(object_state_t *os, anim_data_t *ad) {
	if (os->unk27 == 0 || --os->unk27 != 0) {
		ad = game_logic_op_helper1(os, ad);
	} else {
		if (os->anim_data4_ptr == 0) {
			os->displayed = 0;
		} else {
			ad = os->anim_data4_ptr;
			game_logic_op_helper6(os, ad);
		}
	}
	return ad;
}

static anim_data_t *game_logic_op1(object_state_t *os, anim_data_t *ad) {
	int _ax, _bx;
	uint8 _al, _bl;
	_ax = os->pos_x;
	if (ad->unk6 & 2) {
		_ax += ad->anim_w;
	} else {
		_ax -= 2;
	}
	_ax += decor_state.offs_x - decor_state.delta_x;
	_ax = (_ax >> 4) * _screen_cdg_tile_map_h + decor_state.tile_block_x;
	_bx = os->pos_y + ad->anim_h + decor_state.offs_y - decor_state.delta_y;
	_bx = (_bx >> 4) + decor_state.tile_block_y;
	_bx += _ax;

	_al = res_decor_cdg[_bx - 1];
	_bl = res_decor_cdg[_bx];
	if ((res_decor_ref[_bl * 8] & 1) && (res_decor_ref[_al * 8] & 1) == 0) {
		if (ad->unk6 & 0x80) {
			if (ad->anim_data1_ptr == 0) {
				os->displayed = 0;
			} else {
				ad = ad->anim_data1_ptr;
				game_logic_op_helper6(os, ad);
			}
		} else {
			ad = game_logic_op_helper1(os, ad);
		}
	} else {
		ad = os->anim_data4_ptr;
		game_logic_op_helper6(os, ad);
	}
	return ad;
}

anim_data_t *game_logic_op2(object_state_t *os, anim_data_t *ad) {
	int dy;
	if (os->move_data_ptr[1] == 0x88) {
		os->displayed = 0;
	} else {
		uint8 tile_num;
		s16 _ax, _bx, _cx, _dx;
		dy = (int8)os->move_data_ptr[1];
		os->move_data_ptr += 2;
		os->pos_y += dy - decor_state.delta_y;
		_bx = os->pos_y,
		_dx = ad->anim_h / 2;
		if (dy >= 0) {
			_bx += ad->anim_h;
			_dx = -_dx;
		}
		_bx += decor_state.offs_y;
		_cx = _bx;
		_bx = (_bx >> 4) + decor_state.tile_block_y;
		_ax = os->distance_pos_x;
		if (_ax != 0) {
			if (_ax > 0) {
				_ax += ad->bounding_box_x2;
			}
			_cx += _dx;
			_cx = (_cx >> 4) + decor_state.tile_block_y;
			_ax -= decor_state.delta_x;
			_ax += os->pos_x + decor_state.offs_x;
			_ax = (_ax >> 4) * _screen_cdg_tile_map_h + decor_state.tile_block_x + _cx;
			tile_num = res_decor_cdg[_ax];
			if (res_decor_ref[tile_num * 8] & 1) {
				os->distance_pos_x = 0;
			}
		}
		_ax = os->distance_pos_x - decor_state.delta_x;
		os->pos_x += _ax;
		_ax = os->pos_x + decor_state.offs_x;
		_cx = _ax;
		_ax = (_ax >> 4) * _screen_cdg_tile_map_h + decor_state.tile_block_x + _bx;
		tile_num = res_decor_cdg[_ax];
		if ((res_decor_ref[tile_num * 8] & 1) == 0) {
			_ax = ((ad->bounding_box_x2 + _cx) >> 4) * _screen_cdg_tile_map_h + decor_state.tile_block_x + _bx;
			tile_num = res_decor_cdg[_ax];
			if ((res_decor_ref[tile_num * 8] & 0x10) == 0) {
				goto loc_13479;
			}
		}
		if (dy <= 0) {
			os->move_data_ptr = &ad->move_data_ptr[ad->init_sprite_num];
			goto loc_13479;
		}
		++os->unk27;
		if (os->unk27 < 4) goto loc_1344F;
		if (os->anim_data4_ptr == 0) goto loc_134B2;
		ad = os->anim_data4_ptr;
		os->anim_data1_ptr = ad;
		os->anim_data3_ptr = ad;
		os->anim_data4_ptr = ad->anim_data2_ptr;
		os->distance_pos_x = ad->distance_dx;
		os->move_data_ptr = ad->move_data_ptr;
		os->distance_pos_y = ad->distance_dy;
		os->anim_data_cycles = ad->cycles;
		os->unk3 = ad->unk3;
		os->unk26 = ad->unk4;
		os->unk27 = ad->unk5;
		goto loc_13461;
loc_1344F:
		os->move_data_ptr = &ad->move_data_ptr[os->unk27 * 4];
loc_13461:
		_bx = (os->pos_y + decor_state.offs_y + ad->anim_h) & ~0xF;
		_bx -= ad->anim_h;
		_bx -= decor_state.offs_y;
		os->pos_y = _bx;
loc_13479:
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
			os->anim_data_cycles = ad->cycles;
		}
		return ad;
loc_134B2:
		os->displayed = 0;
	}
	return ad;
}

static anim_data_t *game_logic_op3(object_state_t *os, anim_data_t *ad) {
	s16 _ax, _bx;
	uint8 tile_num;

	_ax = os->pos_x + decor_state.offs_x - decor_state.delta_x;
	_ax = (_ax >> 4) * _screen_cdg_tile_map_h + decor_state.tile_block_x;
	_bx = os->pos_y + ad->anim_h + decor_state.offs_y - decor_state.delta_y;
	_bx = (_bx >> 4) + decor_state.tile_block_y;
	_bx += _ax;

	tile_num = res_decor_cdg[_bx];
	if ((res_decor_ref[tile_num * 8] & 0x10) == 0) {
		anim_data_t *ad2 = os->anim_data4_ptr;
		if (ad2 == 0) {
			os->displayed = 0;
			return ad;
		}
		os->anim_data1_ptr = ad2;
		os->anim_data3_ptr = ad2;
		os->anim_data4_ptr = ad2->anim_data2_ptr;
		os->move_data_ptr = &ad2->move_data_ptr[ad->init_sprite_num];
		os->anim_data_cycles = ad2->cycles;
		os->unk3 = ad2->unk3;
		os->unk26 = ad2->unk4;
		os->unk27 = ad2->unk5;
		ad = ad2;
	}
	os->pos_x -= decor_state.delta_x;
	os->pos_y -= decor_state.delta_y;
	return ad;
}

/* squirrels */
static anim_data_t *game_logic_op4(object_state_t *os, anim_data_t *ad) {
	int offs;
	uint8 _al, _bl;
	s16 x = os->pos_x;
	if (ad->unk6 & 2) {
		x += ad->anim_w + 5;
	} else {
		x -= 5;
	}
	offs = ((x + decor_state.offs_x - decor_state.delta_x) >> 4) * 50 + decor_state.tile_block_x;
	offs += ((os->pos_y + ad->anim_h + decor_state.offs_y - decor_state.delta_y) >> 4) + decor_state.tile_block_y;

	_al = res_decor_cdg[offs - 1];
	_bl = res_decor_cdg[offs];
	if ((res_decor_ref[_bl * 8] & 1) == 0) {
		ad = os->anim_data4_ptr;
		game_logic_op_helper6(os, ad);
		return ad;
	}
	if ((res_decor_ref[_al * 8] & 1) != 0) {
		ad = os->anim_data4_ptr;
		game_logic_op_helper6(os, ad);
		return ad;
	}

	if ((ad->unk6 & 2) == 0) {
		if (os->pos_x < player_state.pos_x || os->pos_x - 150 > player_state.pos_x) goto loc_12AD2;
loc_12AA6:
		if (os->pos_y + ad->anim_h < player_state.pos_y || os->pos_y > player_state.pos_y + player_state.dim_h) goto loc_12AD2;
		++ad;
		game_logic_op_helper6(os, ad);
		return ad;
	}
	if (os->pos_x >= player_state.pos_x) goto loc_12AD2;
	if (os->pos_x + 150 >= player_state.pos_x) goto loc_12AA6;
loc_12AD2:
	if (ad->unk6 & 0x80) {
		ad = ad->anim_data1_ptr;
		game_logic_op_helper6(os, ad);
		return ad;
	}
	ad = game_logic_op_helper1(os, ad);
	return ad;
}

/* birds */
static anim_data_t *game_logic_op5(object_state_t *os, anim_data_t *ad) {
	int x, y;
	os->distance_pos_x = 2;
	os->distance_pos_y = 2;
	y = player_state.pos_y - 8;
	if (os->pos_y >= y) {
		if (os->pos_y + 4 <= y) {
			os->distance_pos_y = 0;
		} else {
			os->distance_pos_y = -os->distance_pos_y;
		}
	}
	x = player_state.pos_x - 8;
	if (os->pos_x >= x) {
		if (os->pos_x <= x + 4) {
			os->distance_pos_x = 0;
		} else {
			os->distance_pos_x = -os->distance_pos_x;
		}
	}
	return game_logic_op_helper7(os, ad);
}

/* glass piece */
static anim_data_t *game_logic_op6(object_state_t *os, anim_data_t *ad) {
	uint8 _al;
	int x, y, offs;
	uint8 dy = os->move_data_ptr[1];
	if (dy == 0x88) {
		os->displayed = 0;
		return ad;
	}
	os->move_data_ptr += 2;
	os->pos_y += (int8)dy - decor_state.delta_y;
	y = os->pos_y;
	if ((dy & 0x80) == 0) {
		y += ad->anim_h;
	}
	y = ((y + decor_state.offs_y) >> 4) + decor_state.tile_block_y; /* _bx */
	os->pos_x += os->distance_pos_x - decor_state.delta_x;
	x = os->pos_x + decor_state.offs_x; /* _cx */
	offs = (x >> 4) * 50 + decor_state.tile_block_x + y;
	_al = res_decor_cdg[offs];
	if ((res_decor_ref[_al * 8] & 1) == 0) {
		offs = ((x + ad->bounding_box_x2) >> 4) * 50 + decor_state.tile_block_x + y;
		_al = res_decor_cdg[offs];
		if ((res_decor_ref[_al * 8] & 1) == 0) {
			goto loc_12DF1;
		}
	}
	os->pos_y = ((os->pos_y + decor_state.offs_y + ad->anim_h) & ~0xF) - ad->anim_h - decor_state.offs_y;
	if (os->anim_data4_ptr == 0) {
		os->displayed = 0;
	} else {
		int i;
		object_state_t *_di = os;
		anim_data_t *_bx = os->anim_data4_ptr;
		x = os->pos_x; /* _bp */
		y = os->pos_y; /* _dx */
		for (i = 0; i < 2; ++i) {
			os->anim_data1_ptr = _bx;
			os->anim_data3_ptr = _bx;
			os->anim_data4_ptr = _bx->anim_data2_ptr;
			os->distance_pos_x = _bx->distance_dx;
			os->distance_pos_y = _bx->distance_dy;
			os->move_data_ptr = _bx->move_data_ptr;
			os->displayed = _bx->unk0;
			os->life = _bx->lifes;
			os->anim_data_cycles = _bx->cycles;
			os->unk3 = _bx->unk3;
			os->displayed = 2;
			os->unk26 = _bx->unk4;
			os->unk27 = _bx->unk5;
			os->pos_x = x;
			os->pos_y = y;
			++os;
			++_bx;
		}
		game_play_anim_data_sfx(_di->anim_data1_ptr);
	}
loc_12DF1:
	return ad;
}

/* boss level 2 */
static anim_data_t *game_logic_op7(object_state_t *os, anim_data_t *ad) {
	int i, _bp;
	object_state_t *_bx = os + 1;
	_bp = 0;
	game_shake_screen();
	for (i = 0; i < 9; ++i, ++_bx) {
		if (_bx->displayed != 2) {
			anim_data_t *_dx = ad->anim_data4_ptr;
			if (_dx == 0) {
				break;
			}
			_bx->pos_y = _bx->map_pos_y - _screen_tile_map_offs_y_shl4 + decor_state.delta_y;
			_bx->pos_x = _bx->map_pos_x - _screen_tile_map_offs_x_shl4 + decor_state.delta_x;
			_bx->pos_x += game_get_random_number(32);
			_bx->anim_data1_ptr = _dx;
			_bx->anim_data3_ptr = _dx;
			_bx->anim_data4_ptr = _dx->anim_data2_ptr;
			_bx->distance_pos_x = _dx->distance_dx;
			_bx->distance_pos_y = _dx->distance_dy;
			_bx->move_data_ptr = &_dx->move_data_ptr[_dx->init_sprite_num];
			_bx->displayed = _dx->unk0;
			_bx->life = _dx->lifes;
			_bx->anim_data_cycles = _dx->cycles;
			_bx->unk3 = _dx->unk3;
			_bx->displayed = 2;
			_bx->unk26 = _dx->unk4;
			_bx->unk27 = _dx->unk5;
			++_bp;
			if (_bp == 4) {
				break;
			}
		}
	}
	return game_logic_op0(os, ad);
}

/* boss explosion */
static anim_data_t *game_logic_op8(object_state_t *os, anim_data_t *ad) {
	ad = game_logic_op_helper5(os, ad);
	return game_logic_op0(os, ad);
}

/* plant */
static anim_data_t *game_logic_op9(object_state_t *os, anim_data_t *ad) {
	++os->tile_num;
	if ((os->tile_num & 1) && os->pos_x <= GAME_SCREEN_W && os->pos_x >= 0 && os->pos_y <= GAME_SCREEN_H && os->pos_y >= 0) {
		game_logic_op_helper2(os, ad->anim_data4_ptr, ad, 8);
	}
	return game_logic_op0(os, ad);
}

/* boss level6 */
static anim_data_t *game_logic_op10(object_state_t *os, anim_data_t *ad) {
	game_shake_screen();
	return game_logic_op0(os, ad);
}

/* boss level4 */
static anim_data_t *game_logic_op11(object_state_t *os, anim_data_t *ad) {
	++os->tile_num;
	if ((os->tile_num & 1) && ad->anim_data4_ptr) {
		loc_13132(os, ad, os + 1, 9);
	}
	return game_logic_op0(os, ad);
}

/* level5 */
static anim_data_t *game_logic_op12(object_state_t *os, anim_data_t *ad) {
	--os->unk27;
	if (os->unk27 <= 0) {
		os->unk27 = ad->unk5;
		os->distance_pos_x = game_get_random_number(5) - 2;
		os->distance_pos_y = game_get_random_number(5) - 2;
	}
	return game_logic_op_helper7(os, ad);
}

/* bat */
static anim_data_t *game_logic_op13(object_state_t *os, anim_data_t *ad) {
	--os->unk27;
	if (os->unk27 <= 0) {
		ad = os->anim_data3_ptr + 1;
		game_logic_op_helper6(os, ad);
	} else {
		ad = game_logic_op1(os, ad);
	}
	return ad;
}

/* levels 5,6 - scarecrow */
static anim_data_t *game_logic_op14(object_state_t *os, anim_data_t *ad) {
	if (ad->anim_data4_ptr) {
		game_logic_op_helper3(os, ad, os - 5, 5);
	}
	return game_logic_op0(os, ad);
}

/* levels 5,6 - gouttes plafond */
static anim_data_t *game_logic_op15(object_state_t *os, anim_data_t *ad) {
	if (ad->anim_data4_ptr) {
		game_logic_op_helper3(os, ad, objects_table_ptr8, 16);
		bonus_count = 1;
	}
	return game_logic_op0(os, ad);
}

/* block falling from the ceiling */
static anim_data_t *game_logic_op16(object_state_t *os, anim_data_t *ad) {
	os->anim_data2_ptr = 0;
	os->displayed = 0;
	(os + 1)->pos_x = os->pos_x;
	(os + 1)->pos_y = os->pos_y;
	(os + 1)->anim_data2_ptr = os->anim_data4_ptr;
	game_logic_op_helper4(os + 1, os->anim_data4_ptr);
	game_set_object_to_initial_pos(os, ad);
	return ad;
}

/* levels 6,7 rondins bois avec pics */
static anim_data_t *game_logic_op17(object_state_t *os, anim_data_t *ad) {
	int y, x;
	y = os->pos_y;
	if (ad->unk6 & 2) {
		y += ad->anim_h;
	}
	y += decor_state.offs_y - decor_state.delta_y;
	y = (y >> 4) + decor_state.tile_block_y;

	x = os->pos_x + decor_state.offs_x - decor_state.delta_x;
	x = (x >> 4) * 50 + decor_state.tile_block_x;

	if (res_decor_ref[res_decor_cdg[y + x] * 8] & 0x10) {
		game_logic_op_helper6(os, os->anim_data4_ptr);
	} else {
		ad = game_logic_op_helper1(os, ad);
	}
	return ad;
}

/* falling platforms levels 7,8 */
static anim_data_t *game_logic_op18(object_state_t *os, anim_data_t *ad) {
	os->anim_data2_ptr = ad;
	game_logic_op_helper4(os, ad->anim_data2_ptr);
	game_set_object_to_initial_pos(os, ad);
	return ad;
}

/* boss level8 */
static anim_data_t *game_logic_op19(object_state_t *os, anim_data_t *ad) {
	int i;
	anim_data_t *_dx = ad->anim_data4_ptr;
	anim_data_t *_bp = _dx->anim_data4_ptr;
	object_state_t *_bx = objects_table_ptr8;
	for (i = 0; i < 9; ++i, ++_bx) {
		_bx->pos_x = os->pos_x + ad->dx2;
		_bx->pos_y = os->pos_y + ad->dy2;
		_bx->displayed = _dx->unk0;
		_bx->life = _dx->lifes;
		_bx->anim_data_cycles = _dx->cycles;
		_bx->unk3 = _dx->unk3;
		_bx->distance_pos_x = 0;
		_bx->distance_pos_y = 0;
		_bx->anim_data1_ptr = _dx;
		_bx->unk26 = _dx->unk4;
		_bx->unk27 = _dx->unk5;
		_dx = _bp;
	}
	_bx = objects_table_ptr8 + 1;
	for (i = 0; i < 8; ++i, ++_bx) {
		_bx->map_pos_x = (i * 128) & ~1;
		_bx->map_pos_y = 0;
	}
	bonus_count = 1;
	boss4_state = 1;
	return game_logic_op_helper1(os, ad);
}

static anim_data_t *game2_logic_nop(object_state_t *os, anim_data_t *ad) {
	return ad;
}

static anim_data_t *game2_logic_op20(object_state_t *os, anim_data_t *ad) {
printf("game2_logic_op20 os->ref_ref_index 0x%X\n", os->ref_ref_index);
	if ((os->ref_ref_index & 0x80) == 0) {
		++os->unk29;
		--os->ref_ref_index;
		if ((os->ref_ref_index & 0xFF) == 0) {
			os->pos_x -= os->distance_pos_x * 2;
			os->ref_ref_index |= 0xFF;
			ad = game_logic_op_helper1(os, ad);
		} else {
			if (os->unk27 == 0 || --os->unk27 != 0) {
				ad = game_logic_op_helper1(os, ad);
			} else {
				ad = os->anim_data4_ptr;
				if (ad) {
					game_logic_op_helper6(os, ad);
				} else {
					os->displayed = 0;
				}
			}
		}
	} else {
		s16 _cx = os->distance_pos_x;
		os->distance_pos_x = 0;
		ad = game_logic_op_helper1(os, ad);
		if ((os->ref_ref_index & 0xFF) != 0x80) {
			os->ref_ref_index = (os->ref_ref_index & 0xFF00) | 0x80;
			os->map_pos_x += (os->unk29 - 2) * _cx;
			os->tile_num = os->ref_ref_index >> 8;
			game_change_decor_tile_map_quad(os);
		}
	}
	return ad;
}

static void game2_logic_op22_helper(object_state_t *os) {
	uint8 _bl, _ax, _cx;
	os->distance_pos_x = 0;
	os->distance_pos_y = 0;
	if ((os->ref_ref_index & 0x8000) != 0 && (os->tile_num & 1) != 0) {
		if (nicky_on_elevator_flag == 0) {
			return;
		}
		if (nicky_elevator_object != os) {
			return;
		}
		os->tile_num &= ~1;
	}
	if ((os->tile_num & 0x80) == 0) {
		if ((os->ref_ref_index & 0x5000) == 0) {
			os->unk29 = (os->ref_ref_index * 4) & 0xF0;
		} else {
			os->unk29 = os->ref_ref_index & 0xF0;
		}
		os->tile_num = ((os->ref_ref_index >> 8) & 0x78) | 0x80;
	}
	_bl = ((os->ref_ref_index >> 8) & 7) + 1;
	if ((os->tile_num & 0x40) == 0 && (os->tile_num & 0x10) == 0) {
		os->unk29 += _bl;
		_cx = os->ref_ref_index & 0xFF;
		_ax = (((_cx >> 4) + _cx) & 0x1F) << 4;
		if (_ax != os->unk29) {
			if ((os->tile_num & 0x20) == 0) {
				os->distance_pos_x += _bl;
			} else {
				os->distance_pos_y += _bl;
			}
			return;
		}
	} else {
		os->unk29 -= _bl;
		if (os->unk29 != 0) {
			if ((os->tile_num & 0x40) == 0) {
				os->distance_pos_x -= _bl;
			} else {
				os->distance_pos_y -= _bl;
			}
			return;
		}
	}
	if (os->tile_num & 0x40) {
		os->tile_num = (os->tile_num & ~0x40) | 0x20;
	} else if (os->tile_num & 0x20) {
		os->tile_num = (os->tile_num & ~0x20) | 0x40;
	} else if (os->tile_num & 0x10) {
		os->tile_num = (os->tile_num & ~0x10) | 0x08;
	} else {
		os->tile_num = (os->tile_num & ~0x08) | 0x10;
	}
	os->tile_num |= 1;
}

/* elevator */
static anim_data_t *game2_logic_op22(object_state_t *os, anim_data_t *ad) {
	game2_logic_op22_helper(os);
	if (os->unk27 == 0 || --os->unk27 != 0) {
		ad = game_logic_op_helper1(os, ad);
	} else {
		ad = os->anim_data4_ptr;
		if (ad) {
			game_logic_op_helper6(os, ad);
		} else {
			os->displayed = 0;
		}
	}
	return ad;
}

static anim_data_t *game2_logic_op23(object_state_t *os, anim_data_t *ad) {
	const uint8 *md;
	if (os->unk27 == 0 || --os->unk27 != 0) {
		md = os->move_data_ptr;
		os->move_data_ptr = 0;
		ad = game_logic_op_helper1(os, ad);
		os->move_data_ptr = md;
	} else {
		ad = os->anim_data4_ptr;
		if (ad) {
			md = os->move_data_ptr;
			os->move_data_ptr = 0;
			game_logic_op_helper6(os, ad);
			os->move_data_ptr = md;
		} else {
			os->displayed = 0;
		}
	}
	return ad;
}

static void game2_logic_op24_helper(object_state_t *os) {
	/* TODO */
}

static anim_data_t *game2_logic_op24(object_state_t *os, anim_data_t *ad) {
	game2_logic_op24_helper(os);
	if (os->unk27 == 0 || --os->unk27 != 0) {
		ad = game_logic_op_helper1(os, ad);
	} else {
		ad = os->anim_data4_ptr;
		if (ad) {
			game_logic_op_helper6(os, ad);
		} else {
			os->displayed = 0;
		}
	}
	return ad;
}

static void game2_logic_op27_helper(object_state_t *os) {
	os->distance_pos_x = 0;
	os->distance_pos_y = 0;
}

static anim_data_t *game2_logic_op27(object_state_t *os, anim_data_t *ad) {
	game2_logic_op27_helper(os);
	if (os->unk27 == 0 || --os->unk27 != 0) {
		ad = game_logic_op_helper1(os, ad);
	} else {
		ad = os->anim_data4_ptr;
		if (ad) {
			game_logic_op_helper6(os, ad);
		} else {
			os->displayed = 0;
		}
	}
	return ad;
}

static anim_data_t *game2_logic_op28(object_state_t *os, anim_data_t *ad) {
	s16 x, y, offs;
	uint8 tile_num;
	x = os->pos_x;
	if ((ad->unk6 & 2) == 0) {
		x -= 2;
	} else {
		x += ad->anim_w;
	}
	x += decor_state.offs_x - decor_state.delta_x;
	x >>= 4;
	x *= _screen_cdg_tile_map_h;
	x += decor_state.tile_block_x;
	if (x > _screen_cdg_tile_map_h) {
		offs = x;
		y = os->pos_y + ad->anim_h + decor_state.offs_y - decor_state.delta_y;
		y >>= 4;
		y += decor_state.tile_block_y;
		offs += y;
		tile_num = res_decor_cdg[offs - 1];
		if (res_decor_ref[tile_num * 8] & 0x1) {
			ad = os->anim_data4_ptr;
		} else {
			if (ad->unk6 & 0x80) {
				if (ad->anim_data1_ptr == 0) {
					os->displayed = 0;
					return ad;
				}
			} else {
				return game_logic_op_helper1(os, ad);
			}
		}
	} else {
		ad = os->anim_data4_ptr;
	}
	game_logic_op_helper6(os, ad);
	return ad;
}

/* chest monster */
static anim_data_t *game2_logic_op29(object_state_t *os, anim_data_t *ad) {
	++os->tile_num;
	os->tile_num &= 7;
	if (os->tile_num == 0) {
		if (os->pos_x <= GAME_SCREEN_W && os->pos_x >= 0 && os->pos_y <= GAME_SCREEN_H && os->pos_y >= 0) {
			game_logic_op_helper2(os, ad->anim_data4_ptr, ad, 8);
		}
	}
	return game_logic_op0(os, ad);
}

static anim_data_t *game2_logic_op31(object_state_t *os, anim_data_t *ad) {
	++os->tile_num;
	os->tile_num &= 15;
	if (os->tile_num == 0) {
		if (os->pos_x <= GAME_SCREEN_W && os->pos_x >= 0 && os->pos_y <= GAME_SCREEN_H && os->pos_y >= 0) {
			game_logic_op_helper2(os, ad->anim_data4_ptr, ad, 8);
		}
	}
	return game_logic_op0(os, ad);
}

static anim_data_t *game2_logic_op32(object_state_t *os, anim_data_t *ad) {
	if (game_state.boss_fight_counter <= 250) {
		game_state.boss_fight_counter += 4;
	}
	return ad;
}

static anim_data_t *game2_logic_op33(object_state_t *os, anim_data_t *ad) {
	ad = game_logic_op_helper5(os, ad);
	ad = game_logic_op0(os, ad);
	jump_to_new_level_after_boss = 1;
	return ad;
}

static logic_op_pf logic_op_table__v1[] = {
	/* 0x00 */
	game_logic_op0,
	game_logic_op1,
	game_logic_op2,
	game_logic_op3,
	/* 0x04 */
	game_logic_op4,
	game_logic_op5,
	game_logic_op6,
	game_logic_op7,
	/* 0x08 */
	game_logic_op8,
	game_logic_op9,
	game_logic_op10,
	game_logic_op11,
	/* 0x0C */
	game_logic_op12,
	game_logic_op13,
	game_logic_op14,
	game_logic_op15,
	/* 0x10 */
	game_logic_op16,
	game_logic_op17,
	game_logic_op18,
	game_logic_op19
};

static logic_op_pf logic_op_table__v2[] = {
	/* 0x00 */
	game_logic_op0,
	game_logic_op1,
	game_logic_op2,
	game_logic_op3,
	/* 0x04 */
	game2_logic_nop,
	game_logic_op5,
	0, /* game2_logic_nop */
	0, /* game_logic_op7 */
	/* 0x08 */
	0, /* game_logic_op8 */
	0, /* game_logic_op9 */
	0, /* game_logic_op10 */
	0, /* game2_logic_nop */
	/* 0x0C */
	0, /* game_logic_op12 */
	0, /* game2_logic_nop */
	0, /* game2_logic_nop */
	0, /* game2_logic_nop */
	/* 0x10 */
	game_logic_op16,
	0, /* game2_logic_nop */
	0, /* game_logic_op18 */
	0, /* game2_logic_nop */
	/* 0x14 */
	game2_logic_op20,
	0, /* game_logic_op0 */
	game2_logic_op22,
	0, /* game2_logic_op23 */
	/* 0x18 */
	0, /* game2_logic_op24 */
	game2_logic_nop,
	0, /* game_logic_op0 */
	game2_logic_op27,
	/* 0x1C */
	game2_logic_op28,
	game2_logic_op29,
	game_logic_op3,
	game2_logic_op31,
	/* 0x20 */
	game2_logic_op32,
	game2_logic_op33
};

anim_data_t *game_logic_op(object_state_t *os, anim_data_t *ad) {
	if (NICKY1) {
		assert(ad->logic_opcode < ARRAYSIZE(logic_op_table__v1));
		ad = (*logic_op_table__v1[ad->logic_opcode])(os, ad);
	}
	if (NICKY2) {
		assert(ad->logic_opcode < ARRAYSIZE(logic_op_table__v2));
		if (!logic_op_table__v2[ad->logic_opcode]) {
			print_warning("game_logic_op() unhandled logic opcode %d", ad->logic_opcode);
		} else {
			ad = (*logic_op_table__v2[ad->logic_opcode])(os, ad);
		}
	}
	return ad;
}
