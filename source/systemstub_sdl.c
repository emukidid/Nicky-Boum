/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#include <SDL/SDL.h>
#ifdef USE_GL
#include <SDL/SDL_opengl.h>
#endif

#include "mixer.h"
#include "scaler.h"
#include "player_mod.h"
#include "systemstub.h"

#define OSB_COPPER  0
#define OSB_TILES   1
#define OSB_SPRITES 2

#define DEFAULT_SCALER 2

#define TILES_SURFACE_W 512
#define TILES_SURFACE_H 256

#define MAX_SPRITES 128

//#define SOUND_SAMPLE_RATE 22050
#define SOUND_SAMPLE_RATE 44100
#define SOUND_BUFFER_SIZE 2048

#define TILE_W 16
#define TILE_H 16
#define MAX_TILES 256

typedef struct {
	const uint8 *data;
	int num;
	int x, y;
	int flags;
	int pal_num;
} sprite_t;

typedef struct {
	const char *name;
	scaler_pf proc;
	int factor;
} scaler_desc_t;

typedef struct {
	uint8 *ptr;
	int size;
} offscreen_buffer_t;

int sys_exit_flag;

static uint16 screen_palette[256];
static SDL_PixelFormat *screen_fmt;
static SDL_Surface *screen_surface, *offscreen_surface;
static offscreen_buffer_t offscreen_buffers_table[3];
static int tilemap_origin_x, tilemap_origin_y;
static int prev_tilemap_origin_x, prev_tilemap_origin_y;
static int key_mask;
static uint8 *tiles_buffer;
static const uint8 *tilemap_lut, *tilemap_data;
static int tilemap_w, tilemap_h;
static int screen_w, screen_h;
static sys_screen_mode_e current_mode;
static int scaler_num;
static int fullscreen_flag;
static int pause_flag;
static uint32 last_frame_time_stamp;
static sprite_t sprite_list[MAX_SPRITES];
static sprite_t *sprite_list_tail;
#ifdef USE_GL
static GLuint offscreen_texture;
static uint8 *texture_data;
static uint8 texture_palette[256 * 3];
#else
static const scaler_desc_t scaler_table[] = {
	{ "point1x", point1x, 1 },
	{ "point2x", point2x, 2 },
	{ "scale2x", scale2x, 2 },
	{ "point3x", point3x, 3 },
	{ "scale3x", scale3x, 3 },
	{ "point4x", point4x, 4 },
	{ "scale4x", scale4x, 4 }
};
#endif

static void sound_callback(void *param, uint8 *buf, int len) {
	if (!pause_flag) {
		memset(buf, 0, len);
		mixer_mix_samples((int8 *)buf, len);
		mpl_play_callback((int8 *)buf, len);
	}
}

#ifdef USE_GL
static void sys_resize(int w, int h) {
	glViewport(0, 0, w, h);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, screen_w, 0, screen_h, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
}
#endif

