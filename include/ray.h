#pragma once
#include <vectors.h>

typedef struct {
	vec3 origin;
	vec3 direction;
} ray;

typedef struct {
	vec3 position;
	float radius;
	vec3 color;
} sphere;


float ray_sphere_intersection(ray r, sphere s);
