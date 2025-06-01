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
    Uint64 current_time;
    Uint64 chip8_timer;
    Uint64 chip8_timer_freq = 1000 / 60;

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
        if (sdl_update(&sdl_ctx, &cpu_ctx)) {
            running = 0;
        }

        /* Update 60Hz timers */
        current_time = SDL_GetTicks64();
        
        if (current_time - chip8_timer >= chip8_timer_freq) {
            if (cpu_ctx.delay_timer > 0) {
                cpu_ctx.delay_timer--;
            }

            if (cpu_ctx.sound_timer > 0) {
                /* TODO: Play sound */
                cpu_ctx.sound_timer--;
            }

            chip8_timer = current_time;
        }

        chip8_cycle(&cpu_ctx);
        sdl_render(&sdl_ctx, cpu_ctx.display);
    }

    sdl_cleanup(&sdl_ctx);

    return 0;
}
