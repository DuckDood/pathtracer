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
	int width, height, pitch;
	int bpp;
	uint8_t *pixels;
} pixel_array;


struct program_state {
	SDL_Window *window;
	pixel_array pixels;

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
	state->cam_yaw = 0;
	state->cam_pitch = 0;
	state->cam_pos = (vec3){0,0,0};

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
					break;
			}
			break;
		case SDL_EVENT_MOUSE_MOTION:
			state->cam_yaw += event->motion.xrel / state->pixels.height;
			state->cam_pitch -= event->motion.yrel / state->pixels.height;
			state->cam_pitch = fmax(state->cam_pitch, -M_PI/2);
			state->cam_pitch = fmin(state->cam_pitch, M_PI/2);
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

vec3 rand_vec() {
	vec3 vec;
	while(true) {
		vec[0] = ((double)rand()/RAND_MAX - 0.5) * 2;
		vec[1] = ((double)rand()/RAND_MAX - 0.5) * 2;
		vec[2] = ((double)rand()/RAND_MAX - 0.5) * 2;
		if(v3_dot(vec, vec) <= 1) break;
	}
	return v3_norm(vec);
}

vec3 random_in_hemiphere(vec3 hem) {
	vec3 vec = rand_vec();
	if(v3_dot(vec, hem) < 0) return -vec;
	return vec;
}

vec3 trace(ray r, sphere *spheres, int sphere_count) {
	int bounce_count = 4;
	ray current_ray = r;
	vec3 light = (vec3){1, 1, 1, 1};
	vec3 in_light = (vec3){0, 0, 0, 0};
	for(int bounce = 0; bounce < bounce_count; ++bounce) {
		float min_t = INFINITY;
		sphere *hit_sphere = NULL;
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
			in_light += hit_sphere->emit * light;
			light *= hit_sphere->color;
			//return 0.5*(normal + 1);
		} else {
			//light *= environment(current_ray.direction);
			break;
		};
		current_ray.origin = hit_info.position;
		current_ray.direction = random_in_hemiphere(hit_info.normal);
	}
	return in_light;
}

SDL_AppResult SDL_AppIterate(void *appstate) {
	struct program_state *state = (struct program_state*)appstate;
	SDL_Surface *win_orig_surface = SDL_GetWindowSurface(state->window);
	SDL_Surface *win_surface = SDL_ConvertSurface(win_orig_surface, SDL_PIXELFORMAT_ABGR8888);

	state->pixels.width = win_surface->w;
	state->pixels.height = win_surface->h;
	state->pixels.pitch = win_surface->pitch;
	state->pixels.pixels = win_surface->pixels;
	state->pixels.bpp = SDL_BYTESPERPIXEL(win_surface->format);

	float aspect_ratio = (float)state->pixels.width / state->pixels.height;

	const _Bool * const kbd_state = SDL_GetKeyboardState(NULL);

	if(kbd_state[SDL_SCANCODE_W]) {
		//state->cam_pos[2] += 0.01;
		state->cam_pos[0] += sin(state->cam_yaw)* 0.1 * cos(state->cam_pitch);
		state->cam_pos[2] += cos(state->cam_yaw)* 0.1 * cos(state->cam_pitch);

		state->cam_pos[1] -= sin(state->cam_pitch) * 0.1;
	}

	if(kbd_state[SDL_SCANCODE_S]) {
		//state->cam_pos[2] -= 0.01;
		//
		state->cam_pos[0] -= sin(state->cam_yaw)* 0.1 * cos(state->cam_pitch);
		state->cam_pos[2] -= cos(state->cam_yaw)* 0.1 * cos(state->cam_pitch);

		state->cam_pos[1] += sin(state->cam_pitch) * 0.1;
	}

	mat4 rot_matrix = gen_rotation_matrix(state->cam_yaw, state->cam_pitch, 0);
	/*printf("%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n",
			cam_matrix.columns[0][0],cam_matrix.columns[1][0],cam_matrix.columns[2][0],cam_matrix.columns[3][0],
			cam_matrix.columns[0][1],cam_matrix.columns[1][1],cam_matrix.columns[2][1],cam_matrix.columns[3][1],
			cam_matrix.columns[0][2],cam_matrix.columns[1][2],cam_matrix.columns[2][2],cam_matrix.columns[3][2],
			cam_matrix.columns[0][3],cam_matrix.columns[1][3],cam_matrix.columns[2][3],cam_matrix.columns[3][3]
			);*/

	sphere spheres[4] = {
	{{0, -0.4, 3}, 1, {0, 0, 0}, {1, 1, 1}},
	{{1, -0.2, 4}, 0.5, {0, 1, 0}, {0}},
	{{-1, -0.14, 2}, 0.3, {0, 0, 1}, {0}},
	{{-1, 100, 2}, 100, {0, 1, 1}, {0}},
	};
	int sphere_count = 4;

#pragma omp parallel for
	for(int y = 0; y < state->pixels.height; ++y) {
		for(int x = 0; x < state->pixels.width; ++x) {
			float ndc_x = ((float)x / state->pixels.width - 0.5) * 2;
			float ndc_y = ((float)y / state->pixels.height - 0.5) * 2;
			ray cam_ray;
			cam_ray.origin = state->cam_pos;
			cam_ray.direction = m4_v4_mul(rot_matrix, (vec4){ndc_x * aspect_ratio, ndc_y, 1, 1});
			cam_ray.direction /= v3_len(cam_ray.direction);

			vec4 color = (vec4){0};

			color = trace(cam_ray, spheres, sphere_count);
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

			color[3] = 1;

			color[0] = fmax(0, color[0]);
			color[1] = fmax(0, color[1]);
			color[2] = fmax(0, color[2]);
			color[3] = fmax(0, color[3]);

			color[0] = fmin(1, color[0]);
			color[1] = fmin(1, color[1]);
			color[2] = fmin(1, color[2]);
			color[3] = fmin(1, color[3]);

			// assuming little endian (i think maybe sdl accounts for it with pixelformats?)
			state->pixels.pixels[y * state->pixels.pitch + x*state->pixels.bpp + 0] = color[0] * 255;
			state->pixels.pixels[y * state->pixels.pitch + x*state->pixels.bpp + 1] = color[1] * 255;
			state->pixels.pixels[y * state->pixels.pitch + x*state->pixels.bpp + 2] = color[2] * 255;
			state->pixels.pixels[y * state->pixels.pitch + x*state->pixels.bpp + 3] = color[3] * 255;
		}
	}

	SDL_BlitSurface(win_surface, NULL, win_orig_surface, NULL);
	SDL_UpdateWindowSurface(state->window);
	return SDL_APP_CONTINUE;
}
