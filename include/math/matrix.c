#include "matrix.h"

vec3 vec3_add(vec3 v1, vec3 v2)
{
    return (vec3){.x = v1.x + v2.x, .y = v1.y + v2.y, .z = v1.z + v2.z};
}

vec3 vec3_sub(vec3 v1, vec3 v2)
{
    return (vec3) {.x=v1.x - v2.x, .y=v1.y - v2.y, .z=v1.z - v2.z };
}

vec3 vec3_mul(vec3 v1, vec3 v2)
{
    return (vec3) {.x = v1.x*v2.x, .y = v1.y*v2.y, .z = v1.z*v2.z};
}

vec3 vec3_mul_scalar(vec3 v1, f32 s)
{
    return (vec3) { .x = v1.x * s, .y = v1.y * s, .z = v1.z * s};
}

vec2 vec2_add(vec2 v1, vec2 v2)
{
    return (vec2){.x = v1.x + v2.x, .y = v1.y + v2.y};
}

vec4 vec4_add(vec4 v1, vec4 v2)
{
    return (vec4){.x = v1.x + v2.x, .y = v1.y + v2.y, .z = v1.z + v2.z, .w = v1.w + v2.w};
}

vec2 vec2_sub(vec2 v1, vec2 v2)
{ 
    return (vec2) {.x = v1.x - v2.x, .y = v1.y - v2.y};
}

vec4 vec4_sub(vec4 v1, vec4 v2)
{
    return (vec4) { .x = v1.x - v2.x, .y = v1.y - v2.y, .z = v1.z - v2.z, .w = v1.w - v2.w };
}

vec2 vec2_mul(vec2 v1, vec2 v2)
{
    return (vec2) {.x = v1.x * v2.x, .y = v1.y * v2.y};
}

vec4 vec4_mul(vec4 v1, vec4 v2)
{
    return (vec4) { .x = v1.x*v2.x, .y = v1.y*v2.y, .z = v1.z*v2.z, .w = v1.w*v2.w };
}

vec3 vec3_transform(vec3 v1, mat4 m)
{
	return (vec3) { .x = v1.x * m.m[0] + v1.y * m.m[4] + v1.z * m.m[8] + m.m[12], 
					.y = v1.x * m.m[1] + v1.y * m.m[5] + v1.z * m.m[9] + m.m[13], 
					.z = v1.x * m.m[2] + v1.y * m.m[6] + v1.z * m.m[10] + m.m[14] };
}

vec4 vec4_transform(vec4 v1, mat4 m)
{
	return (vec4) { .x = v1.x * m.m[0] + v1.y * m.m[4] + v1.z * m.m[8] + v1.w * m.m[12],
					.y = v1.x * m.m[1] + v1.y * m.m[5] + v1.z * m.m[9] + v1.w * m.m[13],
					.z = v1.x * m.m[2] + v1.y * m.m[6] + v1.z * m.m[10] + v1.w * m.m[14],
					.w = v1.x * m.m[3] + v1.y * m.m[7] + v1.z * m.m[11] + v1.w * m.m[15] };
}

vec3 vec3_rotate(vec3 in, vec3 rad)
{
	mat4 rot  = mat4_rotate(mat4_default, (vec3){.x = rad.x,.y = 0,.z = 0});
	rot       = mat4_rotate(rot, (vec3){.x = 0, .y = rad.y, .z = 0});
	rot       = mat4_rotate(rot, (vec3){.x = 0, .y = 0, .z = rad.z});

	return (vec3) { .x = in.x*rot.m[0] + in.y*rot.m[4] + in.z*rot.m[8], 
					.y = in.x*rot.m[1] + in.y*rot.m[5] + in.z*rot.m[9],
					.z = in.x*rot.m[2] + in.y*rot.m[6] + in.z*rot.m[10] };
}

vec3 vec3_normalize(vec3 in)
{
	f32 s = sqrt(in.x*in.x + in.y*in.y + in.z*in.z);
	f32 sl = 1;
	if(s != 0)
	{
		sl = 1.0f / s;
	}

	return (vec3) { .x = in.x * sl, .y = in.y * sl, .z = in.z * sl};
}

f32 vec3_length(vec3 in)
{
	return sqrt(in.x*in.x + in.y*in.y + in.z*in.z);
}

f32 vec3_length_sqr(vec3 in)
{
	return in.x*in.x + in.y*in.y + in.z*in.z;
}

