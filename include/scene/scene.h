#ifndef EE_SCENE_H
#define EE_SCENE_H
#include "../types.h"
#include "../util/log.h"
#include "../util/input.h"
#include "../gfx/video.h"
#include "../math/collision.h"
#include "SDL2/SDL.h"
#define MAX_FRUSTUM_SPLITS 4

typedef struct
{
	vec3 minB, maxB;
	vec3 position;
	vec3 scale;
	quat rotation;
	i32 color;
	i32 animation_clip_index;
	i32 group_index;
} cwsRenderer;

typedef struct
{
	cwsMaterial *material;
	cwsMesh *mesh;

	cws_array(cwsRenderer*,renderers);
	cws_array(cwsRenderer*,hidden_renderers);
} cwsDrawGroup;

typedef struct
{
	float far_distance, near_distance;
	float fov;
	float fog_begin, fog_end;
	vec3 position;
	vec3 dir;
	vec3 rot;

	i32 __padding[2];
} cwsCamera;

typedef struct
{
	vec3 position;
	vec3 scale;
	quat rotation;
	u32 chunks_size, chunks_count;
    cws_array(cwsMesh,chunks);
    cwsMaterial material;
} cwsTerrain_Base;

typedef struct
{
	u32 frame_buffer;
	u32 frame_buffer_texture;
	u32 frame_buffer_count;
	u32 frame_buffer_size;

	mat4 frame_projections[MAX_FRUSTUM_SPLITS];
	mat4 light_view[MAX_FRUSTUM_SPLITS];
	bool shadows_enabled;
} cwsCSMData;

typedef struct
{
	vec3 rot;
	vec3 color;
} cwsDirLight;

typedef struct
{
	vec3 pos;
	vec3 color;
	f32 radius;

	i32 __padding;
} cwsPointLight;

typedef struct
{
	vec3 pos;
	vec3 rot;
	vec3 color;

	//cutoff angle, edge-factor & length
	//angle range (0 -> 127), edge-factor range(0 -> 15), length range (0 -> 2097151)
	i32 cone_data; 

	u32 frame_buffer;
	u32 frame_buffer_texture;

	i32 __padding[4];
} cwsSpotLight;

/*
	Light data for shaders to share
*/
typedef struct
{
	vec4 ambient_light;
	vec4 lights_count;

	mat4 shadow_matrices_lod[4];
	mat4 spotlight_matrices[8];

	vec4 dirlights_dir[2];
	vec4 dirlights_color[2];

	vec4 pointlights_pos[16];
	vec4 pointlights_intensity[16];

	vec4 spotlights_pos[16];
	vec4 spotlights_dir[16];
	vec4 spotlights_intensity[16];
	vec4 spotlights_conedata[16];
} cwsUniformLightData;

void 				update_bounds(cwsRenderer *r);
void				remove_renderer(cwsRenderer *r);
void 				renderer_set_visibility(cwsRenderer *r, bool c);
void 				renderer_make_static(cwsRenderer *r);
cwsMaterial*			renderer_get_material(cwsRenderer *r);
cwsMesh* 				renderer_get_mesh(cwsRenderer *r);
cwsRenderer*			new_renderer(cwsMaterial *mat, cwsMesh *mesh);

cwsCamera* 			new_camera();
cwsCamera*				get_active_camera();
void 				set_active_camera(cwsCamera *c);
ray					camera_build_pick_ray(cwsCamera *camera);

cwsTerrain_Base*   	terrain_from_height_image(i32 chunks_x, i32 chunks_y, const char *file);        
void            	draw_terrain(cwsTerrain_Base *d, cwsCamera *cam);
void            	delete_terrain(cwsTerrain_Base *d);

cwsDirLight*			new_dir_light();
cwsPointLight*			new_point_light();
cwsSpotLight*			new_spot_light();
void 				delete_dir_light(cwsDirLight *l);
void 				delete_point_light(cwsPointLight *l);
void 				delete_spot_light(cwsSpotLight *l);

void 				scene_init();
void 				scene_update();
void 				scene_draw();
void 				scene_destroy();
#endif