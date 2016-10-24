#include "gui.h"
cws_array(cwsGuiSurface*,visible_surfaces);
cws_array(cwsGuiSurface*,hidden_surfaces);

GuiButtonSkin button_skin;
GuiSliderSkin slider_skin;
GuiCheckboxSkin checkbox_skin;
SurfaceSkin surface_skin;

cwsMaterial material;
cwsShader shader;
mat4 ortho;

//Used to store the last global mouse state
i32 last_global_mouse_state[3] = {0,0,0};
i32 internal_mouse_states[3] = {0,0,0};
typedef enum
{
    GMOUSE_NONE        = 0,
    GMOUSE_RELEASED    = 1,
    GMOUSE_CLICKED     = 2,
    GMOUSE_DOWN        = 3
} GuiMouseState;

void cwsGuiInit()
{
    cws_array_init(cwsGuiSurface*, visible_surfaces, 0);
    cws_array_init(cwsGuiSurface*, hidden_surfaces, 0);
    
    button_skin.fill_color          = cwsPackRgb((ivec3){.x = 76, .y = 76, .z = 76});
    button_skin.fill_color_clicked  = cwsPackRgb((ivec3){.x = 102,.y = 102,.z = 102});
    button_skin.outline_color       = cwsPackRgb((ivec3){.x = 85, .y = 85, .z = 85});
    button_skin.outline_size        = 2;

    slider_skin.fill_color      = cwsPackRgb((ivec3){.x = 76, .y = 76, .z = 76});
    slider_skin.outline_color   = cwsPackRgb((ivec3){.x = 85, .y = 85, .z = 85});
    slider_skin.marker_color    = cwsPackRgb((ivec3){.x = 127,.y = 127,.z = 127});
    slider_skin.outline_size    = 2;
    slider_skin.mark_scale = (vec2){.x = 0.05f, .y = 1.5f};

    checkbox_skin.fill_color              = cwsPackRgb((ivec3){.x = 76, .y = 76, .z = 76});
    checkbox_skin.outline_color           = cwsPackRgb((ivec3){.x = 85, .y = 85, .z = 85});
    checkbox_skin.marker_color            = cwsPackRgb((ivec3){.x = 107,.y = 107,.z = 107});
    checkbox_skin.marker_color_checked    = cwsPackRgb((ivec3){.x = 127,.y = 127,.z = 127});
    checkbox_skin.outline_size            = 2;
    checkbox_skin.mark_scale              = 0.5f;
    checkbox_skin.mark_offset             = (vec2){.x = 0.01f, .y = 0.25f};
    
    surface_skin.fill_color          = cwsPackRgb((ivec3){.x = 65, .y = 65, .z = 65});
    surface_skin.outline_color       = cwsPackRgb((ivec3){.x = 90, .y = 90, .z = 90});
    surface_skin.outline_size        = 2;

    cwsShaderInit(shader);
    cwsShaderFromfile(&shader, "./data/shaders/gui_v", "./data/shaders/gui_f", NULL);

    cwsMaterialInit(material);
    material.shader = shader;
    material.rflags = 0;
    ortho = mat4_ortho(0.0f,(f32)cwsScreenSize().x,(f32)cwsScreenSize().y, 0.0f, -1.0f, 1.0f);
}

void delete_surface(cwsGuiSurface *s);
void delete_surface_renderer(cwsSurfaceRenderer* r);
void cwsGuiDestroy()
{
    for(u32 i = 0; i < visible_surfaces.length; ++i)
    {
        delete_surface_renderer(visible_surfaces.data[i]->renderer);
        delete_surface(visible_surfaces.data[i]);
    }
    
    for(u32 i = 0; i < hidden_surfaces.length; ++i)
    {
        delete_surface_renderer(hidden_surfaces.data[i]->renderer);
        delete_surface(hidden_surfaces.data[i]);
    }
    
    cwsDeleteShader(&shader);
}

void delete_surface(cwsGuiSurface *s)
{
    if(s == NULL)
    {
        return;
    }

    for(u32 j = 0; j < s->children.length; ++j)
    {
        delete_surface(s->children.data[j]);
    }
    cws_array_free(s->children);

    cws_bucket_array_free(s->buttons);
    cws_bucket_array_free(s->sliders);
    cws_bucket_array_free(s->checkboxes);
    cws_bucket_array_free(s->toggle_buttons);

    if(s->transform != NULL)
    {
        free(s->transform);
    }

    cws_array_free(s->texts);
    free(s);
}

void delete_surface_renderer(cwsSurfaceRenderer *r)
{
    for(u32 j = 0; j < r->children.length; ++j)
    {
        delete_surface_renderer(r->children.data[j]);
    }
    cws_array_free(r->children);
    cws_array_free(r->item_data);
    
    glDeleteBuffers(1, &r->tex_buffer);
    glDeleteTextures(1, &r->tex);
    cwsDeleteMesh(r->mesh);
    free(r->mesh);
    cwsDeleteTextContext(r->text_context);
    free(r->text_context);
    free(r);
}

