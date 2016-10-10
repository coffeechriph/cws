#ifndef EE_LUA_H
#define EE_LUA_H

/*
	Luajit ffi can directly bind to C functions this is done in luabind.lua.
	However the engine makes use of certain C++ specific functions which makes binding harder.

	Tiny struct wrappers are created around a pointer to a C++ class/struct, wrapper functions are also created 
	which makes use of the wrappers to allow luajit to bind to them. 
*/

#include "stdio.h"
#include "SDL2/SDL.h"
#include "../util/log.h"
#include "../util/input.h"
#include "../gfx/video.h"
#include "../scene/scene.h"
#include "../gui/gui.h"
#include "luajit-2.1/lua.h"
#include "luajit-2.1/lualib.h"
#include "luajit-2.1/lauxlib.h"
#include "luajit-2.1/luajit.h"
#include "../types.h"
#include "../math/matrix.h"

extern bool cws_running;

bool lua_init();
bool lua_destroy();
bool lua_runfile(const char *file);

extern i32 Lua_NEAREST;
extern i32 Lua_LINEAR;

bool 			cws_create(unsigned int scrcwsnW, unsigned int scrcwsnH, const char *title, bool vsync);
void 			cws_close();
void 			cws_run();

bool 			el_is_running();
void			el_stop_running();
void 			el_run();

vec2 			el_screen_size();
void 			el_clear_color(vec3 color);

float 			el_findnoise(float x, float y);

/*======================
	Mesh Bindings
========================*/
Mesh*			el_new_mesh(i32 *vertex_attribs, i32 length);
Mesh*			el_new_plane_mesh();
Mesh*			el_new_cube_mesh();
Mesh*			el_new_mesh_fromfile(const char *file);
void 			el_delete_mesh(Mesh *mesh); //Deletes the mesh - makes it unusable
void 			el_fill_mesh(Mesh *mesh, f32 *vertex_data, i32 vcount, i32 *index_data, i32 icount);
void 			el_fill_submesh(Mesh *mesh, f32 *vertex_data, i32 voffset, i32 vlength, i32 *index_data, i32 ioffset, i32 ilength);
void            el_mesh_set_bounds(Mesh *mesh, vec3 min, vec3 max);
vec3            el_mesh_get_min(Mesh *mesh);
vec3            el_mesh_get_max(Mesh *mesh);
Animation* 		el_mesh_get_animation(Mesh *m);

/*
	Animation
*/
void 			el_animation_set_frame_count(Animation *a, i32 c);
void 			el_animation_set_speed(Animation *a, f32 s);
void 			el_animation_add_clip(Animation *a, i32 start, i32 end);

/*======================
	Material Bindings
========================*/
Material*		el_new_material();
void 			el_delete_material(Material *mat);
void 			el_material_set_textures(Material *mat, Texture2D** tex, i32 size);
void 			el_material_set_shader(Material *mat, Shader* shader);
void 			el_material_set_color(Material *mat, vec3 color);
void 			el_material_set_rflags(Material *mat, i32 f);

/*======================
	Shader Bindings
========================*/
Shader* 		el_new_shader_fromfile(const char *vertex_file, const char *fragment_file);
Shader* 		el_new_shader_fromsrc(const char *vertex_src, const char *fragment_src);
void 			el_delete_shader(Shader* shader);

/*======================
	Texture Bindings
========================*/
Texture2D* 		el_new_texture_fromfile(const char *file, i32 filter);
void 			el_delete_texture(Texture2D* tex);

/*======================
	Gui Surface
========================*/
GuiSurface* 	elg_new_surface(GuiSurface *parent);
void 			elg_delete_surface(GuiSurface* s);
void 			elg_surface_set_visibility(GuiSurface *s, bool c);
void			elg_surface_set_fill(GuiSurface *s, bool c);
void 			elg_surface_clear(GuiSurface *s);
void 			elg_surface_refresh(GuiSurface *s);

