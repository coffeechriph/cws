#ifndef CWS
#define CWS
#include <stdio.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"
#include "GL/glew.h"
#include "types.h"
#include "util/log.h"
#include "util/input.h"
#include "util/cws_bucket_array.h"
#include "gfx/video.h"
#include "scene/scene.h"
#include "gui/gui.h"

extern bool cws_running;

extern f32 cws_delta_time;
extern u32 cws_time_since_last_frame;
extern f32 START_TIME;

bool cws_create(unsigned int scrcwsnW, unsigned int scrcwsnH, const char *title, bool vsync);
void cws_close();
void cws_run();

#endif