void cwsShowSurface(cwsGuiSurface *s, bool c)
{
    if(c)
    {
        for(u32 i = 0; i < hidden_surfaces.length; ++i)
        {
            if(hidden_surfaces.data[i] == s)
            {
                cws_array_push(visible_surfaces,s);
                cws_array_remove(hidden_surfaces, i);
                break;
            }
        }
    }
    else
    {
        for(u32 i = 0; i < visible_surfaces.length; ++i)
        {
            if(visible_surfaces.data[i] == s)
            {
                cws_array_push(hidden_surfaces, s);
                cws_array_remove(visible_surfaces,i);
                break;
            }
        }
    }

    //Reset component events - if the surface is either shown or hidden the state ncwsds to be reset
    for(u32 i = 0; i < cws_bucket_array_item_count(s->buttons); ++i)
    {
        if(!cws_bucket_array_occupied(s->buttons,i))
        {
            continue;
        }
        
        cws_bucket_array_index(s->buttons,i).event_flags = 0;
    }

    for(u32 i = 0; i < cws_bucket_array_item_count(s->sliders); ++i)
    {
        if(!cws_bucket_array_occupied(s->sliders, i))
        {
            continue;
        }
        
        cws_bucket_array_index(s->sliders,i).event_flags = 0;
    }

    for(u32 i = 0; i < cws_bucket_array_item_count(s->checkboxes); ++i)
    {
        if(!cws_bucket_array_occupied(s->checkboxes,i))
        {
            continue;
        }
        
        cws_bucket_array_index(s->checkboxes, i).event_flags = 0;
    }

    for(u32 i = 0; i < cws_bucket_array_item_count(s->toggle_buttons); ++i)
    {
        if(!cws_bucket_array_occupied(s->toggle_buttons,i))
        {
            continue;
        }
        
        cws_bucket_array_index(s->toggle_buttons,i).event_flags = 0;
    }
}

cwsGuiSurface *cwsNewSurface(cwsGuiSurface *parent)
{
    cwsGuiSurface *s =malloc(sizeof(cwsGuiSurface));
    if(s == NULL)
    {
        cws_log("Malloc error on cwsGuiSurface!");
        return NULL;
    }
    
    cws_array_init(cwsGuiSurface*, s->children, 0);
    cws_bucket_array_init(cwsGuiButton, s->buttons, 0);
    cws_bucket_array_init(cwsGuiSlider, s->sliders, 0);
    cws_bucket_array_init(cwsGuiCheckbox, s->checkboxes, 0);
    cws_bucket_array_init(cwsGuiToggleButton, s->toggle_buttons, 0);
    cws_array_init(cwsText*, s->texts, 0);

    s->transform = malloc(sizeof(SurfaceTransform));
    if(s->transform == NULL)
    {
        cws_log("Malloc error on SurfaceTransform!");
        return NULL;
    }
    s->transform->pos = (vec3){.x = 0, .y = 0, .z = 0};
    s->transform->size = (vec2){.x = 300,.y = 300};

    s->renderer = malloc(sizeof(cwsSurfaceRenderer));
    if(s->renderer == NULL)
    {
        cws_log("Malloc error on cwsSurfaceRenderer!");
        return NULL;
    }
    cws_array_init(f32, s->renderer->item_data, 0);
    cws_array_init(cwsSurfaceRenderer*, s->renderer->children, 0);
    s->renderer->fill = false;

    i32 vertex_attribs[3] = {3,2,1};
    s->renderer->mesh = malloc(sizeof(cwsMesh));
    cwsMeshInit(*s->renderer->mesh);
    cwsEmptyMesh(s->renderer->mesh, vertex_attribs, 3);
    glGenBuffers(1, &s->renderer->tex_buffer);
    glGenTextures(1, &s->renderer->tex);
    s->renderer->transform = s->transform;
    
    s->renderer->text_context = malloc(sizeof(cwsTextContext));
    cwsCreateTextContext(s->renderer->text_context, "./test.fnt");

    if(parent == NULL)
    {
        cws_array_push(visible_surfaces, s);
    }
    else
    {
        cws_array_push(parent->children, s);
        
        //Add the new renderer as a child
        cws_array_push(parent->renderer->children,s->renderer);
    }
    return s;
}

cwsGuiButton* cwsSurfaceAddButton(cwsGuiSurface *s)
{
    cwsGuiButton btn;
    btn.pos = (vec2){.x = 0, .y = 0};
    btn.size = (vec2){.x = 100, .y = 20};
    btn.event_flags = 0;
    btn.text = cwsSurfaceAddText(s, (vec2){.x = 0, .y = 0}, (vec2){.x = 0.6f, .y = 0.6f}, "cwsGuiButton");
    
    cws_bucket_array_push(s->buttons, btn);
    
    u32 last_index = 0;
    cws_bucket_array_last(s->buttons, last_index);
    return &cws_bucket_array_index(s->buttons, last_index);
}

cwsGuiSlider *cwsSurfaceAddSlider(cwsGuiSurface *s)
{
    cwsGuiSlider sl;

    sl.pos = (vec2){.x = 0, .y = 0};
    sl.size = (vec2){.x = 100, .y = 20};
    sl.event_flags = 0;
    sl.min = 0;
    sl.max = 100;
    sl.value = 0;
    sl.text = cwsSurfaceAddText(s, (vec2){.x = 0, .y = 0}, (vec2){.x = 0.4f, .y = 0.4f}, "cwsGuiSlider");
    
    cws_bucket_array_push(s->sliders, sl);
    u32 index = 0;
    cws_bucket_array_last(s->sliders,index);
    return &cws_bucket_array_index(s->sliders,index);
}

