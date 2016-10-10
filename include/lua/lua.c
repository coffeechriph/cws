#include "lua.h"
	
i32 Lua_NEAREST = GL_NEAREST;
i32 Lua_LINEAR = GL_LINEAR;

lua_State *EE_lua_state;

bool lua_init()
{
	EE_lua_state = luaL_newstate();
	luaL_openlibs(EE_lua_state);

    if(luaL_dofile(EE_lua_state, "./data/lua_bind.lua") != 0)
    {
    	cws_log("%s\n", lua_tostring(EE_lua_state, -1));
    	return false;
    }

    return true;
}

bool lua_destroy()
{
	lua_close(EE_lua_state);
	return true;
}

bool lua_runfile(const char *file)
{
	if(luaL_dofile(EE_lua_state, file) != 0)
	{
		cws_log("%s", lua_tostring(EE_lua_state, -1));
		return false;
	}

	return true;
}

void el_run()
{
	video_clear();
	scene_draw();
	gui_draw();
	video_swap_buffers();
	cws_run();
}

float el_findnoise(float x, float y)
{
	int n=(int)x+(int)y*57;
	n=(n<<13)^n;
	int nn=(n*(n*n*60493+19990303)+1376312589)&0x7fffffff;
	return 1.0f-((float)nn/1073741824.0f);
}

bool el_is_running()
{
	return cws_running;
}

void el_stop_running()
{
	cws_running = false;
}

vec2 el_screen_size()
{
	return screen_size();
}

/*
  __  __             _     
 |  \/  |  ___  ___ | |__  
 | |\/| | / _ \/ __|| '_ \ 
 | |  | ||  __/\__ \| | | |
 |_|  |_| \___||___/|_| |_|

*/
Mesh *el_new_mesh(i32 *vertex_attribs, i32 length)
{
	return new_mesh(vertex_attribs, length);
}

Mesh *el_new_plane_mesh()
{
	return new_plane_mesh();
}

Mesh *el_new_cube_mesh()
{
	return new_cube_mesh();
}

Mesh *el_new_mesh_fromfile(const char *file)
{
	return new_mesh_fromfile(file);
}

void el_delete_mesh(Mesh *mesh)
{
	delete_mesh(mesh);
}

void el_fill_mesh(Mesh *mesh, f32 *vertex_data, i32 vcount, i32 *index_data, i32 icount)
{
	fill_mesh(mesh,vertex_data,vcount,index_data,icount);
}

void el_fill_submesh(Mesh *mesh, f32 *vertex_data, i32 voffset, i32 vlength, i32 *index_data, i32 ioffset, i32 ilength)
{
	fill_submesh(mesh,vertex_data,voffset,vlength,index_data,ioffset,ilength);
}

void el_mesh_set_bounds(Mesh *mesh, vec3 min, vec3 max)
{
	mesh->minB = min;
	mesh->maxB = max;
}

vec3 el_mesh_get_min(Mesh *mesh)
{
	return mesh->minB;
}

vec3 el_mesh_get_max(Mesh *mesh)
{
	return mesh->maxB;
}

Animation* el_mesh_get_animation(Mesh *m)
{
	if(m->anim_data == NULL || m->anim_data->animations_count == 0)
		return NULL;

	return m->anim_data->animations[0];
}

/*
	Animation
*/
void el_animation_set_frame_count(Animation *a, i32 c)
{
	a->frame_factor = a->duration / c;
}

void el_animation_set_speed(Animation *a, f32 s)
{

}

