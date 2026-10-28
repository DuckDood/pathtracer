#pragma once
#include <vectors.h>

mat4 gen_rotation_matrix(float yaw, float pitch, float roll);
mat4 gen_translation_matrix(vec3 pos);
