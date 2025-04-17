/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#include "game.h"
#include "globals.h"
#include "input.h"

static void game_update_nicky_anim_helper0() {
	s16 _bx, _cx, _dx;
	s16 _bp = 0;

	if (player_state.unk1 == 2) { /* jumping */
		_bp = player_state.move_offsets_data_ptr[0];
		if (_bp < 0) goto loc_119D5;
		if (_bp == 0x7777) {
			_bp = 5;
		} else {
			player_state.hdir_flags |= 4;
			player_state.move_offsets_data_ptr++;
		}
	}
	decor_state.scroll_inc = _bp;
	nicky_cur_pos_x1 = player_state.pos_x;
	_cx = (player_state.pos_x + 4 + player_state.pos_dx_2) >> 4;
	_cx = (_cx * 50) + decor_state.tile_map_offs_x;
	player_state.pos_y += _bp;
	_dx = player_state.pos_y + player_state.pos_dy_2;
	if (_dx > 0 && (_dx + player_state.dim_h) < 167) {
		nicky_cur_pos_x2 = _dx + player_state.dim_h;
		_dx = (nicky_cur_pos_x2 >> 4) + decor_state.tile_map_offs_y;
		_cx += _dx;
		player_state.decor_ref_flag0 &= ~1;
		player_state.decor_ref_flag3 = 0;
		player_state.decor_ref_flag4 = 0;
		game_set_decor_tile_flags4_3_2_0(1, _cx);
		game_set_decor_tile_flag0(4, _cx);
		_cx = (nicky_cur_pos_x1 + player_state.pos_dx_1 + player_state.pos_dx_2) >> 4;
		_cx = (_cx * 50) + decor_state.tile_map_offs_x + _dx;
		game_set_decor_tile_flag0(8, _cx);
		if (game_set_decor_tile_flags4_3_2_0(1, _cx) & 0x40) {
			player_state.next_tilemap_offset = _cx;
		}
		if (player_state.decor_ref_flag4 == 2) {
			player_state.colliding = 1;
		}
		if (player_state.decor_ref_flag3 == 2) {
			goto loc_1194B;
		}
		if ((player_state.decor_ref_flag0 & 1) == 0) {
			if (player_state.decor_ref_flag3 == 1) {
				goto loc_11959;
			} else {
				goto loc_1191B;
			}
		}
		if ((player_state.decor_ref_flag0 & 0xC) == 0) {
			nicky_cur_pos_x2 &= 0xF;
			if (nicky_cur_pos_x2 > nicky_cur_pos_y) {
				goto loc_1191B;
			}
		}
		_bx = (player_state.pos_y + player_state.pos_dy_2 + player_state.dim_h) & ~0xF;
		_bx -= player_state.dim_h;
		_bx -= player_state.pos_dy_2;
		player_state.pos_y = _bx;
		goto loc_11959;
	}
loc_1191B:
	player_state.hdir_flags &= 0x8D;
	if (player_state.hdir_flags & 8) {
		return;
	}
	/* falling */
	if (player_state.hdir_flags & 1) {
		player_state.anim_frame_ptr = &nicky_anim_frames_table__v1[24];
	} else {
		player_state.anim_frame_ptr = &nicky_anim_frames_table__v1[25];
	}
	if (player_state.unk1 != 2) {
		player_state.unk1 = 2;
		player_state.move_offsets_data_ptr = &nicky_move_offsets_table__v1[50];
		player_state.hdir_flags |= 4;
	}
	return;

loc_1194B:
	if ((inp_direction_mask & 0xC) && (player_state.hdir_flags & 0x30)) {
		goto loc_119C8;
	}

loc_11959:
	player_state.unk1 = 1;
	player_state.hdir_flags &= 0xFB;
	if (player_state.hdir_flags & 1) {
		if (inp_direction_mask & GAME_DIR_DOWN) {
			if ((player_state.hdir_flags & 0x40) == 0) {
				player_state.anim_frame_ptr = &nicky_anim_frames_table__v1[34];
				player_state.anim_frames_count = 2;
				player_state.hdir_flags |= 0x40;
			}
		} else {
			if (inp_direction_mask & GAME_DIR_RIGHT) {
				goto loc_119C8;
			}
			player_state.hdir_flags &= 0xBF;
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v1[19];
		}
	} else {
		if (inp_direction_mask & GAME_DIR_DOWN) {
			if ((player_state.hdir_flags & 0x40) == 0) {
				player_state.anim_frame_ptr = &nicky_anim_frames_table__v1[37];
				player_state.anim_frames_count = 2;
				player_state.hdir_flags |= 0x40;
			}
		} else {
			if (inp_direction_mask & GAME_DIR_LEFT) {
				goto loc_119C8;
			}
			player_state.hdir_flags &= 0xBF;
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v1[23];
		}
	}

	game_set_next_nicky_anim_frame();
	player_state.hdir_flags &= 0xC1;

loc_119C8:
	player_state.vdir_flags |= 2;
	player_state.unk1 = 0;
	player_state.hdir_flags &= 0xFB;
	return;

loc_119D5:
	player_state.move_offsets_data_ptr++;
	decor_state.scroll_inc = _bp;
	player_state.hdir_flags &= 0xFB;
	nicky_cur_pos_x1 = player_state.pos_x;
	_cx = (player_state.pos_x + 4 + player_state.pos_dx_2) >> 4;
	_cx = (_cx * 50) + decor_state.tile_map_offs_x;
	player_state.pos_y += _bp;

	_dx = player_state.pos_y + 4 + player_state.pos_dy_2;
	if (_dx <= 0) goto loc_11A5C;
	if (_dx > 136) goto loc_11A5C;
	_dx = (_dx >> 4) + decor_state.tile_map_offs_y;
	_cx += _dx;
	player_state.decor_ref_flag0 &= ~2;
	game_set_decor_tile_flags4_3_2_0(2, _cx);
	_cx = (nicky_cur_pos_x1 + player_state.pos_dx_1 + player_state.pos_dx_2) >> 4;
	_cx = (_cx * 50) + decor_state.tile_map_offs_x + _dx;
	game_set_decor_tile_flags4_3_2_0(2, _cx);
	if (player_state.decor_ref_flag0 & 2) {
loc_11A5C:
		player_state.hdir_flags |= 4;
		player_state.move_offsets_data_ptr = &nicky_move_offsets_table__v1[50];
	}
	player_state.hdir_flags &= 0x8D;
	if (player_state.hdir_flags & 8) {
		return;
	}
	/* jumping */
	if (player_state.hdir_flags & 1) {
		player_state.anim_frame_ptr = &nicky_anim_frames_table__v1[24];
	} else {
		player_state.anim_frame_ptr = &nicky_anim_frames_table__v1[25];
	}
}

