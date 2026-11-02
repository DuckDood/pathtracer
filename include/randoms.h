#pragma once
#include <vectors.h>
#include <includes.h>
#include <stdlib.h>

float hash(uint32_t *rng_seed);

float normal_hash(uint32_t *state);

vec3 rand_vec(uint32_t *rng_seed);

vec3 rand_in_sphere(uint32_t *rng_seed);
