#ifndef EE_SCENE_H
#define EE_SCENE_H
#include "float.h"
#include "../types.h"
#include "../util/log.h"
#include "../util/input.h"
#include "../gfx/video.h"
#include "../math/collision.h"
#include "../util/cws_bucket_array.h"
#include "SDL2/SDL.h"
#define MAX_FRUSTUM_SPLITS 4

typedef struct
{
	vec3 minB, maxB;
	vec3 position;
	vec3 scale;
	quat rotation;
	i32 animation_clip_index;
	i32 group_index;
} cwsRenderer;

typedef struct
{
	cwsMaterial *material;
	cwsMesh *mesh;

	cws_bucket_array(cwsRenderer, 512) renderers;
	cws_bucket_array(cwsRenderer, 128) hidden_renderers;
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
    cws_array(cwsMesh) chunks;
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

    u32 frame_buffer;
    u32 frame_buffer_texture;
    mat4 proj;
    mat4 view[6];
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

void 				cwsUpdateBounds(cwsRenderer *r);
void				cwsRemoveRenderer(cwsRenderer *r);
void 				cwsShowRenderer(cwsRenderer *r, bool c);
void 				cwsRendererMakeStatic(cwsRenderer *r);
cwsMaterial*			cwsRendererGetMaterial(cwsRenderer *r);
cwsMesh* 				cwsRendererGetMesh(cwsRenderer *r);
cwsRenderer*			cwsNewRenderer(cwsMaterial *mat, cwsMesh *mesh);

cwsCamera* 			cwsNewCamera();
cwsCamera*				cwsActiveCamera();
void 				cwsSetActiveCamera(cwsCamera *c);
ray					cwsBuildPickRay(cwsCamera *camera);

cwsTerrain_Base*   	terrain_from_height_image(i32 chunks_x, i32 chunks_y, const char *file);        
void            	draw_terrain(cwsTerrain_Base *d, cwsCamera *cam);
void            	delete_terrain(cwsTerrain_Base *d);

cwsDirLight*			cwsNewDirLight();
cwsPointLight*			cwsNewPointLight();
cwsSpotLight*			cwsNewSpotLight();
void 				cwsDeleteDirLight(cwsDirLight *l);
void 				cwsDeletePointLight(cwsPointLight *l);
void 				cwsDeleteSpotLight(cwsSpotLight *l);

void 				cwsSceneInit();
void 				cwsSceneUpdate();
void 				cwsSceneDraw();
void 				cwsSceneDestroy();
#endif