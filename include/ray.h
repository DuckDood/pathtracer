#pragma once
#include <vectors.h>

typedef struct {
	vec3 origin;
	vec3 direction;
	vec3 color;
	vec3 incoming_light;
} ray;

typedef struct {
	vec3 normal;
	vec3 position;
	float distance;
	_Bool hit;
} hitinfo;
