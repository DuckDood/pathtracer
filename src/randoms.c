#include <randoms.h>
#include <math.h>


// found in https://youtu.be/Qz0KTGYJtUk?t=672 (Sebastian Lague's ray tracing video) 
float hash(uint *rng_seed) {
	uint state = *rng_seed;
	state *= state * 747796405 + 2891336453;
	uint result = ((state >> ((state >> 28) + 4)) ^ state) * 277803737;
	result = (result >> 22) ^ result;
	*rng_seed = state;
	return result / 4294967295.0;
}

float normal_hash(uint *state)
{
    // Thanks to https://stackoverflow.com/a/6178290
    float theta = 2 * 3.1415926 * hash(state);
    float rho = sqrt(-2 * log(hash(state)));
    return rho * cos(theta);
}

vec3 rand_vec(uint *rng_seed) {
	vec3 vec;
	vec[0] = normal_hash(rng_seed) * 2 - 1;
	vec[1] = normal_hash(rng_seed) * 2 - 1;
	vec[2] = normal_hash(rng_seed) * 2 - 1;
	return v3_norm(vec);
}

vec3 rand_in_sphere(uint *rng_seed) {
	vec3 direction = rand_vec(rng_seed);

	return direction * (float)cbrt(hash(rng_seed));
}
