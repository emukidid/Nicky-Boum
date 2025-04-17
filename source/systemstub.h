/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#ifndef __SYSTEMSTUB_H__
#define __SYSTEMSTUB_H__

#include "util.h"

#define SKM_UP        (1 <<  0)
#define SKM_DOWN      (1 <<  1)
#define SKM_RIGHT     (1 <<  2)
#define SKM_LEFT      (1 <<  3)
#define SKM_ACTION    (1 <<  4)
#define SKM_QUIT      (1 <<  5)
#define SKM_PAUSE     (1 <<  6)
#define SKM_INVENTORY (1 <<  7)
#define SKM_F1        (1 <<  8)
#define SKM_F2        (1 <<  9)
#define SKM_F3        (1 << 10)

#define SSF_VISIBLE  0x0000
#define SSF_HITMODE  0x8000
#define SSF_INVALID  0xFFFF

typedef enum {
	SSM_BITMAP,
	SSM_TILED
} sys_screen_mode_e;

extern int sys_exit_flag;

extern void sys_init(int w, int h, const char *title);
extern void sys_destroy();
extern void sys_set_screen_mode(sys_screen_mode_e mode);

extern void sys_set_palette_bmp(const uint8 *pal_data, int num_colors);
extern void sys_set_palette_spr(const uint8 *pal_data, int num_colors, int pal_num);
extern void sys_fade_in_palette();
extern void sys_fade_out_palette();

extern void sys_blit_bitmap(const uint8 *bmp_data, int w, int h);
extern void sys_update_screen();

extern void sys_set_tile_data(const uint8 *tile_data);
extern void sys_set_tilemap_data(const uint8 *map_data, int w, int h, const uint8 *anim_lut);
extern void sys_set_tilemap_origin(int x, int y);
extern void sys_set_copper_pal(const uint8 *pal_data, int num_lines);

extern void sys_get_sprite_dim(const uint8 *data, int num, int *w, int *h);
extern void sys_clear_sprite_list();
extern void sys_add_to_sprite_list(const uint8 *data, int num, int x, int y, int flags, int pal_num);

extern void sys_process_events();
extern int sys_wait_for_keys(int timeout, int mask);
extern int *sys_get_key_mask();

extern void sys_play_sfx(const int8 *sample_data, uint32 sample_size, uint32 sample_period);
extern void sys_play_module(const uint8 *module_data, const uint8 *instrument_data);
extern void sys_stop_module();

extern void sys_print_string(const char *str);

#endif /* __SYSTEMSTUB_H__ */
