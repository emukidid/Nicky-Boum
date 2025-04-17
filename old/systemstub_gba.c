/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#include "gba_dma.h"
#include "gba_input.h"
#include "gba_sound.h"
#include "gba_video.h"
#include "gba_timers.h"
#include "gba_sprites.h"
#include "gba_interrupt.h"
#include "gba_systemcalls.h"
#include "../source/mixer.h"
#include "../source/player_mod.h"
#include "../source/systemstub.h"

#define SOUND_SAMPLE_RATE 13379
#define SOUND_BUFFER_SIZE 224
#define HITMODE_PAL_NUM 15

typedef struct {
	const uint8 *spr_data;
	int spr_size;
	int tiles_count;
	uint16 attr[3];
} oam_entry_t;

int sys_exit_flag;
static int palette_changed_flag;
static int key_mask, inp_mask_cur, inp_mask_prev;
static sys_screen_mode_e current_mode;
static int screen_w, screen_h;
static uint16 screen_palette[256] GCC_EXT;
static uint16 copper_palette[161];
static const uint8 *tilemap_data, *tilemap_lut;
static int tilemap_w, tilemap_h;
static int tilemap_origin_x, tilemap_origin_y;
static int sprites_count, sprites_count_prev;
static int obj_tiles_size;
static const uint8 *tile_data;
static int tile_data_changed_flag;
static oam_entry_t oam_table[96];
static const uint8 *copper_pal_data;
static int copper_pal_changed;
static int sound_current_buffer;
static int8 sound_snd_buffer[512 * 2] GCC_ALIGN(4);

static const uint8 gamma_lut[32] = { /* pow(i / 31., .8) * 31. */
	0x00, 0x01, 0x03, 0x04, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 0x11,
	0x12, 0x13, 0x14, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F
};

static void dma3_copy4(void *dst, const void *src, int len) {
	const uint32 flags = DMA_DST_INC | DMA_SRC_INC | DMA32 | DMA_IMMEDIATE;
	assert((len & 3) == 0);
	DMA3COPY(src, dst, flags | (len >> 2));
}

static void dma3_set4(void *dst, uint32 c, int len) {
	const uint32 flags = DMA_DST_INC | DMA_SRC_FIXED | DMA32 | DMA_IMMEDIATE;
	assert((len & 3) == 0);
	DMA3COPY(&c, dst, flags | (len >> 2));
}

static void sound_init() {
	REG_SOUNDCNT_H = SNDA_VOL_100 | SNDA_R_ENABLE | SNDA_L_ENABLE | (0 << 10) | SNDA_RESET_FIFO; /* SOUND_TIMER_0 */
	REG_SOUNDCNT_X = (1 << 7); /* SOUND_ENABLE */

	memset(sound_snd_buffer, 0, sizeof(sound_snd_buffer));
	mixer_init(SOUND_SAMPLE_RATE);
	mpl_init(SOUND_SAMPLE_RATE);
	sound_current_buffer = 1;

	REG_DMA1SAD = (uint32)sound_snd_buffer;
	REG_DMA1DAD = (uint32)&REG_FIFO_A;
	REG_DMA1CNT = 0;

	REG_TM0CNT_L = 65536 - (16777216 / SOUND_SAMPLE_RATE);
	REG_TM0CNT_H = TIMER_START;
}

static GCC_INT_CALL void sound_vsync() {
	int8 *dst = 0;
	if (sound_current_buffer == 1) {
		REG_DMA1CNT = 0;
		REG_DMA1SAD = (uint32)sound_snd_buffer;
		REG_DMA1CNT = DMA_DST_FIXED | DMA_SRC_INC | DMA_REPEAT | DMA_SPECIAL | DMA32 | DMA_ENABLE;
		dst = (int8 *)&sound_snd_buffer[SOUND_BUFFER_SIZE];
		sound_current_buffer = 0;
	} else {
		dst = (int8 *)&sound_snd_buffer[0];
		sound_current_buffer = 1;
	}
	dma3_set4(dst, 0, SOUND_BUFFER_SIZE);
	mixer_mix_samples(dst, SOUND_BUFFER_SIZE);
	mpl_play_callback(dst, SOUND_BUFFER_SIZE);
}

static void vblank_interrupt() {
	/* reset the DMA */
	REG_DMA0CNT = 0;
	if (current_mode == SSM_TILED) {
		DMA0COPY(&copper_palette[1], &BG_COLORS[7], DMA_DST_FIXED | DMA_SRC_INC | DMA_REPEAT | DMA16 | DMA_HBLANK | 1);
		/* setup first color, hblank occurs at the end of line */
		BG_COLORS[7] = copper_palette[0];
	}
	/* update sound mixer */
	sound_vsync();
}

