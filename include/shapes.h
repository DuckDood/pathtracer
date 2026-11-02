#include <includes.h>
#include <material.h>

typedef struct {
	vec3 position;
	float radius;
	material mat;
} sphere;

typedef struct {
	vec3 vertex_a;
	vec3 vertex_b;
	vec3 vertex_c;
	material mat;
} tri;


hitinfo ray_sphere_intersection(ray r, sphere s);

hitinfo ray_tri_intersection(ray r, tri s);
