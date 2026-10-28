#include <transformations.h>
#include <math.h>

mat4 gen_rotation_matrix(float yaw, float pitch, float roll) {
	return construct_mat4_cm((float[16]){
		cos(roll) * cos(yaw),
		sin(roll) * cos(yaw),
		-sin(yaw),
		0,

		cos(roll) * sin(yaw) * sin(pitch) - sin(roll) * cos(pitch),
		sin(roll) * sin(yaw) * sin(pitch) + cos(roll) * cos(pitch),
		cos(yaw) * sin(pitch),
		0,

		cos(roll) * sin(yaw) * cos(pitch) + sin(roll) * sin(pitch),
		sin(roll) * sin(yaw) * cos(pitch) - cos(roll) * sin(pitch),
		cos(yaw) * cos(pitch),
		0,

		0,
		0,
		0,
		0
		});

}

mat4 gen_translation_matrix(vec3 pos) {
	return construct_mat4_cm((float[16]){
		1,
		0,
		0,
		0,

		0,
		1,
		0,
		0,

		0,
		0,
		1,
		0,

		pos[0],
		pos[1],
		pos[2],
		1
		});

}
