#include "input.h"

i32 key_states[SCANCODE_COUNT];
i32 mouse_states[3];
vec2 mouse_position, mouse_delta;
SDL_Event event;

i32 get_key_state(i32 code)
{
    return key_states[code];
}

i32 get_mouse_state(i32 button)
{
    return mouse_states[button];
}

vec2 get_mouse_position()
{
    return mouse_position;
}

vec2 get_mouse_delta()
{
    return mouse_delta;
}

bool poll_events()
{
    for(i32 i = 0; i < SCANCODE_COUNT; ++i)
    {
        if(key_states[i] == KEY_RELEASED)
        {
            key_states[i] = KEY_NONE;
        }
    }

    mouse_delta.x = 0.0f;
    mouse_delta.y = 0.0f;

    //Events
    while(SDL_PollEvent(&event))
    {
        if(event.type == SDL_QUIT)
        {
            return false;
        }
        
        if(event.type == SDL_KEYDOWN)
        {
            key_states[event.key.keysym.scancode] = KEY_PRESSED;
        }
        else if(event.type == SDL_KEYUP)
        {
            key_states[event.key.keysym.scancode] = KEY_RELEASED;
        }

        if(event.type == SDL_MOUSEBUTTONDOWN)
        {
            mouse_states[event.button.button-1] = MOUSE_PRESSED;
        }
        else if(event.type == SDL_MOUSEBUTTONUP)
        {
            mouse_states[event.button.button-1] = MOUSE_RELEASED;
        }

        if(event.type == SDL_MOUSEMOTION)
        {
            mouse_position.x = (f32)event.motion.x;
            mouse_position.y = (f32)event.motion.y;
            mouse_delta.x = (f32)event.motion.xrel;
            mouse_delta.y = (f32)event.motion.yrel;
        }
    }

    return true;
}

void capture_mouse(bool c)
{
    SDL_SetRelativeMouseMode(c ? SDL_TRUE : SDL_FALSE);
}

void show_mouse(bool c)
{
    SDL_ShowCursor(c ? SDL_ENABLE : SDL_DISABLE);
}