static void game_update_nicky_anim_helper1() {
	s16 _cx, _dx;
	_cx = player_state.pos_x + player_state.dim_w + player_state.pos_dx_2;
	_cx = (_cx >> 4) * 50 + decor_state.tile_map_offs_x;
	nicky_cur_pos_x1 = _cx;
	_dx = player_state.pos_y + player_state.pos_dy_1 + player_state.pos_dy_2;
	if (_dx <= GAME_SCREEN_H) {
		_dx = (_dx >> 4) + decor_state.tile_map_offs_y;
		_cx += _dx;
		player_state.decor_ref_flag0 &= ~4;
		game_set_decor_tile_flags4_3_2_0(4, _cx);
		if ((player_state.decor_ref_flag0 & 4) == 0) {
			_cx = nicky_cur_pos_x1;
			_dx = player_state.pos_y + 12 + player_state.pos_dy_2;
			_dx = (_dx >> 4) + decor_state.tile_map_offs_y;
			_cx += _dx;
			game_set_decor_tile_flags4_3_2_0(4, _cx);
			if ((player_state.decor_ref_flag0 & 4) == 0 && player_state.pos_x < GAME_SCREEN_W - 16) {
				player_state.pos_x += GAME_SCROLL_DX;
/*				player_state.pos_x += 1;*/
			}
		}
		if ((player_state.hdir_flags & 1) == 0) {
			if (player_state.hdir_flags <= 7) {
				player_state.anim_frame_ptr = &nicky_anim_frames_table__v1[16];
				player_state.hdir_flags |= 9;
				player_state.hdir_flags &= 0xBD;
				player_state.anim_frames_count = 2;
			}
		} else {
			if ((player_state.hdir_flags & 0xC) == 0) {
				if (player_state.hdir_flags <= 0xF || (inp_direction_mask & 0xC) == 0) {
					if ((player_state.hdir_flags & 2) == 0) {
						player_state.anim_frame_ptr = &nicky_anim_frames_table__v1[0];
						player_state.hdir_flags |= 2;
						player_state.hdir_flags &= 0x8F;
						player_state.anim_frames_count = 2;
					}
					game_set_next_nicky_anim_frame();
				}
			}
		}
	}
	game_update_nicky_anim_helper0();
}

static void game_update_nicky_anim_helper2() {
	s16 _cx, _dx;
	_cx = (player_state.pos_x + player_state.pos_dx_2) >> 4;
	_cx = (_cx * 50) + decor_state.tile_map_offs_x;
	nicky_cur_pos_x1 = _cx;
	_dx = player_state.pos_y + player_state.pos_dy_1 + player_state.pos_dy_2;
	if (_dx <= GAME_SCREEN_H) {
		_dx = (_dx >> 4) + decor_state.tile_map_offs_y;
		_cx += _dx;
		player_state.decor_ref_flag0 &= ~8;
		game_set_decor_tile_flags4_3_2_0(8, _cx);
		if ((player_state.decor_ref_flag0 & 8) == 0) {
			_cx = nicky_cur_pos_x1;
			_dx = player_state.pos_y + 12 + player_state.pos_dy_2;
			_dx = (_dx >> 4) + decor_state.tile_map_offs_y;
			_cx += _dx;
			game_set_decor_tile_flags4_3_2_0(8, _cx);
			if ((player_state.decor_ref_flag0 & 8) == 0 && player_state.pos_x > 0) {
				player_state.pos_x -= GAME_SCROLL_DX;
/*				player_state.pos_x -= 1;*/
			}
		}
		if (player_state.hdir_flags & 1) {
			if (player_state.hdir_flags <= 7) {
				player_state.anim_frame_ptr = &nicky_anim_frames_table__v1[20];
				player_state.hdir_flags |= 8;
				player_state.hdir_flags &= 0xBC;
				player_state.anim_frames_count = 2;
			}
		} else {
			if ((player_state.hdir_flags & 0x8C) == 0) {
				if (player_state.hdir_flags <= 0xF || (inp_direction_mask & 0xC) == 0) {
					if ((player_state.hdir_flags & 2) == 0) {
						player_state.anim_frame_ptr = &nicky_anim_frames_table__v1[8];
						player_state.hdir_flags |= 2;
						player_state.hdir_flags &= 0x8F;
						player_state.anim_frames_count = 2;
					}
					game_set_next_nicky_anim_frame();
				}
			}
		}
	}
	game_update_nicky_anim_helper0();
}

