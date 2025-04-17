TARGET = nicky
PSPSDK=$(shell psp-config --pspsdk-path)
PSPBIN = $(PSPSDK)/../bin
SDL_CONFIG = $(PSPBIN)/sdl-config

#PSP_EBOOT_SND0 = SND0.AT3
PSP_EBOOT_ICON = ICON0.PNG
PSP_EBOOT_PIC1 = PIC1.PNG


BUILD_PRX=1
PSP_FW_VERSION = 371

OBJS = fileio_std.o game.o input.o main.o mixer.o op_action.o op_anim_helper.o op_collide.o op_logic.o player_mod.o resource.o scaler.o sequence.o sound.o sqx_decoder.o staticres.o systemstub_sdl.o util.o
 


INCDIR =
CFLAGS = -O2  -G0 -Wall  $(shell $(SDL_CONFIG) --cflags) -DNICKY_SDL_VERSION   -Wall -Wno-uninitialized  -ffast-math


#CFLAGS = -O2 -g -G0 -Wall    -I. -I/usr/local/pspdev/psp/sdk/include -O2 -g -G0 -Wall  -I/usr/local/pspdev/psp/include/SDL -DNICKY_SDL_VERSION
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti -fsingle-precision-constant -mno-check-zero-division -ffast-math

LIBDIR =
LDFLAGS =
LIBS = -lstdc++ -lSDL_mixer  -lvorbisidec  -lSDL_image -lpng -lz -ljpeg \
        -lSDL_gfx -lm $(shell $(SDL_CONFIG) --libs) -lpsprtc  -lpspvfpu  -lpsppower

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = nicky

include $(PSPSDK)/lib/build.mak

