#include <material.h>
#include <ray.h>
#include <stdio.h>
#include <stdlib.h>
#include <ray.h>
#include <math.h>

static float hash(uint *rng_seed) {
	uint state = *rng_seed;
	state *= state * 747796405 + 2891336453;
	uint result = ((state >> ((state >> 28) + 4)) ^ state) * 277803737;
	result = (result >> 22) ^ result;
	*rng_seed = state;
	return result / 4294967295.0;
}

static float normal_hash(uint *state)
{
    // Thanks to https://stackoverflow.com/a/6178290
    float theta = 2 * 3.1415926 * hash(state);
    float rho = sqrt(-2 * log(hash(state)));
    return rho * cos(theta);
}

static vec3 rand_vec(uint *rng_seed) {
	vec3 vec;
	vec[0] = normal_hash(rng_seed) * 2 - 1;
	vec[1] = normal_hash(rng_seed) * 2 - 1;
	vec[2] = normal_hash(rng_seed) * 2 - 1;
	return v3_norm(vec);
}

static vec3 rand_in_sphere(uint *rng_seed) {
	vec3 direction = rand_vec(rng_seed);

	return direction * (float)cbrt(hash(rng_seed));
}

void material_bounce(material material, ray *r, hitinfo info, uint *rng_seed) {
	if(hash(rng_seed) < material.specularity) {
		vec3 reflection_fuzz = rand_in_sphere(rng_seed);
		vec3 reflection = v3_norm(r->direction - 2 * v3_dot(r->direction, info.normal) * info.normal + reflection_fuzz * material.roughness);
		if(v3_dot(reflection, info.normal) < 0) {
			reflection = v3_norm(r->direction - 2 * v3_dot(r->direction, info.normal) * info.normal - reflection_fuzz * material.roughness);
		}
		r->direction = reflection;
		vec3 emitted_light = material.emissive_color;
		r->incoming_light += emitted_light * r->color;
		r->color *= material.specular_color;
	} else {
		r->direction = v3_norm(rand_vec(rng_seed) + info.normal);
		r->incoming_light += material.emissive_color * r->color;
		r->color *= material.color;
	}
}