void sys_init(int w, int h, const char *title) {
	/* reset some vars */
	screen_w = w;
	screen_h = h;
	key_mask = 0;
	inp_mask_cur = inp_mask_prev = 0;
	memset(screen_palette, 0, sizeof(screen_palette));
	memset(copper_palette, 0, sizeof(copper_palette));
	sprites_count_prev = 0;
	dma3_set4(oam_table, 0, sizeof(oam_table));
	/* 'white' blending ; set attr[0] |= OBJ_MODE(1) when !HITMODE */
/*	REG_BLDCNT = (1 << 8) | (2 << 6) | (1 << 4);
	REG_BLDALPHA = (16 << 0);
	REG_BLDY = (8 << 0);*/
	/* 'black' blending ; set attr[0] |= OBJ_MODE(1) when HITMODE */
/*	REG_BLDCNT = (1 << 8) | (1 << 6);
	REG_BLDALPHA = (8 << 0);*/
	/* initialize sound */
	sound_init();
	/* setup vblank interrupt handler */
	InitInterrupt();
	EnableInterrupt(IE_VBL);
	SetInterrupt(IE_VBL, vblank_interrupt);
}

void sys_destroy() {
}

static uint16 convert_amiga_color(uint16 color) {
	uint8 r, g, b;
	r = (color >> 8) & 0xF;
	g = (color >> 4) & 0xF;
	b = (color >> 0) & 0xF;
	r = gamma_lut[(r << 1) | (r & 1)];
	g = gamma_lut[(g << 1) | (g & 1)];
	b = gamma_lut[(b << 1) | (b & 1)];
	return RGB5(r, g, b);
}

static void convert_copper_pal(const uint8 *pal_data) {
	int i;
	for (i = 0; i < 160; ++i) {
		copper_palette[i] = convert_amiga_color(read_uint16BE(pal_data));
		pal_data += 2;
	}
}

void sys_set_screen_mode(sys_screen_mode_e mode) {
	int i;
	current_mode = mode;
	switch (current_mode) {
	case SSM_BITMAP:
		REG_DISPCNT = MODE_4 | BG2_ON;
		break;
	case SSM_TILED:
		REG_BG0CNT = BG_16_COLOR | SCREEN_BASE(31) | BG_SIZE_0;
		REG_DISPCNT = MODE_0 | OBJ_1D_MAP | BG0_ON | OBJ_ON;
		for (i = 0; i < 16; ++i) {
			OBJ_COLORS[HITMODE_PAL_NUM * 16 + i] = RGB5(31, 31, 31);
		}
		break;
	}
}

void sys_set_palette_bmp(const uint8 *pal_data, int num_colors) {
	int i;
	for (i = 0; i < num_colors; ++i) {
		uint8 r, g, b;
		b = gamma_lut[pal_data[0] >> 3];
		g = gamma_lut[pal_data[1] >> 3];
		r = gamma_lut[pal_data[2] >> 3];
		pal_data += 4;
		screen_palette[i] = RGB5(r, g, b);
	}
}

void sys_set_palette_spr(const uint8 *pal_data, int num_colors, int pal_num) {
	int i;
	const int offset = pal_num * 16;
	for (i = 0; i < num_colors; ++i) {
		const uint16 color = convert_amiga_color(read_uint16BE(pal_data));
		pal_data += 2;
		OBJ_COLORS[offset + i] = screen_palette[offset + i] = color;
	}
	palette_changed_flag = 1;
}

void sys_fade_in_palette() {
	int step, i;
	uint16 fade_palette[256];

	memcpy(fade_palette, screen_palette, 256 * sizeof(uint16));
	for (step = 0; step <= 64; ++step) {
		for (i = 0; i < 256; ++i) {
			uint16 color = fade_palette[i];
			uint8 r = color & 0x1F;
			uint8 g = (color >> 5) & 0x1F;
			uint8 b = (color >> 10) & 0x1F;
			r = r * step >> 6;
			g = g * step >> 6;
			b = b * step >> 6;
			screen_palette[i] = RGB5(r, g, b);
		}
		VBlankIntrWait();
		dma3_copy4(BG_COLORS, screen_palette, 256 * sizeof(uint16));
	}
}

void sys_fade_out_palette() {
	int step, i;

	for (step = 64; step >= 0; --step) {
		for (i = 0; i < 256; ++i) {
			uint16 color = screen_palette[i];
			uint8 r = color & 0x1F;
			uint8 g = (color >> 5) & 0x1F;
			uint8 b = (color >> 10) & 0x1F;
			r = r * step >> 6;
			g = g * step >> 6;
			b = b * step >> 6;
			screen_palette[i] = RGB5(r, g, b);
		}
		VBlankIntrWait();
		dma3_copy4(BG_COLORS, screen_palette, 256 * sizeof(uint16));
	}
}