cwsGuiCheckbox* cwsSurfaceAddCheckbox(cwsGuiSurface *s)
{
    cwsGuiCheckbox c;
    c.pos = (vec2){.x = 0, .y = 0};
    c.size = (vec2){.x = 100, .y = 20};
    c.event_flags = 0;
    c.checked = 0;
    c.text = cwsSurfaceAddText(s, (vec2){.x = 0, .y = 0}, (vec2){.x = 0.6f, .y = 0.6f}, "cwsGuiCheckbox");

    cws_bucket_array_push(s->checkboxes, c);
    u32 index = 0;
    cws_bucket_array_last(s->checkboxes,index);
    return &cws_bucket_array_index(s->checkboxes,index);
}

cwsGuiToggleButton* cwsSurfaceAddToggleButton(cwsGuiSurface *s)
{
    cwsGuiToggleButton btn;
    btn.pos = (vec2){.x = 0, .y = 0};
    btn.size = (vec2){.x = 100, .y = 20};
    btn.event_flags = 0;
    btn.toggled = 0;
    btn.text = cwsSurfaceAddText(s, (vec2){.x = 0, .y = 0}, (vec2){.x = 0.6f, .y = 0.6f}, "cwsGuiToggleButton");
    
    cws_bucket_array_push(s->toggle_buttons, btn);
    u32 index = 0;
    cws_bucket_array_last(s->toggle_buttons,index);
    return &cws_bucket_array_index(s->toggle_buttons,index);;
}

cwsText* cwsSurfaceAddText(cwsGuiSurface *s, vec2 pos, vec2 scale, const char *str)
{
    cwsText *text = cwsNewText(s->renderer->text_context, vec2_add(pos, (vec2){.x = s->transform->pos.x, .y = s->transform->pos.y}), scale, str);
    text->bounds = (vec4){.x = s->transform->pos.x, 
                          .y = s->transform->pos.y,
                          .z = s->transform->pos.x + s->transform->size.x,
                          .w = s->transform->pos.y + s->transform->size.y};
    
    cws_array_push(s->texts, text);
    return text;
}

/*
    Removes any mesh data & item data from the surface
*/
void cwsClearSurface(cwsGuiSurface *s)
{
    cws_bucket_array_free(s->buttons);
    cws_bucket_array_free(s->sliders);
    cws_bucket_array_free(s->checkboxes);
    cws_bucket_array_free(s->toggle_buttons);
    cws_array_free(s->texts);

    cwsClearMesh(s->renderer->mesh);
    cwsClearTextContext(s->renderer->text_context);
}

/*
    Helper function to fill a GuiSurface mesh data with components
    vindex = index into vertices
    iindex = index into indices
    index = component index
*/
void fill_rect(f32 *vertices, i32 *indices, i32 *vi, i32 *ii, i32 *i, f32 z)
{
    i32 iindex = *ii;
    i32 vindex = *vi;
    i32 index = *i;

    indices[iindex++] = vindex/6;   indices[iindex++] = vindex/6+1; indices[iindex++] = vindex/6+2;
    indices[iindex++] = vindex/6+2; indices[iindex++] = vindex/6+3; indices[iindex++] = vindex/6;

    vertices[vindex++] = 0.0f; vertices[vindex++] = 0.0f; vertices[vindex++] = z; vertices[vindex++] = 0.0f; vertices[vindex++] = 0.0f; vertices[vindex++] = (f32)index;
    vertices[vindex++] = 1.0f; vertices[vindex++] = 0.0f; vertices[vindex++] = z; vertices[vindex++] = 0.0f; vertices[vindex++] = 1.0f; vertices[vindex++] = (f32)index;
    vertices[vindex++] = 1.0f; vertices[vindex++] = 1.0f; vertices[vindex++] = z; vertices[vindex++] = 1.0f; vertices[vindex++] = 1.0f; vertices[vindex++] = (f32)index;
    vertices[vindex++] = 0.0f; vertices[vindex++] = 1.0f; vertices[vindex++] = z; vertices[vindex++] = 1.0f; vertices[vindex++] = 0.0f; vertices[vindex++] = (f32)index;

    (*i)++;
    (*vi) += 24;
    (*ii) += 6;
}

