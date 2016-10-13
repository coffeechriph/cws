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
#include "util/cws_bucket_array.h"

int main(int args, char *argv[])
{
    cws_create(1280, 720, "Game", false);
   
    cwsMaterial mat;
    cwsMaterialInit(mat);
    mat.rflags = RF_NONE;
    
    cwsShaderFromfile(&mat.shader, "./data/shaders/single_v", "./data/shaders/single_f");
    cwsTexture2D tex;
    
    cwsTextureFromfile(&tex, "./data/gfx/tree1.png", IF_NEAREST);
    cwsMaterialAddTexture(&mat, tex);

    cwsMesh mesh;
    cwsMeshFromfile(&mesh, "./data/tree1.dae");
    
    f32 _x = -10.0f, _z = -5.0f;
    for(u32 i = 0; i < 7000; ++i)
    {
        cwsRenderer *renderer = cwsNewRenderer(&mat,&mesh);
        renderer->position = (vec3){.x = _x, .y = -1.0, .z = _z};
        renderer->scale = (vec3){.x = 2, .y = 2, .z = 2};
        cwsUpdateBounds(renderer);
        
        _x += 0.5f;
        if(_x >= 10)
        {
            _x = -10.0f;
            _z -= 0.5f;
        }
    }
    
    /*
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
    cwsShowSurface(surface,false);*/
    while(cws_running)
    {
        cwsClear();
        cwsSceneDraw();
        cwsGuiDraw();
        cwsSwapBuffers();

        cws_run();
    }
    
    cws_close();
    return 0;
}