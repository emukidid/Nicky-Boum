/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#ifndef __GAME_H__
#define __GAME_H__

#include "util.h"
#include "common.h"

//#define GAME_SCREEN_W  240
//#define GAME_SCREEN_H  160
#define GAME_SCREEN_W  320
#define GAME_SCREEN_H  240
#define GAME_SCROLL_DX   4
#define GAME_SCROLL_DY   4

#define GAME_DIR_RIGHT 1
#define GAME_DIR_LEFT  2
#define GAME_DIR_UP    4
#define GAME_DIR_DOWN  8

#define GAME_INV_MEGABOMBS 0
#define GAME_INV_BOMBS     1
#define GAME_INV_KEYS      2
#define GAME_INV_LIFES     3

extern void game_init();
extern void game_run(int start_level_num);
extern void game_destroy();
extern uint16 game_get_random_number(uint16 mod);
extern void game_prepare_level(int level);
extern void game_init_level();
extern void game_init_nicky();
extern void game_enable_nicky_shield();
extern void game_disable_nicky_shield();
extern void game_run_cycle();
extern void game_init_player_state();
extern void game_update_nicky_anim();
extern void game2_update_nicky_anim1();
extern void game2_update_nicky_anim2();
extern void game_adjust_player_position();
extern void game_handle_nicky_shield();
extern void game_play_sound();
extern void game_draw_bomb_object();
extern void game_draw_object(object_state_t *os);
extern void game_shake_screen();
extern void game_update_decor_after_megabomb();
extern void game_draw_nicky();
extern void game_redraw_inventory();
extern uint8 game_set_decor_tile_flags4_3_2_0(uint8 b, int offset);
extern void game_set_decor_tile_flag0(uint8 b, int offset);
extern void inp_copy_state();
extern void game_set_next_nicky_anim_frame();
extern void game_init_level_start_screen();
extern void game_init_ref_ref(anim_data_t *ad, uint16 flags);
extern void game_init_objects_from_positref(const uint8 *posit_ref, uint16 objects_offset);
extern void game_execute_logic_op();
extern void game_update_anim_helper();
extern void game_change_decor_tile_map_line(const object_state_t *os, const anim_data_t *ad);
extern void game_change_decor_tile_map_quad(const object_state_t *os);
extern void game_change_decor_tile_map(const object_state_t *os);
extern void game_update_score();
extern void game_adjust_tilemap();
extern void game_handle_level_change();
extern void game_handle_projectiles_collisions();
extern void game_execute_colliding_op();
extern void game_execute_colliding_op_helper1();
extern void game_execute_colliding_op_helper2();
extern void game_play_anim_data_sfx(const anim_data_t *ad);
extern void game_set_object_to_initial_pos(object_state_t *os, anim_data_t *ad);
extern void game_setup_monster();
extern anim_data_t *game_init_object_from_anim3(object_state_t *os, anim_data_t *ad);
extern void game_handle_projectiles_collisions_helper1(object_state_t *os);
extern void game_handle_projectiles_collisions_helper2(object_state_t *os, int _bp, int count);
extern void game_handle_projectiles_collisions_helper3(object_state_t *os, anim_data_t *ad);
extern void game_execute_action_op();
extern anim_data_t *game_logic_op_helper1(object_state_t *os, anim_data_t *ad);
extern void game_logic_op_helper2(object_state_t *_di, anim_data_t *_si, anim_data_t *ad, uint16 move);
extern anim_data_t *game_logic_op2(object_state_t *os, anim_data_t *ad);
extern anim_data_t *game_logic_op(object_state_t *os, anim_data_t *ad);
extern void game_collides_op(object_state_t *os, anim_data_t *ad);

#endif /* __GAME_H__ */