static void setup_screen_surface(int fullscreen, int scaler) {
#ifdef USE_GL
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	screen_surface = SDL_SetVideoMode(screen_w, screen_h, 0, SDL_OPENGL | SDL_RESIZABLE);
	if (!screen_surface) {
		print_error("Unable to allocate screen_surface");
	}
	//const char *extensions = (const char *)glGetString(GL_EXTENSIONS);
	//printf(extensions);
	glGenTextures(1, &offscreen_texture);
	glBindTexture(GL_TEXTURE_2D, offscreen_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	texture_data = (uint8 *)malloc(screen_w * screen_h * 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
	if (!texture_data) {
		print_error("Unable to allocate texture_data");
	}
	sys_resize(screen_w, screen_h);
#else
	int w, h;
	const scaler_desc_t *sd;

	scaler_num = scaler;
	fullscreen_flag = fullscreen;
	sd = &scaler_table[scaler_num];
	w = screen_w * sd->factor;
	h = screen_h * sd->factor;
	//screen_surface = SDL_SetVideoMode(w, h, 16, fullscreen_flag ? (SDL_FULLSCREEN | SDL_HWSURFACE) : SDL_HWSURFACE);
	screen_surface = SDL_SetVideoMode(w, h, 16, SDL_FULLSCREEN );
	if (!screen_surface) {
		print_error("!Unable to allocate screen_surface");
	}
	screen_fmt = screen_surface->format;
	offscreen_surface = SDL_CreateRGBSurface(SDL_SWSURFACE, screen_w + 2, screen_h + 2, 16, screen_fmt->Rmask, screen_fmt->Gmask, screen_fmt->Bmask, screen_fmt->Amask);
	if (!offscreen_surface) {
		print_error("Unable to allocate offscreen_surface");
	}
#endif
}

void sys_init(int w, int h, const char *title) {
	int i;
	SDL_AudioSpec as;

	SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK);
	SDL_ShowCursor(SDL_DISABLE);
	SDL_WM_SetCaption(title, NULL);
	if (SDL_NumJoysticks() > 0) SDL_JoystickOpen(0);

	key_mask = 0;
	sys_exit_flag = 0;

	assert(w < TILES_SURFACE_W && h < TILES_SURFACE_H);
	screen_w = w;
	screen_h = h;
	current_mode = SSM_BITMAP;
	memset(screen_palette, 0, sizeof(screen_palette));
#ifdef USE_GL
	memset(texture_palette, 0, sizeof(texture_palette));
#endif

	/* initialize screen surface */
	fullscreen_flag = 1;
	setup_screen_surface(fullscreen_flag, DEFAULT_SCALER);

	/* initialize offscreen buffers */
	memset(offscreen_buffers_table, 0, sizeof(offscreen_buffers_table));
	offscreen_buffers_table[OSB_COPPER].size = screen_w * screen_h;
	offscreen_buffers_table[OSB_TILES].size = TILES_SURFACE_W * TILES_SURFACE_H;
	offscreen_buffers_table[OSB_SPRITES].size = screen_w * screen_h;
	for (i = 0; i < ARRAYSIZE(offscreen_buffers_table); ++i) {
		offscreen_buffer_t *ob = &offscreen_buffers_table[i];
		ob->ptr = (uint8 *)malloc(ob->size);
		if (!ob->ptr) {
			print_error("Unable to allocate %d bytes", ob->size);
		}
	}

	tiles_buffer = (uint8 *)malloc(MAX_TILES * TILE_H * TILE_W);
	if (!tiles_buffer) {
		print_error("Unable to allocate tiles buffer");
	}

	/* initialize sound */
	memset(&as, 0, sizeof(as));
	as.freq = SOUND_SAMPLE_RATE;
	as.format = AUDIO_S8;
	//as.format = AUDIO_S16;
	as.channels = 1;
	as.samples = SOUND_BUFFER_SIZE;
	as.callback = sound_callback;
	if (SDL_OpenAudio(&as, NULL) == 0) {
		mixer_init(SOUND_SAMPLE_RATE);
		mpl_init(SOUND_SAMPLE_RATE);
		SDL_PauseAudio(0);
	} else {
		print_warning("Unable to open sound device");
	}

	last_frame_time_stamp = SDL_GetTicks();
	pause_flag = 0;
}

void sys_destroy() {
	int i;

	SDL_CloseAudio();
	for (i = 0; i < ARRAYSIZE(offscreen_buffers_table); ++i) {
		free(offscreen_buffers_table[i].ptr);
		offscreen_buffers_table[i].ptr = 0;
	}
	free(tiles_buffer);
#ifdef USE_GL
	glDeleteTextures(1, &offscreen_texture);
	free(texture_data);
	texture_data = 0;
#else
	SDL_FreeSurface(offscreen_surface);
#endif
	SDL_Quit();
}

void sys_set_screen_mode(sys_screen_mode_e mode) {
	current_mode = mode;
	if (mode == SSM_TILED) {
		screen_palette[255] = SDL_MapRGB(screen_fmt, 0xFF, 0xFF, 0xFF);
	}
}

void sys_set_palette_bmp(const uint8 *pal_data, int num_colors) {
	int i;
	for (i = 0; i < num_colors; ++i) {
		uint8 r, g, b;
		b = pal_data[0];
		g = pal_data[1];
		r = pal_data[2];
		pal_data += 4;
#ifdef USE_GL
		texture_palette[i * 3] = r;
		texture_palette[i * 3 + 1] = g;
		texture_palette[i * 3 + 2] = b;
#endif
		screen_palette[i] = SDL_MapRGB(screen_fmt, r, g, b);
	}
}

#ifdef USE_GL
static void convert_amiga_color(uint8 *dst, uint16 color) {
	dst[0] = (color >> 8) & 0xF;
	dst[1] = (color >> 4) & 0xF;
	dst[2] = (color >> 0) & 0xF;
	dst[0] |= dst[0] << 4;
	dst[1] |= dst[1] << 4;
	dst[2] |= dst[2] << 4;
}
#else
static uint16 convert_amiga_color(SDL_PixelFormat *fmt, uint16 color) {
	uint8 r, g, b;
	r = (color >> 8) & 0xF;
	g = (color >> 4) & 0xF;
	b = (color >> 0) & 0xF;
	r |= r << 4;
	g |= g << 4;
	b |= b << 4;
	return SDL_MapRGB(fmt, r, g, b);
}
#endif

void sys_set_palette_spr(const uint8 *pal_data, int num_colors, int pal_num) {
	int i;
	for (i = 0; i < num_colors; ++i) {
		uint16 color = read_uint16BE(pal_data); pal_data += 2;
#ifdef USE_GL
		convert_amiga_color(texture_palette + (pal_num * 16 + i) * 3, color);
#else
		screen_palette[pal_num * 16 + i] = convert_amiga_color(screen_fmt, color);
#endif
	}
}

void sys_fade_in_palette() {
#ifndef USE_GL
	int step, i;
	uint16 fade_palette[256];
	memcpy(fade_palette, screen_palette, 256 * sizeof(uint16));
	for (step = 0; step <= 16; ++step) {
		for (i = 0; i < 256; ++i) {
			uint8 r, g, b;
			SDL_GetRGB(fade_palette[i], screen_fmt, &r, &g, &b);
			r = r * step >> 4;
			g = g * step >> 4;
			b = b * step >> 4;
			screen_palette[i] = SDL_MapRGB(screen_fmt, r, g, b);
		}
		sys_update_screen();
		SDL_Delay(50);
	}
#endif
}

void sys_fade_out_palette() {
#ifndef USE_GL
	int step, i;
	for (step = 16; step >= 0; --step) {
		for (i = 0; i < 256; ++i) {
			uint8 r, g, b;
			SDL_GetRGB(screen_palette[i], screen_fmt, &r, &g, &b);
			r = r * step >> 4;
			g = g * step >> 4;
			b = b * step >> 4;
			screen_palette[i] = SDL_MapRGB(screen_fmt, r, g, b);
		}
		sys_update_screen();
		SDL_Delay(50);
	}
#endif
}

void sys_blit_bitmap(const uint8 *bmp_data, int w, int h) {
	uint8 *dst = offscreen_buffers_table[OSB_COPPER].ptr;
	bmp_data += (h - 1) * w;
	assert(w <= screen_w && h <= screen_h);
	while (h--) {
		memcpy(dst, bmp_data, w);
		dst += screen_w;
		bmp_data -= w;
	}
}

static void draw_sprite(const uint8 *spr_data, int x, int y, int w, int h, int flags, int pal_num) {
	int j, i, b, p;
	uint8 *dst = offscreen_buffers_table[OSB_SPRITES].ptr + y * screen_w;
	for (j = 0; j < h; ++j) {
		int cy = y + j;
		for (i = 0; i < (w + 7) / 8; ++i) {
			uint8 data[4];
			data[0] = *spr_data++;
			data[1] = *spr_data++;
			data[2] = *spr_data++;
			data[3] = *spr_data++;
			for (b = 0; b < 8; ++b) {
				int cx = x + i * 8 + b;
				if (cx >= 0 && cx < screen_w && cy >= 0 && cy < screen_h) {
					uint8 color = 0;
					for (p = 0; p < 4; ++p) {
						if (data[p] & (1 << (7 - b))) {
							color |= 1 << p;
						}
					}
					if (color != 0) {
						if (flags & SSF_HITMODE) {
							color = 255;
						} else {
							color += pal_num * 16;
						}
						dst[cx] = color;
					}
				}
			}
		}
		dst += screen_w;
	}
}

static void display_sprite_list() {
	int spr_w, spr_h;
	sprite_t *spr = sprite_list;
	while (spr->flags != 0xFFFF) {
		const uint8 *spr_offs = spr->data + spr->num * 2;
		const uint8 *spr_data = spr->data + read_uint16LE(spr_offs);
		spr_w = read_uint16LE(spr_data); spr_data += 2;
		spr_h = read_uint16LE(spr_data); spr_data += 2;
		if (spr->x + spr_w >= 0 && spr->x < screen_w && spr->y + spr_h >= 0 && spr->y < screen_h) {
			draw_sprite(spr_data, spr->x, spr->y, spr_w, spr_h, spr->flags, spr->pal_num);
		}
		++spr;
	}
}

static void draw_background() {
	int j, i;
	uint8 *dst = offscreen_buffers_table[OSB_SPRITES].ptr;
	const uint8 *cop = offscreen_buffers_table[OSB_COPPER].ptr;
	const uint8 *src = offscreen_buffers_table[OSB_TILES].ptr + tilemap_origin_y * TILES_SURFACE_W + tilemap_origin_x;
	for (j = 0; j < screen_h; ++j) {
		for (i = 0; i < screen_w; ++i) {
			if (src[i] != 7) {
				dst[i] = src[i];
			} else {
				dst[i] = cop[i];
			}
		}
		dst += screen_w;
		cop += screen_w;
		src += TILES_SURFACE_W;
	}
}

#ifndef USE_GL
static void scale_surface_rect(SDL_Surface *src_surface, SDL_Surface *dst_surface, int x, int y, int w, int h) {
	uint16 *dst, *src;
	int dst_pitch, src_pitch;
	const scaler_desc_t *sd;

	sd = &scaler_table[scaler_num];
	SDL_LockSurface(dst_surface);
	SDL_LockSurface(src_surface);
	dst_pitch = dst_surface->pitch >> 1;
	src_pitch = src_surface->pitch >> 1;
	dst = (uint16 *)dst_surface->pixels + y * sd->factor * dst_pitch + x * sd->factor;
	src = (uint16 *)src_surface->pixels + (y + 1) * src_pitch + (x + 1);
	(*sd->proc)(dst, dst_pitch, src, src_pitch, w, h);
	SDL_UnlockSurface(src_surface);
	SDL_UnlockSurface(dst_surface);
}
#endif

static void update_screen_from_surface(const uint8 *src) {
#ifdef USE_GL
	int y, x;
	uint8 *dst;

	dst = texture_data;
	src += screen_w * screen_h;
	for (y = 0; y < screen_h; ++y) {
		src -= screen_w;
		for (x = 0; x < screen_w; ++x) {
			uint8 color = src[x];
			*dst++ = texture_palette[color * 3];
			*dst++ = texture_palette[color * 3 + 1];
			*dst++ = texture_palette[color * 3 + 2];
			*dst++ = 255;
		}
	}

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, screen_w);

	glBindTexture(GL_TEXTURE_2D, offscreen_texture);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, screen_w, screen_h, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

	glBindTexture(GL_TEXTURE_2D, offscreen_texture);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0);
		glVertex2i(0, 0);
		glTexCoord2f(1.0, 0.0);
		glVertex2i(256, 0);
		glTexCoord2f(1.0, 1.0);
		glVertex2i(256, 256);
		glTexCoord2f(0.0, 1.0);
		glVertex2i(0, 256);
	glEnd();

	SDL_GL_SwapBuffers();