void el_animation_add_clip(Animation *a, i32 start, i32 end)
{
	Animation_Clip clip;
	clip.start_frame = start;
	clip.end_frame = end;
	if(a->clips == NULL)
	{
		a->clips = malloc(sizeof(Animation_Clip));
		if(a->clips == NULL)
		{
			cws_log("Error allocating space for Animation_Clip!");
			return;
		}
		a->clips_size = 1;
		a->clips_count = 1;
		a->clips[0] = clip;
	}
	else if(a->clips_count >= a->clips_size)
	{
		Animation_Clip* d = realloc(a->clips, sizeof(Animation_Clip)*(a->clips_size+1));
		if(d == NULL)
		{
			cws_log("Error allocating more space for Animation_Clip!");
			return;
		}

		a->clips = d;
		a->clips_size += 1;
		a->clips_count = 0;
		a->clips[a->clips_count++] = clip;
	}
}

/***
 *      __  __         _               _         _ 
 *     |  \/  |  __ _ | |_  ___  _ __ (_)  __ _ | |
 *     | |\/| | / _` || __|/ _ \| '__|| | / _` || |
 *     | |  | || (_| || |_|  __/| |   | || (_| || |
 *     |_|  |_| \__,_| \__|\___||_|   |_| \__,_||_|
 *                                                 
 */
Material *el_new_material()
{
	return new_material();
}

void el_delete_material(Material *mat)
{
	delete_material(mat);
}

void el_material_set_textures(Material *mat, Texture2D** tex, i32 size)
{
	material_set_textures(mat, tex, size);
}

void el_material_set_shader(Material *mat, Shader* shader)
{
	mat->shader = shader;
}

void el_material_set_color(Material *mat, vec3 color)
{
	mat->color.x = color.x;
	mat->color.y = color.y;
	mat->color.z = color.z;
}

void el_material_set_rflags(Material *mat, i32 f)
{
	mat->rflags = f;
}

/***
 *      ____   _                 _             
 *     / ___| | |__    __ _   __| |  ___  _ __ 
 *     \___ \ | '_ \  / _` | / _` | / _ \| '__|
 *      ___) || | | || (_| || (_| ||  __/| |   
 *     |____/ |_| |_| \__,_| \__,_| \___||_|   
 *                                             
 */
Shader* el_new_shader_fromfile(const char *vertex_file, const char *fragment_file)
{
	return new_shader_fromfile(vertex_file, fragment_file);
}

Shader* el_new_shader_fromsrc(const char *vertex_src, const char *fragment_src)
{
	return new_shader_fromsrc(vertex_src, fragment_src);
}

void el_delete_shader(Shader* shader)
{
	delete_shader(shader);
}

/***
 *      _____            _                     
 *     |_   _|___ __  __| |_  _   _  _ __  ___ 
 *       | | / _ \\ \/ /| __|| | | || '__|/ _ \
 *       | ||  __/ >  < | |_ | |_| || |  |  __/
 *       |_| \___|/_/\_\ \__| \__,_||_|   \___|
 *                                             
 */
Texture2D* el_new_texture_fromfile(const char *file, i32 filter)
{
	return new_texture_fromfile(file, filter);
}

void el_delete_texture(Texture2D* tex)
{
	delete_texture(tex);
}

void el_clear_color(vec3 color)
{
	clear_color(color);
}

/***
 *       ____       _ 
 *      / ___|_   _(_)
 *     | |  _| | | | |
 *     | |_| | |_| | |
 *      \____|\__,_|_|
 *                    
 */
GuiSurface* elg_new_surface(GuiSurface *parent)
{
	return new_surface(parent);
}

void elg_delete_surface(GuiSurface* s)
{
	delete_surface(s);
}

void elg_surface_set_visibility(GuiSurface *s, bool c)
{
	surface_set_visibility(s,c);
}

void elg_surface_refresh(GuiSurface *s)
{
	refresh_surface(s);
}

void elg_surface_clear(GuiSurface *s)
{
	clear_surface(s);
}

vec3 elg_surface_get_position(GuiSurface* c)
{
	return c->transform->pos;
}

void elg_surface_set_position(GuiSurface* c, vec2 p)
{
	c->transform->pos.x = p.x;
	c->transform->pos.y = p.y;
}

vec2 elg_surface_get_size(GuiSurface* c)
{
	return c->transform->size;
}

