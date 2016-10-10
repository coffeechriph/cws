#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <x86intrin.h>
#include "../types.h"
#include "../util/log.h"

#define DEG_2_RAD 3.14f / 180.0f
#define RAD_2_DEG 180.0f / 3.14f

typedef struct
{
    f32 x, y;
} vec2;

typedef struct
{
    i32 x, y;
} ivec2;

typedef struct 
{
    f32 x, y, z;
} vec3;

typedef struct 
{
    i32 x, y, z;
} ivec3;

typedef struct 
{
    f32 x, y, z, w;
} vec4;

typedef struct 
{
    i32 x, y, z, w;
} ivec4;

typedef struct
{
    f32 m[16];
} __attribute__((aligned(16))) mat4;

typedef struct
{
    f32 x,y,z,w;    
} quat;

const static vec2 vec2_default = {.x = 0.0f, .y = 0.0f};
const static ivec2 ivec2_default = {.x = 0, .y = 0};
const static vec3 vec3_default = {.x = 0.0f, .y = 0.0f, .z = 0.0f};
const static ivec3 ivec3_default = {.x = 0, .y = 0, .z = 0};
const static vec4 vec4_default = {.x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 0.0f};
const static mat4 mat4_default = {.m = {1.0f,0.0f,0.0f,0.0f,
                                         0.0f,1.0f,0.0f,0.0f,
                                         0.0f,0.0f,1.0f,0.0f,
                                         0.0f,0.0f,0.0f,1.0f}};
const static quat quat_default = {.x = 0.0f, .y = 0.0f, .z = 0.0f, .w = 1.0f};

vec3 vec3_add(vec3 v1, vec3 v2);
vec3 vec3_sub(vec3 v1, vec3 v2);
vec3 vec3_mul(vec3 v1, vec3 v2);
vec3 vec3_mul_scalar(vec3 v1, f32 s);
vec2 vec2_add(vec2 v1, vec2 v2);
vec4 vec4_add(vec4 v1, vec4 v2);
vec2 vec2_sub(vec2 v1, vec2 v2);
vec4 vec4_sub(vec4 v1, vec4 v2);
vec2 vec2_mul(vec2 v1, vec2 v2);
vec4 vec4_mul(vec4 v1, vec4 v2);

vec3    vec3_transform(vec3 v1, mat4 m);
vec4    vec4_transform(vec4 v1, mat4 m);
vec3    vec3_rotate(vec3 in, vec3 rad);
vec3    vec3_normalize(vec3 in);
vec3    vec3_crossproduct(vec3 v1, vec3 v2);
f32     vec3_length(vec3 in);
f32     vec3_length_sqr(vec3 in);
f32     vec3_dot(vec3 v1, vec3 v2);

mat4    mat4_mul(mat4 m2, mat4 m1);
mat4    mat4_ortho(f32 left, f32 right, f32 bot, f32 top, f32 near, f32 far);
mat4    mat4_perspective(i32 scrcwsn_w, i32 scrcwsn_h, f32 fov, f32 near, f32 far);
mat4    mat4_lookat(vec3 pos, vec3 target, vec3 up);
mat4    mat4_inverse(mat4 m);
mat4    mat4_transpose(mat4 m);
mat4    mat4_translate(mat4 m, vec3 t);
mat4    mat4_rotate(mat4 m, vec3 rot);
mat4    mat4_scale(mat4 m, vec3 s);

quat    quat_add(quat q1, quat q2);
quat    quat_mul(quat q1, quat q2);
quat    quat_mul_scalar(quat q1, f32 s);

quat    quat_from_mat4(mat4 m);
quat    quat_from_euler(vec3 r);
quat    quat_lookat(vec3 pos, vec3 target);
mat4    quat_to_mat4(quat q);
quat    quat_normalize(quat q1);
quat    quat_lerp(quat from, quat to, f32 time);
quat    quat_slerp(quat from, quat to, f32 time);
quat    quat_conjugate(quat q);
vec3    quat_rotate_vector(quat q, vec3 v);
f32     quat_dot(quat q1, quat q2);
f32     quat_length(quat q1);

f32 bary_centric1D(vec3 p1, vec3 p2, vec3 p3, vec2 pos);
vec3 bary_centric3D(vec3 p, vec3 a, vec3 b, vec3 c);

void    mat4_print(const char *s, mat4 m);
void    quat_print(const char *s, quat m);
void    vec2_print(const char *s, vec2 v);
void    vec3_print(const char *s, vec3 v);
void    vec4_print(const char *s, vec4 v);
