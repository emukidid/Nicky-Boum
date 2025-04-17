/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#ifndef __MOD_PLAYER_H__
#define __MOD_PLAYER_H__

#include "util.h"

#define MPL_NUM_SAMPLES    31
#define MPL_NUM_TRACKS      4
#define MPL_NUM_PATTERNS  128
#define MPL_NUM_STEPS     999

typedef struct {
	uint16 len;
	uint8 fine_tune;
	int8 volume;
	uint16 loop_pos;
	uint16 loop_len;
	uint16 loop_end;
	const int8 *data;
} mpl_sample_info_t;

typedef struct {
	mpl_sample_info_t sample_table[MPL_NUM_SAMPLES];
	uint8 num_patterns;
	const uint8 *pattern_order_table;
	const uint8 *pattern_table;
} mpl_module_info_t;

typedef struct {
	mpl_sample_info_t *sample;
	int8 volume;
	int pos;
	uint16 period;
	uint16 period_index;
	uint16 effect_data;
	int delay_counter;
} mpl_track_t;

extern const uint16 mpl_period_table[576];

extern void mpl_init(int mixing_rate);
extern void mpl_load(const uint8 *module_data, const uint8 *instrument_data);
extern void mpl_stop();
extern GCC_INT_CALL void mpl_play_callback(int8 *dst, int len);

#endif /* __MOD_PLAYER_H__ */
