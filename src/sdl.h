#ifndef CHIP8_SDL_H
#define CHIP8_SDL_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>

struct sdl_context
{
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Event event;
};

int sdl_init(struct sdl_context *ctx);
void sdl_cleanup(struct sdl_context *ctx);
int sdl_update(struct sdl_context *ctx);
void sdl_render(struct sdl_context *ctx, uint32_t *display);

#endif
