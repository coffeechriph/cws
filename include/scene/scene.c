#include "scene.h"

cws_array(cwsDrawGroup, draw_groups);
cws_array(cwsCamera*,cameras);
cws_array(cwsDirLight*, dirlights);
cws_array(cwsPointLight*,pointlights);
cws_array(cwsSpotLight*,spotlights);

cwsCamera *active_camera;
cwsCamera default_camera;
mat4 active_camera_proj;
mat4 active_camera_view;
mat4 csm_projections[4];
cwsCSMData csm_data;

cwsMaterial shadow_material;
cwsShader shadow_shader;

cwsMaterial shadow_material_animated;
cwsShader shadow_shader_animated;

cwsShader shadow_shader_cubemap;
cwsMaterial shadow_material_cubemap;

vec3 EE_ambient_light = (vec3){.x = 0.9f, .y = 0.9f, .z = 0.9f};

u32 light_ubo_id = 0;
cwsUniformLightData uniform_light_data;

void render();
void render_depth();
void render_depth_ortho(vec3 min, vec3 max);

cwsCamera *cwsActiveCamera()
{
    return active_camera;
}

void extract_frustum_planes(mat4 *matrix, vec4 planes[6])
{
    // Extract frustum planes from matrix
    // Planes are in format: normal(xyz), offset(w)
    //right
    planes[0] = (vec4){.x = matrix->m[3] - matrix->m[0],
        .y = matrix->m[7] - matrix->m[4],
        .z = matrix->m[11] - matrix->m[8],
        .w = matrix->m[15] - matrix->m[12]};
    
    //left
    planes[1] = (vec4){.x = matrix->m[3] + matrix->m[0],
        .y = matrix->m[7] + matrix->m[4],
        .z = matrix->m[11] + matrix->m[8],
        .w = matrix->m[15] + matrix->m[12]};
    
    //bot
    planes[2] = (vec4){.x = matrix->m[3] + matrix->m[1],
        .y = matrix->m[7] + matrix->m[5],
        .z = matrix->m[11] + matrix->m[9],
        .w = matrix->m[15] + matrix->m[13]};
    
    //top
    planes[3] = (vec4){.x = matrix->m[3] - matrix->m[1],
        .y = matrix->m[7] - matrix->m[5],
        .z = matrix->m[11] - matrix->m[9],
        .w = matrix->m[15] - matrix->m[13]};
    
    //far
    planes[4] = (vec4){.x = matrix->m[3] - matrix->m[2],
        .y = matrix->m[7] - matrix->m[6],
        .z = matrix->m[11] - matrix->m[10],
        .w = matrix->m[15] - matrix->m[14]};
    
    //near
    planes[5] = (vec4){.x = matrix->m[3] + matrix->m[2],
        .y = matrix->m[7] + matrix->m[6],
        .z = matrix->m[11] + matrix->m[10],
        .w = matrix->m[15] + matrix->m[14]};
    
    for(int i=0;i<6;i++){
        float invl = sqrt( planes[i].x * planes[i].x +
                          planes[i].y * planes[i].y +
                          planes[i].z * planes[i].z );
        if (invl != 0)
        {
            planes[i].x /= invl;
            planes[i].y /= invl;
            planes[i].z /= invl;
            planes[i].w /= invl;
        }
    }
}

bool sphere_in_frustum(vec3 center, f32 radius, vec4 planes[6])
{
    for(u32 i = 0; i < 6; ++i)
    {
        vec3 p = (vec3){.x = planes[i].x, .y = planes[i].y, .z = planes[i].z};
        if(vec3_dot(center, p) + planes[i].w + radius <= 0)
        {
            return false;
        }
    }
    return true;
}

bool cube_in_frustum(vec3 center, vec3 half, vec4 planes[6])
{
    vec3 points[8] = {
        (vec3){.x = center.x-half.x, 	.y = center.y-half.y, 	.z = center.z-half.z},
        (vec3){.x = center.x+half.x, 	.y = center.y-half.y, 	.z = center.z-half.z},
        (vec3){.x = center.x+half.x,  	.y = center.y+half.y, 	.z = center.z-half.z},
        (vec3){.x = center.x-half.x, 	.y = center.y+half.y, 	.z = center.z-half.z},
        
        (vec3){.x = center.x-half.x, 	.y = center.y-half.y, 	.z = center.z+half.z},
        (vec3){.x = center.x+half.x, 	.y = center.y-half.y, 	.z = center.z+half.z},
        (vec3){.x = center.x+half.x, 	.y = center.y+half.y, 	.z = center.z+half.z},
        (vec3){.x = center.x-half.x, 	.y = center.y+half.y, 	.z = center.z+half.z},
    };
    
    for(u32 i = 0; i < 6; ++i)
    {
        vec3 p = (vec3){.x = planes[i].x, .y = planes[i].y, .z = planes[i].z};
        
        if(vec3_dot(points[0], p) + planes[i].w > 0)
        {
            continue;
        }
        
        if(vec3_dot(points[1], p) + planes[i].w > 0)
        {
            continue;
        }
        
        if(vec3_dot(points[2], p) + planes[i].w > 0)
        {
            continue;
        }
        
        if(vec3_dot(points[3], p) + planes[i].w > 0)
        {
            continue;
        }
        
        if(vec3_dot(points[4], p) + planes[i].w > 0)
        {
            continue;
        }
        
        if(vec3_dot(points[5], p) + planes[i].w > 0)
        {
            continue;
        }
        
        if(vec3_dot(points[6], p) + planes[i].w > 0)
        {
            continue;
        }	
        
        if(vec3_dot(points[7], p) + planes[i].w > 0)
        {
            continue;
        }	
        
        return false;
    }
    
    return true;
}

cwsRenderer *cwsNewRenderer(cwsMaterial *mat, cwsMesh *mesh)
{
    cwsRenderer r;
    r.minB = (vec3){.x = 0, .y = 0, .z = 0};
    r.maxB = (vec3){.x = 1, .y = 1, .z = 1};
    r.animation_clip_index = 0;
    r.position = (vec3){.x = 0, .y = 0, .z = 0};
    r.scale = (vec3){.x = 1, .y = 1, .z = 1};
    r.rotation = (quat){.x = 0, .y = 0, .z = 0, .w = 1};
    r.group_index = 0;
    
    for(u32 i = 0; i < draw_groups.length; ++i)
    {
        if(draw_groups.data[i].material == mat && draw_groups.data[i].mesh == mesh)
        {
            r.group_index = i;
            cws_bucket_array_push(draw_groups.data[i].renderers, r);
            u32 index = 0;
            cws_bucket_array_last(draw_groups.data[i].renderers, index);
            return &cws_bucket_array_index(draw_groups.data[i].renderers, index);
        }
    }
    
    cwsDrawGroup grp;
    grp.material = mat;
    grp.mesh = mesh;
    
    cws_bucket_array_init(cwsRenderer, grp.hidden_renderers, 0);
    cws_bucket_array_init(cwsRenderer, grp.renderers, 0);
    
    r.group_index = draw_groups.length;
    cws_bucket_array_push(grp.renderers,r);
    
    cws_array_push(draw_groups, grp);
    cwsDrawGroup *pgrp = &draw_groups.data[draw_groups.length-1];
    
    u32 index = 0;
    cws_bucket_array_last(pgrp->renderers, index);
    return &cws_bucket_array_index(pgrp->renderers, index);
}

