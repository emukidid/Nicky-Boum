/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */

#include "systemstub.h"
#include "util.h"

int util_debug_mask = 0;

void string_lower(char *p) {
	for (; *p; ++p) {
		if (*p >= 'A' && *p <= 'Z') {
			*p += 'a' - 'A';
		}
	}
}

void string_upper(char *p) {
	for (; *p; ++p) {
		if (*p >= 'a' && *p <= 'z') {
			*p += 'A' - 'a';
		}
	}
}

void print_debug(int debug_channel, const char *msg, ...) {
	if (util_debug_mask & debug_channel) {
		char buf[256];
		va_list va;
		va_start(va, msg);
		vsprintf(buf, msg, va);
		va_end(va);
		sys_print_string(buf);
		sys_print_string("\n");
	}
}

void print_warning(const char *msg, ...) {
	char buf[256];
	va_list va;
	va_start(va, msg);
	vsprintf(buf, msg, va);
	va_end(va);
	sys_print_string("WARNING: ");
	sys_print_string(buf);
	sys_print_string("\n");
}

void print_error(const char *msg, ...) {
	char buf[256];
	va_list va;
	va_start(va, msg);
	vsprintf(buf, msg, va);
	va_end(va);
	sys_print_string("ERROR: ");
	sys_print_string(buf);
	sys_print_string("!\n");
	exit(-1);
}
