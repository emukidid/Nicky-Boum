/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#include "mixer.h"

static int mixing_rate;
static mixer_channel_t channels_table[MIXER_MAX_CHANNELS];

void mixer_init(int sample_rate) {
	mixing_rate = sample_rate;
	memset(channels_table, 0, sizeof(channels_table));
}

void mixer_play_sfx(const int8 *sample_data, uint32 sample_size, uint32 sample_period) {
	int i;
	/* find and setup a free channel */
	for (i = 0; i < ARRAYSIZE(channels_table); ++i) {
		mixer_channel_t *channel = &channels_table[i];
		if (channel->chunk_data) {
			if (channel->chunk_data == sample_data) {
				channel->chunk_pos = 0;
				break;
			}
		} else {
			channel->chunk_data = sample_data;
			channel->chunk_size = sample_size;
			channel->chunk_pos = 0;
			channel->chunk_inc = (sample_period << MIXER_FRAC_BITS) / mixing_rate;
			break;
		}
	}
}

GCC_INT_CALL void mixer_mix_samples(int8 *dst, int len) {
	int ch, i;
	for (ch = 0; ch < ARRAYSIZE(channels_table); ++ch) {
		mixer_channel_t *channel = &channels_table[ch];
		if (channel->chunk_data) {
			for (i = 0; i < len; ++i) {
				int b = dst[i];
				const int pos = channel->chunk_pos >> MIXER_FRAC_BITS;
				if (pos >= channel->chunk_size) {
					channel->chunk_data = 0;
					break;
				}
				b += channel->chunk_data[pos];
				if (b < -128) {
					b = -128;
				} else if (b > 127) {
					b = 127;
				}
				dst[i] = (int8)b;
				channel->chunk_pos += channel->chunk_inc;
			}
		}
	}
}