void cwsRemoveRenderer(cwsRenderer *r)
{
    if(r->group_index >= 0)
    {
        cwsDrawGroup *grp = &draw_groups.data[r->group_index];
        for(u32 i = 0; i < cws_bucket_array_item_count(grp->renderers); ++i)
        {
            if(!cws_bucket_array_occupied(grp->renderers, i))
            {
                continue;
            }
            
            if(&cws_bucket_array_index(grp->renderers,i) == r)
            {
                cws_log("Renderer remove unimplemented!");
                //cws_bucket_array_remove(grp->renderers, i);
                break;
            }
        }
    }
}

cwsMaterial *cwsRendererGetMaterial(cwsRenderer *r)
{
    return draw_groups.data[r->group_index].material;
}

cwsMesh *cwsRendererGetMesh(cwsRenderer *r)
{
    return draw_groups.data[r->group_index].mesh;
}

void cwsUpdateBounds(cwsRenderer *r)
{    
    //Calculate world bounds
    cwsMesh *mesh = draw_groups.data[r->group_index].mesh;
    mat4 rot = quat_to_mat4(r->rotation);
    mat4 tra = mat4_mul(mat4_default,rot);
    
    vec3 min = mesh->minB;
    vec3 max = mesh->maxB;
    
    vec3 points[8] = {
        (vec3){.x = min.x, .y = min.y, .z = min.z},
        (vec3){.x = max.x, .y = min.y, .z = min.z},
        (vec3){.x = max.x, .y = max.y, .z = min.z},
        (vec3){.x = max.x, .y = max.y, .z = max.z},
        
        (vec3){.x = min.x, .y = max.y, .z = max.z},
        (vec3){.x = min.x, .y = min.y, .z = max.z},
        (vec3){.x = max.x, .y = min.y, .z = max.z},
        (vec3){.x = min.x, .y = max.y, .z = min.z}
    };
    
    for(i32 i = 0; i < 8; ++i)
    {
        points[i] = vec3_transform(points[i], tra);
    }
    
    min = (vec3){.x = FLT_MAX, .y = FLT_MAX, .z = FLT_MAX};
    max = (vec3){.x = FLT_MIN, .y = FLT_MIN, .z = FLT_MIN};
    
    for(i32 i = 0; i < 8; ++i)
    {
        if(points[i].x < min.x)
            min.x = points[i].x;
        else if(points[i].x > max.x)
            max.x = points[i].x;
        
        if(points[i].y < min.y)
            min.y = points[i].y;
        else if(points[i].y > max.y)
            max.y = points[i].y;
        
        if(points[i].z < min.z)
            min.z = points[i].z;
        else if(points[i].z > max.z)
            max.z = points[i].z;
    }
    
    r->minB = vec3_add(vec3_mul(min, r->scale), r->position);
    r->maxB = vec3_add(vec3_mul(max, r->scale), r->position);	
}

void cwsRendererMakeStatic(cwsRenderer *r)
{
    
}

void cwsRendererShow(cwsRenderer *r, bool c)
{
    cws_log("Renderer hide/show unimplemented!");
}

cwsCamera *cwsNewCamera()
{
    cwsCamera *cam = malloc(sizeof(cwsCamera));
    cam->far_distance = 1000.0f;
    cam->near_distance = 1.0f;
    cam->fov = 60.0f;
    cam->fog_begin = 600.0f;
    cam->fog_end = 1000.0f;
    cam->position = (vec3){.x = 0, .y = 0, .z = 0};
    cam->dir = (vec3){.x = 0, .y = 0, .z = -1};
    cam->rot = (vec3){.x = 0, .y = 0, .z = 0};
    
    cws_array_push(cameras,cam);
    return cam;
}

void cwsSetActiveCamera(cwsCamera *c)
{
    if(c == NULL)
        return;
    
    active_camera = c;
    vec2 sz = cwsScreenSize();
    active_camera_proj = mat4_perspective(sz.x, sz.y, active_camera->fov, active_camera->near_distance, active_camera->far_distance);
    
    csm_projections[0] = mat4_perspective(sz.x, sz.y, active_camera->fov, 4.0f, 16.0f);
    csm_projections[1] = mat4_perspective(sz.x, sz.y, active_camera->fov, 8.0f, 32.0f);
    csm_projections[2] = mat4_perspective(sz.x, sz.y, active_camera->fov, 32.0f, 128.0f);
    csm_projections[3] = mat4_perspective(sz.x, sz.y, active_camera->fov, 64.0f, 512.0f);
}

ray cwsBuildPickRay(cwsCamera *camera)
{
    vec2 mp = get_mouse_position();
    vec3 coord = (vec3){.x = (2.0f*mp.x) / (f32)cwsScreenSize().x - 1.0f, 
        .y = -((2.0f*mp.y) / (f32)cwsScreenSize().y - 1.0f), 
        .z = -1.0f};
    
    vec3 dir = (vec3){.x = 0, .y = 0, .z = -1};
    dir = vec3_rotate(dir, (vec3){.x = camera->rot.x, .y = 0.0f, .z = 0.0f});
    dir = vec3_rotate(dir, (vec3){.x = 0.0f, .y = camera->rot.y, .z = 0.0f});
    dir = vec3_rotate(dir, (vec3){.x = 0.0f, .y = 0.0f, .z = camera->rot.z});
    
    mat4 proj = mat4_perspective(cwsScreenSize().x, cwsScreenSize().y, camera->fov, camera->near_distance, camera->far_distance);
    vec3 cpd = vec3_add(camera->position, dir);
    mat4 view = mat4_lookat(camera->position, cpd, (vec3){.x = 0, .y = 1, .z = 0});
    
    coord.x /= proj.m[0];
    coord.y /= proj.m[5];
    
    mat4 view_inv = mat4_inverse(view);
    
    coord = vec3_mul_scalar(coord, camera->far_distance);
    coord = vec3_transform(coord, view_inv);
    
    ray r;
    r.origin = camera->position;
    r.dir = vec3_sub(coord,camera->position);
    r.dir = vec3_normalize(r.dir);
    r.length = camera->far_distance;
    return r;
}

cwsDirLight *cwsNewDirLight()
{
    cwsDirLight *light = malloc(sizeof(cwsDirLight));
    light->rot = (vec3){.x=0,.y=0,.z=0};
    light->color = (vec3){.x=1,.y=1,.z=1};
    cws_array_push(dirlights,light);
    return light;
}

