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
    vec2 text_scale;
} GuiButtonSkin;

typedef struct
{
    i32 fill_color;
    i32 outline_color;
    i32 marker_color;
    i32 outline_size;
    vec2 mark_scale;
    vec2 text_scale;
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
    vec2 text_scale;

} GuiCheckboxSkin;

typedef struct
{
    i32 outline_color;
    i32 outline_size;
} GuiViewPanelSkin;

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
    vec2 pos;
    vec2 size;
    i32 bg_color;
    i32 event_flags;
    u32 fbo_id;
    u32 fbo_texid;
    u32 fbo_depthrb;
    mat4 view_matrix;
    mat4 projection_matrix;
    cwsMesh *mesh;
    cwsMaterial *material;

} cwsGuiViewPanel;

typedef struct cwsGuiSurface cwsGuiSurface;
struct cwsGuiSurface
{
    u32 tex_buffer, tex;
    vec3 pos;
    vec2 size;
    bool fill;
    
    //The data contains (pos,size,color_index) of every added item
    cws_array(f32) item_data;
    
    cws_bucket_array(cwsGuiButton, 32) buttons;
    cws_bucket_array(cwsGuiSlider, 32) sliders;
    cws_bucket_array(cwsGuiCheckbox, 32) checkboxes;
    cws_bucket_array(cwsGuiToggleButton, 32) toggle_buttons;
    cws_bucket_array(cwsGuiViewPanel, 32) view_panels;
    cws_array(cwsGuiSurface*) children;
    cws_array(cwsText*) texts;
    
    cwsMesh *mesh;
    cwsTextContext *text_context;
};

extern GuiButtonSkin button_skin;
extern GuiSliderSkin slider_skin;
extern GuiCheckboxSkin checkbox_skin;
extern SurfaceSkin surface_skin;

void 				   cwsGuiInit();
void 				   cwsGuiDestroy();
void 				   cwsGuiDraw();
void 				   cwsGuiUpdate();

cwsGuiSurface * cwsNewSurface(cwsGuiSurface *parent);
void 				    cwsClearSurface(cwsGuiSurface *s);
void 				    cwsRefreshSurface(cwsGuiSurface *s);
void                    cwsShowSurface(cwsGuiSurface *s, bool c);

cwsGuiButton*              cwsSurfaceAddButton(cwsGuiSurface *s);
cwsGuiSlider*              cwsSurfaceAddSlider(cwsGuiSurface *s);
cwsGuiCheckbox*            cwsSurfaceAddCheckbox(cwsGuiSurface *s);
cwsGuiToggleButton*        cwsSurfaceAddToggleButton(cwsGuiSurface *s);
cwsGuiViewPanel* cwsSurfaceAddViewPanel(cwsGuiSurface *s);
cwsText* 	                cwsSurfaceAddText(cwsGuiSurface *s, vec2 pos, vec2 scale, const char *str);