vec3 vec3_crossproduct(vec3 v1, vec3 v2)
{
	return (vec3) { .x = v1.y * v2.z - v1.z * v2.y,
					.y = v1.z * v2.x - v1.x * v2.z,
					.z = v1.x * v2.y - v1.y * v2.x };
}

f32 vec3_dot(vec3 v1, vec3 v2)
{
	return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
}

mat4 mat4_mul(mat4 m2, mat4 m1)
{
    #ifdef __SSE2__
    mat4 res;
        __m128 row1 = _mm_load_ps(&m2.m[0]);
        __m128 row2 = _mm_load_ps(&m2.m[4]);
        __m128 row3 = _mm_load_ps(&m2.m[8]);
        __m128 row4 = _mm_load_ps(&m2.m[12]);

        //1
        __m128 b1 = _mm_set1_ps(m1.m[0]);
        __m128 b2 = _mm_set1_ps(m1.m[1]);
        __m128 b3 = _mm_set1_ps(m1.m[2]);
        __m128 b4 = _mm_set1_ps(m1.m[3]);

        __m128 arow = _mm_add_ps(
                        _mm_add_ps(
                            _mm_mul_ps(b1, row1),
                            _mm_mul_ps(b2, row2)),
                        _mm_add_ps(_mm_mul_ps(b3, row3),
                                   _mm_mul_ps(b4, row4))
                     );

        //2
        b1 = _mm_set1_ps(m1.m[4]);
        b2 = _mm_set1_ps(m1.m[5]);
        b3 = _mm_set1_ps(m1.m[6]);
        b4 = _mm_set1_ps(m1.m[7]);

        __m128 brow = _mm_add_ps(
                        _mm_add_ps(
                            _mm_mul_ps(b1, row1),
                            _mm_mul_ps(b2, row2)),
                        _mm_add_ps(_mm_mul_ps(b3, row3),
                                   _mm_mul_ps(b4, row4))
                     );

        //3
        b1 = _mm_set1_ps(m1.m[8]);
        b2 = _mm_set1_ps(m1.m[9]);
        b3 = _mm_set1_ps(m1.m[10]);
        b4 = _mm_set1_ps(m1.m[11]);

        __m128 crow = _mm_add_ps(
                        _mm_add_ps(
                            _mm_mul_ps(b1, row1),
                            _mm_mul_ps(b2, row2)),
                        _mm_add_ps(_mm_mul_ps(b3, row3),
                                   _mm_mul_ps(b4, row4))
                     );

        //4
        b1 = _mm_set1_ps(m1.m[12]);
        b2 = _mm_set1_ps(m1.m[13]);
        b3 = _mm_set1_ps(m1.m[14]);
        b4 = _mm_set1_ps(m1.m[15]);

        __m128 drow = _mm_add_ps(
                        _mm_add_ps(
                            _mm_mul_ps(b1, row1),
                            _mm_mul_ps(b2, row2)),
                        _mm_add_ps(_mm_mul_ps(b3, row3),
                                   _mm_mul_ps(b4, row4))
                     );

        _mm_store_ps(&res.m[0], arow);
        _mm_store_ps(&res.m[4], brow);
        _mm_store_ps(&res.m[8], crow);
        _mm_store_ps(&res.m[12], drow);
        return res;
    #else
    return (mat4) { .m[0] = m1.m[0 ] * m2.m[0 ] + m1.m[1 ] * m2.m[4 ] + m1.m[2 ] * m2.m[8 ] + m1.m[3 ] * m2.m[12],
                    .m[1] = m1.m[0 ] * m2.m[1 ] + m1.m[1 ] * m2.m[5 ] + m1.m[2 ] * m2.m[9 ] + m1.m[3 ] * m2.m[13],
                    .m[2] = m1.m[0 ] * m2.m[2 ] + m1.m[1 ] * m2.m[6 ] + m1.m[2 ] * m2.m[10] + m1.m[3 ] * m2.m[14],
                    .m[3] = m1.m[0 ] * m2.m[3 ] + m1.m[1 ] * m2.m[7 ] + m1.m[2 ] * m2.m[11] + m1.m[3 ] * m2.m[15],

                    .m[4] = m1.m[4 ] * m2.m[0 ] + m1.m[5 ] * m2.m[4 ] + m1.m[6 ] * m2.m[8 ] + m1.m[7 ] * m2.m[12],
                    .m[5] = m1.m[4 ] * m2.m[1 ] + m1.m[5 ] * m2.m[5 ] + m1.m[6 ] * m2.m[9 ] + m1.m[7 ] * m2.m[13],
                    .m[6] = m1.m[4 ] * m2.m[2 ] + m1.m[5 ] * m2.m[6 ] + m1.m[6 ] * m2.m[10] + m1.m[7 ] * m2.m[14],
                    .m[7] = m1.m[4 ] * m2.m[3 ] + m1.m[5 ] * m2.m[7 ] + m1.m[6 ] * m2.m[11] + m1.m[7 ] * m2.m[15],

                    .m[8] = m1.m[8 ] * m2.m[0 ] + m1.m[9 ] * m2.m[4 ] + m1.m[10] * m2.m[8 ] + m1.m[11] * m2.m[12],
                    .m[9] = m1.m[8 ] * m2.m[1 ] + m1.m[9 ] * m2.m[5 ] + m1.m[10] * m2.m[9 ] + m1.m[11] * m2.m[13],
                    .m[10] = m1.m[8 ] * m2.m[2 ] + m1.m[9 ] * m2.m[6 ] + m1.m[10] * m2.m[10] + m1.m[11] * m2.m[14],
                    .m[11] = m1.m[8 ] * m2.m[3 ] + m1.m[9 ] * m2.m[7 ] + m1.m[10] * m2.m[11] + m1.m[11] * m2.m[15],

                    .m[12] = m1.m[12] * m2.m[0 ] + m1.m[13] * m2.m[4 ] + m1.m[14] * m2.m[8 ] + m1.m[15] * m2.m[12],
                    .m[13] = m1.m[12] * m2.m[1 ] + m1.m[13] * m2.m[5 ] + m1.m[14] * m2.m[9 ] + m1.m[15] * m2.m[13],
                    .m[14] = m1.m[12] * m2.m[2 ] + m1.m[13] * m2.m[6 ] + m1.m[14] * m2.m[10] + m1.m[15] * m2.m[14],
                    .m[15] = m1.m[12] * m2.m[3 ] + m1.m[13] * m2.m[7 ] + m1.m[14] * m2.m[11] + m1.m[15] * m2.m[15] };
    #endif
}

