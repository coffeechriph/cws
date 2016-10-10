#ifndef COLLISION_H
#define COLLISION_H
#include "string.h"
#include "matrix.h"
#include "../types.h"

typedef struct
{
    vec3 origin;
    vec3 dir;
    f32 length;
} ray;

typedef struct
{
    vec3 point;
    bool hit;
} ray_hitinfo;

typedef struct
{
    vec3 center;
    vec3 extent;
} aabb;

ray_hitinfo ray_aabb_test(ray r, aabb b);
#endif