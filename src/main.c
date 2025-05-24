#include "sdl.h"
#include "chip8.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: <chip8 rom path>\n");
        return 0;
    }

    int running = 0;

    struct sdl_context sdl_ctx;
    struct chip8_context cpu_ctx;

    if (sdl_init(&sdl_ctx)) {
        return 1;
    }

    if (chip8_init(&cpu_ctx)) {
        sdl_cleanup(&sdl_ctx);
        return 1;
    }

    printf("Loading %s\n", argv[1]);
    chip8_loadrom(&cpu_ctx, argv[1]);

    running = 1;
    while (running) {
        if (sdl_update(&sdl_ctx)) {
            running = 0;
        }

        chip8_update(&cpu_ctx);
        sdl_render(&sdl_ctx, cpu_ctx.display);
    }

    sdl_cleanup(&sdl_ctx);

    return 0;
}
