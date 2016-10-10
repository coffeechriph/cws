#include <stdio.h>
#include "stdlib.h"
#include "math.h"
#include "time.h"
#include "math/matrix.h"
#include "SDL2/SDL.h"
#include "GL/glew.h"
#include "util/log.h"
#include "scene/scene.h"
#include "gui/gui.h"
#include "cws.h"
 
int main(int args, char *argv[])
{
    cws_create(1280, 720, "Game", true);

    cwsGuiSurface *surface = cwsNewSurface(NULL);
    surface->transform->size = (vec2){.x = 400, .y = 400};
    surface->renderer->fill = true;
    
    cwsGuiButton *btn = cwsSurfaceAddButton(surface);
    btn->size = (vec2){.x = 100, .y = 100};    
    btn->pos = (vec2){.x = 0, .y = 300};
    cwsRebuildText(btn->text->context, btn->text, "Updated!");
    
    cwsGuiButton *btn2 = cwsSurfaceAddButton(surface);
    btn2->size = (vec2){.x = 200, .y = 50};
    btn2->pos = (vec2){.x = 0, .y = 101};
    cwsRebuildText(btn2->text->context, btn2->text, "Brush Tool"); 
    
    cwsGuiSlider *slider = cwsSurfaceAddSlider(surface);
    slider->size = (vec2){.x = 300, .y = 30};
    slider->pos = (vec2){.x = 5, .y = 15};
    cwsRebuildText(slider->text->context, slider->text, "FOV");
    
    cwsGuiCheckbox *checkbox = cwsSurfaceAddCheckbox(surface);
    checkbox->size = (vec2){.x = 100, .y = 30};
    checkbox->pos = (vec2){.x = 105, .y = 200};
    
    cwsGuiToggleButton *tbtn = cwsSurfaceAddToggleButton(surface);
    tbtn->size = (vec2){.x = 50, .y = 50};
    tbtn->pos = (vec2){.x = 220, .y = 100};
    
    cwsRefreshSurface(surface);
    cwsShowSurface(surface,true);
    while(cws_running)
    {
        cwsClear();
        scene_draw();
        cwsGuiDraw();
        cwsSwapBuffers();
        cws_run();
    }
    
    cws_close();
    return 0;
}