static void game_update_nicky_anim_helper7() {
	s16 _cx, _dx;

	nicky_cur_pos_x1 = player_state.pos_x;
	_cx = (player_state.pos_x + 4 + player_state.pos_dx_2) >> 4;
	_cx = _cx * 50 + decor_state.tile_map_offs_x;

	_dx = player_state.pos_y + player_state.dim_h + player_state.pos_dy_2;
	if (_dx > GAME_SCREEN_H) return;

	_dx = (_dx >> 4) + decor_state.tile_map_offs_y;
	_cx += _dx;

	player_state.decor_ref_flag0 = 0;
	player_state.decor_ref_flag3 = 0;
	game_set_decor_tile_flags4_3_2_0(1, _cx);
	_cx = (nicky_cur_pos_x1 + player_state.pos_dx_1 + player_state.pos_dx_2) >> 4;
	_cx = _cx * 50 + decor_state.tile_map_offs_x + _dx;
	game_set_decor_tile_flags4_3_2_0(1, _cx);
	if (player_state.decor_ref_flag0 & 1) {
		player_state.hdir_flags &= ~0x30;
		return;
	}
	if (player_state.decor_ref_flag3 != 2) {
		if (player_state.decor_ref_flag3 == 0) {
			player_state.hdir_flags &= ~0x30;
		}
		return;
	}
	if (player_state.pos_y <= 161) {
		player_state.pos_y += GAME_SCROLL_DY;
	}
	if ((player_state.hdir_flags & 0x20) == 0) {
		player_state.anim_frame_ptr = &nicky_anim_frames_table__v1[30];
		player_state.hdir_flags |= 0x20;
		player_state.hdir_flags &= 0xA1;
		player_state.unk1 = 0;
		player_state.anim_frames_count = 2;
	}
	game_set_next_nicky_anim_frame();
}

static void game_update_nicky_anim_helper8() {
	s16 _cx, _dx;
	nicky_cur_pos_x1 = player_state.pos_x;
	_cx = (player_state.pos_x + 4 + player_state.pos_dx_2) >> 4;
	_cx = _cx * 50 + decor_state.tile_map_offs_x;
	nicky_cur_pos_y = _cx;
	_dx = player_state.pos_y + player_state.pos_dy_1 + player_state.pos_dy_2;
	if (_dx > 0 && _dx <= GAME_SCREEN_H) {
		_dx = (_dx >> 4) + decor_state.tile_map_offs_y;
		_cx += _dx;
		player_state.decor_ref_flag0 = 0;
		player_state.decor_ref_flag3 = 0;
		game_set_decor_tile_flags4_3_2_0(2, _cx);
		_cx = (nicky_cur_pos_x1 + player_state.pos_dx_1 + player_state.pos_dx_2) >> 4;
		_cx = _cx * 50 + decor_state.tile_map_offs_x + _dx;
		game_set_decor_tile_flags4_3_2_0(2, _cx);
		if (player_state.decor_ref_flag0 & 2) {
			player_state.hdir_flags &= ~0x30;
			return;
		}
		if (player_state.decor_ref_flag3 != 2) {
			if (player_state.decor_ref_flag3 == 0) {
				goto loc_1159B;
			}
			player_state.hdir_flags &= ~0x30;
			return;
		}
		if (player_state.pos_y > 0) {
			_cx = nicky_cur_pos_y;
			_dx = (player_state.pos_y + 4 + player_state.pos_dy_2) >> 4;
			_dx += decor_state.tile_map_offs_y;
			_cx += _dx;
			game_set_decor_tile_flags4_3_2_0(2, _cx);
			if (player_state.decor_ref_flag0 & 2) {
				player_state.hdir_flags &= ~0x30;
				return;
			}
			player_state.pos_y -= GAME_SCROLL_DY;
		}
		if ((player_state.hdir_flags & 0x10) == 0) {
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v1[26];
			player_state.hdir_flags |= 0x10;
			player_state.hdir_flags &= 0x91;
			player_state.anim_frames_count = 2;
		}
		game_set_next_nicky_anim_frame();
		return;
	}
loc_1159B:
	player_state.hdir_flags &= ~0x30;
	if (player_state.vdir_flags == 0 && player_state.unk1 == 0) {
		player_state.unk1 = 2;
		player_state.move_offsets_data_ptr = nicky_move_offsets_ptr;
	}
}

