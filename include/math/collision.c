#include "collision.h"

#define swap(x,y) do \
{ unsigned char swap_temp[sizeof(x) == sizeof(y) ? (signed)sizeof(x) : -1]; \
 memcpy(swap_temp,&y,sizeof(x)); \
 memcpy(&y,&x,       sizeof(x)); \
 memcpy(&x,swap_temp,sizeof(x)); \
} while(0)
#define min(a,b) ((a > b) ? (b) : (a))
#define max(a,b) ((a > b) ? (a) : (b))

bool clip_line(f32 aabb_min, f32 aabb_max, f32 v0_c, f32 v1_c, f32* f_low, f32* f_high)
{
    f32 f_dim_low, f_dim_high;

    f_dim_low = (aabb_min - v0_c)/(v1_c - v0_c);
    f_dim_high = (aabb_max - v0_c)/(v1_c - v0_c);

    if (f_dim_high < f_dim_low)
        swap(f_dim_high, f_dim_low);

    if (f_dim_high < *f_low)
        return false;

    if (f_dim_low > *f_high)
        return false;

    (*f_low) = max(f_dim_low, (*f_low));
    (*f_high) = min(f_dim_high, (*f_high));

    if (*f_low > *f_high)
        return false;

    return true;
}

ray_hitinfo ray_aabb_test(ray r, aabb b)
{
    vec3 v0 = r.origin;
    vec3 v1 = vec3_add(r.origin, vec3_mul_scalar(r.dir, r.length));

    float f_low = 0;
    float f_high = 1;
    ray_hitinfo info;
    info.hit = false;

    if (!clip_line(b.center.x-b.extent.x, b.center.x+b.extent.x, v0.x, v1.x, &f_low, &f_high))
        return info;

    if (!clip_line(b.center.y-b.extent.y, b.center.y+b.extent.y, v0.y, v1.y, &f_low, &f_high))
        return info;

    if (!clip_line(b.center.z-b.extent.z, b.center.z+b.extent.z, v0.z, v1.z, &f_low, &f_high))
        return info;

    info.point = vec3_add(r.origin, vec3_mul_scalar(r.dir, f_low));
    info.hit = true;
    return info;
}