mat4 mat4_inverse(mat4 mx)
{
	f64 inv[16], det;
	int i;

	f32 *m = mx.m;
	inv[0] = 	m[5]  * m[10] * m[15] - 
				m[5]  * m[11] * m[14] - 
				m[9]  * m[6]  * m[15] + 
				m[9]  * m[7]  * m[14] +
				m[13] * m[6]  * m[11] - 
				m[13] * m[7]  * m[10];

	inv[4] = 	-m[4]  * m[10] * m[15] + 
				m[4]  * m[11] * m[14] + 
				m[8]  * m[6]  * m[15] - 
				m[8]  * m[7]  * m[14] - 
				m[12] * m[6]  * m[11] + 
				m[12] * m[7]  * m[10];

	inv[8] = 	m[4]  * m[9] * m[15] - 
				m[4]  * m[11] * m[13] - 
				m[8]  * m[5] * m[15] + 
				m[8]  * m[7] * m[13] + 
				m[12] * m[5] * m[11] - 
				m[12] * m[7] * m[9];

	inv[12] = 	-m[4]  * m[9] * m[14] + 
				m[4]  * m[10] * m[13] +
				m[8]  * m[5] * m[14] - 
				m[8]  * m[6] * m[13] - 
				m[12] * m[5] * m[10] + 
				m[12] * m[6] * m[9];

	inv[1] = 	-m[1]  * m[10] * m[15] + 
				m[1]  * m[11] * m[14] + 
				m[9]  * m[2] * m[15] - 
				m[9]  * m[3] * m[14] - 
				m[13] * m[2] * m[11] + 
				m[13] * m[3] * m[10];

	inv[5] = 	m[0]  * m[10] * m[15] - 
				m[0]  * m[11] * m[14] - 
				m[8]  * m[2] * m[15] + 
				m[8]  * m[3] * m[14] + 
				m[12] * m[2] * m[11] - 
				m[12] * m[3] * m[10];

	inv[9] = 	-m[0]  * m[9] * m[15] + 
				m[0]  * m[11] * m[13] + 
				m[8]  * m[1] * m[15] - 
				m[8]  * m[3] * m[13] - 
				m[12] * m[1] * m[11] + 
				m[12] * m[3] * m[9];

	inv[13] = 	m[0]  * m[9] * m[14] - 
				m[0]  * m[10] * m[13] - 
				m[8]  * m[1] * m[14] + 
				m[8]  * m[2] * m[13] + 
				m[12] * m[1] * m[10] - 
				m[12] * m[2] * m[9];

	inv[2] = 	m[1]  * m[6] * m[15] - 
				m[1]  * m[7] * m[14] - 
				m[5]  * m[2] * m[15] + 
				m[5]  * m[3] * m[14] + 
				m[13] * m[2] * m[7] - 
				m[13] * m[3] * m[6];

	inv[6] = 	-m[0]  * m[6] * m[15] + 
				m[0]  * m[7] * m[14] + 
				m[4]  * m[2] * m[15] - 
				m[4]  * m[3] * m[14] - 
				m[12] * m[2] * m[7] + 
				m[12] * m[3] * m[6];

	inv[10] = 	m[0]  * m[5] * m[15] - 
				m[0]  * m[7] * m[13] - 
				m[4]  * m[1] * m[15] + 
				m[4]  * m[3] * m[13] + 
				m[12] * m[1] * m[7] - 
				m[12] * m[3] * m[5];

	inv[14] = 	-m[0]  * m[5] * m[14] + 
				m[0]  * m[6] * m[13] + 
				m[4]  * m[1] * m[14] - 
				m[4]  * m[2] * m[13] - 
				m[12] * m[1] * m[6] + 
				m[12] * m[2] * m[5];

	inv[3] = 	-m[1] * m[6] * m[11] + 
				m[1] * m[7] * m[10] + 
				m[5] * m[2] * m[11] - 
				m[5] * m[3] * m[10] - 
				m[9] * m[2] * m[7] + 
				m[9] * m[3] * m[6];

	inv[7] = 	m[0] * m[6] * m[11] - 
				m[0] * m[7] * m[10] - 
				m[4] * m[2] * m[11] + 
				m[4] * m[3] * m[10] + 
				m[8] * m[2] * m[7] - 
				m[8] * m[3] * m[6];

	inv[11] = 	-m[0] * m[5] * m[11] + 
				m[0] * m[7] * m[9] + 
				m[4] * m[1] * m[11] - 
				m[4] * m[3] * m[9] - 
				m[8] * m[1] * m[7] + 
				m[8] * m[3] * m[5];

	inv[15] = 	m[0] * m[5] * m[10] - 
				m[0] * m[6] * m[9] - 
				m[4] * m[1] * m[10] + 
				m[4] * m[2] * m[9] + 
				m[8] * m[1] * m[6] - 
				m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	if (det == 0)
	{
		return (mat4) { .m[0] = 1.0f, .m[1] = 0.0f, .m[2] = 0.0f, .m[3] = 0.0f,
						.m[4] = 0.0f, .m[5] = 1.0f, .m[6] = 0.0f, .m[7] = 0.0f,
						.m[8] = 0.0f, .m[9] = 0.0f, .m[10] = 1.0f, .m[11] = 0.0f,
						.m[12] = 0.0f, .m[13] = 0.0f, .m[14] = 0.0f, .m[15] = 1.0f};
	}

	det = 1.0f / det;

	mat4 res;
	for (i = 0; i < 16; i++)
	{
		res.m[i] = inv[i] * det;
	}

	return res;
}

