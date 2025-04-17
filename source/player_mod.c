/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#include "mixer.h"
#include "sound.h"
#include "globals.h"
#include "player_mod.h"

static int playing_flag;
static int samples_left;
static uint8 current_tick;
static uint8 song_speed;
static uint8 current_pattern_pos, current_pattern_order;
static mpl_module_info_t module_info;
static mpl_track_t tracks_table[MPL_NUM_TRACKS];
static int pos_steps_table[MPL_NUM_STEPS];
static int tick_samples_size;

void mpl_init(int mixing_rate) {
	int i;

	pos_steps_table[0] = 1;
	for (i = 1; i < MPL_NUM_STEPS; ++i) {
		pos_steps_table[i] = ((PAULA_FREQ / i) << MIXER_FRAC_BITS) / mixing_rate;
	}
	tick_samples_size = mixing_rate / 50;
}

void mpl_load(const uint8 *module_data, const uint8 *instrument_data) {
	int i;

	memset(&module_info, 0, sizeof(module_info));
	module_data += 20; /* skip song name */

	/* read samples */
	for (i = 0; i < MPL_NUM_SAMPLES; ++i) {
		mpl_sample_info_t *sample = &module_info.sample_table[i];
		if (NICKY2 && i == 6 && memcmp(module_data, "FLUTE-DE-PAN", 12) != 0) {
			instrument_data += 8192;
		}
		module_data += 22; /* skip sample name */
		sample->len = read_uint16BE(module_data) * 2; module_data += 2;
		sample->fine_tune = *module_data++;
		sample->volume = *module_data++;
		sample->loop_pos = read_uint16BE(module_data) * 2; module_data += 2;
		sample->loop_len = read_uint16BE(module_data) * 2; module_data += 2;
		sample->loop_end = sample->loop_pos + sample->loop_len;
		sample->data = (const int8 *)instrument_data; instrument_data += sample->len;
		assert(sample->len == 0 || sample->loop_pos + sample->loop_len <= sample->len);
	}

	module_info.num_patterns = *module_data++;
	assert(module_info.num_patterns < MPL_NUM_PATTERNS);
	++module_data; /* skip 0x7F */
	module_info.pattern_order_table = module_data; module_data += MPL_NUM_PATTERNS;
	module_data += 4; /* skip 'M.K.' */

	module_info.pattern_table = module_data;

	/* setup playback */
	current_tick = 0;
	current_pattern_pos = 0;
	current_pattern_order = 0;
	memset(tracks_table, 0, sizeof(tracks_table));
	song_speed = 6;
	samples_left = 0;
	playing_flag = 1;
}

static GCC_INT_CALL uint16 find_period(uint16 period, uint8 fine_tune) {
	int p;
	for (p = 0; p < 36; ++p) {
		if (mpl_period_table[p] == period) {
			return fine_tune * 36 + p;
		}
	}
	print_error("Invalid period %d", period);
	return 0;
}

static GCC_INT_CALL void handle_note(int track_num, uint32 note_data) {
	mpl_track_t *track = &tracks_table[track_num];
	uint16 sample_num = ((note_data >> 24) & 0xF0) | ((note_data >> 12) & 0xF);
	uint16 sample_per = (note_data >> 16) & 0xFFF;
	uint16 effect_data = note_data & 0xFFF;
	if (sample_num != 0) {
		track->sample = &module_info.sample_table[sample_num - 1];
		track->volume = track->sample->volume;
		track->pos = 0;
	}
	if (sample_per != 0) {
		track->period_index = find_period(sample_per, track->sample->fine_tune);
		track->period = mpl_period_table[track->period_index];
	}
	track->effect_data = effect_data;
}

