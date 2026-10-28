#include <SDL3/SDL_pixels.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <vectors.h>

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

	return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event) {
	switch(event->type) {
		case SDL_EVENT_QUIT:
			return SDL_APP_SUCCESS;
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

	for(int x = 0; x < state->pixels.width; ++x) {
		for(int y = 0; y < state->pixels.height; ++y) {
			float ndc_x = (float)x / state->pixels.width - 0.5;
			float ndc_y = (float)y / state->pixels.height - 0.5;
			vec4 ray_dir = (vec4){ndc_x + 0.5, ndc_y + 0.5, 0, 0};

			vec4 color = ray_dir;
			color[3] = 1;



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