mat4 mat4_transpose(mat4 m)
{
	return mat4_default;    
}

mat4 mat4_ortho(f32 left, f32 right, f32 bot, f32 top, f32 near, f32 far)
{
	return (mat4) { .m[0] = 2.0f / (right-left), .m[1] = 0.0f, .m[2] = 0.0f, .m[3] = 0.0f, 
					.m[4] = 0.0f, .m[5] = 2.0f / (top-bot), .m[6] = 0.0f, .m[7] = 0.0f,
					.m[8] = 0.0f, .m[9] = 0.0f, .m[10] = (-2.0f) / (far-near), .m[11] = 0.0f,
					.m[12] = -(right+left) / (right-left), .m[13] = -(top+bot) / (top-bot), .m[14] = -(far+near) / (far-near), .m[15] = 1.0f};
}

mat4 mat4_perspective(i32 scrcwsn_w, i32 scrcwsn_h, f32 fov, f32 near, f32 far)
{
	fov *= DEG_2_RAD;
	f32 aspect = (f32)scrcwsn_w/(f32)scrcwsn_h;
	f32 thf = 1.0f / tan(fov*0.5f);
	f32 range = near - far;

	return (mat4) { .m[0] = thf / aspect, .m[1] = 0.0f, .m[2] = 0.0f, .m[3] = 0.0f,
					.m[4] = 0.0f,         .m[5] = thf,  .m[6] = 0.0f, .m[7] = 0.0f,  
					.m[8] = 0.0f,         .m[9] = 0.0f, .m[10] = -far / (far-(2*near)), .m[11] = -1.0f,
					.m[12] = 0.0f,        .m[13] = 0.0f,.m[14] = 2.0f * far * near / range, .m[15] = 0.0f };
}

