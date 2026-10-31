#include <material.h>
#include <ray.h>
#include <stdio.h>
#include <stdlib.h>
#include <ray.h>
#include <math.h>
#include <randoms.h>


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
