/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#include "fileio.h"
#include "sound.h"
#include "sequence.h"
#include "systemstub.h"

#define FL_QUIT (1 << 0)

int seq_display_image(const char *image, int delay) {
	const uint8 *file_data;
	int slot, event_mask;

	slot = fio_open(image, FIO_READ, 1);
	file_data = fio_fetch(slot);
	if (read_uint16LE(file_data) == 0x4D42) {
		const uint32 offs_bmp = read_uint16LE(file_data + 10);
		const uint32 offs_pal = 14 + read_uint32LE(file_data + 14);
		const int w = read_uint32LE(file_data + 18);
		const int h = read_uint32LE(file_data + 22);
		const int num_palette_colors = 1 << read_uint16LE(file_data + 28);
		assert(num_palette_colors <= 256);
		sys_set_palette_bmp(file_data + offs_pal, num_palette_colors);
		sys_blit_bitmap(file_data + offs_bmp, w, h);
	}
	sys_fade_in_palette();
	event_mask = sys_wait_for_keys(delay, SKM_ACTION);
	sys_fade_out_palette();
	fio_close(slot);
	return (event_mask & SKM_ACTION) != 0;
}

static void seq_play(const char *filename, int loop_flag) {
	const uint8 *file_data;
	int slot, i, images_count;

	slot = fio_open(filename, FIO_READ, 1);
	file_data = fio_fetch(slot);
	file_data += 4; /* skip tag */
	images_count = read_uint16BE(file_data); file_data += 2;
	do {
		const uint8 *seq_data = file_data;
		for (i = 0; i < images_count; ++i) {
			const char *image_filename;
			int quit_flag, image_delay, image_flags;

			image_filename = (const char *)seq_data; seq_data += strlen(image_filename) + 1;
			image_delay = read_uint16BE(file_data); seq_data += 2;
			image_flags = *seq_data++;

			quit_flag = seq_display_image(image_filename, image_delay);
			if (sys_exit_flag || ((image_flags & FL_QUIT) && quit_flag)) {
				loop_flag = 0;
				break;
			}
		}
	} while (loop_flag);
	fio_close(slot);
}

void seq_play_intro() {
	sys_set_screen_mode(SSM_BITMAP);
	snd_play_song(SND_MUS_SONGPRES);
	seq_play("INTRO.SEQ", 1);
}

void seq_play_ending() {
	sys_set_screen_mode(SSM_BITMAP);
	snd_play_song(SND_MUS_SONGWIN);
	seq_play("ENDING.SEQ", 0);
}