void elg_surface_set_size(GuiSurface* c, vec2 p)
{
	c->transform->size = p;
}

void elg_surface_set_fill(GuiSurface *s, bool c)
{
	s->renderer->fill = c;
}

Text* elg_new_text(GuiSurface *s, vec2 pos, vec2 scale, const char *str)
{
	return new_gui_text(s,pos,scale,str);
}

/*
	BUTTON
*/
GuiButton* elg_new_button(GuiSurface* s)
{
	return new_button(s);
}

bool elg_button_clicked(GuiButton* c)
{
	return (c->event_flags & EVENT_CLICKED);
}

bool elg_button_hovered(GuiButton* c)
{
	return (c->event_flags & EVENT_HOVER);
}

bool elg_button_mouse_enter(GuiButton* c)
{
	return (c->event_flags & EVENT_MOUSE_ENTER);
}

bool elg_button_mouse_leave(GuiButton* c)
{
	return (c->event_flags & EVENT_MOUSE_LEAVE);
}

bool elg_button_focused(GuiButton* c)
{
	return (c->event_flags & EVENT_FOCUSED);	
}

void elg_button_set_text(GuiButton *c, const char *text)
{
	rebuild_text(c->text->context, c->text, text);
}

vec2 elg_button_get_position(GuiButton* c)
{
	vec2 v;
	v.x = c->pos.x;
	v.y = c->pos.y;
	return v;
}

void elg_button_set_position(GuiButton* c, vec2 p)
{
	c->pos = p;
}

vec2 elg_button_get_size(GuiButton* c)
{
	return c->size;
}

void elg_button_set_size(GuiButton* c, vec2 p)
{
	c->size = p;
}

/*
	SLIDER
*/
GuiSlider* elg_new_slider(GuiSurface* s)
{
	return new_slider(s);
}

bool elg_slider_value_changed(GuiSlider* c)
{
	return (c->event_flags & EVENT_SLIDER_CHANGED);	
}

bool elg_slider_clicked(GuiSlider* c)
{
	return (c->event_flags & EVENT_CLICKED);
}

bool elg_slider_hovered(GuiSlider* c)
{
	return (c->event_flags & EVENT_HOVER);
}

bool elg_slider_mouse_enter(GuiSlider* c)
{
	return (c->event_flags & EVENT_MOUSE_ENTER);
}

bool elg_slider_mouse_leave(GuiSlider* c)
{
	return (c->event_flags & EVENT_MOUSE_LEAVE);
}

bool elg_slider_focused(GuiSlider* c)
{
	return (c->event_flags & EVENT_FOCUSED);	
}

void elg_slider_set_text(GuiSlider *c, const char *text)
{
	rebuild_text(c->text->context, c->text, text);
}

vec2 elg_slider_get_position(GuiSlider* c)
{
	return c->pos;
}

void elg_slider_set_position(GuiSlider* c, vec2 p)
{
	c->pos.x = p.x;
	c->pos.y = p.y;
}

vec2 elg_slider_get_size(GuiSlider* c)
{
	return c->size;
}

void elg_slider_set_size(GuiSlider* c, vec2 p)
{
	c->size = p;
}

i32 elg_slider_get_value(GuiSlider* c)
{
	return c->value;
}

void elg_slider_set_value(GuiSlider *c, i32 v)
{
	if(v < c->min)
	{
		c->value = 0;
	}
	else if(v > c->max)
	{
		c->value = c->max;
	}	
	else
	{
		c->value = v;
	}

	char buf[8];
    sprintf(buf, "%d", c->value);
    rebuild_text(c->text->context, c->text, buf);
}