static void game_update_nicky_anim_helper3() {
	game_update_nicky_anim_helper8();
	game_update_nicky_anim_helper0();
}

static void game_update_nicky_anim_helper4() {
	game_update_nicky_anim_helper8();
	game_update_nicky_anim_helper1();
}

static void game_update_nicky_anim_helper5() {
	game_update_nicky_anim_helper8();
	game_update_nicky_anim_helper2();
}

static void game_update_nicky_anim_helper6() {
	game_update_nicky_anim_helper7();
	game_update_nicky_anim_helper0();
}

static void game2_update_nicky_anim_helper0() {
	s16 _cx, _dx, _bp = 0;

	if (nicky_on_elevator_flag) {
		player_state.decor_ref_flag0 |= 1;
	} else {
		if (player_state.unk1 == 2) {
			_bp = player_state.move_offsets_data_ptr[0];
			if (_bp < 0) goto loc_2643;
			if (_bp == 0x7777) {
				_bp = 5;
/*				nicky_anim_flags2 = 5;*/
			} else {
				player_state.hdir_flags |= 4;
				player_state.move_offsets_data_ptr++;
			}
		}
		decor_state.scroll_inc = _bp;
		nicky_cur_pos_x1 = player_state.pos_x;
		_cx = ((player_state.pos_x + 4 + player_state.pos_dx_2) >> 4) * _screen_cdg_tile_map_h +  decor_state.tile_map_offs_x;
		player_state.pos_y += _bp;
		_dx = player_state.pos_y + player_state.pos_dy_2;
		if (_dx > 0 && (_dx + player_state.dim_h) < 168) {
			nicky_cur_pos_x2 = _dx + player_state.dim_h;
			player_state.decor_ref_flag4 = 0;
			game_set_decor_tile_flags4_3_2_0(1, _cx + ((nicky_cur_pos_x2 - 16) >> 4) + decor_state.tile_map_offs_y);
			player_state.decor_ref_flag0 &= ~1;
			player_state.decor_ref_flag3 = 0;
			_dx = (nicky_cur_pos_x2 >> 4) + decor_state.tile_map_offs_y;
			_cx += _dx;
			game_set_decor_tile_flags4_3_2_0(1, _cx);
			game_set_decor_tile_flag0(4, _cx);
			_cx = ((nicky_cur_pos_x1 + player_state.pos_dx_1 + player_state.pos_dx_2) >> 4) * _screen_cdg_tile_map_h;
			_cx += decor_state.tile_map_offs_x + _dx;
			game_set_decor_tile_flag0(8, _cx);
			if (game_set_decor_tile_flags4_3_2_0(1, _cx) & 0x40) {
				player_state.next_tilemap_offset = _cx;
			}
			if (player_state.decor_ref_flag4 != 0 && cycle_counter == 0) {
				player_state.colliding += 1;
			}
			if (player_state.decor_ref_flag3 == 2) {
				goto loc_255F;
			}
			if (player_state.decor_ref_flag0 & 1) {
				if ((player_state.decor_ref_flag0 & 0xC) != 0 || (nicky_cur_pos_x2 &= 15) <= nicky_cur_pos_y) {

					player_state.pos_y = (player_state.pos_y + player_state.pos_dy_2 + player_state.dim_h) & ~15;
					player_state.pos_y -= player_state.dim_h;
					player_state.pos_y -= player_state.pos_dy_2;
					goto loc_2570;
				}
			} else if (player_state.decor_ref_flag3 == 1) {
				goto loc_2570;
			}
		}
		player_state.hdir_flags &= ~0x72;
		if ((player_state.hdir_flags & 8) == 0) {
			if (player_state.hdir_flags & 1) {
				player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[40];
			} else {
				player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[41];
			}
			if (player_state.unk1 != 2) {
				player_state.unk1 = 2;
				player_state.move_offsets_data_ptr = &nicky_move_offsets_table__v2[57];
				player_state.hdir_flags |= 4;
			}
		}
		return;
	}
loc_255F:
	if ((inp_direction_mask & 0xC) == 0 || (player_state.hdir_flags & 0x30) == 0) {
loc_2570:
		player_state.unk1 = 1;
		player_state.hdir_flags &= ~4;
		if (player_state.hdir_flags & 1) {
			if (inp_direction_mask & 8) {
				if ((player_state.hdir_flags & 0x40) == 0) {
					player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[54];
					player_state.anim_frames_count = 2;
					player_state.hdir_flags |= 0x40;
				}
			} else if (inp_direction_mask & 1) {
				goto loc_260D;
			} else {
				player_state.hdir_flags &= ~0x40;
				if (player_state.rock_hit_counter == 0) {
					player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[19];
				} else {
					if ((player_state.hdir_flags & 0x80) == 0) {
						player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[20];
						player_state.hdir_flags |= 0x80;
					}
				}
			}
		} else {
			if (inp_direction_mask & 8) {
				if ((player_state.hdir_flags & 0x40) == 0) {
					player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[57];
					player_state.anim_frames_count = 2;
					player_state.hdir_flags |= 0x40;
				}
			} else if (inp_direction_mask & 2) {
				goto loc_260D;
			} else {
				player_state.hdir_flags &= ~0x40;
				if (player_state.rock_hit_counter == 0) {
					player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[31];
				} else {
					if ((player_state.hdir_flags & 0x80) == 0) {
						player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[32];
						player_state.hdir_flags |= 0x80;
					}
				}
			}
		}
		game_set_next_nicky_anim_frame();
		player_state.hdir_flags &= ~0x3E;
	}

loc_260D:
	player_state.unk1 = 0;
	player_state.hdir_flags &= ~4;
	if (nicky_on_elevator_flag == 0) {
		player_state.vdir_flags |= 2;
	} else {
		if (inp_direction_mask & 4) {
			if (player_state.vdir_flags == 0) {
				player_state.move_offsets_data_ptr = nicky_move_offsets_ptr;
				player_state.unk1 = 2;
				player_state.vdir_flags |= 2;
			}
		} else {
			player_state.vdir_flags = 0;
		}
	}
	return;

loc_2643:
/*	nicky_anim_flags2 = -_bp;*/
	player_state.move_offsets_data_ptr++;
	decor_state.scroll_inc = _bp;
	player_state.hdir_flags &= ~4;
	nicky_cur_pos_x1 = player_state.pos_x;
	_cx = ((player_state.pos_x + 4 + player_state.pos_dx_2) >> 4) * _screen_cdg_tile_map_h + decor_state.tile_map_offs_x;
	player_state.pos_y += _bp;
	_dx = player_state.pos_y + 4 + player_state.pos_dy_2;
	if (_dx <= 0 || _dx > 137) goto loc_26D1;
	_dx = (_dx >> 4) + decor_state.tile_map_offs_y;
	_cx += _dx;
	player_state.decor_ref_flag0 &= ~2;
	game_set_decor_tile_flags4_3_2_0(2, _cx);
	_cx = ((nicky_cur_pos_x1 + player_state.pos_dx_1 + player_state.pos_dx_2) >> 4) * _screen_cdg_tile_map_h;
	_cx += decor_state.tile_map_offs_x + _dx;
	game_set_decor_tile_flags4_3_2_0(2, _cx);
	if (player_state.decor_ref_flag0 & 2) {
loc_26D1:
		player_state.hdir_flags |= 4;
		player_state.move_offsets_data_ptr = &nicky_move_offsets_table__v2[57];
	}
	player_state.hdir_flags &= ~0x72;
	if ((player_state.hdir_flags & 8) == 0) {
		if (player_state.hdir_flags & 1) {
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[40];
		} else {
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[41];
		}
	}
}

