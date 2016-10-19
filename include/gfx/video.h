#ifndef EE_VIDEO
#define EE_VIDEO
#include <time.h>
#include "stdlib.h"
#include "float.h"
#include "GL/glew.h"
#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "../util/cws_array.h"
#include "../util/cws_string.h"
#include "../io/file.h"
#include "../math/matrix.h"
#define FONT_ASCII_CHAR_COUNT 256
typedef struct aiScene aiScene;
typedef struct aiMesh aiMesh;
typedef struct aiNode aiNode;
typedef struct aiMatrix4x4 aiMatrix4x4;

typedef enum
{
	IF_NEAREST,
	IF_LINEAR,
	IF_NEAREST_MIP_NEAREST,
	IF_LINEAR_MIP_NEAREST,
	IF_LINEAR_MIP_LINEAR
} ImageFilter;

typedef enum
{
	RF_NONE = 0,
	RF_CULLING_ENABLED = 1,
	RF_WIREFRAME_ENABLED = 2,
    RF_NO_DEPTH_TEST = 4
} RenderingFlags;

typedef enum
{
    SH_NONE = 0,
    SH_LIGHTING = 1,
    SH_SHADOWS = 2
} ShaderHeaderFlags;

typedef struct
{
	ivec2 size;
	u32 *pixels;
} cwsImage2D;

typedef struct
{
	u32 id;
    u32 mvp_id, view_id, model_id, projection_id;
    cws_array(u32, uniforms);
    cws_array(cws_string, unames);
} cwsShader;

typedef struct
{
	u32 id;
	ivec2 size;
} cwsTexture2D;

typedef struct
{
	u32 id;
	ivec2 size;
	u32 images_size, images_count;
	cwsImage2D **images;
} cwsTexture2DArray;

typedef struct
{
	cwsShader shader;
	vec3 color;

    cws_array(cwsTexture2D, texture_array);
    cws_array(u32, texture_uniforms);
	u32 rflags;
} cwsMaterial;

/*
	AnimBone stores the necessary animation data for the Nodes (If the node is part of the animation)
*/
typedef struct
{
	mat4 offset_matrix;
	mat4 transform;

	char *gpu_name;
} cwsAnimBone;

/*
	Each animation node stores the animation for a certain node
*/
typedef struct
{
	i32 positions_count;
	i32 rotations_count;
	i32 scalings_count;

	char *name;
	vec3 *positions;
	f32  *ptime;

	quat *rotations;
	f32 *rtime;

	vec3 *scalings;
	f32 *stime;
} cwsAnimNode;

/*
	Each model contains a tree of nodes, these nodes might be part of a animation.
*/
typedef struct cwsNode cwsNode;
struct cwsNode
{
	mat4 transform;
	i32 children_count;
				
	char *name;

	//If this node is used in animations it will have a AnimBone - else it's null
	cwsAnimBone *anim_bone;

 	//This points to the anim node in the currently selected animation which is related to this node
 	//NULL if it doesn't exist! (In case the node is not used in the current animation)
	cwsAnimNode *current_anim_node;
	cwsNode *children;
};

typedef struct
{
	i32 start_frame;
	i32 end_frame;
} cwsAnimation_Clip;

typedef struct
{
	f32 duration;
	f32 ticks_per_second;
	i32 channels_count;
	f32 frame_factor; //How much percent of the duration 1 frame takes up

	u32 clips_size, clips_count;
	cwsAnimNode **channels;

	cwsAnimation_Clip *selected_clip;
	cwsAnimation_Clip* clips;
} cwsAnimation;

/*
	The complete data for a mesh with animation
	Animation **animations -> The animations of the mesh
*/
typedef struct
{
	f32 animation_time;
	i32 animations_count;
	mat4 inverse_transform;
	
	cwsNode root_node;
	cwsAnimation **animations;
} cwsMesh_AnimData;

typedef struct
{
	u32 vao;
	u32 buffers[2];
	vec3 minB, maxB;
	i32 _icount;
	cwsMesh_AnimData *anim_data;
} cwsMesh;

/*
	tx,ty,tw,th contains texture clip bounds
	offset_x, offset_y contains pixel offsets when rendering glyphs
	advance_x contains advancement in x 
*/
typedef struct
{
	i32 x, y, w, h;
	i32 offset_x, offset_y;
	i32 advance_x;
} cwsFontChar;

typedef struct cwsTextContext cwsTextContext;
typedef struct
{
	vec2 pos;
	vec2 scale;
	vec2 __size;
	vec4 bounds;

	cws_string str;
	cwsTextContext *context;
} cwsText;

/*
	A text context contains character data, the font texture and the mesh containing all texts using the font
*/
struct cwsTextContext
{
	cwsFontChar chars[FONT_ASCII_CHAR_COUNT];
	u32 tex_buffer, buffer_texture_id;
	u32 texts_size, texts_count;
	cwsText** texts;
	cwsTexture2D *texture;
	cwsMesh *mesh;
};