/*
    A surface must be refreshed after components have bcwsn added! 
    Refreshing the surface will update the mesh data & item data
*/
void cwsRefreshSurface(cwsGuiSurface *s)
{
    //Clear old data
    cws_array_free(s->renderer->item_data);

    //GuiButtons uses 1 item (16 floats for rendering) while sliders & checkboxes uses 2 items (32)
    i32 vcount = cws_bucket_array_item_count(s->buttons) * 24 + cws_bucket_array_item_count(s->sliders) * 48 + cws_bucket_array_item_count(s->checkboxes) * 48 + cws_bucket_array_item_count(s->toggle_buttons) * 24;
    i32 icount = cws_bucket_array_item_count(s->buttons) * 6  + cws_bucket_array_item_count(s->sliders) * 12 + cws_bucket_array_item_count(s->checkboxes) * 12 + cws_bucket_array_item_count(s->toggle_buttons) * 6;

    u32 isize = 0;
    if(s->renderer->fill)
    {
        vcount += 24;
        icount += 6;
        isize = 8 + cws_bucket_array_item_count(s->buttons) * 8 + cws_bucket_array_item_count(s->sliders) * 16 + cws_bucket_array_item_count(s->checkboxes) * 16 + cws_bucket_array_item_count(s->toggle_buttons) * 8;
    }
    else
    {
        isize = cws_bucket_array_item_count(s->buttons) * 8 + cws_bucket_array_item_count(s->sliders) * 16 + cws_bucket_array_item_count(s->checkboxes) * 16 + cws_bucket_array_item_count(s->toggle_buttons) * 8;
    }

    cws_array_init(f32, s->renderer->item_data, isize);
    s->renderer->item_data.length = s->renderer->item_data.size;

    f32 vertices[vcount];
    i32 indices [icount];
    
    i32 index = 0;
    i32 vindex = 0;
    i32 iindex = 0;

    for(u32 i = 0; i < cws_bucket_array_item_count(s->buttons); ++i)
    {
        if(!cws_bucket_array_occupied(s->buttons,i))
        {
            continue;
        }
        
        fill_rect(vertices, indices, &vindex, &iindex, &index, 0.0f);
    }

    for(u32 i = 0; i < cws_bucket_array_item_count(s->sliders); ++i)
    {
        if(!cws_bucket_array_occupied(s->sliders, i))
        {
            continue;
        }
        
        //GuiSlider mark
        fill_rect(vertices, indices, &vindex, &iindex, &index, 0.5f);
        
        //GuiSlider base
        fill_rect(vertices, indices, &vindex, &iindex, &index, 0.0f);
    }

    for(u32 i = 0; i < cws_bucket_array_item_count(s->checkboxes); ++i)
    {
        if(!cws_bucket_array_occupied(s->checkboxes,i))
        {
            continue;
        }
        
        //GuiCheckbox base
        fill_rect(vertices, indices, &vindex, &iindex, &index, 0.0f);

        //GuiCheckbox mark
        fill_rect(vertices, indices, &vindex, &iindex, &index, 0.5f);
    }

    for(u32 i = 0; i < cws_bucket_array_item_count(s->toggle_buttons); ++i)
    {
        if(!cws_bucket_array_occupied(s->toggle_buttons,i))
        {
            continue;
        }
        
        fill_rect(vertices, indices, &vindex, &iindex, &index, 0.0f);
    }

    //Surface renders with a background
    if(s->renderer->fill)
    {
        fill_rect(vertices, indices, &vindex, &iindex, &index, 0.0f);
    }

    cwsFillMesh(s->renderer->mesh, vertices, vindex, indices, iindex);
}

/*
    Centers a text relative to pos,size and sets the texts
    boundaries to the bounds of the rectangle
*/
void text_center_and_clip(cwsText* t, vec2 pos, vec2 size, i32 outline_size)
{
   t->pos = pos;
   t->pos.x += size.x * 0.5f - (t->__size.x*t->scale.x) * 0.5f;
   t->pos.y += size.y * 0.5f - (t->__size.y*t->scale.y) * 0.5f;
   t->bounds = (vec4){.x = pos.x + outline_size, 
                      .y = pos.y + outline_size, 
                      .z = pos.x + size.x - outline_size, 
                      .w = pos.y + size.y - outline_size};
}