void elg_slider_set_range(GuiSlider *c, i32 min, i32 max)
{
	if(min >= max || max <= min)
	{
		c->min = min;
		c->max = max+1;		
	}
	else
	{
		c->min = min;
		c->max = max;
	}

	if(c->value < min)
	{
		c->value = min;

		char buf[8];
	    sprintf(buf, "%d", c->value);
	    rebuild_text(c->text->context, c->text, buf);
	}
	else if(c->value > max)
	{
		c->value = max;

		char buf[8];
	    sprintf(buf, "%d", c->value);
	    rebuild_text(c->text->context, c->text, buf);	
	}
}

/*
	CHECKBOX
*/
GuiCheckbox* elg_new_checkbox(GuiSurface* s)
{
	return new_checkbox(s);
}

bool elg_checkbox_clicked(GuiCheckbox* c)
{
	return (c->event_flags & EVENT_CLICKED);
}

bool elg_checkbox_hovered(GuiCheckbox* c)
{
	return (c->event_flags & EVENT_HOVER);
}

bool elg_checkbox_mouse_enter(GuiCheckbox* c)
{
	return (c->event_flags & EVENT_MOUSE_ENTER);
}

bool elg_checkbox_mouse_leave(GuiCheckbox* c)
{
	return (c->event_flags & EVENT_MOUSE_LEAVE);
}

bool elg_checkbox_focused(GuiCheckbox* c)
{
	return (c->event_flags & EVENT_FOCUSED);	
}

void elg_checkbox_set_text(GuiCheckbox *c, const char *text)
{
	rebuild_text(c->text->context, c->text, text);
}

vec2 elg_checkbox_get_position(GuiCheckbox* c)
{
	return c->pos;
}

void elg_checkbox_set_position(GuiCheckbox* c, vec2 p)
{
	c->pos = p;
}

vec2 elg_checkbox_get_size(GuiCheckbox* c)
{
	return c->size;
}

void elg_checkbox_set_size(GuiCheckbox* c, vec2 p)
{
	c->size = p;
}

bool elg_checkbox_toggled(GuiCheckbox* c)
{
	return (c->event_flags & EVENT_CHECKBOX_CHANGED);	
}

bool elg_checkbox_get_toggle(GuiCheckbox* c)
{
	return c->checked;
}

void elg_checkbox_set_toggle(GuiCheckbox* c, bool b)
{
	c->checked = b;
}

/*========================
	Gui GuiToggleButton
==========================*/
GuiToggleButton* elg_new_toggle_button(GuiSurface* s)
{
	return new_toggle_button(s);
}

bool elg_toggle_button_clicked(GuiToggleButton* c)
{
	return (c->event_flags & EVENT_CLICKED);
}

bool elg_toggle_button_hovered(GuiToggleButton* c)
{
	return (c->event_flags & EVENT_HOVER);
}

bool elg_toggle_button_mouse_enter(GuiToggleButton* c)
{
	return (c->event_flags & EVENT_MOUSE_ENTER);
}

bool elg_toggle_button_mouse_leave(GuiToggleButton* c)
{
	return (c->event_flags & EVENT_MOUSE_LEAVE);
}

bool elg_toggle_button_focused(GuiToggleButton* c)
{
	return (c->event_flags & EVENT_FOCUSED);	
}

void elg_toggle_button_set_text(GuiToggleButton *c, const char *text)
{
	rebuild_text(c->text->context, c->text, text);
}

vec2 elg_toggle_button_get_position(GuiToggleButton* c)
{
	return c->pos;
}

void elg_toggle_button_set_position(GuiToggleButton* c, vec2 p)
{
	c->pos = p;
}

vec2 elg_toggle_button_get_size(GuiToggleButton* c)
{
	return c->size;
}

void elg_toggle_button_set_size(GuiToggleButton* c, vec2 p)
{
	c->size = p;
}

bool elg_toggle_button_toggled(GuiToggleButton* c)
{
	return (c->event_flags & EVENT_CHECKBOX_CHANGED);	
}

bool elg_toggle_button_get_toggle(GuiToggleButton* c)
{
	return c->toggled;
}

void elg_toggle_button_set_toggle(GuiToggleButton* c, bool b)
{
	c->toggled = b;
}

