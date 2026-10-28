#include <SDL3/SDL_pixels.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <vectors.h>
#include <transformations.h>

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

	/*printf("%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n",
			x.columns[0][0],x.columns[1][0],x.columns[2][0],x.columns[3][0],
			x.columns[0][1],x.columns[1][1],x.columns[2][1],x.columns[3][1],
			x.columns[0][2],x.columns[1][2],x.columns[2][2],x.columns[3][2],
			x.columns[0][3],x.columns[1][3],x.columns[2][3],x.columns[3][3]
			);*/
	state->cam_yaw = 0;
	state->cam_pitch = 0;

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
	struct program_state *state = (struct program_state*)appstate;
	switch(event->type) {
		case SDL_EVENT_QUIT:
			return SDL_APP_SUCCESS;
			break;
		case SDL_EVENT_MOUSE_MOTION:
			state->cam_yaw += event->motion.xrel / 100;
			state->cam_pitch -= event->motion.yrel / 100;
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

SDL_AppResult SDL_AppIterate(void *appstate) {
	struct program_state *state = (struct program_state*)appstate;
	SDL_Surface *win_orig_surface = SDL_GetWindowSurface(state->window);
	SDL_Surface *win_surface = SDL_ConvertSurface(win_orig_surface, SDL_PIXELFORMAT_ABGR8888);

	state->pixels.width = win_surface->w;
	state->pixels.height = win_surface->h;
	state->pixels.pitch = win_surface->pitch;
	state->pixels.pixels = win_surface->pixels;
	state->pixels.bpp = SDL_BYTESPERPIXEL(win_surface->format);

	//memset(state->pixels.pixels, 0, state->pixels.pitch * state->pixels.height);
	mat4 rot_matrix = gen_rotation_matrix(state->cam_yaw, state->cam_pitch, 0);

#pragma omp parallel for
	for(int y = 0; y < state->pixels.height; ++y) {
		for(int x = 0; x < state->pixels.width; ++x) {
			float ndc_x = ((float)x / state->pixels.width - 0.5) * 2;
			float ndc_y = ((float)y / state->pixels.height - 0.5) * 2;
			vec4 ray_dir = m4_v4_mul(rot_matrix, (vec4){ndc_x, ndc_y, 1, 0});

			vec4 color = ray_dir;
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