static void game2_update_nicky_anim_helper1() {
	s16 _cx, _dx;

	_cx = ((player_state.pos_x + player_state.dim_w + player_state.pos_dx_2) >> 4) * _screen_cdg_tile_map_h;
	_cx += decor_state.tile_map_offs_x;
	nicky_cur_pos_x1 = _cx;
	_dx = player_state.pos_y + player_state.pos_dy_1 + player_state.pos_dy_2;
	if (_dx <= 160) {
		_dx >>= 4;
		_dx += decor_state.tile_map_offs_y;
		_cx += _dx;
		player_state.decor_ref_flag0 &= ~4;
		game_set_decor_tile_flags4_3_2_0(4, _cx);
		if ((player_state.decor_ref_flag0 & 4) == 0) {
			_dx = ((player_state.pos_y + 10 + player_state.pos_dy_2) >> 4) + decor_state.tile_map_offs_y;
			_cx = nicky_cur_pos_x1 + _dx;
			game_set_decor_tile_flags4_3_2_0(4, _cx);
			if ((player_state.decor_ref_flag0 & 4) == 0 && player_state.pos_x < GAME_SCREEN_W - 16) {
				player_state.pos_x += GAME_SCROLL_DX;
			}
		}
		if (player_state.hdir_flags & 1) {
			if ((player_state.hdir_flags & 0xC) == 0) {
				if ((player_state.hdir_flags <= 0xF) || (inp_direction_mask & 0xC) == 0) {
					if ((player_state.hdir_flags & 2) == 0) {
						player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[0];
						player_state.hdir_flags |= 2;
						player_state.hdir_flags &= ~0x70;
						player_state.anim_frames_count = 2;
					}
					game_set_next_nicky_anim_frame();
				}
			}
		} else if (player_state.hdir_flags <= 7) {
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[16];
			player_state.hdir_flags |= 9;
			player_state.hdir_flags &= ~0x42;
			player_state.anim_frames_count = 2;
		}
	}
	game2_update_nicky_anim_helper0();
}