mat4 mat4_lookat(vec3 pos, vec3 target, vec3 up)
{
	vec3 zaxis = vec3_sub(pos, target);
	zaxis = vec3_normalize(zaxis);
	vec3 xaxis = vec3_crossproduct(up, zaxis);
	xaxis = vec3_normalize(xaxis);

	vec3 yaxis = vec3_crossproduct(zaxis, xaxis);

	return (mat4) { .m[0] = xaxis.x, .m[1] = yaxis.x, .m[2] = zaxis.x, .m[3] = 0,
					.m[4] = xaxis.y, .m[5] = yaxis.y, .m[6] = zaxis.y, .m[7] = 0,
					.m[8] = xaxis.z, .m[9] = yaxis.z, .m[10] = zaxis.z, .m[11] = 0,
					.m[12] = -vec3_dot(xaxis,pos), .m[13] = -vec3_dot(yaxis,pos), .m[14] = -vec3_dot(zaxis,pos), .m[15] = 1 };
}

mat4 mat4_translate(mat4 m, vec3 t)
{
	mat4 tm = mat4_default;
	tm.m[12] = t.x;
	tm.m[13] = t.y;
	tm.m[14] = t.z;
	return (mat4) {.m[0] = m.m[0 ], .m[1] = m.m[1 ], .m[2] = m.m[2 ], .m[3] = m.m[3 ],
				   .m[4] = m.m[4 ], .m[5] = m.m[5 ], .m[6] = m.m[6 ], .m[7] = m.m[7 ],
				   .m[8] = m.m[8 ], .m[9] = m.m[9 ], .m[10] = m.m[10],.m[11] =  m.m[11],
		           .m[12] = tm.m[12] * m.m[0 ] + tm.m[13] * m.m[4 ] + tm.m[14] * m.m[8 ] + m.m[12],
		           .m[13] = tm.m[12] * m.m[1 ] + tm.m[13] * m.m[5 ] + tm.m[14] * m.m[9 ] + m.m[13],
		           .m[14] = tm.m[12] * m.m[2 ] + tm.m[13] * m.m[6 ] + tm.m[14] * m.m[10] + m.m[14],
		           .m[15] = tm.m[12] * m.m[3 ] + tm.m[13] * m.m[7 ] + tm.m[14] * m.m[11] + m.m[15]};
}