#else
	int y, x;
	uint16 *dst;

	/* 'unpalettize' the game screen buffer */
	SDL_LockSurface(offscreen_surface);
	dst = (uint16 *)offscreen_surface->pixels + (offscreen_surface->pitch >> 1) + 1;
	for (y = 0; y < screen_h; ++y) {
		for (x = 0; x < screen_w; ++x) {
			dst[x] = screen_palette[src[x]];
		}
		src += screen_w;
		dst += offscreen_surface->pitch >> 1;
	}
	SDL_UnlockSurface(offscreen_surface);

	/* redraw */
	scale_surface_rect(offscreen_surface, screen_surface, 0, 0, screen_w, screen_h);
	SDL_UpdateRect(screen_surface, 0, 0, 0, 0);
#endif
}

void sys_update_screen() {
	switch (current_mode) {
	case SSM_BITMAP:
		update_screen_from_surface(offscreen_buffers_table[OSB_COPPER].ptr);
		break;
	case SSM_TILED:
		draw_background();
		display_sprite_list();
		update_screen_from_surface(offscreen_buffers_table[OSB_SPRITES].ptr);
		break;
	}
}

static void decode_bitplane_tile(uint8 *dst, const uint8 *src) {
	int y, d, b, p;
	for (y = 0; y < 16; ++y) {
		uint8 data[2][4];
		data[0][0] = *src++;
		data[1][0] = *src++;
		data[0][1] = *src++;
		data[1][1] = *src++;
		data[0][2] = *src++;
		data[1][2] = *src++;
		data[0][3] = *src++;
		data[1][3] = *src++;
		for (d = 0; d < 2; ++d) {
			for (b = 0; b < 8; ++b) {
				*dst = 0;
				for (p = 0; p < 4; ++p) {
					if (data[d][p] & (1 << (7 - b))) {
						*dst |= 1 << p;
					}
				}
				++dst;
			}
		}
	}
}

