/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#ifndef __COMMON_H__
#define __COMMON_H__

#include "util.h"

typedef struct anim_data_t {
	int8 unk0;
	s16 lifes;
	int8 cycles;
	int8 unk3;
	uint8 unk4;
	uint8 unk5;
	uint8 unk6;
	uint8 init_sprite_num;
	uint8 colliding_opcode;
	uint8 logic_opcode;
	int8 sound_num;
	uint8 rnd;
	uint16 sprite_num;
	uint16 sprite_flags;
	uint16 default_sprite_num;
	uint16 default_sprite_flags;
	uint16 anim_w;
	uint16 anim_h;
	uint16 score;
	s16 bounding_box_x1;
	s16 bounding_box_x2;
	s16 bounding_box_y1;
	s16 bounding_box_y2;
	const uint8 *move_data_ptr;
	uint16 distance_dx;
	uint16 distance_dy;
	struct anim_data_t *anim_data1_ptr;
	struct anim_data_t *anim_data2_ptr;
	struct anim_data_t *anim_data3_ptr;
	uint16 dx;
	uint16 dy;
	struct anim_data_t *anim_data4_ptr;
	s16 dx2; /* map_pos_dx */
	s16 dy2; /* map_pos_dy */
	s16 draw_dx; /* Nicky2 specific */
	s16 draw_dy; /* Nicky2 specific */
} anim_data_t;

typedef struct object_state_t {
	uint8 displayed;
	s16 life;
	int8 anim_data_cycles;
	int8 unk3;
	s16 map_pos_x; /* init_map_pos_x ? */
	s16 map_pos_y; /* init_map_pos_y ? */
	anim_data_t *anim_data1_ptr;
	anim_data_t *anim_data2_ptr;
	anim_data_t *anim_data3_ptr;
	anim_data_t *anim_data4_ptr;
	const uint8 *move_data_ptr;
	s16 distance_pos_x;
	s16 distance_pos_y;
	s16 pos_x;
	s16 pos_y;
	uint16 ref_ref_index;
	uint8 unk26;
	uint8 unk27;
	uint8 transparent_flag;
	uint8 unk29;
	uint8 tile_num;
	uint8 visible;
	s16 anim_data3_ptr_; /* not in the original */
	s16 anim_data4_ptr_; /* not in the original */
	s16 anim_data4_seg_; /* not in the original */
} object_state_t;

typedef struct anim_frame_t {
	uint8 frames_count;
	uint8 action_op3_flag;
	s16 frame_w;
	s16 frame_h;
	s16 _null1;
	s16 _null2;
	s16 frame_w2;
	s16 delta_y;
	s16 frame_h2;
	s16 sprite_num;
	s16 _null3;
	struct anim_frame_t *next_anim_frame;
	s16 _null4;
	s16 pos_dx;
	s16 pos_dy;
	s16 pos_x;
	s16 pos_y;
	int8 draw_dx; /* Nicky2 specific */
	int8 draw_dy; /* Nicky2 specific */
} anim_frame_t;

typedef struct {
	s16 delta_x;
	s16 delta_y;
	s16 tile_map_offs_x;
	s16 tile_map_offs_y;
	s16 tile_map_end_offs_x;
	s16 tile_map_end_offs_y;
	s16 scroll_dx;
	s16 scroll_dy;
	s16 tile_block_x;
	s16 tile_block_y;
	s16 offs_x;
	s16 offs_y;
	s16 scroll_inc;
} decor_state_t;

typedef struct {
	uint8 tile_num;
	uint8 anim;
	uint8 tiles_table[4];
} tile_anim_t;

typedef struct {
	object_state_t *starshield_object_state;
	int score;
	int extra_life_score;
	uint8 boss_fight_counter;
	uint8 boss_explosion_counter;
	int8 tile_cycling_counter;
	tile_anim_t tile_anims_table[4];
} game_state_t;

typedef struct {
	uint8 unk0;
	uint8 unk1; /* mode 2:jumping */
	uint8 hdir_flags; /* 1:right, 0x10:climb */
	uint8 vdir_flags;
	s16 pos_x;
	s16 pos_y;
	s16 dim_w;
	s16 dim_h;
	s16 pos_dx_1;
	s16 pos_dy_1;
	s16 pos_dx_2;
	s16 pos_dy_2;
	const uint16 *move_offsets_data_ptr;
	uint16 anim_frames_count;
	uint8 fire_button_counter;
	uint8 throwing_ball_counter;
	uint8 decor_ref_flag0;
	uint8 increment_life_bar;
	uint8 bombs_count;
	uint8 megabombs_count;
	uint8 keys_count;
	uint8 lifes_count;
	uint8 decor_ref_flag4;
	uint8 decor_ref_flag3;
	uint8 dead_flag;
	uint8 decor_ref_flag2;
	uint8 has_blue_ball;
	int8 throwing_apple_counter;
	uint8 energy;
	uint8 colliding;
	uint8 action_code;
	s16 has_red_ball;
	s16 has_wood;
	s16 shield_duration;
	s16 tile_blinking_duration;
	s16 has_spring;
	s16 tilemap_offset;
	uint16 next_tilemap_offset;
	anim_frame_t *anim_frame_ptr;
	uint8 colliding_flag;
	uint8 rock_hit_counter; /* Nicky2 specific */
	uint8 bird_move_speed; /* Nicky2 specific */
	uint8 bird_dir_flags; /* Nicky2 specific */
	uint8 bird_counter; /* Nicky2 specific */
	const uint16 *bird_move_dy_ptr;
} player_state_t;

typedef void (*action_op_pf)();

typedef void (*update_nicky_anim_helper_pf)();

typedef void (*colliding_op_pf)(object_state_t *os, anim_data_t *ad);

typedef anim_data_t *(*logic_op_pf)(object_state_t *os, anim_data_t *ad);

#endif /* __COMMON_H__ */