static GCC_INT_CALL void handle_effect(int track_num, int tick_flag) {
	mpl_track_t *track = &tracks_table[track_num];
	uint8 eff_num = track->effect_data >> 8;
	uint8 eff_xy = track->effect_data & 0xFF;
	uint8 eff_x = eff_xy >> 4;
	uint8 eff_y = eff_xy & 0xF;
	switch (eff_num) {
	case 0x0: /* arpeggio */
		if (tick_flag && eff_xy != 0) {
			switch (current_tick & 3) {
			case 1:
				track->period = mpl_period_table[track->period_index + eff_x];
				break;
			case 2:
				track->period = mpl_period_table[track->period_index + eff_y];
				break;
			}
		}
		break;
	case 0x1: /* portamento up */
		if (tick_flag) {
			track->period -= eff_xy;
			if (track->period < 113) {
				track->period = 113;
			}
		}
		break;
	case 0x2: /* portamento down */
		if (tick_flag) {
			track->period += eff_xy;
			if (track->period > 856) {
				track->period = 856;
			}
		}
		break;
	case 0xA: /* slide volume */
		if (tick_flag) {
			track->volume += eff_x - eff_y;
			if (track->volume < 0) {
				track->volume = 0;
			} else if (track->volume > 64) {
				track->volume = 64;
			}
		}
		break;
	case 0xC: /* set volume */
		if (!tick_flag) {
			track->volume = eff_xy;
		}
		break;
	case 0xD: /* pattern break */
		if (!tick_flag) {
			current_pattern_pos = eff_x * 10 + eff_y;
			assert(current_pattern_pos < 64);
			++current_pattern_order;
		}
		break;
	case 0xF: /* set speed */
		if (!tick_flag) {
			assert(eff_xy < 0x20);
			song_speed = eff_xy;
		}
		break;
	default:
		print_warning("Unhandled effect 0x%X params=0x%X", eff_num, eff_xy);
		break;
	}
}

static GCC_INT_CALL void handle_tick() {
	int i;
	if (current_tick == 0) {
		uint8 pat = module_info.pattern_order_table[current_pattern_order];
		const uint8 *p = module_info.pattern_table + (pat * 64 + current_pattern_pos) * 16;
		for (i = 0; i < MPL_NUM_TRACKS; ++i) {
			uint32 note_data = read_uint32BE(p); p += 4;
			handle_note(i, note_data);
		}
		++current_pattern_pos;
		if (current_pattern_pos == 64) {
			++current_pattern_order;
			current_pattern_pos = 0;
		}
	}
	for (i = 0; i < MPL_NUM_TRACKS; ++i) {
		handle_effect(i, (current_tick != 0));
	}
	++current_tick;
	if (current_tick == song_speed) {
		current_tick = 0;
	}
	if (current_pattern_order == module_info.num_patterns) {
		current_pattern_order = 0;
		print_debug(DBG_MODPLAYER, "handle_tick() restarting module");
	}
}

void mpl_stop() {
	playing_flag = 0;
}

static GCC_INT_CALL void mix_samples(int8 *dst, int len) {
	int t, i, pos_step;
	for (t = 0; t < MPL_NUM_TRACKS; ++t) {
		mpl_track_t *track = &tracks_table[t];
		if (track->sample != 0 && track->delay_counter == 0) {
			mpl_sample_info_t *sample = track->sample;
			assert(track->period != 0 && track->period < MPL_NUM_STEPS);
			pos_step = pos_steps_table[track->period];
			for (i = 0; i < len; ++i) {
				int b = dst[i];
				int pos = track->pos >> MIXER_FRAC_BITS;
				if (sample->loop_len > 2) {
					if (pos >= sample->loop_end) {
						pos = sample->loop_pos;
						track->pos = pos << MIXER_FRAC_BITS;
					}
				} else {
					if (pos >= sample->len) {
						break;
					}
				}
				/* soften the music a bit (compared to sfx) */
				b += sample->data[pos] * track->volume >> 7;
				if (b < -128) {
					b = -128;
				} else if (b > 127) {
					b = 127;
				}
				dst[i] = (int8)b;
				track->pos += pos_step;
			}
		}
	}
}

GCC_INT_CALL void mpl_play_callback(int8 *dst, int len) {
	if (playing_flag) {
		while (len != 0) {
			int count;
			if (samples_left == 0) {
				handle_tick();
				samples_left = tick_samples_size;
			}
			count = MIN(samples_left, len);
			mix_samples(dst, count);
			dst += count;
			samples_left -= count;
			len -= count;
		}
	}
}