#define cwsShaderInit(s) s = (cwsShader){.id = 0, .mvp_id = 0, .view_id = 0, .model_id = 0, .projection_id = 0}
#define cwsMeshInit(m) m = (cwsMesh){.vao = 0, \
                                                                  .buffers[0] = 0, \
                                                                  .buffers[1] = 0, \
                                                                  .minB = (vec3){FLT_MAX,FLT_MAX,FLT_MAX}, \
                                                                  .maxB = (vec3){FLT_MIN, FLT_MIN, FLT_MIN}, \
                                                                  ._icount = 0, \
                                                                  .anim_data = 0}
#define cwsMaterialInit(m) m = (cwsMaterial){ \
    cwsShaderInit(.shader), \
        .color = (vec3){1,1,1}, \
     .texture_array.data = NULL, \
    .texture_array.length = 0, \
     .texture_uniforms.data = NULL, \
    .texture_uniforms.data = 0, \
    .rflags = RF_CULLING_ENABLED}

#define cwsTexture2DArrayInit(a) (a = (Texture2DArray){.size = (ivec2){.x=0,.y=0},.images_size = 0, .images_count = 0, .images = NULL }; glGenTextures(1, &a.id))
extern SDL_Window *main_window;
extern SDL_GLContext main_gl_context;

void cwsVideoInit();
void cwsVideoDestroy();

void cwsClearColor(vec3 color);

bool cwsShaderFromfile(cwsShader* s, const char *vertex_file, const char *fragment_file, i32 hflags);
bool cwsShaderFromsrc(cwsShader* s, const char *vertex_src, const char *fragment_src);
void cwsShaderCreateUniform(cwsShader *s, const char *name);
void cwsShaderBufferUniform(cwsShader *s, const char *name, f32 *values, i32 length);
void cwsDeleteShader(cwsShader *shader);

bool cwsTextureFromfile(cwsTexture2D *tex, const char *file, i32 filter);
bool cwsTextureFromsrc(cwsTexture2D *tex, SDL_Surface *s, i32 filter);
void cwsDeleteTexture(cwsTexture2D* tex);

void cwsTexture2DArrayAppend(cwsTexture2DArray *a, cwsImage2D *i);
void cwsTexture2DArrayUpdate(cwsTexture2DArray *a);
void cwsDeleteTexture2DArray(cwsTexture2DArray *a);

bool cwsImageFromfile(cwsImage2D *img, const char *file);
void cwsDeleteImage(cwsImage2D *i);

void  cwsEmptyMesh(cwsMesh *mesh, i32 *vertex_attribs, i32 count);
void  cwsPlaneMesh(cwsMesh *mesh, f32 uv_repeat);
void  cwsCubeMesh(cwsMesh *mesh);
void  cwsBoundMesh(cwsMesh *mesh);
bool  cwsMeshFromfile(cwsMesh *mesh, const char *file);
void  cwsDeleteMesh(cwsMesh *mesh); //Deletes the mesh - makes it unusable
void  cwsClearMesh(cwsMesh *mesh); //Clears the mesh data
void cwsFillMesh(cwsMesh *mesh, f32 *vertex_data, i32 vcount, i32 *index_data, i32 icount);
void cwsFillsubMesh(cwsMesh *mesh, f32 *vertex_data, i32 voffset, i32 vlength, i32 *index_data, i32 ioffset, i32 ilength);

void cwsMaterialAddTexture(cwsMaterial *mat, cwsTexture2D tex);
void cwsBindMaterial(cwsMaterial *mat);
void cwsDeleteMaterial(cwsMaterial *mat);

bool cwsCreateTextContext(cwsTextContext *context, const char *font_file);
void cwsDeleteTextContext(cwsTextContext *context);
void cwsClearTextContext(cwsTextContext *context);

cwsText* cwsNewText(cwsTextContext *context, vec2 pos, vec2 scale, const char *tx);
void cwsRebuildText(cwsTextContext *context, cwsText *text, const char *tx);
void cwsDeleteText(cwsTextContext *context, cwsText *t);

void cwsDrawTextContext(cwsTextContext *context);
void cwsBindMesh(cwsMesh *mesh);
void cwsDrawMesh(mat4 transform, cwsMesh *mesh, i32 primitive);
void cwsDrawAnimatedMesh(mat4 transform, cwsMesh *mesh, i32 primitive, f32 time_in_secs);

void cwsSetPVMatrices(mat4 projection, mat4 view);
void cwsClear();
void cwsSwapBuffers();

void cwsSetPixel(SDL_Surface *s, i32 x, i32 y, u32 c);
u32 cwsGetPixel(SDL_Surface *s, i32 x, i32 y);
i32 cwsPackRgb(ivec3 rgb);
vec3 cwsUnpackRgb(i32 rgb);
vec2 cwsScreenSize();
#endif 