void update_surface_component_data(cwsGuiSurface *s)
{
    i32 index = 0;
    for(u32 i = 0; i < cws_bucket_array_item_count(s->buttons); ++i)
    {
        if(!cws_bucket_array_occupied(s->buttons,i))
        {
            continue;
        }
        cwsGuiButton* btn = &cws_bucket_array_index(s->buttons, i);
        text_center_and_clip(btn->text, (vec2){.x = btn->pos.x+s->transform->pos.x, 
                                                         .y = btn->pos.y+s->transform->pos.y}, 
                                                         btn->size, button_skin.outline_size);

        s->renderer->item_data.data[index++] = btn->pos.x;
        s->renderer->item_data.data[index++] = btn->pos.y;
        s->renderer->item_data.data[index++] = btn->size.x;
        s->renderer->item_data.data[index++] = btn->size.y;

        /*
            Skin data
            The first index decides which skin to use in the shader (0 for button skin)
            The second is the index of the fill color to use for the item part, a button uses 2 colors to 
            indicate the state (pressed or released)
        */
        s->renderer->item_data.data[index++] = 0;
        s->renderer->item_data.data[index++] = ((btn->event_flags&EVENT_CLICKED) || (btn->event_flags&EVENT_MOUSE_DOWN)) ? 1.0f : 0.0f;
        s->renderer->item_data.data[index++] = 0.0f;
        s->renderer->item_data.data[index++] = 0.0f;
    }

    for(u32 i = 0; i < cws_bucket_array_item_count(s->sliders); ++i)
    {
        if(!cws_bucket_array_occupied(s->sliders,i))
        {
            continue;
        }
        
        cwsGuiSlider *slider = &cws_bucket_array_index(s->sliders, i);
        text_center_and_clip(slider->text, 
                            (vec2){.x = slider->pos.x+s->transform->pos.x,
                            .y = slider->pos.y+s->transform->pos.y},
                            slider->size, 
                            slider_skin.outline_size);
        //mark
        s->renderer->item_data.data[index++] = slider->pos.x + ((f32)slider->size.x / (f32)(slider->max-slider->min)) * ((f32)slider->value - slider->min) - ((f32)slider->size.x * slider_skin.mark_scale.x * 0.5f);
        s->renderer->item_data.data[index++] = slider->pos.y - slider->size.y / (slider->size.y * slider_skin.mark_scale.y);
        s->renderer->item_data.data[index++] = (f32)slider->size.x * slider_skin.mark_scale.x;
        s->renderer->item_data.data[index++] = (f32)slider->size.y * slider_skin.mark_scale.y;

        s->renderer->item_data.data[index++] = 1;
        s->renderer->item_data.data[index++] = 1.0f;
        s->renderer->item_data.data[index++] = 0.0f;
        s->renderer->item_data.data[index++] = 0.0f;
        
        s->renderer->item_data.data[index++] = slider->pos.x;
        s->renderer->item_data.data[index++] = slider->pos.y;
        s->renderer->item_data.data[index++] = slider->size.x;
        s->renderer->item_data.data[index++] = slider->size.y;

        /*
            Skin data
            The first index decides which skin to use in the shader (1 for slider skin)
            The second is the index of the fill color to use for the item part, index 0 is used for the base
            and index 1 is used for the marker
        */
        s->renderer->item_data.data[index++] = 1;
        s->renderer->item_data.data[index++] = 0.0f;
        s->renderer->item_data.data[index++] = 0.0f;
        s->renderer->item_data.data[index++] = 0.0f;

    }

    for(u32 i = 0; i < cws_bucket_array_item_count(s->checkboxes); ++i)
    {
        if(!cws_bucket_array_occupied(s->checkboxes,i))
        {
            continue;
        }
        
        cwsGuiCheckbox *checkbox = &cws_bucket_array_index(s->checkboxes, i);
        text_center_and_clip(checkbox->text,
                            (vec2){.x = checkbox->pos.x+s->transform->pos.x,
                            .y = checkbox->pos.y+s->transform->pos.y}, 
                            checkbox->size, checkbox_skin.outline_size);

        s->renderer->item_data.data[index++] = checkbox->pos.x;
        s->renderer->item_data.data[index++] = checkbox->pos.y;
        s->renderer->item_data.data[index++] = checkbox->size.x;
        s->renderer->item_data.data[index++] = checkbox->size.y;

        /*
            Skin data
            The first index decides which skin to use in the shader (2 for checkbox skin)
            The second is the index of the fill color to use for the item part, index 0 is used for the base
            and index 1 is used for the marker
        */
        s->renderer->item_data.data[index++] = 2;
        s->renderer->item_data.data[index++] = 0.0f;
        s->renderer->item_data.data[index++] = 0.0f;
        s->renderer->item_data.data[index++] = 0.0f;

        //mark
        s->renderer->item_data.data[index++] = checkbox->pos.x + ((f32)checkbox->size.x*checkbox_skin.mark_offset.x);
        s->renderer->item_data.data[index++] = checkbox->pos.y + ((f32)checkbox->size.y*checkbox_skin.mark_offset.y);
        s->renderer->item_data.data[index++] = (f32)checkbox->size.y * checkbox_skin.mark_scale;
        s->renderer->item_data.data[index++] = (f32)checkbox->size.y * checkbox_skin.mark_scale;

        s->renderer->item_data.data[index++] = 2;
        s->renderer->item_data.data[index++] = 1.0f + checkbox->checked;
        s->renderer->item_data.data[index++] = 0.0f;
        s->renderer->item_data.data[index++] = 0.0f;
    }

    for(u32 i = 0; i < cws_bucket_array_item_count(s->toggle_buttons); ++i)
    {
        if(!cws_bucket_array_occupied(s->toggle_buttons,i))
        {
            continue;
        }
        
        cwsGuiToggleButton *tbtn = &cws_bucket_array_index(s->toggle_buttons,i);
        text_center_and_clip(tbtn->text,
                            (vec2){.x = tbtn->pos.x+s->transform->pos.x,
                            .y = tbtn->pos.y+s->transform->pos.y}, tbtn->size, button_skin.outline_size);

        s->renderer->item_data.data[index++] = tbtn->pos.x;
        s->renderer->item_data.data[index++] = tbtn->pos.y;
        s->renderer->item_data.data[index++] = tbtn->size.x;
        s->renderer->item_data.data[index++] = tbtn->size.y;

        /*
            Skin data
            The first index decides which skin to use in the shader (0 for button skin)
            The second is the index of the fill color to use for the item part, a button uses 2 colors to 
            indicate the state (pressed or released)
        */
        s->renderer->item_data.data[index++] = 0;
        s->renderer->item_data.data[index++] = tbtn->toggled;
        s->renderer->item_data.data[index++] = 0.0f;
        s->renderer->item_data.data[index++] = 0.0f;
    }
    
    if(s->renderer->fill)
    {
        s->renderer->item_data.data[index++] = 0.0f;
        s->renderer->item_data.data[index++] = 0.0f;
        s->renderer->item_data.data[index++] = s->transform->size.x;
        s->renderer->item_data.data[index++] = s->transform->size.y;                
        
        s->renderer->item_data.data[index++] = 3;
        s->renderer->item_data.data[index++] = 0.0f;
        s->renderer->item_data.data[index++] = 0.0f;
        s->renderer->item_data.data[index++] = 0.0f;
    }

    glBindBuffer(GL_TEXTURE_BUFFER, s->renderer->tex_buffer);
    glBufferData(GL_TEXTURE_BUFFER, sizeof(f32)*s->renderer->item_data.length, s->renderer->item_data.data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);

    for(u32 i = 0; i < s->children.length; ++i)
    {
        update_surface_component_data(s->children.data[i]);
    }
}