void sys_blit_bitmap(const uint8 *bmp_data, int w, int h) {
	uint16 *dst = (uint16 *)VRAM;
	bmp_data += (h - 1) * w;
	assert(w <= screen_w && h <= screen_h);
	while (h--) {
		int i;
		for (i = 0; i < screen_w; i += 2) {
			dst[i >> 1] = (bmp_data[i + 1] << 8) | bmp_data[i];
		}
		dst += screen_w >> 1;
		bmp_data -= w;
	}
}

static void update_tilemap(int x, int y) {
	uint16 *dstmap = (uint16 *)MAP_BASE_ADR(31);
	const uint8 *srcmap;
	int i, j;

	BG0HOFS = x & 0xF;
	BG0VOFS = y & 0xF;
	x >>= 4;
	y >>= 4;

	srcmap = &tilemap_data[x * tilemap_h + y];
	for (j = 0; j < 16; ++j) {
		for (i = 0; i < 16 * tilemap_h; i += tilemap_h) {
			uint16 tile_num = tilemap_lut[srcmap[i]] << 2;
			*(uint32 *)(dstmap + ( j      << 5)) =  tile_num      | ((tile_num + 1) << 16);
			*(uint32 *)(dstmap + ((j + 1) << 5)) = (tile_num + 2) | ((tile_num + 3) << 16);
			dstmap += 2;
		}
		++srcmap;
	}
}

static void sync_display() {
	uint8 *dst;
	int i, j, tiles_count;

	/* update palette */
	if (palette_changed_flag) {
		dma3_copy4(BG_COLORS, screen_palette, 16 * sizeof(uint16));
		palette_changed_flag = 0;
	}

	/* update copper data */
	if (copper_pal_changed) {
		convert_copper_pal(copper_pal_data);
		copper_pal_changed = 0;
	}

	/* update tilemap data */
	if (tile_data_changed_flag) {
		dma3_copy4(CHAR_BASE_ADR(0), tile_data + 4, 0x8000);
		tile_data_changed_flag = 0;
	}

	/* update tilemap */
	update_tilemap(tilemap_origin_x, tilemap_origin_y);

	/* setup sprites, in reverse order */
	tiles_count = 0;
	dst = OBJ_BASE_ADR;
	i = 0;
	j = sprites_count - 1;
	while (i < sprites_count) {
		const oam_entry_t *oe = &oam_table[j];
		OAM[i].attr0 = oe->attr[0];
		OAM[i].attr1 = oe->attr[1];
		OAM[i].attr2 = oe->attr[2] | OBJ_CHAR(tiles_count);
		dma3_copy4(dst, oe->spr_data, oe->spr_size);
		tiles_count += oe->tiles_count;
		dst += oe->tiles_count * 32;
		++i;
		--j;
	}

	/* hide previously setup sprites */
	for (i = sprites_count; i < sprites_count_prev; ++i) {
		OAM[i].attr0 = OBJ_X(240);
		OAM[i].attr1 = OBJ_Y(160);
	}

	sprites_count_prev = sprites_count;
}

void sys_update_screen() {
	VBlankIntrWait();
	sync_display();
	VBlankIntrWait();
}

void sys_set_tile_data(const uint8 *data) {
	tile_data = data;
	tile_data_changed_flag = 1;
}

void sys_set_tilemap_data(const uint8 *data, int w, int h, const uint8 *anim_lut) {
	tilemap_data = data;
	tilemap_lut = anim_lut;
	tilemap_origin_x = 0;
	tilemap_origin_y = 0;
	tilemap_w = w;
	tilemap_h = h;
}

void sys_set_tilemap_origin(int x, int y) {
	tilemap_origin_x = x;
	tilemap_origin_y = y;
}

void sys_set_copper_pal(const uint8 *pal_data, int num_lines) {
	copper_pal_data = pal_data;
	copper_pal_changed = 1;
}

void sys_get_sprite_dim(const uint8 *data, int num, int *w, int *h) {
	const int offs = read_uint16LE(data + 4 + 4 * num);
	data += 4 + offs;
	*w = *(uint16 *)(data);
	*h = *(uint16 *)(data + 2);
}

void sys_clear_sprite_list() {
	sprites_count = 0;
	obj_tiles_size = 0;
}

