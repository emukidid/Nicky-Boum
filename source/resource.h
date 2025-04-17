/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#ifndef __RESOURCE_H__
#define __RESOURCE_H__

#include "util.h"
#include "common.h"

extern void res_load_level_data__v1(int level);
extern void res_load_level_data__v2(int level);
extern void res_unload_level_data();
extern void res_init_static_data__v1();
extern void res_init_static_data__v2();

#endif /* __RESOURCE_H__ */
