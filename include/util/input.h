#ifndef INPUT_H
#define INPUT_H
#include "../types.h"
#include "SDL2/SDL.h"
#include "../math/matrix.h"

#define SCANCODE_COUNT SDL_NUM_SCANCODES

extern SDL_Window *main_window;
typedef enum
{
    KEY_NONE     = 0,
    KEY_PRESSED  = 1,
    KEY_RELEASED = 2
} KEY_STATE;

typedef enum
{
    MOUSE_NONE     = 0,
    MOUSE_PRESSED  = 1,
    MOUSE_RELEASED = 2
} MOUSE_STATE;

i32         get_key_state(i32 code);
i32         get_mouse_state(i32 button);
vec2        get_mouse_position();
vec2        get_mouse_delta();
void reset_mouse_state();
void        capture_mouse(bool c);
void        show_mouse(bool c);
bool        poll_events();
#endif