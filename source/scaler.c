/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#include "scaler.h"

void point1x(uint16 *dst, int dst_pitch, const uint16 *src, int src_pitch, int w, int h) {
	while (h--) {
		memcpy(dst, src, w * 2);
		dst += dst_pitch;
		src += src_pitch;
	}
}

void point2x(uint16 *dst, int dst_pitch, const uint16 *src, int src_pitch, int w, int h) {
	while (h--) {
		int i;
		uint16 *p = dst;
		for (i = 0; i < w; ++i, p += 2) {
			uint16 c = *(src + i);
			*(p) = c;
			*(p + 1) = c;
			*(p + dst_pitch) = c;
			*(p + dst_pitch + 1) = c;
		}
		dst += dst_pitch * 2;
		src += src_pitch;
	}
}

void point3x(uint16 *dst, int dst_pitch, const uint16 *src, int src_pitch, int w, int h) {	
	while (h--) {
		int i;
		uint16 *p = dst;
		for (i = 0; i < w; ++i, p += 3) {
			uint16 c = *(src + i);
			*(p) = c;
			*(p + 1) = c;
			*(p + 2) = c;
			*(p + dst_pitch) = c;
			*(p + dst_pitch + 1) = c;
			*(p + dst_pitch + 2) = c;
			*(p + 2 * dst_pitch) = c;
			*(p + 2 * dst_pitch + 1) = c;
			*(p + 2 * dst_pitch + 2) = c;
		}
		dst += dst_pitch * 3;
		src += src_pitch;
	}
}

void point4x(uint16 *dst, int dst_pitch, const uint16 *src, int src_pitch, int w, int h) {	
	while (h--) {
		int i;
		uint16 *p = dst;
		for (i = 0; i < w; ++i, p += 4) {
			uint16 c = *(src + i);
			*(p) = c;
			*(p + 1) = c;
			*(p + 2) = c;
			*(p + 3) = c;
			*(p + dst_pitch) = c;
			*(p + dst_pitch + 1) = c;
			*(p + dst_pitch + 2) = c;
			*(p + dst_pitch + 3) = c;
			*(p + 2 * dst_pitch) = c;
			*(p + 2 * dst_pitch + 1) = c;
			*(p + 2 * dst_pitch + 2) = c;
			*(p + 2 * dst_pitch + 3) = c;
			*(p + 3 * dst_pitch) = c;
			*(p + 3 * dst_pitch + 1) = c;
			*(p + 3 * dst_pitch + 2) = c;
			*(p + 3 * dst_pitch + 3) = c;
		}
		dst += dst_pitch * 4;
		src += src_pitch;
	}
}

void scale2x(uint16 *dst, int dst_pitch, const uint16 *src, int src_pitch, int w, int h) {
	while (h--) {
		int i;
		uint16 *p = dst;
		for (i = 0; i < w; ++i, p += 2) {
			uint16 B = *(src + i - src_pitch);
			uint16 D = *(src + i - 1);
			uint16 E = *(src + i);
			uint16 F = *(src + i + 1);
			uint16 H = *(src + i + src_pitch);
			if (B != H && D != F) {
				*(p) = D == B ? D : E;
				*(p + 1) = B == F ? F : E;
				*(p + dst_pitch) = D == H ? D : E;
				*(p + dst_pitch + 1) = H == F ? F : E;
			} else {
				*(p) = E;
				*(p + 1) = E;
				*(p + dst_pitch) = E;
				*(p + dst_pitch + 1) = E;
			}
		}
		dst += dst_pitch * 2;
		src += src_pitch;
	}
}

void scale3x(uint16 *dst, int dst_pitch, const uint16 *src, int src_pitch, int w, int h) {
	while (h--) {
		int i;
		uint16 *p = dst;
		for (i = 0; i < w; ++i, p += 3) {
			uint16 A = *(src + i - src_pitch - 1);
			uint16 B = *(src + i - src_pitch);
			uint16 C = *(src + i - src_pitch + 1);
			uint16 D = *(src + i - 1);
			uint16 E = *(src + i);
			uint16 F = *(src + i + 1);
			uint16 G = *(src + i + src_pitch - 1);
			uint16 H = *(src + i + src_pitch);
			uint16 I = *(src + i + src_pitch + 1);
			if (B != H && D != F) {
				*(p) = D == B ? D : E;
				*(p + 1) = (D == B && E != C) || (B == F && E != A) ? B : E;
				*(p + 2) = B == F ? F : E;
				*(p + dst_pitch) = (D == B && E != G) || (D == B && E != A) ? D : E;
				*(p + dst_pitch + 1) = E;
				*(p + dst_pitch + 2) = (B == F && E != I) || (H == F && E != C) ? F : E;
				*(p + 2 * dst_pitch) = D == H ? D : E;
				*(p + 2 * dst_pitch + 1) = (D == H && E != I) || (H == F && E != G) ? H : E;
				*(p + 2 * dst_pitch + 2) = H == F ? F : E;
			} else {
				*(p) = E;
				*(p + 1) = E;
				*(p + 2) = E;
				*(p + dst_pitch) = E;
				*(p + dst_pitch + 1) = E;
				*(p + dst_pitch + 2) = E;
				*(p + 2 * dst_pitch) = E;
				*(p + 2 * dst_pitch + 1) = E;
				*(p + 2 * dst_pitch + 2) = E;
			}
		}
		dst += dst_pitch * 3;
		src += src_pitch;
	}
}

void scale4x(uint16 *dst, int dst_pitch, const uint16 *src, int src_pitch, int w, int h) {
	static uint16 *ibuf_ptr = 0;
	static int ibuf_w = 0;
	static int ibuf_h = 0;
	static int ibuf_pitch = 0;
	static int ibuf_size = 0;
	
	int buf_size = (w * 2 + 2) * (h * 2 + 2) * sizeof(uint16);
	if (ibuf_size < buf_size) {
		free(ibuf_ptr);
		ibuf_size = buf_size;
		ibuf_w = w * 2;
		ibuf_h = h * 2;
		ibuf_pitch = ibuf_w + 1;
		ibuf_ptr = (uint16 *)malloc(ibuf_size);
		if (!ibuf_ptr) {
			print_error("Unable to allocate scale4x intermediate buffer");
		}
	}
	scale2x(ibuf_ptr + ibuf_pitch + 1, ibuf_pitch, src, src_pitch, w, h);
	scale2x(dst, dst_pitch, ibuf_ptr + ibuf_pitch + 1, ibuf_pitch, w * 2, h * 2);
}