void sys_add_to_sprite_list(const uint8 *data, int num, int x, int y, int flags, int pal_num) {
	const uint8 *spr_data = data + 4;
	uint32 offs = *(uint32 *)(spr_data + 4 * num);
	if (offs != 0) {
		int obj_x, obj_y;
		uint8 j, i, num_spr_h, num_spr_w;

		spr_data += offs;
		num_spr_h = spr_data[5];
		num_spr_w = spr_data[6];
		spr_data += 8;

		if (flags & SSF_HITMODE) {
			pal_num = HITMODE_PAL_NUM;
		}

		for (j = 0, obj_y = y; j < num_spr_h; ++j, obj_y += 32) {
			for (i = 0, obj_x = x; i < num_spr_w; ++i, obj_x += 32) {
				uint16 tile_h, tile_w;

				tile_h = *(uint16 *)spr_data; spr_data += 2;
				tile_w = *(uint16 *)spr_data; spr_data += 2;
				if (obj_x + tile_w < 0 || obj_x >= screen_w || obj_y + tile_h < 0 || obj_y >= screen_h) {
					spr_data += 2;
					spr_data += *(uint16 *)spr_data + 2;
				} else {
					uint8 gba_mode;
					oam_entry_t *oe;
					int cur_y, cur_x;

					if (sprites_count >= ARRAYSIZE(oam_table)) {
						print_warning("sys_add_to_sprite_list() oam_table overflow");
						break;
					}
					if (obj_tiles_size >= 32 * 1024) {
						print_warning("sys_add_to_sprite_list() obj_tiles_size %d >= 32ko", obj_tiles_size);
						break;
					}
					cur_x = obj_x;
					if (cur_x < 0) {
						cur_x = 512 + cur_x;
					}
					cur_y = obj_y;
					if (cur_y < 0) {
						cur_y = 256 + cur_y;
					}

					oe = &oam_table[sprites_count];
					++sprites_count;

					gba_mode = *spr_data++;
					oe->tiles_count = *spr_data++;
					oe->spr_size = *(uint16 *)spr_data; spr_data += 2;
					oe->spr_data = spr_data;
					oe->attr[0] = OBJ_Y(cur_y) | OBJ_16_COLOR | OBJ_SHAPE((gba_mode >> 2));
					oe->attr[1] = OBJ_X(cur_x) | OBJ_SIZE((gba_mode & 3));
					oe->attr[2] = OBJ_PRIORITY(0) | OBJ_PALETTE(pal_num);
					spr_data += oe->spr_size;
					obj_tiles_size += oe->spr_size;
				}
			}
		}
	}
}

static void update_key_mask() {
	static const struct {
		int gba_key_mask;
		int sys_key_mask;
	} key_mask_table[] = {
		{ KEY_LEFT, SKM_LEFT },
		{ KEY_RIGHT, SKM_RIGHT },
		{ KEY_UP, SKM_UP },
		{ KEY_DOWN, SKM_DOWN },
		{ KEY_START, SKM_PAUSE },
		{ KEY_SELECT, SKM_INVENTORY },
		{ KEY_A | KEY_B, SKM_ACTION },
		{ KEY_L, SKM_F1 },
		{ KEY_R, SKM_F2 }
	};
	int i;
	inp_mask_prev = inp_mask_cur;
	inp_mask_cur = ~REG_KEYINPUT;
	for (i = 0; i < ARRAYSIZE(key_mask_table); ++i) {
		const int gba_key_mask = key_mask_table[i].gba_key_mask;
		const int sys_key_mask = key_mask_table[i].sys_key_mask;
		if ((inp_mask_cur ^ inp_mask_prev) & gba_key_mask) {
			if (inp_mask_cur & gba_key_mask) {
				key_mask |= sys_key_mask;
			} else {
				key_mask &= ~sys_key_mask;
			}
		}
	}
}

void sys_process_events() {
	update_key_mask();
}

int sys_wait_for_keys(int timeout, int mask) {
	int i;
	const int vbl_wait_count = 2 * timeout * 60 / 1000;

	for (i = 0; i < vbl_wait_count; ++i) {
		VBlankIntrWait();
		update_key_mask();
		if ((key_mask & mask) != 0) {
			break;
		}
	}
	return key_mask;
}

int *sys_get_key_mask() {
	return &key_mask;
}

void sys_play_sfx(const int8 *sample_data, uint32 sample_size, uint32 sample_period) {
	mixer_play_sfx(sample_data, sample_size, sample_period);
}

void sys_play_module(const uint8 *module_data, const uint8 *instrument_data) {
	mpl_load(module_data, instrument_data);
}

void sys_stop_module() {
	mpl_stop();
}

void sys_print_string(const char *str) {
#ifdef NICKY_DEBUG
	__asm volatile(
		"mov r0, %0;"
		"swi 0xFF;"
			: /* no output */
			: "r" (str)
			: "r0"
	);
#endif
}