//Called when the left mouse button was just clicked!
void surface_events_clicked(cwsGuiSurface *s, vec2 mouse)
{
    mouse.x -= s->transform->pos.x;
    mouse.y -= s->transform->pos.y;

    for(u32 i = 0; i < cws_bucket_array_item_count(s->buttons); ++i)
    {
        if(!cws_bucket_array_occupied(s->buttons,i))
        {
            continue;
        }
        
        cwsGuiButton *btn = &cws_bucket_array_index(s->buttons, i);
        //Mouse is not above button
        if(mouse.x < btn->pos.x || mouse.x > btn->pos.x + btn->size.x
        || mouse.y < btn->pos.y || mouse.y > btn->pos.y + btn->size.y)
        {
            continue;
        }

        btn->event_flags |= EVENT_CLICKED;
    }

    for(u32 i = 0; i < cws_bucket_array_item_count(s->sliders); ++i)
    {
        if(!cws_bucket_array_occupied(s->sliders,i))
        {
            continue;
        }
        
        cwsGuiSlider *slider = &cws_bucket_array_index(s->sliders, i);
        //Mouse is not above slider
        if(mouse.x < slider->pos.x - 1 || mouse.x > slider->pos.x + slider->size.x + 1
        || mouse.y < slider->pos.y - 1 || mouse.y > slider->pos.y + slider->size.y + 1)
        {
            continue;
        }

        i32 localX = mouse.x - slider->pos.x;
        slider->event_flags |= EVENT_SLIDER_CHANGED;
        slider->value = ((f32)localX/(f32)slider->size.x) * (slider->max-slider->min) + slider->min;

        char buf[8];
        sprintf(buf, "%d", slider->value);
        cwsRebuildText(s->renderer->text_context, slider->text, buf);
    }

    for(u32 i = 0; i < cws_bucket_array_item_count(s->checkboxes); ++i)
    {
        if(!cws_bucket_array_occupied(s->checkboxes,i))
        {
            continue;
        }
        
        cwsGuiCheckbox *checkbox = &cws_bucket_array_index(s->checkboxes,i);
        
        //Mouse is not above checkbox
        if(mouse.x < checkbox->pos.x + ((f32)checkbox->size.y*checkbox_skin.mark_offset.x) || mouse.x > checkbox->pos.x + ((f32)checkbox->size.y*checkbox_skin.mark_offset.x) + (checkbox->size.y*checkbox_skin.mark_scale)
        || mouse.y < checkbox->pos.y + ((f32)checkbox->size.y*checkbox_skin.mark_offset.y) || mouse.y > checkbox->pos.y + ((f32)checkbox->size.y*checkbox_skin.mark_offset.y) + (checkbox->size.y*checkbox_skin.mark_scale))
        {
            continue;
        }

        checkbox->checked ^= 1;
        checkbox->event_flags |= EVENT_CLICKED;
    }

    for(u32 i = 0; i < cws_bucket_array_item_count(s->toggle_buttons); ++i)
    {
        if(!cws_bucket_array_occupied(s->toggle_buttons,i))
        {
            continue;
        }
        
        cwsGuiToggleButton *tbtn = &cws_bucket_array_index(s->toggle_buttons,i);
        
        //Mouse is not above checkbox
        if(mouse.x < tbtn->pos.x || mouse.x > tbtn->pos.x + tbtn->size.x
        || mouse.y < tbtn->pos.y || mouse.y > tbtn->pos.y + tbtn->size.y)
        {
            continue;
        }

        tbtn->toggled ^= 1;
        tbtn->event_flags |= EVENT_CLICKED;
    }
}

//Called when the left mouse is held down
void surface_events_down(cwsGuiSurface *s, vec2 mouse)
{
    mouse.x -= s->transform->pos.x;
    mouse.y -= s->transform->pos.y;

    for(size_t i = 0; i < cws_bucket_array_item_count(s->buttons); ++i)
    {
        if(!cws_bucket_array_occupied(s->buttons,i))
        {
            continue;
        }
        
        cwsGuiButton *btn = &cws_bucket_array_index(s->buttons, i);
        //Mouse is not above button
        if(mouse.x < btn->pos.x || mouse.x > btn->pos.x + btn->size.x
        || mouse.y < btn->pos.y || mouse.y > btn->pos.y + btn->size.y)
        {
            continue;
        }

        if((btn->event_flags & EVENT_CLICKED))
        {
            btn->event_flags &= ~EVENT_CLICKED;
            btn->event_flags |= EVENT_MOUSE_DOWN;
        }
    }

    for(u32 i = 0; i < cws_bucket_array_item_count(s->sliders); ++i)
    {
        if(!cws_bucket_array_occupied(s->sliders,i))
        {
            continue;
        }
        
        cwsGuiSlider *slider = &cws_bucket_array_index(s->sliders, i);
        //Mouse is not above slider
        if(mouse.x < slider->pos.x - 1 || mouse.x > slider->pos.x + slider->size.x + 1
        || mouse.y < slider->pos.y - 1  || mouse.y > slider->pos.y + slider->size.y + 1)
        {
            continue;
        }

        i32 localX = mouse.x - slider->pos.x;
        slider->event_flags |= EVENT_SLIDER_CHANGED;
        slider->value = ((f32)localX/(f32)slider->size.x) * (slider->max-slider->min) + slider->min;
        
        char buf[8];
        sprintf(buf, "%d", slider->value);
        cwsRebuildText(s->renderer->text_context, slider->text, buf);
    }

    for(u32 i = 0; i < cws_bucket_array_item_count(s->checkboxes); ++i)
    {
        if(!cws_bucket_array_occupied(s->checkboxes,i))
        {
            continue;
        }
        
        cwsGuiCheckbox *checkbox = &cws_bucket_array_index(s->checkboxes,i);
        
        //Mouse is not above checkbox
        if(mouse.x < checkbox->pos.x + ((f32)checkbox->size.y*checkbox_skin.mark_offset.x) || mouse.x > checkbox->pos.x + ((f32)checkbox->size.y*checkbox_skin.mark_offset.x) + (checkbox->size.y*checkbox_skin.mark_scale)
        || mouse.y < checkbox->pos.y + ((f32)checkbox->size.y*checkbox_skin.mark_offset.y) || mouse.y > checkbox->pos.y + ((f32)checkbox->size.y*checkbox_skin.mark_offset.y) + (checkbox->size.y*checkbox_skin.mark_scale))
        {
            continue;
        }

        checkbox->event_flags &= ~EVENT_CLICKED;
    }

    for(u32 i = 0; i < cws_bucket_array_item_count(s->toggle_buttons); ++i)
    {
        if(!cws_bucket_array_occupied(s->toggle_buttons,i))
        {
            continue;
        }
        
        cwsGuiToggleButton *tbtn = &cws_bucket_array_index(s->toggle_buttons,i);
        
        //Mouse is not above checkbox
        if(mouse.x < tbtn->pos.x || mouse.x > tbtn->pos.x + tbtn->size.x
        || mouse.y < tbtn->pos.y || mouse.y > tbtn->pos.y + tbtn->size.y)
        {
            continue;
        }

        tbtn->event_flags &= ~EVENT_CLICKED;
    }
}