cwsPointLight *cwsNewPointLight()
{
    cwsPointLight *light = malloc(sizeof(cwsPointLight));
    light->pos = (vec3){.x=0,.y=0,.z=0};
    light->color = (vec3){.x=1,.y=1,.z=1};
    light->radius = 10.0f;
    
    glGenFramebuffers(1, &light->frame_buffer);
    glGenTextures(1, &light->frame_buffer_texture);
    
    glBindTexture(GL_TEXTURE_CUBE_MAP, light->frame_buffer_texture);
    
    for(i32 i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i, 0, GL_DEPTH_COMPONENT, 256, 256, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        GLfloat borderColor[4] = { 1.0, 1.0, 1.0, 1.0 };
        glTexParameterfv(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BORDER_COLOR, borderColor); 
    }
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, light->frame_buffer);
    glFramebufferTexture(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, light->frame_buffer_texture, 0); 
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE); 
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    cws_array_push(pointlights,light);
    return light;
}

cwsSpotLight *cwsNewSpotLight()
{
    cwsSpotLight *light = (cwsSpotLight*)malloc(sizeof(cwsSpotLight));
    light->pos = (vec3){.x=0,.y=0,.z=0};
    light->rot = (vec3){.x=0,.y=0,.z=0};
    light->color = (vec3){.x=1,.y=1,.z=1};
    light->cone_data = 45 + (5 << 7) + (10 << 11);
    glGenFramebuffers(1, &light->frame_buffer);
    glGenTextures(1, &light->frame_buffer_texture);
    glBindTexture(GL_TEXTURE_2D, light->frame_buffer_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    GLfloat borderColor[4] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor); 
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, light->frame_buffer);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, light->frame_buffer_texture, 0); 
    
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE); 
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    cws_array_push(spotlights,light);
    return light;
}

void cwsDeleteDirLight(cwsDirLight *l)
{
}

typedef struct
{
    f32 z;
    i32 i;
} ZIPair;

/*
 Merge sort of a Z Index pair
 The Z is a f32 used for the actual test and I is used to
 keep track of the items original index into the list
*/
void merge_sort_zipair(ZIPair in[], ZIPair *out, i32 list_size)
{
    if(list_size > 1)
    {
        i32 p = list_size / 2;
        i32 d = list_size % 2;
        
        ZIPair left[p];
        ZIPair right[p+d];
        ZIPair left_out[p], right_out[p+d];
        
        //Split the list into 2 sub lists
        for(i32 i = 0; i < p; ++i)
        {
            left[i] = in[i];
            right[i] = in[i+p];
        }
        
        //If the list is uneven add 1 additional element to the right list
        //which will then continue to split until solved
        if(d == 1)
        {
            right[p] = in[list_size-1];
        }
        
        merge_sort_zipair(left, left_out, p);
        merge_sort_zipair(right, right_out, p+d);
        
        //Merge the returned lists
        i32 i = 0;
        i32 li = 0, ri = 0;
        
        //Sort the two sub lists until one is empty
        while(li < p && ri < p+d)
        {
            if(left_out[li].z < right_out[ri].z)
            {
                out[i] = left_out[li];
                li++;
            }
            else
            {
                out[i] = right_out[ri];
                ri++;
            }
            
            i++;
        }
        
        //Continue to add elements from the list which is not yet empty to the final out list
        if(li < p)
        {
            while(li < p)
            {
                out[i] = left_out[li];
                i++;
                li++;
            }
        }
        else 
        {
            while(ri < p+d)
            {
                out[i] = right_out[ri];
                i++;
                ri++;
            }
        }
    }
    else
    {
        out[0] = in[0];
        return;
    }
}