void sys_set_tile_data(const uint8 *src) {
	int i;
	uint8 *dst = tiles_buffer;

	for (i = 0; i < MAX_TILES; ++i) {
		decode_bitplane_tile(dst, src);
		src += 16 * 8;
		dst += 16 * 16;
	}
}

static void draw_tile(uint8 *dst, int pitch, uint8 tile) {
	int y;
	const uint8 *src = tiles_buffer + tile * TILE_H * TILE_W;

	for (y = 0; y < TILE_H; ++y) {
		memcpy(dst, src, TILE_W);
		src += TILE_W;
		dst += pitch;
	}
}

static void redraw_tilemap(const uint8 *tilemap, int x, int y) {
	int i, j;
	uint8 *dst = offscreen_buffers_table[OSB_TILES].ptr;
	for (j = 0; j < TILES_SURFACE_H / 16; ++j) {
		uint8 *p = dst;
		for (i = 0; i < TILES_SURFACE_W / 16; ++i) {
			uint8 tile = tilemap[(x + i) * tilemap_h + (y + j)];
			draw_tile(p, TILES_SURFACE_W, tilemap_lut[tile]);
			p += 16;
		}
		dst += TILES_SURFACE_W * 16;
	}
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
	prev_tilemap_origin_x = tilemap_origin_x;
	prev_tilemap_origin_y = tilemap_origin_y;
	tilemap_origin_x = x & 15;
	tilemap_origin_y = y & 15;
	redraw_tilemap(tilemap_data, x >> 4, y >> 4);
}