//Called when the left mouse is released
void surface_events_released(cwsGuiSurface *s, vec2 mouse)
{
    for(u32 i = 0; i < cws_bucket_array_item_count(s->buttons); ++i)
    {
        if(!cws_bucket_array_occupied(s->buttons,i))
        {
            continue;
        }
        cwsGuiButton *btn = &cws_bucket_array_index(s->buttons, i);
        btn->event_flags &= ~EVENT_CLICKED;
        btn->event_flags &= ~EVENT_MOUSE_DOWN;
    }

    for(u32 i = 0; i < cws_bucket_array_item_count(s->checkboxes); ++i)
    {
        if(!cws_bucket_array_occupied(s->checkboxes, i))
        {
            continue;
        }
        
        cws_bucket_array_index(s->checkboxes,i).event_flags &= ~EVENT_CLICKED;
    }

    for(u32 i = 0; i < cws_bucket_array_item_count(s->toggle_buttons); ++i)
    {
        if(!cws_bucket_array_occupied(s->toggle_buttons,i))
        {
            continue;
        }
        
        cws_bucket_array_index(s->toggle_buttons, i).event_flags &= ~EVENT_CLICKED;
    }
}

void cwsGuiUpdate()
{
    if(get_mouse_state(0) != 0)
    {
        last_global_mouse_state[0] = get_mouse_state(0);
    }
    
    //Register internal mouse events
    if (last_global_mouse_state[0] == MOUSE_PRESSED)
    {
        if(internal_mouse_states[0] == GMOUSE_NONE || internal_mouse_states[0] == GMOUSE_RELEASED)
        {
            internal_mouse_states[0] = GMOUSE_CLICKED;
        }
        else
        {
            internal_mouse_states[0] = GMOUSE_DOWN;
        }
    }
    //Keep the released event only for 1 frame!
    else if(internal_mouse_states[0] != GMOUSE_RELEASED)
    {
        internal_mouse_states[0] = GMOUSE_RELEASED;
    }
    else
    {
        internal_mouse_states[0] = GMOUSE_NONE;
    }

    vec2 mp = get_mouse_position();
    bool reset = false;
    for(u32 i = 0; i < visible_surfaces.length; ++i)
    {
        update_surface_component_data(visible_surfaces.data[i]);
            
        if(internal_mouse_states[0] == GMOUSE_CLICKED)
        {
            surface_events_clicked(visible_surfaces.data[i], get_mouse_position());
        }
        else if(internal_mouse_states[0] == GMOUSE_DOWN)
        {
            surface_events_down(visible_surfaces.data[i], get_mouse_position());
        }
        else if(internal_mouse_states[0] == GMOUSE_RELEASED)
        {
            surface_events_released(visible_surfaces.data[i], get_mouse_position());
        }
        
        if(!(mp.x < visible_surfaces.data[i]->transform->pos.x ||
             mp.x > visible_surfaces.data[i]->transform->pos.x + visible_surfaces.data[i]->transform->size.x ||
             mp.y < visible_surfaces.data[i]->transform->pos.y ||
             mp.y > visible_surfaces.data[i]->transform->pos.y + visible_surfaces.data[i]->transform->size.y))
        {
            reset = true;
        }
    }
    
    if(reset)
    {
        reset_mouse_state();
    }
}

