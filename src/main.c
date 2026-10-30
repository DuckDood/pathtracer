#include <SDL3/SDL_pixels.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <vectors.h>
#include <transformations.h>
#include <ray.h>

#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#define WIDTH 800
#define HEIGHT 600

typedef struct {
	int width, height;
	vec4 *pixels;
} screen;


struct program_state {
	SDL_Window *window;
	screen pixels;

	int frame_accumulation_count;
	float cam_yaw;
	float cam_pitch;
	vec3 cam_pos;
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[]) {
	*appstate = malloc(sizeof(struct program_state));
	struct program_state *state = (struct program_state*)*appstate;

	if(!SDL_Init(SDL_INIT_VIDEO)) {
		fprintf(stderr, "Failed to init SDL3: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}

	state->window = SDL_CreateWindow("pathtracer", WIDTH, HEIGHT, 0);
	if(state->window == NULL) {
		fprintf(stderr, "Failed to create window: %s\n", SDL_GetError());
		return SDL_APP_FAILURE;
	}
	SDL_SetWindowRelativeMouseMode(state->window, true);


	/*printf("%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n",
			x.columns[0][0],x.columns[1][0],x.columns[2][0],x.columns[3][0],
			x.columns[0][1],x.columns[1][1],x.columns[2][1],x.columns[3][1],
			x.columns[0][2],x.columns[1][2],x.columns[2][2],x.columns[3][2],
			x.columns[0][3],x.columns[1][3],x.columns[2][3],x.columns[3][3]
			);*/
	state->cam_yaw = -0.956666;
	state->cam_pitch = 0.248333;
	state->cam_pos = (vec3){1.937445, -0.791324, 0.774603};

	SDL_Surface *win_surface = SDL_GetWindowSurface(state->window);

	state->pixels.width = win_surface->w;
	state->pixels.height = win_surface->h;
	state->pixels.pixels = malloc(state->pixels.width * state->pixels.height * sizeof(vec4));

	state->frame_accumulation_count = 0;


	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
	struct program_state *state = (struct program_state*)appstate;
	switch(event->type) {
		case SDL_EVENT_QUIT:
			return SDL_APP_SUCCESS;
			break;
		case SDL_EVENT_KEY_DOWN:
			switch(event->key.key) {
				case SDLK_P:
					SDL_SavePNG(SDL_GetWindowSurface(state->window), "screenshot.png");

				case SDLK_C:
					printf("{%f, %f, %f}, %f, %f", state->cam_pos[0], state->cam_pos[1], state->cam_pos[2], state->cam_yaw, state->cam_pitch);
					break;
			}
			break;
		case SDL_EVENT_MOUSE_MOTION:
			state->cam_yaw += event->motion.xrel / state->pixels.height;
			state->cam_pitch -= event->motion.yrel / state->pixels.height;
			state->cam_pitch = fmax(state->cam_pitch, -M_PI/2);
			state->cam_pitch = fmin(state->cam_pitch, M_PI/2);

			state->frame_accumulation_count = 0;
			break;
	}

	return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void *appstate, SDL_AppResult result) {
	struct program_state *state = (struct program_state*)appstate;
	if(state->window != NULL) {
		SDL_DestroyWindow(state->window);
	}
}

vec3 environment(vec3 direction) {
	return (vec3){0.6, 0.6, 0.8};
}

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

/*vec3 random_in_hemiphere(vec3 hem) {
	vec3 vec = rand_vec();
	if(v3_dot(vec, hem) < 0) return -vec;
	return vec;
}*/

vec3 sample_env_light(vec3 direction) {
	vec3 ground_col = {0.2, 0.2, 0.2};
	vec3 sky_col = (vec3){0.8, 0.8, 1} * 0.8f;
	sky_col += sky_col * (1-(float)fmin(1-direction[1], 2) / 2) * 1.f;
	vec3 ground = ground_col * (float)(direction[1] > 0);
	vec3 sky = sky_col * (float)!(direction[1] > 0);

	vec3 sun_dir = v3_norm((vec3){-1, -1, -0.2});

	float sun = pow(v3_dot(sun_dir, direction) * 0.5 + 0.5, 300) * 10;

	//return (vec3){0};
	return ground + sky + sun;
}

vec3 trace(ray r, const sphere *spheres, int sphere_count, uint *rng_seed) {
	int bounce_count = 10;
	ray current_ray = r;
	vec3 ray_color = (vec3){1, 1, 1, 1};
	vec3 incoming_light = (vec3){0, 0, 0, 0};
	for(int bounce = 0; bounce < bounce_count; ++bounce) {
		float min_t = INFINITY;
		const sphere *hit_sphere = NULL;
		hitinfo hit_info;
		for(int i = 0; i < sphere_count; ++i) {
			hitinfo info = ray_sphere_intersection(current_ray, spheres[i]);
			if(!info.hit) continue;
			if(info.distance < min_t) {
				hit_sphere = spheres + i;
				min_t = info.distance;
				hit_info = info;
			}
		}
		if(min_t != INFINITY) {
			vec3 emitted_light = hit_sphere->emit;
			incoming_light += emitted_light * ray_color;
			ray_color *= hit_sphere->color;
			//return 0.5*(normal + 1);
		} else {
			//light *= environment(current_ray.direction);
			incoming_light += sample_env_light(current_ray.direction) * ray_color;
			break;
		};
		current_ray.origin = hit_info.position;
		if((hit_sphere == spheres || hit_sphere == spheres + 1)) {
			current_ray.direction = v3_norm(current_ray.direction - 2 * v3_dot(current_ray.direction, hit_info.normal) * hit_info.normal + rand_in_sphere(rng_seed) * 0.3f);
			//current_ray.direction = v3_norm(current_ray.direction - 2 * v3_dot(current_ray.direction, hit_info.normal) * hit_info.normal);
			//current_ray.direction = v3_norm(rand_vec(rng_seed) + hit_info.normal);
		} else {
			current_ray.direction = v3_norm(rand_vec(rng_seed) + hit_info.normal);
		}
	}
	incoming_light[3] = 1;
	return incoming_light;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
	struct program_state *state = (struct program_state*)appstate;
	SDL_Surface *win_orig_surface = SDL_GetWindowSurface(state->window);
	SDL_Surface *win_surface = SDL_ConvertSurface(win_orig_surface, SDL_PIXELFORMAT_ABGR8888);


	float aspect_ratio = (float)state->pixels.width / state->pixels.height;

	const _Bool * const kbd_state = SDL_GetKeyboardState(NULL);

	if(kbd_state[SDL_SCANCODE_W]) {
		//state->cam_pos[2] += 0.01;
		state->cam_pos[0] += sin(state->cam_yaw)* 0.1 * cos(state->cam_pitch);
		state->cam_pos[2] += cos(state->cam_yaw)* 0.1 * cos(state->cam_pitch);

		state->cam_pos[1] -= sin(state->cam_pitch) * 0.1;

		state->frame_accumulation_count = 0;
	}

	if(kbd_state[SDL_SCANCODE_S]) {
		//state->cam_pos[2] -= 0.01;
		//
		state->cam_pos[0] -= sin(state->cam_yaw)* 0.1 * cos(state->cam_pitch);
		state->cam_pos[2] -= cos(state->cam_yaw)* 0.1 * cos(state->cam_pitch);

		state->cam_pos[1] += sin(state->cam_pitch) * 0.1;
		state->frame_accumulation_count = 0;
	}

	mat4 rot_matrix = gen_rotation_matrix(state->cam_yaw, state->cam_pitch, 0);
	/*printf("%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n",
			cam_matrix.columns[0][0],cam_matrix.columns[1][0],cam_matrix.columns[2][0],cam_matrix.columns[3][0],
			cam_matrix.columns[0][1],cam_matrix.columns[1][1],cam_matrix.columns[2][1],cam_matrix.columns[3][1],
			cam_matrix.columns[0][2],cam_matrix.columns[1][2],cam_matrix.columns[2][2],cam_matrix.columns[3][2],
			cam_matrix.columns[0][3],cam_matrix.columns[1][3],cam_matrix.columns[2][3],cam_matrix.columns[3][3]
			);*/

	const sphere spheres[4] = {
	{{0, -0.4, 3}, 1, {1, 1, 1}, {0}},
	{{2, -0.5, 3}, 0.5, {0, 1, 0}, {0}},
	{{-1, -0.14, 2}, 0.3, {0, 0, 1}, {0, 0, 0}},
	{{-1, 100, 2}, 100, {0.7, 0.1, 1}, {0}},
	};
	const int sphere_count = 4;

#pragma omp parallel for
	for(int i = 0; i < state->pixels.height * state->pixels.width; ++i) {
		//for(int x = 0; x < state->pixels.width; ++x) {
		uint rng_seed = i + state->frame_accumulation_count * 2304894;
		const int x = i % state->pixels.width;
		const int y = i / state->pixels.width;
		float ndc_x = ((float)x / state->pixels.width - 0.5) * 2;
		float ndc_y = ((float)y / state->pixels.height - 0.5) * 2;

		float aa = 1;
		float x_halfpix = 0.5/state->pixels.width * aa;
		float y_halfpix = 0.5/state->pixels.height * aa;

		ray cam_rays[4];
		cam_rays[0].origin = state->cam_pos;
		cam_rays[0].direction = v3_norm(m4_v4_mul(rot_matrix, (vec4){ndc_x * aspect_ratio + x_halfpix, ndc_y, 1, 1}));

		cam_rays[1].origin = state->cam_pos;
		cam_rays[1].direction = v3_norm(m4_v4_mul(rot_matrix, (vec4){ndc_x * aspect_ratio - x_halfpix, ndc_y, 1, 1}));

		cam_rays[2].origin = state->cam_pos;
		cam_rays[2].direction = v3_norm(m4_v4_mul(rot_matrix, (vec4){ndc_x * aspect_ratio + y_halfpix, ndc_y, 1, 1}));

		cam_rays[3].origin = state->cam_pos;
		cam_rays[3].direction = v3_norm(m4_v4_mul(rot_matrix, (vec4){ndc_x * aspect_ratio - y_halfpix, ndc_y, 1, 1}));

		vec4 colors[4];

		colors[0] = trace(cam_rays[0], spheres, sphere_count, &rng_seed);
		colors[1] = trace(cam_rays[1], spheres, sphere_count, &rng_seed);
		colors[2] = trace(cam_rays[2], spheres, sphere_count, &rng_seed);
		colors[3] = trace(cam_rays[3], spheres, sphere_count, &rng_seed);
		/*float min_t = INFINITY;
		  sphere *hit_sphere = NULL;
		  for(int i = 0; i < sphere_count; ++i) {
		  float t = ray_sphere_intersection(cam_ray, spheres[i]);
		  if(t <= 0) continue;
		  if(t < min_t) {
		  hit_sphere = spheres + i;
		  min_t = t;
		  }
		  }
		  if(min_t != INFINITY) {
		  color = hit_sphere->color;
		  }*/
		/*if(hit_sphere == NULL) {
		  printf("hi\n");
		  }*/
		//float t = ray_sphere_intersection(cam_ray, (vec3){0, 0, 3}, 1);
		//float t = ray_sphere_intersection(cam_ray, (vec3){1, 0, 2}, 0.5);
		/*float t = ray_sphere_intersection(cam_ray, spheres[0]);
		  if(t > 0) {
		  vec3 normal = cam_ray.direction * t - (vec3){0, 0, 3} + cam_ray.origin;
		  color = (normal + 1) * 0.5;
		//color[0] = 1;
		}*/
		//}	color[0] = 1;

		colors[0][3] = 1;
		colors[1][3] = 1;
		colors[2][3] = 1;
		colors[3][3] = 1;

		vec4 color = (colors[0] + colors[1] + colors[2] + colors[3]) / 4;


		state->pixels.pixels[y * state->pixels.width + x] *= (float)state->frame_accumulation_count;
		state->pixels.pixels[y * state->pixels.width + x] += color;
		state->pixels.pixels[y * state->pixels.width + x] /= (float)state->frame_accumulation_count+1;
		vec4 pix_color = state->pixels.pixels[y * state->pixels.width + x];
		pix_color[0] = fmax(0, pix_color[0]);
		pix_color[1] = fmax(0, pix_color[1]);
		pix_color[2] = fmax(0, pix_color[2]);
		pix_color[3] = fmax(0, pix_color[3]);

		pix_color[0] = fmin(1, pix_color[0]);
		pix_color[1] = fmin(1, pix_color[1]);
		pix_color[2] = fmin(1, pix_color[2]);
		pix_color[3] = fmin(1, pix_color[3]);

		//if(isnan(pix_color[0]) || isnan(pix_color[1]) || isnan(pix_color[2]))  pix_color = (vec4){0, 1, 0, 1};

		// assuming little endian (i think maybe sdl accounts for it with pixelformats?)
		((uint8_t*)win_surface->pixels)[y * win_surface->pitch + x*SDL_BYTESPERPIXEL(win_surface->format) + 0] = pix_color[0] * 255;
		((uint8_t*)win_surface->pixels)[y * win_surface->pitch + x*SDL_BYTESPERPIXEL(win_surface->format) + 1] = pix_color[1] * 255;
		((uint8_t*)win_surface->pixels)[y * win_surface->pitch + x*SDL_BYTESPERPIXEL(win_surface->format) + 2] = pix_color[2] * 255;
		((uint8_t*)win_surface->pixels)[y * win_surface->pitch + x*SDL_BYTESPERPIXEL(win_surface->format) + 3] = pix_color[3] * 255;
		//}
	}
	state->frame_accumulation_count++;

	SDL_BlitSurface(win_surface, NULL, win_orig_surface, NULL);
	SDL_UpdateWindowSurface(state->window);
	return SDL_APP_CONTINUE;
}