static void game2_update_nicky_anim_helper2() {
	s16 _cx, _dx;

	_cx = ((player_state.pos_x + player_state.pos_dx_2) >> 4) * _screen_cdg_tile_map_h;
	_cx += decor_state.tile_map_offs_x;
	nicky_cur_pos_x1 = _cx;
	_dx = player_state.pos_y + player_state.pos_dy_1 + player_state.pos_dy_2;
	if (_dx <= 160) {
		_dx >>= 4;
		_dx += decor_state.tile_map_offs_y;
		_cx += _dx;
		player_state.decor_ref_flag0 &= ~8;
		game_set_decor_tile_flags4_3_2_0(8, _cx);
		if ((player_state.decor_ref_flag0 & 8) == 0) {
			_dx = ((player_state.pos_y + 10 + player_state.pos_dy_2) >> 4) + decor_state.tile_map_offs_y;
			_cx = nicky_cur_pos_x1 + _dx;
			game_set_decor_tile_flags4_3_2_0(8, _cx);
			if ((player_state.decor_ref_flag0 & 8) == 0 && player_state.pos_x >= 0) {
				player_state.pos_x -= GAME_SCROLL_DX;
			}
		}
		if ((player_state.hdir_flags & 1) == 0) {
			if ((player_state.hdir_flags & 0x8C) == 0) {
				if ((player_state.hdir_flags <= 0xF) || (inp_direction_mask & 0xC) == 0) {
					if ((player_state.hdir_flags & 2) == 0) {
						player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[8];
						player_state.hdir_flags |= 2;
						player_state.hdir_flags &= ~0x70;
						player_state.anim_frames_count = 2;
					}
					game_set_next_nicky_anim_frame();
				}
			}
		} else if (player_state.hdir_flags <= 7) {
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[28];
			player_state.hdir_flags |= 8;
			player_state.hdir_flags &= ~0x43;
			player_state.anim_frames_count = 2;
		}
	}
	game2_update_nicky_anim_helper0();
}

static void game2_update_nicky_anim_helper10() {
	s16 _cx, _dx;

	nicky_cur_pos_x1 = player_state.pos_x;
	_cx = ((player_state.pos_x + 4 + player_state.pos_dx_2) >> 4) * _screen_cdg_tile_map_h;
	_cx += decor_state.tile_map_offs_x;
	nicky_cur_pos_y = _cx;
	_dx = player_state.pos_y + player_state.pos_dy_1 + player_state.pos_dy_2;
	if (_dx > 0 && _dx <= 160) {
		_dx >>= 4;
		_dx += decor_state.tile_map_offs_y;
		_cx += _dx;
		player_state.decor_ref_flag0 = 0;
		player_state.decor_ref_flag3 = 0;
		game_set_decor_tile_flags4_3_2_0(2, _cx);
		_cx = ((nicky_cur_pos_x1 + player_state.pos_dx_1 + player_state.pos_dx_2) >> 4) * _screen_cdg_tile_map_h;
		_cx += decor_state.tile_map_offs_x;
		_cx += _dx;
		game_set_decor_tile_flags4_3_2_0(2, _cx);
		if (player_state.decor_ref_flag0 & 2) {
			player_state.hdir_flags &= ~0x30;
			return;
		}
		if (player_state.decor_ref_flag3 == 2) {
			if (player_state.pos_y > 0) {
				_cx = nicky_cur_pos_y;
				_dx = (player_state.pos_y + 4 + player_state.pos_dy_2) >> 4;
				_dx += decor_state.tile_map_offs_y;
				_cx += _dx;
				game_set_decor_tile_flags4_3_2_0(2, _cx);
				if (player_state.decor_ref_flag0 & 2) {
					player_state.hdir_flags &= ~0x30;
					return;
				}
				player_state.pos_y -= GAME_SCROLL_DY;
			}
			if ((player_state.hdir_flags & 0x10) == 0) {
				player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[42];
				player_state.hdir_flags |= 0x10;
				player_state.hdir_flags &= ~0x6E;
				player_state.anim_frames_count = 2;
			}
			game_set_next_nicky_anim_frame();
			return;
		} else if (player_state.decor_ref_flag3 == 0) {
			goto loc_1F14;
		}
		player_state.hdir_flags &= ~0x30;
		return;
	}
loc_1F14:
	player_state.hdir_flags &= ~0x30;
	if (player_state.vdir_flags == 0 && player_state.unk1 == 0) {
		player_state.unk1 = 2;
		player_state.move_offsets_data_ptr = nicky_move_offsets_ptr;
	}
}

static void game2_update_nicky_anim_helper3() {
	game2_update_nicky_anim_helper10();
	game2_update_nicky_anim_helper0();
}

static void game2_update_nicky_anim_helper4() {
	game2_update_nicky_anim_helper10();
	game2_update_nicky_anim_helper1();
}

static void game2_update_nicky_anim_helper5() {
	game2_update_nicky_anim_helper10();
	game2_update_nicky_anim_helper2();
}

