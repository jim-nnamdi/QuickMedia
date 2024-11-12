#include <SDL.h>
#include <SDL_thread.h>

int main(void)
{
    if(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
        fprintf(stderr, "could not initialize sdl: '%s'\n", SDL_GetError());
        exit(1);
    }
    int w = 10, h = 10;
    SDL_Window* screen;
    screen = SDL_CreateWindow("video",SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,w,h,0);
    if(!screen) fprintf(stderr, "cannot create window:'%s'\n", SDL_GetError()); exit(1);
    SDL_Renderer* renderer;
    renderer = SDL_CreateRenderer(screen,-1, SDL_RENDERER_ACCELERATED);
    if(!renderer) fprintf(stderr, "cannot create renderer: '%s'\n", SDL_GetError()); exit(1);
}