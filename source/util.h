/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#ifndef __UTIL_H__
#define __UTIL_H__
#include <gccore.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum {
	GAME_VER_NICKY1 = 0,
	GAME_VER_NICKY2
} game_version_e;

#ifdef NICKY_SDL_VERSION
#define GCC_PACK __attribute__((packed))
#define GCC_EXT
#define GCC_INT_CALL
#define GCC_ALIGN(x) __attribute__ ((__aligned__ (x)))
#define GCC_INLINE __inline__
#define NICKY1 (detected_game_version == GAME_VER_NICKY1)
#define NICKY2 (detected_game_version == GAME_VER_NICKY2)
/*#define CARLOS (detected_game_version == GAME_VER_CARLOS)*/
#endif

#ifdef NICKY_GBA_VERSION
#define GCC_PACK __attribute__((packed))
#define GCC_EXT __attribute__((section(".ewram")))
#define GCC_INT_CALL __attribute__ ((section(".iwram"),long_call))
#define GCC_ALIGN(x) __attribute__ ((__aligned__ (x)))
#define GCC_INLINE inline
/*#define NICKY1 */
/*#define NICKY2 */
/*#define CARLOS */
#endif

#define DBG_GAME       (1 << 0)
#define DBG_FILEIO     (1 << 1)
#define DBG_RESOURCE   (1 << 2)
#define DBG_SOUND      (1 << 3)
#define DBG_SYSTEM     (1 << 4)
#define DBG_MODPLAYER  (1 << 5)
#define DBG_INPUT      (1 << 6)

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif
#define ABS(a) (((a)<0)?(-(a)):(a))
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

typedef unsigned char uint8;
typedef signed char int8;
typedef unsigned short uint16;
//typedef int16_t s16;
//typedef int32_t int32;

//typedef signed short s16;
typedef unsigned long uint32;
//typedef signed long int32;

extern game_version_e detected_game_version;
extern int util_debug_mask;

extern void string_lower(char *p);
extern void string_upper(char *p);
extern void print_debug(int debug_channel, const char *msg, ...);
extern void print_warning(const char *msg, ...);
extern void print_error(const char *msg, ...);

static GCC_INLINE uint16 read_uint16LE(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[1] << 8) | b[0];
}

static GCC_INLINE uint32 read_uint32LE(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0];
}

static GCC_INLINE uint16 read_uint16BE(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[0] << 8) | b[1];
}

static GCC_INLINE uint32 read_uint32BE(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
}

#endif /* __UTIL_H__ */