static void game2_update_nicky_anim_helper11() {
	s16 _cx, _dx;

	if (nicky_on_elevator_flag) {
		game_set_next_nicky_anim_frame();
		return;
	}
	nicky_cur_pos_x1 = player_state.pos_x;
	_cx = ((player_state.pos_x + 4 + player_state.pos_dx_2) >> 4) * _screen_cdg_tile_map_h;
	_cx += decor_state.tile_map_offs_x;
	_dx = player_state.pos_y + player_state.dim_h + player_state.pos_dy_2;
	if (_dx > 160) {
		return;
	}
	_dx >>= 4;
	_dx += decor_state.tile_map_offs_y;
	_cx += _dx;
	player_state.decor_ref_flag0 = 0;
	player_state.decor_ref_flag3 = 0;
	game_set_decor_tile_flags4_3_2_0(1, _cx);
	_cx = ((nicky_cur_pos_x1 + player_state.pos_dx_1 + player_state.pos_dx_2) >> 4) * _screen_cdg_tile_map_h;
	_cx += decor_state.tile_map_offs_x + _dx;
	game_set_decor_tile_flags4_3_2_0(1, _cx);
	if (player_state.decor_ref_flag0 & 1) {
		player_state.hdir_flags &= ~0x30;
		return;
	}
	if (player_state.decor_ref_flag3 == 2) {
		if (player_state.pos_y <= 161) {
			player_state.pos_y += GAME_SCROLL_DY;
		}
		if ((player_state.hdir_flags & 0x20) == 0) {
			player_state.anim_frame_ptr = &nicky_anim_frames_table__v2[48];
			player_state.hdir_flags |= 0x20;
			player_state.hdir_flags &= ~0x5E;
			player_state.unk1 = 0;
			player_state.anim_frames_count = 2;
		}
		game_set_next_nicky_anim_frame();
	} else if (player_state.decor_ref_flag3 == 0) {
		player_state.hdir_flags &= ~0x30;
	}
}

static void game2_update_nicky_anim_helper6() {
	game2_update_nicky_anim_helper11();
	game2_update_nicky_anim_helper0();
}

static void game2_update_nicky_anim_helper9() {
	s16 _cx, _dx;

	_cx = ((player_state.pos_x + player_state.dim_w + player_state.pos_dx_2) >> 4) * _screen_cdg_tile_map_h;
	_cx += decor_state.tile_map_offs_x;
	nicky_cur_pos_x1 = _cx;
	_dx = player_state.pos_y + player_state.pos_dy_1 + player_state.pos_dy_2;
	if (_dx > 160) {
		return;
	}
	_dx >>= 4;
	_dx += decor_state.tile_map_offs_y;
	_cx += _dx;
	player_state.decor_ref_flag0 &= ~4;
	game_set_decor_tile_flags4_3_2_0(4, _cx);
	if (player_state.decor_ref_flag0 & 4) {
		return;
	}
	_cx = nicky_cur_pos_x1;
	_dx = (player_state.pos_y + 10 + player_state.pos_dy_2) >> 4;
	_dx += decor_state.tile_map_offs_y;
	_cx += _dx;
	game_set_decor_tile_flags4_3_2_0(4, _cx);
	if (player_state.decor_ref_flag0 & 4) {
		nicky_cur_pos_x1 += _screen_cdg_tile_map_h;
		_cx = nicky_cur_pos_x1;
		_dx = player_state.pos_y + player_state.pos_dy_1 + player_state.pos_dy_2;
		if (_dx > 160) {
			return;
		}
		_dx >>= 4;
		_dx += decor_state.tile_map_offs_y;
		_cx += _dx;
		player_state.decor_ref_flag0 &= ~4;
		game_set_decor_tile_flags4_3_2_0(4, _cx);
		if (player_state.decor_ref_flag0 & 4) {
			return;
		}
		_cx = nicky_cur_pos_x1;
		_dx = (player_state.pos_y + 10 + player_state.pos_dy_2) >> 4;
		_dx += decor_state.tile_map_offs_y;
		_cx += _dx;
		player_state.decor_ref_flag0 &= ~4;
		game_set_decor_tile_flags4_3_2_0(4, _cx);
		if (player_state.decor_ref_flag0 & 4) {
			return;
		}
	} else {
		nicky_cur_pos_x1 -= _screen_cdg_tile_map_h;
		_cx = nicky_cur_pos_x1;
		_dx = (player_state.pos_y + 10 + player_state.pos_dy_2) >> 4;
		_dx += decor_state.tile_map_offs_y;
		_cx += _dx;
		player_state.decor_ref_flag0 &= ~4;
		game_set_decor_tile_flags4_3_2_0(4, _cx);
		if ((player_state.decor_ref_flag0 & 4) == 0) {
			return;
		}
	}
	player_state.pos_x += GAME_SCROLL_DX;
	inp_direction_mask |= 8;
	nicky_anim_flags1 = 0xFFFF;
}

static void game2_update_nicky_anim_helper7() {
	game2_update_nicky_anim_helper11();
	game2_update_nicky_anim_helper9();
	game2_update_nicky_anim_helper0();
}

