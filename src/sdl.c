#include "sdl.h"

#include "chip8.h"

static void sdl_update_key(struct chip8_context *cpu_ctx,
                           SDL_Scancode scancode, int down);

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

int sdl_update(struct sdl_context *ctx, struct chip8_context *cpu_ctx)
{
    while (SDL_PollEvent(&ctx->event)) {
        switch (ctx->event.type) {
        case SDL_QUIT:
            return 1;

        case SDL_KEYDOWN:
            sdl_update_key(cpu_ctx, ctx->event.key.keysym.scancode, 1);
            break;

        case SDL_KEYUP:
            sdl_update_key(cpu_ctx, ctx->event.key.keysym.scancode, 0);
            break;

        default:
            break;
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

static void sdl_update_key(struct chip8_context *cpu_ctx,
                           SDL_Scancode scancode, int down)
{
    uint8_t is_down = down ? 1 : 0;

    switch (scancode) {
    case SDL_SCANCODE_1:
        cpu_ctx->keys[0x1] = is_down;
        break;

    case SDL_SCANCODE_2:
        cpu_ctx->keys[0x2] = is_down;
        break;

    case SDL_SCANCODE_3:
        cpu_ctx->keys[0x3] = is_down;
        break;

    case SDL_SCANCODE_4:
        cpu_ctx->keys[0xC] = is_down;
        break;

    case SDL_SCANCODE_Q:
        cpu_ctx->keys[0x4] = is_down;
        break;

    case SDL_SCANCODE_W:
        cpu_ctx->keys[0x5] = is_down;
        break;

    case SDL_SCANCODE_E:
        cpu_ctx->keys[0x6] = is_down;
        break;

    case SDL_SCANCODE_R:
        cpu_ctx->keys[0xD] = is_down;
        break;

    case SDL_SCANCODE_A:
        cpu_ctx->keys[0x7] = is_down;
        break;

    case SDL_SCANCODE_S:
        cpu_ctx->keys[0x8] = is_down;
        break;

    case SDL_SCANCODE_D:
        cpu_ctx->keys[0x9] = is_down;
        break;

    case SDL_SCANCODE_F:
        cpu_ctx->keys[0xE] = is_down;
        break;

    case SDL_SCANCODE_Z:
        cpu_ctx->keys[0xA] = is_down;
        break;

    case SDL_SCANCODE_X:
        cpu_ctx->keys[0x0] = is_down;
        break;

    case SDL_SCANCODE_C:
        cpu_ctx->keys[0xB] = is_down;
        break;

    case SDL_SCANCODE_V:
        cpu_ctx->keys[0xF] = is_down;
        break;

    default:
        break;
    }
}
