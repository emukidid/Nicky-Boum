/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#ifndef __INPUT_H__
#define __INPUT_H__

#include "util.h"

typedef enum {
	IRS_NONE = 0,
	IRS_REPLAY,
	IRS_RECORD
} input_recording_state_e;

extern void inp_init(input_recording_state_e rec_state);
extern void inp_start_level(int level_num);
extern void inp_end_level();
extern void inp_copy_state();

#endif /* __INPUT_H__ */