void cwsSceneInit()
{
    active_camera_proj = mat4_default;
    active_camera_view = mat4_default;
    
    memset(&uniform_light_data, 0, sizeof(cwsUniformLightData));
    default_camera.far_distance = 1000.0f;
    default_camera.near_distance = 1.0f;
    default_camera.fov = 60.0f;
    default_camera.fog_begin = 600.0f;
    default_camera.fog_end = 1000.0f;
    default_camera.position = (vec3){.x = 0, .y = 0, .z = 0};
    default_camera.dir = (vec3){.x = 0, .y = 0, .z = -1};
    default_camera.rot = (vec3){.x = 0, .y = 0, .z = 0};
    cwsSetActiveCamera(&default_camera);
    
    cws_array_init(cwsDrawGroup, draw_groups, 0);
    cws_array_init(cwsCamera*, cameras,0);
    cws_array_init(cwsDirLight*, dirlights, 0);
    cws_array_init(cwsPointLight*, pointlights, 0);
    cws_array_init(cwsSpotLight*, spotlights, 0);
    
    cwsShaderInit(shadow_shader);
    cwsShaderFromsrc(&shadow_shader,
                     "#version 330\n"
                     "layout(location = 0) in vec3 pos;\n"
                     "layout (location = 1) in vec3 normal;\n"
                     "layout (location = 2) in vec2 uv;\n"
                     "layout (location = 3) in vec3 color;\n"
                     
                     "uniform mat4 mvp_matrix;\n"
                     "void main()\n"
                     "{\n"
                     "gl_Position = (mvp_matrix) * vec4(pos,1.0f);\n"
                     "}\n"
                     , 
                     "#version 330\n"
                     "out vec4 out_color;\n"
                     "void main()\n"
                     "{\n"
                     "out_color = vec4(1.0);\n"
                     "}\n",
                     NULL);
    cwsMaterialInit(shadow_material);
	shadow_material.shader = shadow_shader;

    cwsShaderInit(shadow_shader_animated);
	cwsShaderFromsrc(&shadow_shader_animated, 
                     "#version 330\n"
                     "layout(location = 0) in vec3 pos;\n"
                     "layout (location = 1) in vec3 normal;\n"
                     "layout (location = 2) in vec2 uv;\n"
                     "layout (location = 3) in vec3 color;\n"
                     "layout (location = 4) in vec4 bone_indices;\n"
                     "layout (location = 5) in vec4 bone_weights;\n"
                     
                     "uniform mat4 EE_bone_matrices[32];\n"
                     
                     "uniform mat4 mvp_matrix;\n"
                     "void main()\n"
                     "{\n"
                     "mat4 bone_transform = mat4(0.0f);\n"
                     "if(bone_indices[0] >= 0)\n"
                     "	bone_transform += EE_bone_matrices[int(bone_indices[0])] * bone_weights[0];\n"
                     
                     "if(bone_indices[1] >= 0)\n"
                     "	bone_transform += EE_bone_matrices[int(bone_indices[1])] * bone_weights[1];\n"
                     
                     "if(bone_indices[2] >= 0)\n"
                     "	bone_transform += EE_bone_matrices[int(bone_indices[2])] * bone_weights[2];\n"
                     
                     "if(bone_indices[3] >= 0)\n"
                     "	bone_transform += EE_bone_matrices[int(bone_indices[3])] * bone_weights[3];\n"
                     "vec4 posL = bone_transform * vec4(pos,1.0f);\n"
                     
                     "gl_Position = (mvp_matrix) * posL;\n"
                     "}\n"
                     , 
                     "#version 330\n"
                     "out vec4 out_color;\n"
                     "void main()\n"
                     "{\n"
                     "out_color = vec4(1);\n"
                     "}\n",
                     NULL);
    cwsMaterialInit(shadow_material_animated);
	shadow_material_animated.shader = shadow_shader_animated;

    cwsShaderInit(shadow_shader_cubemap);
    cwsShaderFromsrc(&shadow_shader_cubemap,
                     "#version 330\n"
                     "layout(location = 0) in vec3 pos;\n"
                     "layout (location = 1) in vec3 normal;\n"
                     "layout (location = 2) in vec2 uv;\n"
                     "layout (location = 3) in vec3 color;\n"
                     
                     "uniform mat4 mvp_matrix;\n"
                     "uniform mat4 model_matrix;\n"
                     "void main()\n"
                     "{\n"
                     "gl_Position = (model_matrix) * vec4(pos,1.0f);\n"
                     "}\n"
                     , 
                     "#version 330\n"
                     "in vec4 FragPos;\n"
                     "uniform vec3 lightPos;\n"
                     "uniform float far_plane;\n"
                     "out vec4 out_color;\n"
                     "void main()\n"
                     "{\n"
                     "float dst = length(FragPos.xyz - lightPos);\n"
                     "dst = dst / far_plane;\n"
                     "gl_FragDepth = dst;\n"
                     "}\n",
                     "#version 330\n"
                     "layout(triangles) in;"
                     "layout(triangle_strip, max_vertices=18) out;"
                     "uniform mat4 shadowMatrices[6];\n"
                     "out vec4 FragPos;\n"
                     "void main(){\n"
                     "for(int face = 0; face < 6; ++face){\n"
                     "gl_Layer = face;\n"
                     "for(int i = 0; i < 3; ++i){\n"
                     "FragPos = gl_in[i].gl_Position;\n"
                     "gl_Position = shadowMatrices[face] * FragPos;\n"
                     "EmitVertex();\n"
                     "}"
                     "EndPrimitive();\n"
                     "}"
                     "}");
    cwsMaterialInit(shadow_material_cubemap);
    shadow_material_cubemap.shader = shadow_shader_cubemap;
    
    cwsShaderCreateUniform(&shadow_material_cubemap.shader, "far_plane");
    cwsShaderCreateUniform(&shadow_material_cubemap.shader, "shadowMatrices[0]");
    cwsShaderCreateUniform(&shadow_material_cubemap.shader, "shadowMatrices[1]");
    cwsShaderCreateUniform(&shadow_material_cubemap.shader, "shadowMatrices[2]");
    cwsShaderCreateUniform(&shadow_material_cubemap.shader, "shadowMatrices[3]");
    cwsShaderCreateUniform(&shadow_material_cubemap.shader, "shadowMatrices[4]");
    cwsShaderCreateUniform(&shadow_material_cubemap.shader, "shadowMatrices[5]");
    cwsShaderCreateUniform(&shadow_material_cubemap.shader, "lightPos");
    
	glGenBuffers(1, &light_ubo_id);
	glBindBuffer(GL_UNIFORM_BUFFER, light_ubo_id);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(uniform_light_data), &uniform_light_data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	csm_data.frame_buffer_count = 4;
	csm_data.frame_buffer_size = 2048;
	csm_data.shadows_enabled = true;
    for(u32 i = 0; i < 4; ++i)
    {
        csm_data.light_view[i] = mat4_default;
        csm_data.frame_projections[i] = mat4_default;
    }
	glGenFramebuffers(1, &csm_data.frame_buffer);
	glGenTextures(1, &csm_data.frame_buffer_texture);

	glBindTexture(GL_TEXTURE_2D, csm_data.frame_buffer_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, csm_data.frame_buffer_size, csm_data.frame_buffer_size, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	GLfloat borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, csm_data.frame_buffer);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, csm_data.frame_buffer_texture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void cwsSceneDestroy()
{
    cwsDeleteShader(&shadow_shader);
    cwsDeleteShader(&shadow_shader_animated);
    
	for(u32 i = 0; i < draw_groups.length; ++i)
	{
        cwsDrawGroup *grp = &draw_groups.data[i];
		cws_bucket_array_free(grp->renderers);
		cws_bucket_array_free(grp->hidden_renderers);
	}

	cws_array_free(draw_groups);

	for(u32 i = 0; i < cameras.length; ++i)
	{
		free(cameras.data[i]);
	}
	cws_array_free(cameras);

	glDeleteFramebuffers(1, &csm_data.frame_buffer);
	glDeleteTextures(1, &csm_data.frame_buffer_texture);

	for(u32 i = 0; i < dirlights.length; ++i)
	{
		free(dirlights.data[i]);
	}
	cws_array_free(dirlights);

	for(u32 i = 0; i < pointlights.length; ++i)
	{
		free(pointlights.data[i]);
	}
	cws_array_free(pointlights);

	for(u32 i = 0; i < spotlights.length; ++i)
	{
		free(spotlights.data[i]);
	}
	cws_array_free(spotlights);
}


vec3* split_frustum(mat4 vp_inv)
{
	vec3 *s = malloc(sizeof(vec3)*8);
	if(s == NULL)
	{
		cws_log("Error allocating memory for split_frustum!");
		return NULL;
	}

	s[0] = (vec3){.x=-1, .y=1, .z=-1};
	s[1] = (vec3){.x=1,  .y=1, .z=-1};
	s[2] = (vec3){.x=1, .y=-1, .z=-1};
	s[3] = (vec3){.x=-1, .y=-1, .z=-1};

	s[4] = (vec3){.x=-1,  .y=1, .z=0};
	s[5] = (vec3){.x=1,  .y=1, .z=0};
	s[6] = (vec3){.x=1, .y=-1, .z=0};
	s[7] = (vec3){.x=-1, .y=-1, .z=0};

	for(u32 i = 0; i < 8; ++i)
	{
		vec4 d = (vec4){.x = s[i].x, .y = s[i].y, .z = s[i].z, .w = 1.0f};
		d = vec4_transform(d, vp_inv);

		s[i].x = d.x / d.w;
		s[i].y = d.y / d.w;
		s[i].z = d.z / d.w;
	}

	return s;
}

f32 world_depth_to_clip(f32 depth, mat4 proj)
{
	vec4 v2 = (vec4){.x = 0, .y = 0, .z = depth, .w = 1};
	v2 = vec4_transform(v2, proj);
	return v2.z / v2.w;
}

void ee_update_lights()
{
	vec2 win_size = cwsScreenSize();
	if(active_camera != NULL && dirlights.length >= 1)
	{
		//The first directional light is used by default to cast shadows
		cwsDirLight *light = dirlights.data[0];

		//Get the light direction
		vec3 dir = (vec3){.x = 0, .y = 0, .z = -1};
		dir = vec3_rotate(dir, (vec3){.x = light->rot.x, .y = 0, .z = 0});
		dir = vec3_rotate(dir, (vec3){.x = 0, .y = light->rot.y, .z = 0});
		dir = vec3_rotate(dir, (vec3){.x = 0, .y = 0, .z = light->rot.z});
		dir = vec3_normalize(dir);

        glBindFramebuffer(GL_FRAMEBUFFER, csm_data.frame_buffer);
        cwsBindMaterial(&shadow_material);
		glColorMaski(csm_data.frame_buffer, 1, 0, 0, 0);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		u32 x = 0, y = 0;
		for(u32 j = 0; j < csm_data.frame_buffer_count; ++j)
		{
			//Each frustum slice has got its own projection matrix
			//which ranges from different near/far
			mat4 camproj = mat4_mul(csm_projections[j],active_camera_view);

			//Calculate the frustum points
			mat4 inv = mat4_inverse(camproj);
			vec3* frustum_points = split_frustum(inv);

			//Instead of constructing a ortho projection from the frustum points we use
			//a sphere instead which ensures that the size doesn't change when rotating 
			//which removes swimming during rotation
			vec3 fps = vec3_sub(frustum_points[0], frustum_points[6]);
			f32 radius = vec3_length(fps)*0.5f;

			//Texels-per-unit used to snap the shadow map to texels which removes swimming at movement
			f32 tpu = (f32)csm_data.frame_buffer_size / 2.0f / (radius*2);

			//Calculate the frustum center
			vec3 center = (vec3){.x = 0.0f, .y = 0.0f, .z = 0.0f};
			for(u32 i = 0; i < 8; ++i)
			{
				center = vec3_add(center,frustum_points[i]);
			}
			center = vec3_mul_scalar(center, 1.0f / 8.0f);

			//Create a basic - none texel-snapped look-at matrix
			mat4 LOOKAT = mat4_lookat((vec3){.x = 0, .y = 0, .z = 0},
									  (vec3){.x = dir.x, .y = dir.y, .z = -dir.z}, 
									  (vec3){.x = 0, .y = 1, .z = 0});
            //Scale the lookat matrix by our tpu scale
			LOOKAT = mat4_scale(LOOKAT, (vec3){.x = tpu, .y = tpu, .z = tpu});

            //Transform the frustrum center by the scaled lookat matrix - This will cause the center to be 
            //in texel space
            center = vec3_transform(center, LOOKAT);
            
            //Floor the result which will place the center on even texels
			center.x = floor(center.x);
			center.y = floor(center.y);

			//scale back to world coords - Now the world coords of the frustum center will be placed on even texels
			mat4 lookat_inv = mat4_inverse(LOOKAT);
			center = vec3_transform(center, lookat_inv);

			//calculate the real look-at matrix which uses the snapped frustum center
			vec3 sdir = (vec3){.x = -dir.x, .y = -dir.y, .z = dir.z};
			sdir = vec3_mul_scalar(sdir, radius*2);
			vec3 eye = vec3_sub(center, sdir);
			csm_data.light_view[j] = mat4_lookat(eye, center, (vec3){.x = 0, .y = 1, .z = 0});

			//Create our projection matrix from the sphere radius
			csm_data.frame_projections[j] = mat4_ortho(-radius, radius, -radius, radius, -50.0f, 1000.0);

			//Render shadow maps
			glViewport(x,y,csm_data.frame_buffer_size/2,csm_data.frame_buffer_size/2);
			cwsSetPVMatrices(csm_data.frame_projections[j], csm_data.light_view[j]);
			vec3 rad2 = (vec3){.x = radius*2, .y = radius*2, .z = radius*2};
			vec3 min = vec3_sub(center, rad2);
			vec3 max = vec3_add(center, rad2);
			render_depth_ortho(min,max);

			//update the UBO data
			uniform_light_data.shadow_matrices_lod[j] = mat4_mul(csm_data.frame_projections[j],csm_data.light_view[j]);

			x += csm_data.frame_buffer_size/2;
			if(x >= csm_data.frame_buffer_size)
			{
				x = 0;
				y += csm_data.frame_buffer_size/2;
			}

			free(frustum_points);
		}

		//Calculate directional lights direction
		for(u32 i = 0; i < dirlights.length; ++i)
		{
			vec3 vdir = (vec3){.x = 0, .y = 0, .z = 1};
			vdir = vec3_rotate(vdir, (vec3){.x = light->rot.x, .y = 0, .z = 0});
			vdir = vec3_rotate(vdir, (vec3){.x = 0, .y = light->rot.y, .z = 0});
			vdir = vec3_rotate(vdir, (vec3){.x = 0, .y = 0, .z = light->rot.z});
			vdir = vec3_normalize(vdir);
			uniform_light_data.dirlights_dir[i] = (vec4){.x = vdir.x, .y = vdir.y, .z = vdir.z, .w = 0};
			uniform_light_data.dirlights_color[i] = (vec4){.x = light->color.x, .y = light->color.y, .z = light->color.z, .w = 1.0f};
		}
	}

	for(u32 i = 0; i < spotlights.length; ++i)
	{
		f32 cutoff = (f32)((spotlights.data[i]->cone_data & 127));
		f32 edge_factor = (f32)((spotlights.data[i]->cone_data>>7) & 15);
		f32 length = (f32)(spotlights.data[i]->cone_data>>11);

		//Render shadow maps
		glBindFramebuffer(GL_FRAMEBUFFER, spotlights.data[i]->frame_buffer);
        cwsBindMaterial(&shadow_material);
		glViewport(0,0,1024,1024);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		mat4 light_view = mat4_rotate(mat4_default, (vec3){.x = spotlights.data[i]->rot.x, .y = 0, .z = 0});
		light_view 		= mat4_rotate(light_view, (vec3){.x = 0, .x = spotlights.data[i]->rot.y, .z = 0});
		light_view 		= mat4_rotate(light_view, (vec3){.x = 0, .x = 0, .z = spotlights.data[i]->rot.z});
		
		vec3 neg_pos = (vec3){.x = -spotlights.data[i]->pos.x,
						      .y = -spotlights.data[i]->pos.y,
							  .z = -spotlights.data[i]->pos.z};
		light_view = mat4_translate(light_view, neg_pos);
		mat4 p = mat4_perspective(1, 1, cutoff*2+edge_factor, 1.0f, length*2);
		cwsSetPVMatrices(p, light_view);

		render_depth();

		//update the UBO data
		if(i < 8)
		{
			uniform_light_data.spotlight_matrices[i] = mat4_mul(p,light_view);
		}

		vec3 vdir = (vec3){.x = 0, .y = 0, .z = 1};
		vdir = vec3_rotate(vdir, (vec3){.x = spotlights.data[i]->rot.x, .y = 0, .z = 0});
		vdir = vec3_rotate(vdir, (vec3){.x = 0, .y = spotlights.data[i]->rot.y, .z = 0});
		vdir = vec3_rotate(vdir, (vec3){.x = 0, .y = 0, .z = spotlights.data[i]->rot.z});
		
		uniform_light_data.spotlights_pos[i] = (vec4){.x = spotlights.data[i]->pos.x, .y = spotlights.data[i]->pos.y, .z = spotlights.data[i]->pos.z, .w = 1.0f};
		uniform_light_data.spotlights_dir[i] = (vec4){.x = vdir.x, .y = vdir.y, .z = vdir.z, .w = 0};
		uniform_light_data.spotlights_intensity[i] = (vec4){.x = spotlights.data[i]->color.x, .y = spotlights.data[i]->color.y, .z = spotlights.data[i]->color.z, .w = length};
		uniform_light_data.spotlights_conedata[i] = (vec4){.x = cutoff*DEG_2_RAD, .y = edge_factor*DEG_2_RAD, .z = 0.0f, .w = 0.0f};
	}

	for(u32 i = 0; i < pointlights.length; ++i)
	{
		uniform_light_data.pointlights_pos[i] = (vec4){.x = pointlights.data[i]->pos.x, .y = pointlights.data[i]->pos.y, .z = pointlights.data[i]->pos.z, .w = 1.0f};
		uniform_light_data.pointlights_intensity[i] = (vec4){.x = pointlights.data[i]->color.x, .y = pointlights.data[i]->color.y, .z = pointlights.data[i]->color.z, .w = pointlights.data[i]->radius};
        
        mat4 rots[6] = {
            mat4_lookat(pointlights.data[i]->pos, vec3_add(pointlights.data[i]->pos, (vec3){.x = 1, .y = 0, .z = 0}), (vec3){.x = 0, .y = -1, .z = 0}),
            mat4_lookat(pointlights.data[i]->pos, vec3_add(pointlights.data[i]->pos, (vec3){.x = -1, .y = 0, .z = 0}), (vec3){.x = 0, .y = -1, .z = 0}),
            mat4_lookat(pointlights.data[i]->pos, vec3_add(pointlights.data[i]->pos, (vec3){.x = 0, .y = 1, .z = 0}), (vec3){.x = 0, .y = 0, .z = 1}),
            mat4_lookat(pointlights.data[i]->pos, vec3_add(pointlights.data[i]->pos, (vec3){.x = 0, .y = -1, .z = 0}), (vec3){.x = 0, .y = 0, .z = -1}),
            mat4_lookat(pointlights.data[i]->pos, vec3_add(pointlights.data[i]->pos, (vec3){.x = 0, .y = 0, .z = 1}), (vec3){.x = 0, .y = -1, .z = 0}),
            mat4_lookat(pointlights.data[i]->pos, vec3_add(pointlights.data[i]->pos, (vec3){.x = 0, .y = 0, .z = -1}), (vec3){.x = 0, .y = -1, .z = 0}),
        };
        
        glBindFramebuffer(GL_FRAMEBUFFER, pointlights.data[i]->frame_buffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0,0,256,256);
        cwsBindMaterial(&shadow_material_cubemap);
        pointlights.data[i]->proj = mat4_perspective(1, 1, 90, 1.0f, 1000); 
        f32 farPlane = 1000.0f;
        cwsShaderBufferUniform(&shadow_material_cubemap.shader, "far_plane", &farPlane, 1);
        f32 pp[3] = {pointlights.data[i]->pos.x, pointlights.data[i]->pos.y, pointlights.data[i]->pos.z};
        cwsShaderBufferUniform(&shadow_material_cubemap.shader, "lightPos", pp, 3);
        for(u32 j = 0; j < 6; ++j)
        {
            mat4 light_view = rots[j];
            pointlights.data[i]->view[j] = light_view;
            mat4 shp;
            shp = mat4_mul(pointlights.data[i]->proj, light_view);
            char buf[32];
            sprintf(buf, "shadowMatrices[%d]", j);
            cwsShaderBufferUniform(&shadow_material_cubemap.shader, buf, shp.m, 16);
        }
        
        render_depth();
	}
    
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    glViewport(0,0,win_size.x,win_size.y);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

	uniform_light_data.ambient_light = (vec4){.x = EE_ambient_light.x, .y = EE_ambient_light.y, .z = EE_ambient_light.z, .w = 1.0f};
	uniform_light_data.lights_count.x = dirlights.length;
	uniform_light_data.lights_count.y = pointlights.length;
	uniform_light_data.lights_count.z = spotlights.length;

	glBindBuffer(GL_UNIFORM_BUFFER, light_ubo_id);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(uniform_light_data), &uniform_light_data);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void cwsSceneUpdate()
{
	ee_update_lights();
}

/*
	This function renders all the scene nodes but without applying any materials
	this is essential for the depth rendering where we apply a custom material to only render depth info
*/
void render_depth()
{
	for(u32 i = 0; i < draw_groups.length; ++i)
	{
        cwsDrawGroup *grp = &draw_groups.data[i];
		cwsBindMesh(grp->mesh);
        for(u32 j = 0; j < cws_bucket_array_item_count(grp->renderers); ++j)
		{
            if(!cws_bucket_array_occupied(grp->renderers, j))
            {
                continue;
            }
            
            cwsRenderer *obj 		= &cws_bucket_array_index(grp->renderers, j);
			mat4 tr = mat4_translate(mat4_default, obj->position);
				
			mat4 res = quat_to_mat4(obj->rotation);
			tr = mat4_mul(tr, res);
			tr = mat4_scale(tr, obj->scale);
			
			if(grp->mesh->anim_data != NULL)
			{
				cwsDrawAnimatedMesh(tr, grp->mesh, GL_TRIANGLES, (f32)(SDL_GetTicks())/1000.0f);
			}
			else
            {
				cwsDrawMesh(tr, grp->mesh, GL_TRIANGLES);
			}
		}
	}
}

void render_depth_ortho(vec3 min, vec3 max)
{
	vec3 pmin, pmax;
	for(u32 i = 0; i < draw_groups.length; ++i)
	{
        cwsDrawGroup *grp = &draw_groups.data[i];
		cwsBindMesh(grp->mesh);
        for(u32 j = 0; j < cws_bucket_array_item_count(grp->renderers); ++j)
		{
            if(!cws_bucket_array_occupied(grp->renderers, j))
            {
                continue;
            }
            
            cwsRenderer *obj 		= &cws_bucket_array_index(grp->renderers,j);

			pmin = obj->minB;
			pmax = obj->maxB;

			//frustum cull
			if((pmax.x < min.x || pmin.x > max.x
			|| pmax.y < min.y || pmin.y > max.y
			|| pmax.z < min.z || pmin.z > max.z))
			{
			//	continue;
			}

			mat4 tr = mat4_translate(mat4_default, obj->position);
				
			mat4 res = quat_to_mat4(obj->rotation);
			tr = mat4_mul(tr, res);
			tr = mat4_scale(tr, obj->scale);

			if(grp->mesh->anim_data != NULL)
			{
				cwsDrawAnimatedMesh(tr, grp->mesh, GL_TRIANGLES, (f32)(SDL_GetTicks())/1000.0f);
			}
			else
			{
				cwsDrawMesh(tr, grp->mesh, GL_TRIANGLES);
			}
		}
	}
}

void render()
{
	vec4 planes[6];
	mat4 sm = mat4_mul(active_camera_proj,active_camera_view);
	extract_frustum_planes(&sm, planes);

	glBindBuffer(GL_UNIFORM_BUFFER, light_ubo_id);

	//Render draw groups
	for(u32 i = 0; i < draw_groups.length; ++i)
	{
        cwsDrawGroup *grp = &draw_groups.data[i];
		cwsBindMaterial(grp->material);
		
		//Bind the light data 
		u32 light_ubo_index = glGetUniformBlockIndex(grp->material->shader.id, "LightData");
		glUniformBlockBinding(grp->material->shader.id, light_ubo_index, 1);
		glBindBufferBase(GL_UNIFORM_BUFFER, 1, light_ubo_id);

		//Pass the light data to the shaders
		i32 tex_offset = grp->material->texture_array.length;
		if(dirlights.length >= 1)
		{
			glActiveTexture(GL_TEXTURE0+tex_offset);
			glBindTexture(GL_TEXTURE_2D, csm_data.frame_buffer_texture);
			glUniform1i(glGetUniformLocation(grp->material->shader.id, "shadow_map"), tex_offset);
			tex_offset++;
		}

		char buf[32];
		for(u32 k = 0; k < spotlights.length; ++k)
		{
			sprintf(buf, "EE_spotlights_shadow_map[%d]", k);
			glActiveTexture(GL_TEXTURE0+tex_offset);
			glBindTexture(GL_TEXTURE_2D, spotlights.data[k]->frame_buffer_texture);
			glUniform1i(glGetUniformLocation(grp->material->shader.id, buf), tex_offset);
			
			mat4 light_view = mat4_rotate(mat4_default, (vec3){.x = spotlights.data[k]->rot.x, .y = 0.0f, .z = 0.0f});
			light_view 		= mat4_rotate(light_view, (vec3){.x = 0.0f, .y = spotlights.data[k]->rot.y, .z = 0.0f});
			light_view 		= mat4_rotate(light_view, (vec3){.x = 0.0f, .y = 0.0f, .z = spotlights.data[k]->rot.z});
			
			vec3 pos_inv = (vec3){.x = -spotlights.data[k]->pos.x,
								  .y = -spotlights.data[k]->pos.y,
								  .z = -spotlights.data[k]->pos.z};
			light_view = mat4_translate(light_view, pos_inv);
			tex_offset++;
		}
        
        for(u32 k = 0; k < pointlights.length; ++k)
        {
            glActiveTexture(GL_TEXTURE0+tex_offset);
            
            sprintf(buf, "cube_shadow_map[%d]", k);
            glBindTexture(GL_TEXTURE_CUBE_MAP, pointlights.data[k]->frame_buffer_texture);
            glUniform1i(glGetUniformLocation(grp->material->shader.id, buf), tex_offset);
            
            tex_offset++;
        }

		cwsBindMesh(grp->mesh);

		//Render every renderer that's using the bound material & mesh
        for(u32 j = 0; j < cws_bucket_array_item_count(grp->renderers); ++j)
		{
            if(!cws_bucket_array_occupied(grp->renderers, j))
            {
                continue;
            }
            
            cwsRenderer *obj = &cws_bucket_array_index(grp->renderers,j);
			vec3 b1 = vec3_add(obj->minB,obj->maxB);
			vec3 b2 = vec3_sub(obj->maxB,obj->minB);
			b1 = vec3_mul_scalar(b1, 0.5f);
			b2 = vec3_mul_scalar(b2, 0.5f);
			if(!cube_in_frustum(b1, b2, planes))
			{
			//	continue;
			}

			mat4 t = mat4_translate(mat4_default, obj->position);
			mat4 rt = quat_to_mat4(obj->rotation);
			t = mat4_mul(t, rt);
			t = mat4_scale(t, obj->scale);

			if(grp->mesh->anim_data != NULL)
			{
				if(obj->animation_clip_index >= 0 && obj->animation_clip_index < (i32)grp->mesh->anim_data->animations[0]->clips_count)
				{
					grp->mesh->anim_data->animations[0]->selected_clip = &grp->mesh->anim_data->animations[0]->clips[obj->animation_clip_index];
					cwsDrawAnimatedMesh(t, grp->mesh, GL_TRIANGLES, (f32)(SDL_GetTicks()*0.001f));
				}
			}
			else
            {
				cwsDrawMesh(t, grp->mesh, GL_TRIANGLES);
			}
		}
	}
}

cwsTerrain_Base *terrain_from_height_image(i32 chunks_x, i32 chunks_y, const char *file)
{
    SDL_Surface *img = IMG_Load(file);
    if(img == NULL)
    {
        cws_log("Couldn't load %s", file);
        return NULL;
    }

    SDL_Surface *conv = SDL_ConvertSurfaceFormat(img, SDL_PIXELFORMAT_RGBA8888, 0);

    cwsTerrain_Base *data = (cwsTerrain_Base*)malloc(sizeof(cwsTerrain_Base));
	data->position = (vec3){.x = 0, .y = 0, .z = 0};
	data->scale = (vec3){.x = 1, .y = 1, .z = 1};
	data->rotation = (quat){.x = 0, .y = 0, .z = 0, .w = 1};
    cwsMaterialInit(data->material);
    cwsShaderFromfile(&data->material.shader, "./data/shaders/single_v", "./data/shaders/single_f", NULL);
    cwsTexture2D tex0;
    cwsTextureFromfile(&tex0, "./data/gfx/tex.jpg", IF_LINEAR);
    cwsMaterialAddTexture(&data->material, tex0);

    cws_array_init(cwsMesh, data->chunks, 0);
    data->chunks_size = chunks_x*chunks_y;
    data->chunks_count = 0;

    i32 x = 0, y = 0;
    i32 w = img->w / chunks_x;
    i32 h = img->h / chunks_y;
    i32 attribs[] = {3, 3, 2, 3};
    for(i32 i = 0; i < chunks_x*chunks_y; ++i)
    {
        cwsMesh cm;
        cwsEmptyMesh(&cm, attribs, 4);
        i32 vx = 0, vy = 0;

        //Create vertices
        cws_array(f32,verts);
        cws_array_init(f32,verts,(w+1)*(h+1) * 11);

        cws_array(i32,indices);
        cws_array_init(i32,indices,(w+1)*(h+1) * 6);
        for(i32 j = 0; j < (w+1)*(h+1); ++j)
        {
            u32 pixel 		= cwsGetPixel(conv, vx + x*w, vy + y*h);
            f32 hL 			= (f32)(cwsGetPixel(conv, vx + x*w - 1, vy + y*h) >> 24);
            f32 hR 			= (f32)(cwsGetPixel(conv, vx + x*w + 1, vy + y*h) >> 24);
            f32 hD 			= (f32)(cwsGetPixel(conv, vx + x*w, vy + y*h - 1) >> 24);
            f32 hU 			= (f32)(cwsGetPixel(conv, vx + x*w, vy + y*h + 1) >> 24);

            vec3 norm = (vec3){.x = hL-hR, .y = 255.0f, .z = hD-hU};
            norm = vec3_normalize(norm);

            cws_array_push(verts, vx + x*w);
            cws_array_push(verts,(f32)(pixel>>24) * 0.04f);
            cws_array_push(verts,vy + y*h);

            cws_array_push(verts,norm.x);
            cws_array_push(verts,norm.y);
            cws_array_push(verts,norm.z);

            cws_array_push(verts,vx);
            cws_array_push(verts,vy);

            cws_array_push(verts,1.0f);
            cws_array_push(verts,1.0f);
            cws_array_push(verts,1.0f);

            vx ++;
            if(vx > w)
            {
                vx = 0;
                vy++;
            }
        }

        vx = 0;
        vy = 0;

        for(i32 j = 0; j < (w+1)*(h+1); ++j)
        {
        	if(vx < w && vy < h)
        	{
        		i32 ax = vx + (w+1) * vy;
        		cws_array_push(indices,ax);
        		cws_array_push(indices,ax+w+1);
        		cws_array_push(indices,ax+1);
        		
        		cws_array_push(indices,ax+w+1);
        		cws_array_push(indices,ax+w+2);
        		cws_array_push(indices,ax+1);
        	}

        	vx += 1;
        	if(vx > w)
        	{
        		vy += 1;
        		vx = 0;
        	}
        }

        cwsFillMesh(&cm, &verts.data[0], verts.length, &indices.data[0], indices.length);
        cws_array_push(data->chunks, cm);

        x++;
        if(x >= chunks_x)
        {
            x = 0;
            y ++;
        }

        cws_array_free(verts);
        cws_array_free(indices);
    }

    SDL_FreeSurface(img);
    return data;
}

void draw_terrain(cwsTerrain_Base *d, cwsCamera *cam)
{
	glDisable(GL_CULL_FACE);
    cwsBindMaterial(&d->material);
    
	//Bind the light data 
	u32 light_ubo_index = glGetUniformBlockIndex(d->material.shader.id, "LightData");
	glUniformBlockBinding(d->material.shader.id, light_ubo_index, 1);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, light_ubo_id);

	//Pass the light data to the shaders
	i32 tex_offset = d->material.texture_array.length;
	if(dirlights.length >= 1)
	{
		glActiveTexture(GL_TEXTURE0+tex_offset);
		glBindTexture(GL_TEXTURE_2D, csm_data.frame_buffer_texture);
		glUniform1i(glGetUniformLocation(d->material.shader.id, "shadow_map"), tex_offset);
		tex_offset++;
	}

	char buf[32];
	for(u32 k = 0; k < spotlights.length; ++k)
	{
		sprintf(buf, "EE_spotlights_shadow_map[%d]", k);
		glActiveTexture(GL_TEXTURE0+tex_offset);
		glBindTexture(GL_TEXTURE_2D, spotlights.data[k]->frame_buffer_texture);
		glUniform1i(glGetUniformLocation(d->material.shader.id, buf), tex_offset);
		
		mat4 light_view = mat4_rotate(light_view, (vec3){.x = spotlights.data[k]->rot.x, .y = 0.0f, .z = 0.0f});
		light_view = mat4_rotate(light_view, (vec3){.x = 0.0f, .y = spotlights.data[k]->rot.y, .z = 0.0f});
		light_view = mat4_rotate(light_view, (vec3){.x = 0.0f, .y = 0.0f, .z = spotlights.data[k]->rot.z});
		
		vec3 inv_pos = (vec3){.x = -spotlights.data[k]->pos.x,
						      .y = -spotlights.data[k]->pos.y,
							  .z = -spotlights.data[k]->pos.z};
		light_view = mat4_translate(light_view, inv_pos);
		tex_offset++;
	}

    mat4 t = mat4_translate(mat4_default, d->position);
    mat4 qmat = quat_to_mat4(d->rotation);
    t = mat4_mul(t, qmat);
    t = mat4_scale(t, d->scale);

    for(u32 i = 0; i < d->chunks_count; ++i)
    {
        cwsBindMesh(&d->chunks.data[i]);
        cwsDrawMesh(t, &d->chunks.data[i], GL_TRIANGLES);
    }
}

