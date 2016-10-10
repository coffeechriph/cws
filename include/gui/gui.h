#pragma once
#include "stdlib.h"
#include "memory.h"
#include "GL/glew.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"
#include "../io/file.h"
#include "../gfx/video.h"
#include "../types.h"
#include "../util/input.h"
#include "../util/cws_array.h"
#include "../util/cws_bucket_array.h"

typedef enum 
{
    EVENT_NONE                  = 1 << 0,
    EVENT_CLICKED               = 1 << 1,
    EVENT_HOVER                 = 1 << 2,
    EVENT_MOUSE_DOWN            = 1 << 3,
    EVENT_MOUSE_ENTER           = 1 << 4,
    EVENT_MOUSE_LEAVE           = 1 << 5,
    EVENT_FOCUSED               = 1 << 6,
    EVENT_UNFOCUSED             = 1 << 7,
    EVENT_VISIBILITY_CHANGED    = 1 << 8,

    EVENT_SLIDER_CHANGED        = 1 << 9,
    EVENT_CHECKBOX_CHANGED      = 1 << 10,
    EVENT_TEXTBOX_ENTERED       = 1 << 11,
    EVENT_FILE_SELECTED		    = 1 << 12
} EVENT_TYPE;

typedef struct
{
    i32 fill_color;
    i32 outline_color;
    i32 outline_size;
} SurfaceSkin;

typedef struct
{
    i32 fill_color;
    i32 fill_color_clicked;
    i32 outline_color;
    i32 outline_size;
} GuiButtonSkin;

typedef struct
{
    i32 fill_color;
    i32 outline_color;
    i32 marker_color;
    i32 outline_size;
    vec2 mark_scale;
} GuiSliderSkin;

typedef struct
{
    i32 fill_color;
    i32 outline_color;
    i32 marker_color;
    i32 marker_color_checked;
    i32 outline_size;
    f32 mark_scale;
    vec2 mark_offset;

} GuiCheckboxSkin;

typedef struct
{
    vec2 pos;
    vec2 size;
    i32 event_flags;
    cwsText *text;
} cwsGuiButton;

typedef struct
{
    vec2 pos;
    vec2 size;
    i32 event_flags;
    i32 min, max, value;
    cwsText *text;
} cwsGuiSlider;

typedef struct
{
    vec2 pos;
    vec2 size;
    i32 event_flags;
    i32 checked;
    cwsText *text;  
} cwsGuiCheckbox;

typedef struct
{
    vec2 pos;
    vec2 size;
    i32 event_flags;
	i32 toggled;
    cwsText *text;
} cwsGuiToggleButton;

typedef struct
{
    vec3 pos;
    vec2 size;
} SurfaceTransform;

typedef struct cwsSurfaceRenderer cwsSurfaceRenderer;
struct cwsSurfaceRenderer
{
    u32 tex_buffer, tex;

    cwsMesh *mesh;
    SurfaceTransform *transform;
    cwsTextContext *text_context;
    cws_array(cwsSurfaceRenderer*, children);

    //The data contains (pos,size,color_index) of every added item
    cws_array(f32, item_data);

    bool fill;
};

typedef struct cwsGuiSurface cwsGuiSurface;
struct cwsGuiSurface
{
    cwsSurfaceRenderer *renderer;
    SurfaceTransform *transform;
    
    cws_bucket_array(cwsGuiButton, buttons);
    cws_bucket_array(cwsGuiSlider, sliders);
    cws_bucket_array(cwsGuiCheckbox, checkboxes);
    cws_bucket_array(cwsGuiToggleButton, toggle_buttons);
    cws_array(cwsGuiSurface*, children);
    cws_array(cwsText*, texts);
};

void 				   cwsGuiInit();
void 				   cwsGuiDestroy();
void 				   cwsGuiDraw();
void 				   cwsGuiUpdate();

cwsGuiSurface * cwsNewSurface(cwsGuiSurface *parent);
void                    cwsDeleteSurface(cwsGuiSurface *s);
void 				    cwsClearSurface(cwsGuiSurface *s);
void 				    cwsRefreshSurface(cwsGuiSurface *s);
void                    cwsShowSurface(cwsGuiSurface *s, bool c);

cwsGuiButton*              cwsSurfaceAddButton(cwsGuiSurface *s);
cwsGuiSlider*              cwsSurfaceAddSlider(cwsGuiSurface *s);
cwsGuiCheckbox*            cwsSurfaceAddCheckbox(cwsGuiSurface *s);
cwsGuiToggleButton*        cwsSurfaceAddToggleButton(cwsGuiSurface *s);
cwsText* 	                cwsSurfaceAddText(cwsGuiSurface *s, vec2 pos, vec2 scale, const char *str);