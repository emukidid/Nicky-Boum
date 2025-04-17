/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#include "util.h"
#include "common.h"

#ifdef VARDECL
#define VARDEF
#else
#define VARDEF extern
#endif

#if 0 /* NICKY1 */
#define REF_REF_SIZE 282
#define OBJECTS_SIZE 453
#define DECOR_CDG_SIZE 20000
#endif
#if 1 /* NICKY2 */
#define REF_REF_SIZE 338
#define OBJECTS_SIZE 453
#define DECOR_CDG_SIZE 20240
#endif

VARDEF player_state_t player_state;
VARDEF int inventory_enabled_flag;
VARDEF int new_level_flag;
VARDEF int quit_level_flag;
VARDEF uint16 random_seed;
VARDEF int current_level;
VARDEF game_state_t game_state;
VARDEF decor_state_t decor_state;
VARDEF int player_pos_x, player_pos_y;
VARDEF int cycle_counter;
VARDEF int bonus_count;
VARDEF int boss4_state;
VARDEF int cycle_score;
VARDEF const uint16 *nicky_move_offsets_ptr;
VARDEF int restart_level_flag;
VARDEF anim_data_t *cur_anim_data_ptr;
VARDEF int cur_anim_data_count;
VARDEF int unk_ref_index;
VARDEF anim_data_t *ref_ref_1, *ref_ref_7, *ref_ref_43;
VARDEF int update_nicky_anim_flag;
VARDEF s16 megabomb_origin_x, megabomb_origin_y;
VARDEF s16 nicky_cur_pos_x1, nicky_cur_pos_y, nicky_cur_pos_x2;
VARDEF int execute_action_op_flag;
VARDEF int update_decor_counter;
VARDEF int throw_apple_var;
VARDEF int bounding_box_x1, bounding_box_x2, bounding_box_y1, bounding_box_y2;
VARDEF int monster_fight_flag;
VARDEF int change_pal_flag;
VARDEF object_state_t objects_table[OBJECTS_SIZE] GCC_EXT;
VARDEF object_state_t *objects_table_ptr1;
VARDEF object_state_t *objects_table_ptr2;
VARDEF object_state_t *objects_table_ptr3;
VARDEF object_state_t *objects_table_ptr4;
VARDEF object_state_t *objects_table_ptr5;
VARDEF object_state_t *objects_table_ptr6;
VARDEF object_state_t *objects_table_ptr7;
VARDEF object_state_t *objects_table_ptr8;
VARDEF object_state_t *objects_table_ptr9;
VARDEF object_state_t *objects_list_cur, *objects_list_head, *objects_list_last;
VARDEF int _screen_tile_origin_x, _screen_tile_origin_y;
VARDEF int _screen_tile_map_offs_x, _screen_tile_map_offs_y, _screen_tile_map_offs_x_shl4, _screen_tile_map_offs_y_shl4;
VARDEF int _screen_cdg_tile_map_start_x, _screen_cdg_tile_map_start_y, _screen_cdg_tile_map_w, _screen_cdg_tile_map_h;

/* NICKY2 specific */
VARDEF uint16 nicky_anim_flags1;
VARDEF uint16 nicky_anim_flags3;
VARDEF const object_state_t *nicky_elevator_object;
VARDEF int nicky_on_elevator_flag;
VARDEF const int8 *nicky_colliding_op39_moving_table;
VARDEF int colliding_op35_flag;
VARDEF int jump_to_new_level_after_boss;

/* input.c */
VARDEF uint8 inp_direction_mask;
VARDEF uint8 inp_fire_button;
VARDEF uint8 inp_fire_button_num_cycles_pressed;

/* resource.c */
VARDEF uint8 res_decor_cdg[DECOR_CDG_SIZE] GCC_EXT;
VARDEF anim_data_t res_ref_ref[REF_REF_SIZE] GCC_EXT;
VARDEF const uint8 *res_decor_pal;
VARDEF const uint8 *res_decor_blk;
VARDEF const uint8 *res_decor_ref;
VARDEF const uint8 *res_level_spr;
VARDEF const uint8 *res_posit_ref;
VARDEF const uint8 *res_monster_spr;
VARDEF const uint8 *res_flash_pal;
VARDEF const uint8 *res_nicky_spr;
VARDEF const uint8 *res_digits_spr;
VARDEF const uint8 *res_lifebar_spr;
VARDEF const uint8 *res_new_new;

/* staticres.c */
VARDEF anim_data_t anim_data0[23];
VARDEF anim_data_t anim_data_starshield[8];
extern anim_frame_t nicky_anim_frames_table__v1[52];
extern anim_frame_t nicky_anim_frames_table__v2[147];
extern const uint16 nicky_move_offsets_table__v1[58];
extern const uint16 nicky_move_offsets_table__v2[102];
extern const uint16 nicky_move_offsets_table5[126];
extern const uint16 cos_table[512];
extern const uint16 sin_table[512];
extern const int move_delta_offsets_table[15];
extern const uint8 move_delta_table[942];
extern const uint8 move_delta_table2[204];
extern const uint16 bird_move_dy_table[15];

#endif /* __GAME_H__ */
