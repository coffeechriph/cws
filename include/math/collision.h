#ifndef COLLISION_H
#define COLLISION_H
#include "float.h"
#include "string.h"
#include "math.h"
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

typedef struct
{
    vec3 points[3];
} triangle;

ray_hitinfo ray_aabb_test(ray r, aabb b);
ray_hitinfo ray_triangle_test(ray r, triangle t);
#endif