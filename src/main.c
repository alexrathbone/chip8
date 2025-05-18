#include <SDL2/SDL.h>
#include <SDL2/SDL_main.h>

#include <stdio.h>
#include <stdint.h>
#include <string.h>

struct chip8_context {
    uint32_t video[64 * 32];

    uint16_t index;
    uint16_t pc;
    uint16_t stack[16];

    uint8_t mem[0xFFF + 1];
    uint8_t reg[16];
    uint8_t sp;

    uint8_t delay_timer;
    uint8_t sound_timer;
};

int main(int argc, char **argv)
{
    SDL_Window *window;
    SDL_Event e;
    int running = 0;
    struct chip8_context ctx;

    memset(&ctx, 0, sizeof(ctx));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return 1;
    }

    window = SDL_CreateWindow("chip8",
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              480, 320,
                              0);

    if (window == NULL) {
        SDL_Quit();
        return 1;
    }

    running = 1;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = 0;
            }
        }
    }

    SDL_Quit();

    return 0;
}