/*
	Renderer
*/
Renderer *el_new_renderer(Material *mat, Mesh *mesh)
{
	return new_renderer(mat,mesh);
}

void el_remove_renderer(Renderer *r)
{
	remove_renderer(r);
}

void el_renderer_update_bounds(Renderer *obj)
{
	update_bounds(obj);
}

Mesh *el_renderer_get_mesh(Renderer *obj)
{
	return renderer_get_mesh(obj);
}

Material *el_renderer_get_material(Renderer *obj)
{
	return renderer_get_material(obj);
}

vec3 el_renderer_get_min(Renderer *obj)
{
	return obj->minB;
}

vec3 el_renderer_get_max(Renderer *obj)
{
	return obj->maxB;
}

void el_renderer_set_color(Renderer *obj, vec3 c)
{
	ivec3 pc;
	pc.x = (i32)(c.x*255);
	pc.y = (i32)(c.y*255);
	pc.z = (i32)(c.z*255);
	obj->color = pack_rgb(pc);
}

void el_renderer_set_animation_clip_index(Renderer *obj, i32 i)
{
	obj->animation_clip_index = i;
}

vec3 el_renderer_get_pos(Renderer *obj)
{
	return obj->position;
}

void el_renderer_set_pos(Renderer *obj, vec3 p)
{
	obj->position = p;
}

vec3 el_renderer_get_scale(Renderer *obj)
{
	return obj->scale;
}

void el_renderer_set_scale(Renderer *obj, vec3 p)
{
	obj->scale = p;
}

quat el_renderer_get_rot(Renderer *obj)
{
	return obj->rotation;
}

void el_renderer_set_rot_euler(Renderer *obj, vec3 r)
{
	obj->rotation = quat_from_euler(r);
}

void el_renderer_set_rot(Renderer *obj, quat q)
{
	obj->rotation = q;
}

void el_renderer_set_visibility(Renderer *obj, bool c)
{
	renderer_set_visibility(obj, c);
}

void el_renderer_make_static(Renderer *obj)
{
	renderer_make_static(obj);
}

/***
 *       ____                                    
 *      / ___| __ _  _ __ ___    ___  _ __  __ _ 
 *     | |    / _` || '_ ` _ \  / _ \| '__|/ _` |
 *     | |___| (_| || | | | | ||  __/| |  | (_| |
 *      \____|\__,_||_| |_| |_| \___||_|   \__,_|
 *                                               
 */
Camera* el_new_camera()
{
	return new_camera();
}

Camera* el_get_active_camera()
{
	return get_active_camera();
}

void el_set_active_camera(Camera* c)
{
	set_active_camera(c);
}

void el_camera_set_pos(Camera* obj, vec3 pos)
{
	obj->position = pos;
}

void el_camera_set_perspective(Camera* obj, f32 fov, f32 near, f32 far)
{
	obj->fov = fov;
	obj->near_distance = near;
	obj->far_distance = far;
}
	
void el_camera_set_rot(Camera* obj, vec3 r)
{
	obj->rot = r;
}

vec3 el_camera_get_rot(Camera* obj)
{
	return obj->rot;
}

vec3 el_camera_get_pos(Camera* obj)
{
	return obj->position;
}

vec3 el_camera_get_dir(Camera* obj)
{
	return obj->dir;
}

ray el_camera_build_pick_ray(Camera* cam)
{
	return camera_build_pick_ray(cam);
}

/***
 *      ____   _        _      _         _      _   
 *     |  _ \ (_) _ __ | |    (_)  __ _ | |__  | |_ 
 *     | | | || || '__|| |    | | / _` || '_ \ | __|
 *     | |_| || || |   | |___ | || (_| || | | || |_ 
 *     |____/ |_||_|   |_____||_| \__, ||_| |_| \__|
 *                                |___/             
 */
DirLight* el_new_dir_light()
{
	return new_dir_light();
}