mat4 mat4_rotate(mat4 m, vec3 rot)
{  
	rot.x *= DEG_2_RAD;
	rot.y *= DEG_2_RAD;
	rot.z *= DEG_2_RAD;

	mat4 tm = mat4_default;
	f32 cr = cos( rot.x );
	f32 sr = sin( rot.x );
	f32 cp = cos( rot.y );
	f32 sp = sin( rot.y );
	f32 cy = cos( rot.z );
	f32 sy = sin( rot.z );

	tm.m[0] = cp*cy;
	tm.m[1] = cp*sy;
	tm.m[2] = -sp;

	f32 srsp = sr*sp;
	f32 crsp = cr*sp;

	tm.m[4] = srsp*cy-cr*sy;
	tm.m[5] = srsp*sy+cr*cy;
	tm.m[6] = sr*cp;

	tm.m[8] = crsp*cy+sr*sy;
	tm.m[9] = crsp*sy-sr*cy;
	tm.m[10] = cr*cp;

	return (mat4){	.m[0] = tm.m[0 ] * m.m[0 ] + tm.m[1 ] * m.m[4 ] + tm.m[2 ] * m.m[8 ],
                    .m[1] = tm.m[0 ] * m.m[1 ] + tm.m[1 ] * m.m[5 ] + tm.m[2 ] * m.m[9 ],
                    .m[2] = tm.m[0 ] * m.m[2 ] + tm.m[1 ] * m.m[6 ] + tm.m[2 ] * m.m[10],
                    .m[3] = tm.m[0 ] * m.m[3 ] + tm.m[1 ] * m.m[7 ] + tm.m[2 ] * m.m[11],

                    .m[4] = tm.m[4 ] * m.m[0 ] + tm.m[5 ] * m.m[4 ] + tm.m[6 ] * m.m[8 ],
                    .m[5] = tm.m[4 ] * m.m[1 ] + tm.m[5 ] * m.m[5 ] + tm.m[6 ] * m.m[9 ],
                    .m[6] = tm.m[4 ] * m.m[2 ] + tm.m[5 ] * m.m[6 ] + tm.m[6 ] * m.m[10],
                    .m[7] = tm.m[4 ] * m.m[3 ] + tm.m[5 ] * m.m[7 ] + tm.m[6 ] * m.m[11],

                    .m[8] = tm.m[8 ] * m.m[0 ] + tm.m[9 ] * m.m[4 ] + tm.m[10] * m.m[8 ],
                    .m[9] = tm.m[8 ] * m.m[1 ] + tm.m[9 ] * m.m[5 ] + tm.m[10] * m.m[9 ],
                    .m[10] = tm.m[8 ] * m.m[2 ] + tm.m[9 ] * m.m[6 ] + tm.m[10] * m.m[10],
                    .m[11] = tm.m[8 ] * m.m[3 ] + tm.m[9 ] * m.m[7 ] + tm.m[10] * m.m[11],

                    .m[12] = tm.m[15] * m.m[12],
                    .m[13] = tm.m[15] * m.m[13],
                    .m[14] = tm.m[15] * m.m[14],
                    .m[15] = tm.m[15] * m.m[15]};
}

mat4 mat4_scale(mat4 m, vec3 s)
{
	mat4 tm = mat4_default;
	tm.m[0] = s.x;
	tm.m[5] = s.y;
	tm.m[10] = s.z;

	return (mat4) {
		.m[0] = tm.m[0 ] * m.m[0 ],
        .m[1] = tm.m[0 ] * m.m[1 ],
        .m[2] = tm.m[0 ] * m.m[2 ],
        .m[3] = tm.m[0 ] * m.m[3 ],

        .m[4] = tm.m[5 ] * m.m[4 ],
        .m[5] = tm.m[5 ] * m.m[5 ],
        .m[6] = tm.m[5 ] * m.m[6 ],
        .m[7] = tm.m[5 ] * m.m[7 ],

        .m[8] = tm.m[10] * m.m[8 ],
        .m[9] = tm.m[10] * m.m[9 ],
        .m[10] = tm.m[10] * m.m[10],
        .m[11] = tm.m[10] * m.m[11],

        .m[12] = tm.m[15] * m.m[12],
        .m[13] = tm.m[15] * m.m[13],
        .m[14] = tm.m[15] * m.m[14],
        .m[15] = tm.m[15] * m.m[15]
	};
}

quat quat_mul(quat q1, quat q2)
{
	return (quat) { .x = q1.x * q2.w + q1.y * q2.z - q1.z * q2.y + q1.w * q1.x,
					.y = -q1.x * q2.z + q1.y * q2.w - q1.z * q2.x + q1.w * q1.y,
					.z = q1.x * q2.y + q1.y * q2.x - q1.z * q2.w + q1.w * q1.z,
					.w = -q1.x * q2.x + q1.y * q2.y - q1.z * q2.z + q1.w * q1.w 	};
}

quat quat_mul_scalar(quat q1, f32 s)
{
	return (quat) { .x = q1.x * s, .y = q1.y * s, .z = q1.z * s, .w = q1.w * s };
}

quat quat_add(quat q1, quat q2)
{
	return (quat) { .x = q1.x + q2.x, .y = q1.y + q2.y, .z = q1.z + q2.z, .w = q1.w + q2.w };
}