vec3 			elg_surface_get_position(GuiSurface* c);
void 			elg_surface_set_position(GuiSurface* c, vec2 p);

vec2 			elg_surface_get_size(GuiSurface* c);
void 			elg_surface_set_size(GuiSurface* c, vec2 p);

/*======================
	Gui Text
========================*/
Text*					elg_new_text(GuiSurface *s, vec2 pos, vec2 scale, const char *str);

/*======================
	GuiButton
========================*/
GuiButton* 		elg_new_button(GuiSurface* s);
vec2 			elg_button_get_position(GuiButton* c);
void 			elg_button_set_position(GuiButton* c, vec2 p);

vec2 			elg_button_get_size(GuiButton* c);
void 			elg_button_set_size(GuiButton* c, vec2 p);

bool 			elg_button_clicked(GuiButton* c);
bool 			elg_button_hovered(GuiButton* c);
bool 			elg_button_mouse_enter(GuiButton* c);
bool 			elg_button_mouse_leave(GuiButton* c);
bool 			elg_button_focused(GuiButton* c);

void 			elg_button_set_text(GuiButton *c, const char *text);

/*========================
	GuiSlider
==========================*/
GuiSlider* 		elg_new_slider(GuiSurface* s);
vec2 			elg_slider_get_position(GuiSlider* c);
void 			elg_slider_set_position(GuiSlider* c, vec2 p);

vec2 			elg_slider_get_size(GuiSlider* c);
void 			elg_slider_set_size(GuiSlider* c, vec2 p);

bool 			elg_slider_clicked(GuiSlider* c);
bool 			elg_slider_hovered(GuiSlider* c);
bool 			elg_slider_mouse_enter(GuiSlider* c);
bool 			elg_slider_mouse_leave(GuiSlider* c);
bool 			elg_slider_focused(GuiSlider* c);
bool 			elg_slider_value_changed(GuiSlider* c);
void			elg_slider_set_value(GuiSlider* c, i32 v);
i32 			elg_slider_get_value(GuiSlider* c);

void			elg_slider_set_range(GuiSlider *c, i32 min, i32 max);
/*========================
	Gui GuiCheckbox
==========================*/
GuiCheckbox* 	elg_new_checkbox(GuiSurface* s);
vec2 			elg_checkbox_get_position(GuiCheckbox* c);
void 			elg_checkbox_set_position(GuiCheckbox* c, vec2 p);

vec2 			elg_checkbox_get_size(GuiCheckbox* c);
void 			elg_checkbox_set_size(GuiCheckbox* c, vec2 p);

bool 			elg_checkbox_clicked(GuiCheckbox* c);
bool 			elg_checkbox_hovered(GuiCheckbox* c);
bool 			elg_checkbox_mouse_enter(GuiCheckbox* c);
bool 			elg_checkbox_mouse_leave(GuiCheckbox* c);
bool 			elg_checkbox_focused(GuiCheckbox* c);

void 			elg_checkbox_set_text(GuiCheckbox *c, const char *text);
bool 			elg_checkbox_toggled(GuiCheckbox* c);
bool 			elg_checkbox_get_toggle(GuiCheckbox* c);
void 			elg_checkbox_set_toggle(GuiCheckbox* c, bool b);

/*========================
	Gui GuiToggleButton
==========================*/
GuiToggleButton* 	elg_new_toggle_button(GuiSurface* s);
vec2 				elg_toggle_button_get_position(GuiToggleButton* c);
void 				elg_toggle_button_set_position(GuiToggleButton* c, vec2 p);

vec2 				elg_toggle_button_get_size(GuiToggleButton* c);
void 				elg_toggle_button_set_size(GuiToggleButton* c, vec2 p);

bool 				elg_toggle_button_clicked(GuiToggleButton* c);
bool 				elg_toggle_button_hovered(GuiToggleButton* c);
bool 				elg_toggle_button_mouse_enter(GuiToggleButton* c);
bool 				elg_toggle_button_mouse_leave(GuiToggleButton* c);
bool 				elg_toggle_button_focused(GuiToggleButton* c);

