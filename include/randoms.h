#pragma once
#include <vectors.h>
#include <stdlib.h>

float hash(uint *rng_seed);

float normal_hash(uint *state);

vec3 rand_vec(uint *rng_seed);

vec3 rand_in_sphere(uint *rng_seed);
