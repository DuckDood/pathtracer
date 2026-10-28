#pragma once
typedef float vec3 __attribute__((vector_size(4 * sizeof(float))));

typedef float vec4 __attribute__((vector_size(4 * sizeof(float))));

typedef struct {
	vec4 columns[4];
} mat4;
//typedef float mat4 __attribute__((vector_size(16 * sizeof(float)))); // column major (like opengl)

float v3_dot(vec3 a, vec3 b);
float v3_len(vec3 a);

vec4 v3_to_4(vec3 a);

mat4 m4_m4_mul(mat4 a, mat4 b);
vec4 m4_v4_mul(mat4 a, vec4 b);

mat4 transpose(mat4 mat);
// construct matrix using column major array
mat4 construct_mat4_cm(float array[16]);

// construct matrix using row major array
mat4 construct_mat4_rm(float array[16]);