void sys_set_copper_pal(const uint8 *pal_data, int num_lines) {
	int i;
	const int offset = 240 - num_lines;
	uint8 *copper_buf = offscreen_buffers_table[OSB_COPPER].ptr;
	for (i = 0; i < num_lines; ++i) {
		uint16 color = read_uint16BE(pal_data); pal_data += 2;
#ifdef USE_GL
		convert_amiga_color(texture_palette + (offset + i) * 3, color);
#else
		screen_palette[offset + i] = convert_amiga_color(screen_fmt, color);
#endif
	}
	for (i = 0; i < num_lines; ++i) {
		memset(copper_buf, offset + i, screen_w);
		copper_buf += screen_w;
	}
	for (i = 0; i < 80; ++i) {
		memset(copper_buf, offset +num_lines-1, screen_w);
		copper_buf += screen_w;
	}
}

void sys_get_sprite_dim(const uint8 *data, int num, int *w, int *h) {
	const int offs = read_uint16LE(data + num * 2);
	*w = read_uint16LE(data + offs);
	*h = read_uint16LE(data + offs + 2);
}

void sys_clear_sprite_list() {
	sprite_list_tail = sprite_list;
	sprite_list_tail->flags = 0xFFFF;
}

void sys_add_to_sprite_list(const uint8 *data, int num, int x, int y, int flags, int pal_num) {
	if (sprite_list_tail < sprite_list + MAX_SPRITES - 1) {
		sprite_list_tail->data = data;
		sprite_list_tail->num = num;
		sprite_list_tail->x = x;
		sprite_list_tail->y = y;
		sprite_list_tail->flags = flags;
		sprite_list_tail->pal_num = pal_num;
		++sprite_list_tail;
		sprite_list_tail->flags = 0xFFFF;
	} else {
		print_warning("sys_add_to_sprite_list() sprite_list overflow");
	}
}

void do_button_held(u16 buttons_held, u16 gc_button_mask, int game_input_mask, bool analog_input) {
    if((buttons_held & gc_button_mask) || analog_input)
        key_mask |= game_input_mask;
    else key_mask &= ~game_input_mask;
}