quat quat_from_mat4(mat4 m)
{
	quat q = (quat){.x = 0, .y = 0, .z = 0, .w = 1};
	f32 diag = m.m[0] + m.m[5] + m.m[10] + 1;

	if(diag > 0.0f)
	{
		f32 scale = (sqrtf(diag) * 2.0f);
		f32 s = 1.0f / scale;
		q.x = (m.m[6] - m.m[9]) * s;
		q.y = (m.m[8] - m.m[2]) * s;
		q.z = (m.m[1] - m.m[4]) * s;
		q.w = 0.25f * scale;
	}
	else
	{
		if(m.m[0] > m.m[5] && m.m[0] > m.m[10])
		{
			f32 scale = sqrtf(1.0f + m.m[0] - m.m[5] - m.m[10]) * 2.0f;
			f32 s = 1.0f / scale;

			q.x = 0.25f * scale;
			q.y = (m.m[4] + m.m[1]) * s;
			q.z = (m.m[2] + m.m[8]) * s;
			q.w = (m.m[6] - m.m[9]) * s;
		}
		else if(m.m[5] > m.m[10])
		{
			f32 scale = sqrtf(1.0f + m.m[5] - m.m[0] - m.m[10]) * 2.0f;
			f32 s = 1.0f / scale;

			q.x = (m.m[4] + m.m[1]) * s;
			q.y = 0.25f * scale;
			q.z = (m.m[9] + m.m[6]) * s;
			q.w = (m.m[8] - m.m[2]) * s;
		}
		else
		{
			f32 scale = sqrtf(1.0f + m.m[10] - m.m[0] - m.m[5]) * 2.0f;
			f32 s = 1.0f / scale;

			q.x = (m.m[8] + m.m[2]) * s;
			q.y = (m.m[9] + m.m[6]) * s;
			q.z = 0.25f * scale;
			q.w = (m.m[1] - m.m[4]) * s;
		}
	}

	return quat_normalize(q);
}

quat quat_from_euler(vec3 r)
{
	f32 angle;

	angle = r.x * DEG_2_RAD * 0.5;
	const f32 sr = sin(angle);
	const f32 cr = cos(angle);

	angle = r.y * DEG_2_RAD * 0.5;
	const f32 sp = sin(angle);
	const f32 cp = cos(angle);

	angle = r.z * DEG_2_RAD * 0.5;
	const f32 sy = sin(angle);
	const f32 cy = cos(angle);

	const f32 cpcy = cp * cy;
	const f32 spcy = sp * cy;
	const f32 cpsy = cp * sy;
	const f32 spsy = sp * sy;

	quat qt;
	qt.x = (f32)(sr * cpcy - cr * spsy);
	qt.y = (f32)(cr * spcy + sr * cpsy);
	qt.z = (f32)(cr * cpsy - sr * spcy);
	qt.w = (f32)(cr * cpcy + sr * spsy);

	return quat_normalize(qt);
}

quat quat_from_axis_angle(vec3 axis, f32 angle)
{
	f32 half_angle = angle * 0.5f;
	f32 s = sin(half_angle);
	quat q;
	q.x = axis.x * s;
	q.y = axis.y * s;
	q.z = axis.z * s;
	q.w = cos(half_angle);
	return q;
}

quat quat_lookat(vec3 pos, vec3 target)
{
	return quat_conjugate(quat_from_mat4(mat4_lookat(pos,target,(vec3){.x=0,.y=1,.z=0})));
}

mat4 quat_to_mat4(quat q)
{
	f32 X = q.x;
	f32 Y = q.y;
	f32 Z = q.z;
	f32 W = q.w;

	return (mat4) { .m[0] = 1.0f - 2.0f*Y*Y - 2.0f*Z*Z,
					.m[1] = 2.0f*X*Y + 2.0f*Z*W,
					.m[2] = 2.0f*X*Z - 2.0f*Y*W,
					.m[3] = 0.0f,

					.m[4] = 2.0f*X*Y - 2.0f*Z*W,
					.m[5] = 1.0f - 2.0f*X*X - 2.0f*Z*Z,
					.m[6] = 2.0f*Z*Y + 2.0f*X*W,
					.m[7] = 0.0f,

					.m[8] = 2.0f*X*Z + 2.0f*Y*W,
					.m[9] = 2.0f*Z*Y - 2.0f*X*W,
					.m[10] = 1.0f - 2.0f*X*X - 2.0f*Y*Y,
					.m[11] = 0.0f,

					.m[12] = 0.0f,
					.m[13] = 0.0f,
					.m[14] = 0.0f,
					.m[15] = 1.0f	};
}

