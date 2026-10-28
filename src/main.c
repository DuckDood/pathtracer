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
	unsigned char *pixels;
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

	mat4 x = construct_mat4_cm((float[16]){0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15});
	x = m4_m4_mul(x, x);

	printf("%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n%f, %f, %f, %f\n",
			x.columns[0][0],x.columns[1][0],x.columns[2][0],x.columns[3][0],
			x.columns[0][1],x.columns[1][1],x.columns[2][1],x.columns[3][1],
			x.columns[0][2],x.columns[1][2],x.columns[2][2],x.columns[3][2],
			x.columns[0][3],x.columns[1][3],x.columns[2][3],x.columns[3][3]
			);

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
	SDL_Surface *win_surface = SDL_GetWindowSurface(state->window);

	SDL_ClearSurface(win_surface, 0, 0, 0, 1);
	state->pixels.width = win_surface->w;
	state->pixels.height = win_surface->h;
	state->pixels.pitch = win_surface->pitch;
	state->pixels.pixels = win_surface->pixels;

	memset(state->pixels.pixels, 0, state->pixels.pitch * state->pixels.height);

	//For(int y = 

	SDL_UpdateWindowSurface(state->window);
	return SDL_APP_CONTINUE;
}

