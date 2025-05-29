#include <stdint.h>

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define DISPLAY_SIZE DISPLAY_WIDTH * DISPLAY_HEIGHT

#define RAM_SIZE 4096
#define STACK_SIZE 16

struct chip8_context {
    void (*opcode_table[0xF + 1])(struct chip8_context *);

    uint32_t display[DISPLAY_SIZE];

    uint16_t i;
    uint16_t pc;
    uint16_t stack[STACK_SIZE];

    uint16_t opcode;

    uint8_t mem[RAM_SIZE];
    uint8_t registers[16];
    uint8_t sp;

    uint8_t delay_timer;
    uint8_t sound_timer;
};

int chip8_init(struct chip8_context *ctx);
void chip8_loadrom(struct chip8_context *ctx, const char *filepath);
void chip8_cycle(struct chip8_context *ctx);