static void game2_update_nicky_anim_helper12() {
	s16 _cx, _dx;

	_cx = ((player_state.pos_x - 2 + player_state.pos_dx_2) >> 4) * _screen_cdg_tile_map_h;
	_cx += decor_state.tile_map_offs_x;
	nicky_cur_pos_x1 = _cx;
	_dx = player_state.pos_y + player_state.pos_dy_1 + player_state.pos_dy_2;
	if (_dx > 160) {
		return;
	}
	_dx >>= 4;
	_dx += decor_state.tile_map_offs_y;
	_cx += _dx;
	player_state.decor_ref_flag0 &= ~8;
	game_set_decor_tile_flags4_3_2_0(8, _cx);
	if (player_state.decor_ref_flag0 & 8) {
		return;
	}
	_cx = nicky_cur_pos_x1;
	_dx = (player_state.pos_y + 10 + player_state.pos_dy_2) >> 4;
	_dx += decor_state.tile_map_offs_y;
	_cx += _dx;
	player_state.decor_ref_flag0 &= ~8;
	game_set_decor_tile_flags4_3_2_0(8, _cx);
	if (player_state.decor_ref_flag0 & 8) {
		nicky_cur_pos_x1 -= _screen_cdg_tile_map_h;
		_cx = nicky_cur_pos_x1;
		_dx = (player_state.pos_y + 10 + player_state.pos_dy_2) >> 4;
		_dx += decor_state.tile_map_offs_y;
		if (_dx > 160) {
			return;
		}
		_dx >>= 4;
		_dx += decor_state.tile_map_offs_y;
		_cx += _dx;
		player_state.decor_ref_flag0 &= ~8;
		game_set_decor_tile_flags4_3_2_0(8, _cx);
		if (player_state.decor_ref_flag0 & 8) {
			return;
		}
		_cx = nicky_cur_pos_x1;
		_dx = (player_state.pos_y + 10 + player_state.pos_dy_2) >> 4;
		_dx += decor_state.tile_map_offs_y;
		_cx += _dx;
		player_state.decor_ref_flag0 &= ~8;
		game_set_decor_tile_flags4_3_2_0(8, _cx);
		if (player_state.decor_ref_flag0 & 8) {
			return;
		}
	} else {
		nicky_cur_pos_x1 += _screen_cdg_tile_map_h;
		_cx = nicky_cur_pos_x1;
		_dx = (player_state.pos_y + 10 + player_state.pos_dy_2) >> 4;
		_dx += decor_state.tile_map_offs_y;
		_cx += _dx;
		player_state.decor_ref_flag0 &= ~8;
		game_set_decor_tile_flags4_3_2_0(8, _cx);
		if ((player_state.decor_ref_flag0 & 8) == 0) {
			return;
		}
	}
	player_state.pos_x -= GAME_SCROLL_DX;
	inp_direction_mask |= 8;
}

static void game2_update_nicky_anim_helper8() {
	game2_update_nicky_anim_helper11();
	game2_update_nicky_anim_helper12();
	game2_update_nicky_anim_helper0();
}

static update_nicky_anim_helper_pf update_nicky_anim_helper_table__v1[] = {
	/* 0x00 */
	game_update_nicky_anim_helper0,
	game_update_nicky_anim_helper1, /* GAME_DIR_RIGHT */
	game_update_nicky_anim_helper2, /* GAME_DIR_LEFT */
	0, /* GAME_DIR_RIGHT | GAME_DIR_LEFT */
  	/* 0x04 */
 	game_update_nicky_anim_helper3, /* GAME_DIR_UP */
 	game_update_nicky_anim_helper4, /* GAME_DIR_UP | GAME_DIR_RIGHT */
 	game_update_nicky_anim_helper5, /* GAME_DIR_UP | GAME_DIR_LEFT */
 	0, /* GAME_DIR_UP | GAME_DIR_LEFT | GAME_DIR_RIGHT */
  	/* 0x08 */
 	game_update_nicky_anim_helper6, /* GAME_DIR_DOWN */
 	game_update_nicky_anim_helper6, /* GAME_DIR_DOWN | GAME_DIR_RIGHT */
 	game_update_nicky_anim_helper6  /* GAME_DIR_DOWN | GAME_DIR_LEFT */
};

static update_nicky_anim_helper_pf update_nicky_anim_helper_table__v2[] = {
	/* 0x00 */
	game2_update_nicky_anim_helper0,
	game2_update_nicky_anim_helper1,
	game2_update_nicky_anim_helper2,
	game2_update_nicky_anim_helper0,
  	/* 0x04 */
 	game2_update_nicky_anim_helper3,
 	game2_update_nicky_anim_helper4,
 	game2_update_nicky_anim_helper5,
 	game2_update_nicky_anim_helper0,
  	/* 0x08 */
 	game2_update_nicky_anim_helper6,
 	game2_update_nicky_anim_helper7,
 	game2_update_nicky_anim_helper8
};

void game_update_anim_helper() {
	if (NICKY1) {
		assert(inp_direction_mask < ARRAYSIZE(update_nicky_anim_helper_table__v1) && inp_direction_mask != 3 && inp_direction_mask != 7);
		(*update_nicky_anim_helper_table__v1[inp_direction_mask])();
	}
	if (NICKY2) {
		assert(inp_direction_mask < ARRAYSIZE(update_nicky_anim_helper_table__v2));
		(*update_nicky_anim_helper_table__v2[inp_direction_mask])();
	}
}
