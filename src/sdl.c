#include "sdl.h"

int sdl_init(struct sdl_context *ctx)
{
    memset(ctx, 0, sizeof(*ctx));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return 1;
    }

    ctx->window = SDL_CreateWindow("chip8",
                                   SDL_WINDOWPOS_CENTERED,
                                   SDL_WINDOWPOS_CENTERED,
                                   640, 320,
                                   0);

    if (!ctx->window) {
        sdl_cleanup(ctx);
        return 1;
    }

    ctx->renderer = SDL_CreateRenderer(ctx->window, -1,
                                       SDL_RENDERER_ACCELERATED);

    if (!ctx->renderer) {
        sdl_cleanup(ctx);
        return 1;
    }

    ctx->texture = SDL_CreateTexture(ctx->renderer,
                                     SDL_PIXELFORMAT_RGBA8888,
                                     SDL_TEXTUREACCESS_STREAMING,
                                     64, 32);

    if (!ctx->texture) {
        sdl_cleanup(ctx);
        return 1;
    }

    return 0;
}

void sdl_cleanup(struct sdl_context *ctx)
{
    if (ctx->texture) {
        SDL_DestroyTexture(ctx->texture);
    }

    if (ctx->renderer) {
        SDL_DestroyRenderer(ctx->renderer);
    }

    if (ctx->window) {
        SDL_DestroyWindow(ctx->window);
    }

    SDL_Quit();
}

int sdl_update(struct sdl_context *ctx)
{
    while (SDL_PollEvent(&ctx->event)) {
        if (ctx->event.type == SDL_QUIT) {
            return 1;
        }
    }

    return 0;
}

void sdl_render(struct sdl_context *ctx, uint32_t *display)
{
    /* Update screen texture */
    void *pixels;
    int pitch;

    if (SDL_LockTexture(ctx->texture, NULL, &pixels, &pitch) == 0) {
        memcpy(pixels, display, 64 * 32 * sizeof(uint32_t));
        SDL_UnlockTexture(ctx->texture);
    }

    /* Update window */
    SDL_SetRenderDrawColor(ctx->renderer, 255, 255, 255, 255);
    SDL_RenderClear(ctx->renderer);
    SDL_RenderCopy(ctx->renderer, ctx->texture, NULL, NULL);
    SDL_RenderPresent(ctx->renderer);
}
