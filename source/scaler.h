/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#ifndef __SCALER_H__
#define __SCALER_H__
 
#include "util.h"

typedef void (*scaler_pf)(uint16 *dst, int dst_pitch, const uint16 *src, int src_pitch, int w, int h);

extern void point1x(uint16 *dst, int dst_pitch, const uint16 *src, int src_pitch, int w, int h);
extern void point2x(uint16 *dst, int dst_pitch, const uint16 *src, int src_pitch, int w, int h);
extern void point3x(uint16 *dst, int dst_pitch, const uint16 *src, int src_pitch, int w, int h);
extern void point4x(uint16 *dst, int dst_pitch, const uint16 *src, int src_pitch, int w, int h);
extern void scale2x(uint16 *dst, int dst_pitch, const uint16 *src, int src_pitch, int w, int h);
extern void scale3x(uint16 *dst, int dst_pitch, const uint16 *src, int src_pitch, int w, int h);
extern void scale4x(uint16 *dst, int dst_pitch, const uint16 *src, int src_pitch, int w, int h);

#endif /* __SCALER_H__ */