void el_delete_dir_light(DirLight* l)
{
	delete_dir_light(l);
}

void el_dir_light_set_rot(DirLight* obj, vec3 rot)
{
	obj->rot = rot;
}

vec3 el_dir_light_get_rot(DirLight* obj)
{
	return obj->rot;
}

void el_dir_light_set_color(DirLight* obj, vec3 color)
{
	f32 a = 1.0f / 255.0f;
	obj->color.x = color.x*a;
	obj->color.y = color.y*a;
	obj->color.z = color.z*a;
}

/***
 *      ____                 _    _      _         _      _   
 *     / ___|  _ __    ___  | |_ | |    (_)  __ _ | |__  | |_ 
 *     \___ \ | '_ \  / _ \ | __|| |    | | / _` || '_ \ | __|
 *      ___) || |_) || (_) || |_ | |___ | || (_| || | | || |_ 
 *     |____/ | .__/  \___/  \__||_____||_| \__, ||_| |_| \__|
 *            |_|                           |___/             
 */
SpotLight* el_new_spot_light()
{
	return new_spot_light();
}

void el_delete_spot_light(SpotLight* l)
{
	//delete_spot_light((SpotLight*)l);	
}

void el_spot_light_set_pos(SpotLight* obj, vec3 pos)
{
	obj->pos = pos;
}

void el_spot_light_set_rot(SpotLight* obj, vec3 rot)
{
	obj->rot = rot;
}

void el_spot_light_set_color(SpotLight* obj, vec3 color)
{
	f32 a = 1.0f / 255.0f;
	obj->color.x = color.x*a;
	obj->color.y = color.y*a;
	obj->color.z = color.z*a;
}

void el_spot_light_set_cutoff(SpotLight* obj, f32 c)
{
	i32 m = (obj->cone_data>>7) & 15;
	i32 h = (obj->cone_data>>11);
	obj->cone_data = (((i32)c) & 127) + (m<<7) + (h<<11);
}

void el_spot_light_set_edge_factor(SpotLight* obj, f32 e)
{
	i32 m = (obj->cone_data) & 127;
	i32 h = (obj->cone_data>>11);
	obj->cone_data = m + (((i32)(e)&15)<<7) + ((i32)h<<11);
}

void el_spot_light_set_length(SpotLight* obj, f32 e)
{
	i32 m = (obj->cone_data) & 127;
	i32 h = (obj->cone_data>>7) & 15;
	obj->cone_data = m + (h<<7) + ((i32)e<<11);
}

vec3 el_spot_light_get_pos(SpotLight* obj)
{
	return obj->pos;
}

vec3 el_spot_light_get_rot(SpotLight* obj)
{
	return obj->rot;
}

/***
 *      ____         _         _    _      _         _      _   
 *     |  _ \  ___  (_) _ __  | |_ | |    (_)  __ _ | |__  | |_ 
 *     | |_) |/ _ \ | || '_ \ | __|| |    | | / _` || '_ \ | __|
 *     |  __/| (_) || || | | || |_ | |___ | || (_| || | | || |_ 
 *     |_|    \___/ |_||_| |_| \__||_____||_| \__, ||_| |_| \__|
 *                                            |___/             
 */
PointLight* el_new_point_light()
{
	return new_point_light();
}

void el_delete_point_light(PointLight* l)
{
	//delete_point_light((PointLight*)l);
}

void el_point_light_set_pos(PointLight* obj, vec3 pos)
{
	obj->pos = pos;
}

void el_point_light_set_radius(PointLight* obj, f32 x)
{
	obj->radius = x;
}

void el_point_light_set_color(PointLight* obj, vec3 color)
{
	f32 a = 1.0f / 255.0f;
	obj->color.x = color.x*a;
	obj->color.y = color.y*a;
	obj->color.z = color.z*a;
}

vec3 el_point_light_get_pos(PointLight* obj)
{
	return obj->pos;
}