/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#ifndef __MIXER_H__
#define __MIXER_H__

#include "util.h"

#define MIXER_MAX_CHANNELS 2
#define MIXER_BUFFER_SIZE 2048
#define MIXER_FRAC_BITS 16

typedef struct {
	const int8 *chunk_data;
	uint32 chunk_size;
	uint32 chunk_pos;
	uint32 chunk_inc;
} mixer_channel_t;

extern void mixer_init(int sample_rate);
extern void mixer_play_sfx(const int8 *sample_data, uint32 sample_size, uint32 sample_rate);
extern GCC_INT_CALL void mixer_mix_samples(int8 *dst, int len);

#endif