quat quat_normalize(quat q1)
{
	f32 n = sqrt(q1.x * q1.x + q1.y * q1.y + q1.z * q1.z + q1.w * q1.w);
	if(n != 0) n = 1.0f / n;

	return (quat) { .x = q1.x * n,
					.y = q1.y * n,
					.z = q1.z * n,
					.w = q1.w * n };
}

quat quat_lerp(quat from, quat to, f32 time)
{
	f32 scale = 1.0f - time;
	quat q1 = quat_mul_scalar(from, scale);
	quat q2 = quat_mul_scalar(to,time);
	return quat_add(q1,q2);
}

quat quat_slerp(quat from, quat to, f32 time)
{
	f32 angle = quat_dot(from, to);
	quat res1 = from;

	if(angle < 0.0f)
	{
		res1 = quat_mul_scalar(from, -1.0f);
		angle = abs(angle);
	}   

	if(angle < 0.9f)
	{
		f32 the = acosf(angle);
		f32 inv_sin_theta = 1.0f / sinf(the);
		f32 scale = sinf(the * (1.0f - time)) * inv_sin_theta;
		f32 invscale = sinf(the * time) * inv_sin_theta;

		quat res2 = quat_mul_scalar(res1, scale);
		quat res3 = quat_mul_scalar(to, invscale);
		return quat_add(res2, res3);
	}
	else
	{
		return quat_lerp(res1, to, time);
	}
}

quat quat_conjugate(quat q)
{
	return (quat){.x = -q.x, .y = -q.y, .z = -q.z, .w = q.w};
}

vec3 quat_rotate_vector(quat q, vec3 v)
{
	vec3 qv = (vec3){.x = q.x, .y = q.y, .z = q.z};
	vec3 t = vec3_mul_scalar(vec3_crossproduct(qv,v), 2.0f);
	return vec3_add(vec3_add(v, vec3_mul_scalar(t, q.w)), vec3_crossproduct(qv, t));
}

f32 quat_dot(quat q1, quat q2)
{
	return (q1.x * q2.x) + (q1.y * q2.y) + (q1.z * q2.z) + (q1.w * q2.w);
}

f32 quat_length(quat q1)
{
	return sqrt(q1.x*q1.x + q1.y*q1.y + q1.z*q1.z + q1.w*q1.w);
}

f32 bary_centric1D(vec3 p1, vec3 p2, vec3 p3, vec2 pos)
{
    f32 det = (p2.z - p3.z) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.z - p3.z);
    f32 l1 = ((p2.z - p3.z) * (pos.x - p3.x) + (p3.x - p2.x) * (pos.y - p3.z)) / det;
    f32 l2 = ((p3.z - p1.z) * (pos.x - p3.x) + (p1.x - p3.x) * (pos.y - p3.z)) / det;
    f32 l3 = 1.0f - l1 - l2;
    return l1 * p1.y + l2 * p2.y + l3 * p3.y;
}

vec3 bary_centric3D(vec3 p, vec3 a, vec3 b, vec3 c)
{
    vec3 v0 = vec3_sub(b,a);
    vec3 v1 = vec3_sub(c,a);
    vec3 v2 = vec3_sub(p,a);

    f32 d00 = vec3_dot(v0, v0);
    f32 d01 = vec3_dot(v0, v1);
    f32 d11 = vec3_dot(v1, v1);
    f32 d20 = vec3_dot(v2, v0);
    f32 d21 = vec3_dot(v2, v1);
    f32 denom = d00 * d11 - d01 * d01;
    f32 v = (d11 * d20 - d01 * d21) / denom;
    f32 w = (d00 * d21 - d01 * d20) / denom;
    f32 u = 1.0f - v - w;

    return (vec3){.x = u,.y = v,.z = w};
}

void mat4_print(const char *s, mat4 m)
{
	printf("%s", s);
	for(i32 i = 0; i < 16; ++i)
	{
		if(i != 0 && i%4 == 0)
			printf("\n");
		printf("%f ", m.m[i]);
	}
	printf("\n");
}

void quat_print(const char *s, quat q)
{
	printf("%s %f %f %f %f", s, q.x, q.y, q.z, q.w);
}

void vec2_print(const char *s, vec2 v)
{
	printf("%s %f %f\n", s, v.x, v.y);
}

void vec3_print(const char *s, vec3 v)
{
	printf("%s %f %f %f\n", s, v.x, v.y, v.z);
}

void vec4_print(const char *s, vec4 v)
{
	printf("%s %f %f %f %f\n", s, v.x, v.y, v.z, v.w);
}
