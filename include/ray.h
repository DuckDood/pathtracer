#pragma once
#include <vectors.h>

typedef struct {
	vec3 origin;
	vec3 direction;
} ray;

float ray_sphere_intersection(ray r, vec3 circ_center, float circ_rad);
