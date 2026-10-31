#include <material.h>

typedef struct {
	vec3 position;
	float radius;
	material mat;
} sphere;


hitinfo ray_sphere_intersection(ray r, sphere s);