void draw_surface(cwsSurfaceRenderer *s, vec3 parent_pos, vec2 parent_size)
{
    cwsBindMaterial(&material);

    /*
        Buffer skin data

        This should be changed to a uniform buffer and only update once every frame!
    */
    vec3 fill0 = cwsUnpackRgb(button_skin.fill_color);
    vec3 fill1 = cwsUnpackRgb(button_skin.fill_color_clicked);
    vec3 outline0 = cwsUnpackRgb(button_skin.outline_color);

    glUniform3f(glGetUniformLocation(material.shader.id, "SkinData[0].fill_color[0]"),  fill0.x, fill0.y, fill0.z);
    glUniform3f(glGetUniformLocation(material.shader.id, "SkinData[0].fill_color[1]"),  fill1.x, fill1.y, fill1.z);
    glUniform3f(glGetUniformLocation(material.shader.id, "SkinData[0].outline_color"),  outline0.x, outline0.y, outline0.z);
    glUniform1f(glGetUniformLocation(material.shader.id, "SkinData[0].outline_size"),   button_skin.outline_size);

    fill0 = cwsUnpackRgb(slider_skin.fill_color);
    fill1 = cwsUnpackRgb(slider_skin.marker_color);
    outline0 = cwsUnpackRgb(slider_skin.outline_color);
    glUniform3f(glGetUniformLocation(material.shader.id, "SkinData[1].fill_color[0]"),  fill0.x, fill0.y, fill0.z);
    glUniform3f(glGetUniformLocation(material.shader.id, "SkinData[1].fill_color[1]"),  fill1.x, fill1.y, fill1.z);
    glUniform3f(glGetUniformLocation(material.shader.id, "SkinData[1].outline_color"),  outline0.x, outline0.y, outline0.z);
    glUniform1f(glGetUniformLocation(material.shader.id, "SkinData[1].outline_size"),   slider_skin.outline_size);

    fill0 = cwsUnpackRgb(checkbox_skin.fill_color);
    fill1 = cwsUnpackRgb(checkbox_skin.marker_color);
    vec3 fill2 = cwsUnpackRgb(checkbox_skin.marker_color_checked);
    outline0 = cwsUnpackRgb(checkbox_skin.outline_color);
    glUniform3f(glGetUniformLocation(material.shader.id, "SkinData[2].fill_color[0]"),   fill0.x, fill0.y, fill0.z);
    glUniform3f(glGetUniformLocation(material.shader.id, "SkinData[2].fill_color[1]"),   fill1.x, fill1.y, fill1.z);
    glUniform3f(glGetUniformLocation(material.shader.id, "SkinData[2].fill_color[2]"),   fill2.x, fill2.y, fill2.z);
    glUniform3f(glGetUniformLocation(material.shader.id, "SkinData[2].outline_color"),   outline0.x, outline0.y, outline0.z);
    glUniform1f(glGetUniformLocation(material.shader.id, "SkinData[2].outline_size"),    checkbox_skin.outline_size);

    fill0 = cwsUnpackRgb(surface_skin.fill_color);
    outline0 = cwsUnpackRgb(surface_skin.outline_color);

    glUniform3f(glGetUniformLocation(material.shader.id, "SkinData[3].fill_color[0]"),  fill0.x, fill0.y, fill0.z);
    glUniform3f(glGetUniformLocation(material.shader.id, "SkinData[3].outline_color"),  outline0.x, outline0.y, outline0.z);
    glUniform1f(glGetUniformLocation(material.shader.id, "SkinData[3].outline_size"),   surface_skin.outline_size);

    //Combined position of this surface & top visible_surfaces
    vec3 pp = (vec3){.x = (f32)parent_pos.x, .y = (f32)parent_pos.y, .z = (f32)parent_pos.z};
    vec3 sp = (vec3){.x = (f32)s->transform->pos.x, .y = (f32)s->transform->pos.y, .z = (f32)s->transform->pos.z};
    vec3 comb = vec3_add(sp, pp);

    //Calculates the maximum size the surface can be when contained within other surface(s)
    f32 width = (comb.x+s->transform->size.x) > parent_size.x ? parent_size.x - comb.x : s->transform->size.x;
    f32 height = (comb.y+s->transform->size.y) > parent_size.y ? parent_size.y - comb.y : s->transform->size.y;

    glScissor(comb.x,cwsScreenSize().y-(comb.y+s->transform->size.y), width, height);
    mat4 t = mat4_translate(mat4_default, comb);

    //The first texture unit will have the detail texture bound to it!
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER, s->tex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32F, s->tex_buffer);
    glUniform1i(glGetUniformLocation(material.shader.id, "tex_buffer"), 0);

    cwsBindMesh(s->mesh);
    cwsDrawMesh(t, s->mesh, GL_TRIANGLES);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER, 0);

    for(u32 i = 0; i < s->children.length; ++i)
    {
        draw_surface(s->children.data[i], comb, s->transform->size);
    }
}

void cwsGuiDraw()
{
    glClear(GL_DEPTH_BUFFER_BIT);
    glDisable(GL_CULL_FACE);
    glEnable(GL_SCISSOR_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    mat4 id = mat4_default;
    cwsSetPVMatrices(ortho, id);

    for(u32 i = 0; i < visible_surfaces.length; ++i)
    {
        draw_surface(visible_surfaces.data[i]->renderer, (vec3){.x = 0, .y = 0, .z = 0}, cwsScreenSize());

        glDisable(GL_DEPTH_TEST);
        cwsDrawTextContext(visible_surfaces.data[i]->renderer->text_context);
        glEnable(GL_DEPTH_TEST);
    }

    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
    glEnable(GL_CULL_FACE);
}
