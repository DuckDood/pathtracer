#include <math.h>
#include <vectors.h>
#include <string.h>

float v3_dot(vec3 a, vec3 b) {
	vec3 c = a * b;
	return c[0] + c[1] + c[2];
	//return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

float v3_len(vec3 a) {
	return sqrt(v3_dot(a,a));
}

vec3 v3_norm(vec3 a) {
	return a/(float)sqrt(v3_dot(a,a));
}

vec4 v3_to_4(vec3 a) {
	a[3] = 0;
	return a;
}
// regex'd these with wolfram alpha output i felt like a genius

mat4 m4_m4_mul(mat4 x, mat4 y) { 
	return (mat4)
	{	
		x.columns[0][0] * y.columns[0][0] + x.columns[0][1] * y.columns[1][0] + x.columns[0][2] * y.columns[2][0] + x.columns[0][3] * y.columns[3][0] , x.columns[0][0] * y.columns[0][1] + x.columns[0][1] * y.columns[1][1] + x.columns[0][2] * y.columns[2][1] + x.columns[0][3] * y.columns[3][1] , x.columns[0][0] * y.columns[0][2] + x.columns[0][1] * y.columns[1][2] + x.columns[0][2] * y.columns[2][2] + x.columns[0][3] * y.columns[3][2] , x.columns[0][0] * y.columns[0][3] + x.columns[0][1] * y.columns[1][3] + x.columns[0][2] * y.columns[2][3] + x.columns[0][3] * y.columns[3][3],
		x.columns[1][0] * y.columns[0][0] + x.columns[1][1] * y.columns[1][0] + x.columns[1][2] * y.columns[2][0] + x.columns[1][3] * y.columns[3][0] , x.columns[1][0] * y.columns[0][1] + x.columns[1][1] * y.columns[1][1] + x.columns[1][2] * y.columns[2][1] + x.columns[1][3] * y.columns[3][1] , x.columns[1][0] * y.columns[0][2] + x.columns[1][1] * y.columns[1][2] + x.columns[1][2] * y.columns[2][2] + x.columns[1][3] * y.columns[3][2] , x.columns[1][0] * y.columns[0][3] + x.columns[1][1] * y.columns[1][3] + x.columns[1][2] * y.columns[2][3] + x.columns[1][3] * y.columns[3][3],
		x.columns[2][0] * y.columns[0][0] + x.columns[2][1] * y.columns[1][0] + x.columns[2][2] * y.columns[2][0] + x.columns[2][3] * y.columns[3][0] , x.columns[2][0] * y.columns[0][1] + x.columns[2][1] * y.columns[1][1] + x.columns[2][2] * y.columns[2][1] + x.columns[2][3] * y.columns[3][1] , x.columns[2][0] * y.columns[0][2] + x.columns[2][1] * y.columns[1][2] + x.columns[2][2] * y.columns[2][2] + x.columns[2][3] * y.columns[3][2] , x.columns[2][0] * y.columns[0][3] + x.columns[2][1] * y.columns[1][3] + x.columns[2][2] * y.columns[2][3] + x.columns[2][3] * y.columns[3][3],
		x.columns[3][0] * y.columns[0][0] + x.columns[3][1] * y.columns[1][0] + x.columns[3][2] * y.columns[2][0] + x.columns[3][3] * y.columns[3][0] , x.columns[3][0] * y.columns[0][1] + x.columns[3][1] * y.columns[1][1] + x.columns[3][2] * y.columns[2][1] + x.columns[3][3] * y.columns[3][1] , x.columns[3][0] * y.columns[0][2] + x.columns[3][1] * y.columns[1][2] + x.columns[3][2] * y.columns[2][2] + x.columns[3][3] * y.columns[3][2] , x.columns[3][0] * y.columns[0][3] + x.columns[3][1] * y.columns[1][3] + x.columns[3][2] * y.columns[2][3] + x.columns[3][3] * y.columns[3][3]
	};
}

// '<,'>s#\vm\[([0-9]|\d{2})\]#\='m.columns[' .. submatch(1)/4 .. '][' .. submatch(1)%4 .. ']' #g
// the very cool regex i used to convert this to use columns and stuff instead of size 16 vector
vec4 m4_v4_mul(mat4 m, vec4 v) {
	return (vec4)
	{	
		v[0] * m.columns[0][0] + v[1] * m.columns[1][0] + v[2] * m.columns[2][0] + v[3] * m.columns[3][0],
		v[0] * m.columns[0][1] + v[1] * m.columns[1][1] + v[2] * m.columns[2][1] + v[3] * m.columns[3][1],
		v[0] * m.columns[0][2] + v[1] * m.columns[1][2] + v[2] * m.columns[2][2] + v[3] * m.columns[3][2],
		v[0] * m.columns[0][3] + v[1] * m.columns[1][3] + v[2] * m.columns[2][3] + v[3] * m.columns[3][3]
	};
}

mat4 transpose(mat4 mat) {
	mat4 out;
	out.columns[0][0] = mat.columns[0][0];
	out.columns[1][0] = mat.columns[0][1];
	out.columns[2][0] = mat.columns[0][2];
	out.columns[3][0] = mat.columns[0][3];

	out.columns[0][1] = mat.columns[1][0];
	out.columns[1][1] = mat.columns[1][1];
	out.columns[2][1] = mat.columns[1][2];
	out.columns[3][1] = mat.columns[1][3];

	out.columns[0][2] = mat.columns[2][0];
	out.columns[1][2] = mat.columns[2][1];
	out.columns[2][2] = mat.columns[2][2];
	out.columns[3][2] = mat.columns[2][3];

	out.columns[0][2] = mat.columns[2][0];
	out.columns[1][2] = mat.columns[2][1];
	out.columns[2][2] = mat.columns[2][2];
	out.columns[3][2] = mat.columns[2][3];

	return out;
}

mat4 construct_mat4_cm(float array[16]) {
	mat4 out;
	memcpy(&out, array, sizeof(float) * 16);
	return out;
}

mat4 construct_mat4_rm(float array[16]) {
	mat4 out;
	memcpy(&out, array, sizeof(float) * 16);
	return transpose(out);
}