void delete_terrain(cwsTerrain_Base *d)
{
    for(u32 i = 0; i < d->material.texture_array.length; ++i)
    {
        cwsDeleteTexture(&d->material.texture_array.data[i]);
    }
    
    cws_array_free(d->chunks);
    free(d);
}

void cwsSceneDraw()
{
	if(active_camera != NULL)
	{
		vec3 dir = (vec3){.x = 0, .y = 0, .z = -1};
		dir = vec3_rotate(dir, (vec3){.x = active_camera->rot.x, .y = 0.0f, .z = 0.0f});
		dir = vec3_rotate(dir, (vec3){.x = 0.0f, .y = active_camera->rot.y, .z = 0.0f});
		dir = vec3_rotate(dir, (vec3){.x = 0.0f, .y = 0.0f, .z = active_camera->rot.z});
		active_camera->dir = dir;

		vec3 up = (vec3){.x = 0, .y = 1, .z = 0};
		up = vec3_rotate(up, (vec3){.x = active_camera->rot.x, .y = 0.0f, .z = 0.0f});
		up = vec3_rotate(up, (vec3){.x = 0.0f, .y = active_camera->rot.y, .z = 0.0f});
		up = vec3_rotate(up, (vec3){.x = 0.0f, .y = 0.0f, .z = active_camera->rot.z});

		vec3 pd = vec3_add(active_camera->position,dir);
		active_camera_view = mat4_lookat(active_camera->position, pd, up);
		cwsSetPVMatrices(active_camera_proj, active_camera_view);
        
        if(get_key_state(SDL_SCANCODE_G) == KEY_PRESSED)
        {
            cwsSetPVMatrices(csm_data.frame_projections[0], csm_data.light_view[0]);
        }
        else if(get_key_state(SDL_SCANCODE_H) == KEY_PRESSED)
        {
            cwsSetPVMatrices(csm_data.frame_projections[1], csm_data.light_view[1]);
        }
        else if(get_key_state(SDL_SCANCODE_J) == KEY_PRESSED)
        {
            cwsSetPVMatrices(csm_data.frame_projections[2], csm_data.light_view[2]);
        }
        else if(get_key_state(SDL_SCANCODE_K) == KEY_PRESSED)
        {
            cwsSetPVMatrices(csm_data.frame_projections[3], csm_data.light_view[3]);
        }
	}
	
	render();
}