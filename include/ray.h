#pragma once
#include <vectors.h>

typedef struct {
	vec3 origin;
	vec3 direction;
} ray;

typedef struct {
	vec3 normal;
	vec3 position;
	float distance;
	_Bool hit;
} hitinfo;


typedef struct {
	vec3 position;
	float radius;
	vec3 color;
	vec3 emit;
} sphere;


hitinfo ray_sphere_intersection(ray r, sphere s);
