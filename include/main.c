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
   
    cwsGuiSurface *surface = cwsNewSurface(NULL);
    surface->size = (vec2){.x = 600, .y = 600};
    surface->fill = true;
    
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
    
    cwsMesh tree_mesh;
    cwsMeshFromfile(&tree_mesh, "./data/tree1.dae");
    
    cwsTexture2D tree_tex;
    cwsTextureFromfile(&tree_tex, "./data/gfx/tree1.png", IF_NEAREST);
    cwsMaterial tree_mat;
    cws_array_init(u32, tree_mat.texture_uniforms, 0);
    cws_array_init(cwsTexture2D, tree_mat.texture_array, 0);
    cwsShaderFromfile(&tree_mat.shader, "./data/shaders/single_v", "./data/shaders/single_f", NULL);
    cwsMaterialAddTexture(&tree_mat, tree_tex);
    
    cwsGuiViewPanel *vpanel = cwsSurfaceAddViewPanel(surface);
    vpanel->bg_color = cwsPackRgb((ivec3){.x = 220, .y = 200, .z = 100});
    vpanel->pos = (vec2){220, 200};
    vpanel->size = (vec2){.x = 300, .y = 300};
    vpanel->material = &tree_mat;
    vpanel->mesh = &tree_mesh;
    vpanel->view_matrix = mat4_translate(mat4_default, (vec3){.x = 0.0f, .y = 0.0f, .z = -4.0f});
    cwsRefreshSurface(surface);
    cwsShowSurface(surface,true);
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