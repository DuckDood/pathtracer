#include <vectors.h>
#include <includes.h>
#include <ray.h>

typedef struct {
	vec3 color;
	vec3 emissive_color;
	vec3 specular_color;
	float roughness;
	float specularity;
} material;

void material_bounce(material material, ray *r, hitinfo info, uint32_t *rng_seed);
