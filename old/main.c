/*
 * Nicky - Nicky Boum engine rewrite
 * Copyright (C) 2006-2007 Gregory Montoir
 */
#include <pspkernel.h>
#include "game.h"
#include "sound.h"
#include "input.h"
#include "fileio.h"
#include "systemstub.h"


PSP_HEAP_SIZE_KB(21480);

static const char *USAGE = 
	"Usage: %s [OPTIONS]...\n"
	"  --datapath=PATH   Path to data files (default 'DATA')\n"
	"  --level=NUM       Start at level NUM\n"
	"  --record_input    Record inputs to NICKY-LEVEL%%d.COD\n"
	"  --replay_input    Read inputs from NICKY-LEVEL%%d.COD";

static int parse_option(const char *arg, const char *long_cmd, const char **opt) {
	if (strlen(arg) > 2 && arg[0] == '-' && arg[1] == '-') {
		if (strncmp(arg, long_cmd, strlen(long_cmd)) == 0) {
			*opt = arg + strlen(long_cmd);
			return 1;
		}
	}
	return 0;
}

static int parse_flag(const char *arg, const char *long_cmd, int *flag) {
	if (strlen(arg) > 2 && arg[0] == '-' && arg[1] == '-') {
		if (strncmp(arg, long_cmd, strlen(long_cmd)) == 0) {
			*flag = 1;
			return 1;
		}
	}
	return 0;
}

game_version_e detected_game_version;

static void detect_game_version() {
	if (fio_exists("REF1.REF")) {
		detected_game_version = GAME_VER_NICKY1;
	} else if (fio_exists("REF1REF.SQX")) {
		detected_game_version = GAME_VER_NICKY2;
	}
}


#undef main
int SDL_main( int argc, char *argv[] ) {
scePowerSetClockFrequency (333,333,166);
//int main(int argc, char *argv[]) {
	int i, level_num;
	const char *game_title = "";
	const char *data_path = ".";
	const char *level_num_str = "0";
	int record_input_flag = 0;
	int replay_input_flag = 0;
	input_recording_state_e rec_state = IRS_NONE;
	/* parse arguments */
	for (i = 1; i < argc; ++i) {
		int ret = 0;
		ret |= parse_option(argv[i], "--datapath=", &data_path);
		ret |= parse_option(argv[i], "--level=", &level_num_str);
		ret |= parse_flag(argv[i], "--record_input", &record_input_flag);
		ret |= parse_flag(argv[i], "--replay_input", &replay_input_flag);
		if (!ret) {
			printf(USAGE, argv[0]);
			return 0;
		}
	}
	if (record_input_flag) {
		rec_state = IRS_RECORD;
	}
	if (replay_input_flag) {
		rec_state = IRS_REPLAY;
	}
	level_num = atoi(level_num_str);
	if (level_num >= 8) {
		level_num = 0;
	}
	/* start game */
#ifdef NICKY_DEBUG
	util_debug_mask = DBG_GAME | DBG_RESOURCE | DBG_SOUND | DBG_MODPLAYER | DBG_INPUT;
#else
	util_debug_mask = 0;
#endif
	fio_init(data_path);
	detect_game_version();
	if (NICKY1) {
		game_title = "Nicky Boum";
	}
	if (NICKY2) {
		game_title = "Nicky 2";
	}
	snd_init();
	sys_init(GAME_SCREEN_W, GAME_SCREEN_H, game_title);
	inp_init(rec_state);
	game_init();
	game_run(level_num);
	game_destroy();
	sys_destroy();
	snd_stop();
	return 0;
}