void do_button_press(u16 buttons_down, u16 buttons_up, u16 gc_button_mask, int game_input_mask) {
    if(buttons_down & gc_button_mask)
        key_mask |= game_input_mask;
    else if(buttons_up & gc_button_mask)
        key_mask &= ~game_input_mask;
}

void update_input() {
    PAD_ScanPads();
    u16 gc_buttons = PAD_ButtonsHeld(0);
    u16 gc_buttons_down = PAD_ButtonsDown(0);
    u16 gc_buttons_up = PAD_ButtonsUp(0);

    do_button_press(gc_buttons_down, gc_buttons_up, PAD_TRIGGER_L, SKM_F1); // Cheats
    do_button_press(gc_buttons_down, gc_buttons_up, PAD_TRIGGER_R, SKM_F2);   // Level Skip
    do_button_press(gc_buttons_down, gc_buttons_up, PAD_BUTTON_X, SKM_INVENTORY);   // X - Inventory
    do_button_press(gc_buttons_down, gc_buttons_up, PAD_BUTTON_START, SKM_PAUSE);   // Start - Pause
    do_button_press(gc_buttons_down, gc_buttons_up, PAD_TRIGGER_Z, SKM_QUIT);   // Z - Quit
    do_button_held(gc_buttons, PAD_BUTTON_B, SKM_ACTION, false);   // B - Action
    do_button_held(gc_buttons, PAD_BUTTON_A|PAD_BUTTON_UP, SKM_UP, PAD_StickY(0) > 18);   // D-Pad Up/A - Jump/Up
    do_button_held(gc_buttons, PAD_BUTTON_LEFT, SKM_LEFT, PAD_StickX(0) < -18);   // D-Pad Left - Left
    do_button_held(gc_buttons, PAD_BUTTON_RIGHT, SKM_RIGHT, PAD_StickX(0) > 18);   // D-Pad Right - Right
    do_button_held(gc_buttons, PAD_BUTTON_DOWN, SKM_DOWN, PAD_StickY(0) < -18);   // D-Pad Down - Down

#ifdef HW_RVL

#endif
}

void sys_process_events() {
	SDL_Event ev;

    while (sys_exit_flag == 0) {
        update_input();
		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {

			case SDL_QUIT:
				sys_exit_flag = 1;
				break;
#ifdef USE_GL
			case SDL_VIDEORESIZE:
				sys_resize(ev.resize.w, ev.resize.h);
				break;
#endif
			case SDL_ACTIVEEVENT:
				if (ev.active.state & SDL_APPINPUTFOCUS) {
					pause_flag = !ev.active.gain;
				}
				break;
			default:
				break;
			}
		}
		if (!pause_flag && SDL_GetTicks() - last_frame_time_stamp >= 2000 / 60) {
			last_frame_time_stamp = SDL_GetTicks();
			break;
		}
		SDL_Delay(10);
	}
}

int sys_wait_for_keys(int timeout, int mask) {
	const int timestamp_end = SDL_GetTicks() + timeout;
	do {
		sys_process_events();
		if ((key_mask & mask) != 0 || sys_exit_flag != 0) {
			break;
		}
		SDL_Delay(20);
	} while (SDL_GetTicks() < timestamp_end);
	return key_mask;
}

int *sys_get_key_mask() {
	return &key_mask;
}

void sys_play_sfx(const int8 *sample_data, uint32 sample_size, uint32 sample_period) {
	SDL_LockAudio();
	mixer_play_sfx(sample_data, sample_size, sample_period);
	SDL_UnlockAudio();
}

void sys_play_module(const uint8 *module_data, const uint8 *instrument_data) {
	SDL_LockAudio();
	mpl_load(module_data, instrument_data);
	SDL_UnlockAudio();
}

void sys_stop_module() {
	SDL_LockAudio();
	mpl_stop();
	SDL_UnlockAudio();
}

void print_gecko(const char *fmt, ...) {
    char tempstr[2048];
    va_list arglist;
    va_start(arglist, fmt);
    vsprintf(tempstr, fmt, arglist);
    va_end(arglist);
    usb_sendbuffer_safe(1, tempstr, strlen(tempstr));
}

void sys_print_string(const char *str) {
    fprintf(stdout, str);
//    print_gecko(str);
}
