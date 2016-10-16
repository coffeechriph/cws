#include "collision.h"

void _swap(f32 *f1, f32* f2)
{
    f32 f3 = *f1;
    *f1 = *f2;
    *f2 = f3;
}

bool clip_line(f32 aabb_min, f32 aabb_max, f32 v0_c, f32 v1_c, f32* f_low, f32* f_high)
{
    f32 f_dim_low, f_dim_high;

    f_dim_low = (aabb_min - v0_c)/(v1_c - v0_c);
    f_dim_high = (aabb_max - v0_c)/(v1_c - v0_c);

    if (f_dim_high < f_dim_low)
        _swap(&f_dim_high, &f_dim_low);
    
    if (f_dim_high < *f_low)
        return false;

    if (f_dim_low > *f_high)
        return false;

    (*f_low) = fmaxf(f_dim_low, (*f_low));
    (*f_high) = fminf(f_dim_high, (*f_high));
    
    if (*f_low > *f_high)
        return false;

    return true;
}

ray_hitinfo ray_aabb_test(ray r, aabb b)
{
    vec3 v0 = r.origin;
    vec3 v1 = vec3_add(r.origin, vec3_mul_scalar(r.dir, r.length));

    f32 f_low = 0;
    f32 f_high = 1;
    ray_hitinfo info;
    info.hit = false;
    info.point = (vec3){.x = 0, .y = 0, .z = 0};

    if (!clip_line(b.center.x-b.extent.x, b.center.x+b.extent.x, v0.x, v1.x, &f_low, &f_high))
        return info;

    if (!clip_line(b.center.y-b.extent.y, b.center.y+b.extent.y, v0.y, v1.y, &f_low, &f_high))
        return info;

    if (!clip_line(b.center.z-b.extent.z, b.center.z+b.extent.z, v0.z, v1.z, &f_low, &f_high))
        return info;

    info.point = vec3_add(r.origin, vec3_mul_scalar(r.dir, r.length*f_low));
    info.hit = true;
    return info;
}

ray_hitinfo ray_triangle_test(ray r, triangle t)
{
    ray_hitinfo info;
    info.hit = false;
    info.point = (vec3){0,0,0};
    
    vec3 e1, e2;
    e1 = vec3_sub(t.points[1], t.points[0]);
    e2 = vec3_sub(t.points[2], t.points[0]);
    vec3 p = vec3_crossproduct(r.dir, e2);
    f32 det = vec3_dot(e1, p);
    
    if(det > -0.000001f && det < -0.000001f)
    {
        return info;
    }
    
    f32 inv_det = 1.0f / det;
    
    vec3 dist = vec3_sub(r.origin, t.points[0]);
    
    f32 u = vec3_dot(dist, p);
    if(u < 0.0f || u > 1.0f)
    {
        return info;
    }
    
    vec3 q = vec3_crossproduct(dist, e1);
    f32 v = vec3_dot(r.dir, q);
    
    if(v < 0.0f || u + v > 1.0f)
    {
        return info;
    }
    
    f32 D = vec3_dot(e2, q) * inv_det;
    
    if(D > 0.000001f)
    {
        info.point = vec3_add(r.origin, vec3_mul_scalar(r.dir, r.length*D));
        info.hit = true;
        return info;
    }
    
    return info;
}