void 				elg_toggle_button_set_text(GuiToggleButton *c, const char *text);
bool 				elg_toggle_button_toggled(GuiToggleButton* c);
bool 				elg_toggle_button_get_toggle(GuiToggleButton* c);
void 				elg_toggle_button_set_toggle(GuiToggleButton* c, bool b);

/*===============================
	Renderer
=================================*/
void			el_renderer_update_bounds(Renderer *obj);
Renderer*		el_new_renderer(Material *mat, Mesh *mesh);
void			el_remove_renderer(Renderer *r);
Mesh*			el_renderer_get_mesh(Renderer *obj);
Material*		el_renderer_get_material(Renderer *obj);
vec3 			el_renderer_get_pos(Renderer *obj);
void			el_renderer_set_pos(Renderer *obj, vec3 p);
vec3 			el_renderer_get_scale(Renderer *obj);
void			el_renderer_set_scale(Renderer *obj, vec3 p);
quat 			el_renderer_get_rot(Renderer *obj);
void			el_renderer_set_rot_euler(Renderer *obj, vec3 r);
void			el_renderer_set_rot(Renderer *obj, quat q);
vec3 			el_renderer_get_min(Renderer *obj);
vec3 			el_renderer_get_max(Renderer *obj);
void			el_renderer_set_color(Renderer *obj, vec3 c);
void 			el_renderer_set_animation_clip_index(Renderer *obj, i32 i);
void			el_renderer_set_visibility(Renderer *obj, bool c);
void 			el_renderer_make_static(Renderer *obj);

/*======================
	Camera Bindings
========================*/
Camera* 		el_new_camera();
Camera* 		el_get_active_camera();
void 			el_set_active_camera(Camera* c);
vec3 			el_camera_get_pos(Camera* obj);
void 			el_camera_set_pos(Camera* obj, vec3 pos);
void 			el_camera_set_rot(Camera* obj, vec3 r);
vec3 			el_camera_get_rot(Camera* obj);
vec3 			el_camera_get_dir(Camera* obj);
void 			el_camera_set_perspective(Camera* obj, f32 fov, f32 near, f32 far);
ray el_camera_build_pick_ray(Camera* cam);

/*======================
	DirLight Bindings
========================*/
DirLight* 		el_new_dir_light();
void 			el_delete_dir_light(DirLight* l);
void 			el_dir_light_set_rot(DirLight* obj, vec3 rot);
void 			el_dir_light_set_color(DirLight* obj, vec3 color);
vec3 			el_dir_light_get_rot(DirLight* obj);

/*======================
	SpotLight Bindings
========================*/
SpotLight* 		el_new_spot_light();
void 			el_delete_spot_light(SpotLight* l);
void 			el_spot_light_set_pos(SpotLight* obj, vec3 pos);
void 			el_spot_light_set_rot(SpotLight* obj, vec3 rot);
void 			el_spot_light_set_color(SpotLight* obj, vec3 color);
void 			el_spot_light_set_cutoff(SpotLight* obj, f32 c);
void 			el_spot_light_set_edge_factor(SpotLight* obj, f32 e);
void 			el_spot_light_set_length(SpotLight* obj, f32 e);
vec3 			el_spot_light_get_pos(SpotLight* obj);
vec3 			el_spot_light_get_rot(SpotLight* obj);

/*======================
	PointLight Bindings
========================*/
PointLight* 	el_new_point_light();
void 			el_delete_point_light(PointLight* l);
vec3 			el_point_light_get_pos(PointLight* obj);
void 			el_point_light_set_pos(PointLight* obj, vec3 pos);
void 			el_point_light_set_radius(PointLight* obj, f32 x);
void 			el_point_light_set_color(PointLight* obj, vec3 color);

#endif