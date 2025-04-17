/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#include "util.h"

typedef struct {
	uint16 code;
	int carry;
	uint8 c1, c2, c3;
	const uint8 *src;
	uint8 *dst;
} decrunch_ctx_t;

static void rcr(uint16 *reg, int *carry) {
	int c = (*reg & 1) == 1;
	*reg >>= 1;
	if (*carry) {
		*reg |= 0x8000;
	}
	*carry = c;
}

static void rcl(uint16 *reg, int *carry) {
	int c = (*reg & 0x8000) == 0x8000;
	*reg <<= 1;
	if (*carry) {
		*reg |= 0x1;
	}
	*carry = c;
}

static void shr(uint16 *reg, int *carry) {
	*carry = (*reg & 1) == 1;
	*reg >>= 1;
}

static int sqx_decode_helper1(decrunch_ctx_t *uc) {
	*uc->dst++ = *uc->src++;
	return 0;
}

static int sqx_decode_helper2(decrunch_ctx_t *uc) {
	int i;
	const uint8 *src_data;
	uint16 offset, len = 0;
	shr(&uc->code, &uc->carry);
	if (uc->code == 0) {
		uc->code = read_uint16LE(uc->src); uc->src += 2;
		rcr(&uc->code, &uc->carry);
		rcl(&len, &uc->carry);
		shr(&uc->code, &uc->carry);
	} else {
		rcl(&len, &uc->carry);
		shr(&uc->code, &uc->carry);
		if (uc->code == 0) {
			uc->code = read_uint16LE(uc->src); uc->src += 2;
			rcr(&uc->code, &uc->carry);
		}
	}
	rcl(&len, &uc->carry);
	offset = 0xFF00 | *uc->src++;
	src_data = uc->dst + (s16)offset;
	for (i = 0; i < len + 2; ++i) {
		*uc->dst++ = *src_data++;
	}
	return 0;
}

static int sqx_decode_helper3(decrunch_ctx_t *uc) {
	int i;
	uint16 len;
	const uint8 *src_data;
	uint16 offset = read_uint16LE(uc->src); uc->src += 2;
	len = offset & 0xFF;
	offset >>= uc->c1;
	offset |= uc->c2 << 8;
	len &= uc->c3;
	if (len == 0) {
		len = *uc->src++;
		if (len == 0) {
			return 1;
		}
	}
	src_data = uc->dst + (s16)offset;
	for (i = 0; i < len + 2; ++i) {
		*uc->dst++ = *src_data++;
	}
	return 0;
}

typedef int (*sqx_decode_helper)(decrunch_ctx_t *);
static sqx_decode_helper sqx_decode_helper_table[3] = {
	sqx_decode_helper1,
	sqx_decode_helper2,
	sqx_decode_helper3
};

int sqx_decode(const uint8 *src, uint8 *dst) {
	int i, end;
	decrunch_ctx_t uc;
	uint8 j1 = *src++;
	uint8 j2 = *src++;
	uint8 j3 = *src++;
	assert(j1 + j2 + j3 == 3 && j1 != j2 && j2 != j3 && j3 != j1);
	memset(&uc, 0, sizeof(decrunch_ctx_t));
	uc.c1 = *src++;
	uc.c2 = 0;
	uc.c3 = 0;
	uc.src = src;
	uc.dst = dst;
	for (i = 0; i < uc.c1; ++i) {
		uc.c2 >>= 1;
		uc.c2 |= (1 << 7);
		uc.c3 <<= 1;
		uc.c3 |= 1;
	}
	uc.code = 1;
	end = 0;
	while (!end) {
		shr(&uc.code, &uc.carry);
		if (uc.code == 0) {
			uc.code = read_uint16LE(uc.src); uc.src += 2;
			rcr(&uc.code, &uc.carry);
		}
		if (!uc.carry) {
			end = (*sqx_decode_helper_table[j1])(&uc);
		} else {
			shr(&uc.code, &uc.carry);
			if (uc.code == 0) {
				uc.code = read_uint16LE(uc.src); uc.src += 2;
				rcr(&uc.code, &uc.carry);
			}
			if (!uc.carry) {
				end = (*sqx_decode_helper_table[j2])(&uc);
			} else {
				end = (*sqx_decode_helper_table[j3])(&uc);
			}
		}
	}
	return uc.dst - dst;
}
