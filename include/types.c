#include "types.h"

/*
void EE_Mat3_Set(mat3 *m1, f32 array[9])
{
	if(m1 == NULL)
	{
		return;
	}

	for(i32 i = 0; i < 9; ++i)
	{
		m1->array[i] = array[i];
	}
}

void EE_Mat4_Set(mat4 *m1, f32 array[16])
{
	if(m1 == NULL)
	{
		return;
	}

	for(i32 i = 0; i < 16; ++i)
	{
		m1->array[i] = array[i];
	}
}

void EE_Mat3_Identity(mat3 *m1)
{
	if(m1 == NULL)
	{
		return;
	}

	m1->array[0] = 1.0f; m1->array[1] = 0.0f; m1->array[2] = 0.0f;
	m1->array[3] = 0.0f; m1->array[4] = 1.0f; m1->array[5] = 0.0f;
	m1->array[6] = 0.0f; m1->array[7] = 0.0f; m1->array[8] = 1.0f;
}

void EE_Mat4_Identity(mat4 *m1)
{
	if(m1 == NULL)
	{
		return;
	}

	m1->array[0] = 1.0f; m1->array[1] = 0.0f; m1->array[2] = 0.0f; m1->array[3] = 0.0f;
	m1->array[4] = 0.0f; m1->array[5] = 1.0f; m1->array[6] = 0.0f; m1->array[7] = 0.0f;
	m1->array[8] = 0.0f; m1->array[9] = 0.0f; m1->array[10] = 1.0f; m1->array[11] = 0.0f;
	m1->array[12] = 0.0f; m1->array[13] = 0.0f; m1->array[14] = 0.0f; m1->array[15] = 1.0f;
}

void EE_Mat4_Mul(mat3 *m1, mat3 *m2, mat3 *res)
{
	if(m1 == NULL || m2 == NULL || res == NULL)
	{
		return;
	}

	f32 tmp[9];

	tmp[0] = m1->array[0] * m2->array[0] + m1->array[1] * m2->array[3] + m1->array[2] * m2->array[6];
	tmp[1] = m1->array[0] * m2->array[1] + m1->array[1] * m2->array[4] + m1->array[2] * m2->array[7];
	tmp[2] = m1->array[0] * m2->array[2] + m1->array[1] * m2->array[5] + m1->array[2] * m2->array[8];

	tmp[3] = m1->array[3] * m2->array[0] + m1->array[4] * m2->array[3] + m1->array[5] * m2->array[6];
	tmp[4] = m1->array[3] * m2->array[1] + m1->array[4] * m2->array[4] + m1->array[5] * m2->array[7];
	tmp[5] = m1->array[3] * m2->array[2] + m1->array[4] * m2->array[5] + m1->array[5] * m2->array[8];

	tmp[6] = m1->array[6] * m2->array[0] + m1->array[7] * m2->array[3] + m1->array[8] * m2->array[6];
	tmp[7] = m1->array[6] * m2->array[1] + m1->array[7] * m2->array[4] + m1->array[8] * m2->array[7];
	tmp[8] = m1->array[6] * m2->array[2] + m1->array[7] * m2->array[5] + m1->array[8] * m2->array[8];

	for(i32 i = 0; i < 9; ++i)
	{
		res->array[i] = tmp[i];
	}
}

void EE_Mat4_Mul(mat4 *m1, mat4 *m2, mat4 *res)
{
	if(m1 == NULL || m2 == NULL || res == NULL)
	{
		return;
	}

	f32 tmp[16];

	tmp[0] = m1->array[0] * m2->array[0] + m1->array[1] * m2->array[4] + m1->array[2] * m2->array[8] + m1->array[3] * m2->array[12];
	tmp[1] = m1->array[0] * m2->array[1] + m1->array[1] * m2->array[5] + m1->array[2] * m2->array[9] + m1->array[3] * m2->array[13];
	tmp[2] = m1->array[0] * m2->array[2] + m1->array[1] * m2->array[6] + m1->array[2] * m2->array[10] + m1->array[3] * m2->array[14];
	tmp[3] = m1->array[0] * m2->array[3] + m1->array[1] * m2->array[7] + m1->array[2] * m2->array[11] + m1->array[3] * m2->array[15];

	tmp[4] = m1->array[4] * m2->array[0] + m1->array[5] * m2->array[4] + m1->array[6] * m2->array[8] + m1->array[7] * m2->array[12];
	tmp[5] = m1->array[4] * m2->array[1] + m1->array[5] * m2->array[5] + m1->array[6] * m2->array[9] + m1->array[7] * m2->array[13];
	tmp[6] = m1->array[4] * m2->array[2] + m1->array[5] * m2->array[6] + m1->array[6] * m2->array[10] + m1->array[7] * m2->array[14];
	tmp[7] = m1->array[4] * m2->array[3] + m1->array[5] * m2->array[7] + m1->array[6] * m2->array[11] + m1->array[7] * m2->array[15];

	tmp[8] = m1->array[8] * m2->array[0] + m1->array[9] * m2->array[4] + m1->array[10] * m2->array[8] + m1->array[11] * m2->array[12];
	tmp[9] = m1->array[8] * m2->array[1] + m1->array[9] * m2->array[5] + m1->array[10] * m2->array[9] + m1->array[11] * m2->array[13];
	tmp[10] = m1->array[8] * m2->array[2] + m1->array[9] * m2->array[6] + m1->array[10] * m2->array[10] + m1->array[11] * m2->array[14];
	tmp[11] = m1->array[8] * m2->array[3] + m1->array[9] * m2->array[7] + m1->array[10] * m2->array[11] + m1->array[11] * m2->array[15];

	tmp[12] = m1->array[12] * m2->array[0] + m1->array[13] * m2->array[4] + m1->array[14] * m2->array[8] + m1->array[15] * m2->array[12];
	tmp[13] = m1->array[12] * m2->array[1] + m1->array[13] * m2->array[5] + m1->array[14] * m2->array[9] + m1->array[15] * m2->array[13];
	tmp[14] = m1->array[12] * m2->array[2] + m1->array[13] * m2->array[6] + m1->array[14] * m2->array[10] + m1->array[15] * m2->array[14];
	tmp[15] = m1->array[12] * m2->array[3] + m1->array[13] * m2->array[7] + m1->array[14] * m2->array[11] + m1->array[15] * m2->array[15];

	for(i32 i = 0; i < 16; ++i)
	{
		res->array[i] = tmp[i];
	}
}

void EE_Vec3_Mul_Mat3(vec3 *v1, mat3 *m1, vec3 *res)
{
	if(v1 == NULL || m1 == NULL || res == NULL)
	{
		return;
	}

	f32 x = v1->x * m1->array[0] + v1->y * m1->array[1] + v1->z * m1->array[2];
	f32 y = v1->x * m1->array[3] + v1->y * m1->array[4] + v1->z * m1->array[5];
	f32 z = v1->x * m1->array[6] + v1->y * m1->array[7] + v1->z * m1->array[8];

	res->x = x;
	res->y = y;
	res->z = z;
}

void EE_Vec4_Mul_Mat4(vec4 *v1, mat4 *m1, vec4 *res)
{
	if(v1 == NULL || m1 == NULL || res == NULL)
	{
		return;
	}

	f32 x = v1->x * m1->array[0] + v1->y * m1->array[1] + v1->z * m1->array[2] + v1->w * m1->array[3];
	f32 y = v1->x * m1->array[4] + v1->y * m1->array[5] + v1->z * m1->array[6] + v1->w * m1->array[7];
	f32 z = v1->x * m1->array[8] + v1->y * m1->array[9] + v1->z * m1->array[10] + v1->w * m1->array[11];
	f32 w = v1->x * m1->array[12] + v1->y * m1->array[13] + v1->z * m1->array[14] + v1->w * m1->array[15];

	res->x = x;
	res->y = y;
	res->z = z;
	res->w = w;
}

void EE_Mat4_Ortho(mat4 *m1, i32 left, i32 right, i32 top, i32 bot, f32 near, f32 far)
{
	if(m1 == NULL)
	{
		return;
	}

	m1->array[0] = 2.0f / (right-left);
	m1->array[1] = 0;
	m1->array[2] = 0;
	m1->array[3] = 0;

	m1->array[4] = 0;
	m1->array[5] = (2.0f / (top-bot));
	m1->array[6] = 0;
	m1->array[7] = 0;

	m1->array[8] = 0;
	m1->array[9] = 0;
	m1->array[10] = -2.0f / (far-near);
	m1->array[11] = 0;

	m1->array[12] = -((right+left)/(right-left));
	m1->array[13] = -((top+bot)/(top-bot));
	m1->array[14] = -((far+near)/(far-near));
	m1->array[15] = 1.0f;
}

void EE_Mat4_Perspective(mat4 *m1, i32 width, i32 height, f32 fov, f32 near, f32 far)
{
	f32 asp = (f32)width/(f32)height;
	f32 y = 1.0f / tan((fov/180.0f*3.14f) / 2.0f);
	f32 x = y / asp;
	f32 fl = far - near;	

	EE_Mat4_Identity(m1);
	m1->array[0] = x;
	m1->array[5] = y;
	m1->array[10] = -(far + near) / fl;
	m1->array[14] = -1;
	m1->array[11] = -(2 * near * far) / fl;
	m1->array[15] = 0;
}

void EE_Mat4_Translate(mat4 *m1, vec3 *v1)
{
	if(m1 == NULL || v1 == NULL)
	{
		return;
	}

	mat4 t;
	f32 f[16] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		v1->x, v1->y, v1->z, 1.0f
	};
	EE_Mat4_Set(&t, f);
	EE_Mat4_Mul(m1,&t,m1);
}

void EE_Mat4_Translate(mat4 *m1, vec4 *v1)
{
	if(m1 == NULL || v1 == NULL)
	{
		return;
	}

	mat4 t;
	f32 f[16] = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		v1->x, v1->y, v1->z, v1->w
	};
	EE_Mat4_Set(&t, f);
	EE_Mat4_Mul(m1,&t,m1);
}

void EE_Mat4_Scale(mat4 *m1, vec3 *v1)
{
	if(m1 == NULL || v1 == NULL)
	{
		return;
	}

	mat4 t;
	f32 f[16] = {
		v1->x, 0.0f, 0.0f, 0.0f,
		0.0f, v1->y, 0.0f, 0.0f,
		0.0f, 0.0f, v1->z, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f
	};
	EE_Mat4_Set(&t, f);
	EE_Mat4_Mul(m1,&t,m1);
}

void EE_Mat4_Scale(mat4 *m1, vec4 *v1)
{
	if(m1 == NULL || v1 == NULL)
	{
		return;
	}

	mat4 t;
	f32 f[16] = {
		v1->x, 0.0f, 0.0f, 0.0f,
		0.0f, v1->y, 0.0f, 0.0f,
		0.0f, 0.0f, v1->z, 0.0f,
		1.0f, 1.0f, 1.0f, v1->w
	};
	EE_Mat4_Set(&t, f);
	EE_Mat4_Mul(m1,&t,m1);
}

void EE_Mat4_Rotate(mat4 *m1, vec3 *axis, f32 angle)
{
	if(m1 == NULL || axis == NULL)
	{
		return;
	}

	//axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    mat4 t;
    f32 f[16] = {oc * axis->x * axis->x + c,           oc * axis->x * axis->y - axis->z * s,  oc * axis->z * axis->x + axis->y * s,  0.0,
                oc * axis->x * axis->y + axis->z * s,  oc * axis->y * axis->y + c,           oc * axis->y * axis->z - axis->x * s,  0.0,
                oc * axis->z * axis->x - axis->y * s,  oc * axis->y * axis->z + axis->x * s,  oc * axis->z * axis->z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0};

	EE_Mat4_Set(&t, f);
	EE_Mat4_Mul(m1, &t, m1);
}

void EE_Mat4_RotateEuler(mat4 *m1, vec3 euler)
{
	if(m1 == NULL)
	{
		return;
	}

	f32 sa = sin(euler.z);
	f32 ca = cos(euler.z);

	f32 sb = sin(euler.x);
	f32 cb = cos(euler.x);

	f32 sh = sin(euler.y);
	f32 ch = cos(euler.y);

	mat4 t;
    f32 f[16] = {ch*ca, -ch*sa*cb + sh*sb, ch*sa*sb + sh*cb,
    			 sa,	 ca*cb,			   -ca*sb,
    			 -sh*ca, sh*sa*cb + ch*sb, -sh*sa*sb + ch*cb,
    			 0, 0, 0, 1};

	EE_Mat4_Set(&t, f);
	EE_Mat4_Mul(m1, &t, m1);
}

void EE_Mat4_Translate(mat4 *m1, vec3 off)
{
	EE_Mat4_Translate(m1, &off);
}

void EE_Mat4_Translate(mat4 *m1, vec4 off)
{
	EE_Mat4_Translate(m1, &off);
}

void EE_Mat4_Scale(mat4 *m1, vec3 scale)
{
	EE_Mat4_Scale(m1, &scale);
}

void EE_Mat4_Scale(mat4 *m1, vec4 scale)
{
	EE_Mat4_Scale(m1, &scale);
}

void EE_Mat4_Rotate(mat4 *m1, vec3 axis, f32 angle)
{
	EE_Mat4_Rotate(m1, &axis, angle);
}

void EE_Vec2_Normalize(vec2 *v1)
{
	f32 l = sqrt(v1->x*v1->x+
				 v1->y*v1->y);
	v1->x /= l;
	v1->y /= l;
}

void EE_Vec3_Normalize(vec3 *v1)
{
	f32 l = sqrt(v1->x*v1->x+
				 v1->y*v1->y+
				 v1->z*v1->z);
	v1->x /= l;
	v1->y /= l;
	v1->z /= l;
}

void EE_Vec4_Normalize(vec4 *v1)
{
	f32 l = sqrt(v1->x*v1->x+
				 v1->y*v1->y+
				 v1->z*v1->z+
				 v1->w*v1->w);
	v1->x /= l;
	v1->y /= l;
	v1->z /= l;
	v1->w /= l;
}*/