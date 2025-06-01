#include "chip8.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* http://devernay.free.fr/hacks/chip8/C8TECH10.HTM */
/* Mostly used this as a reference */

#define RESERVED_START 0x000
#define RESERVED_END 0x1FF
#define PROGRAM_START 0x200
#define PROGRAM_END 0xFFF

static void op_invalid(struct chip8_context *ctx);

static void op_0_decode(struct chip8_context *ctx);
static void op_0nnn(struct chip8_context *ctx); /* SYS addr */
static void op_00E0(struct chip8_context *ctx); /* CLS */
static void op_00EE(struct chip8_context *ctx); /* RET */

static void op_1nnn(struct chip8_context *ctx); /* JP addr */
static void op_2nnn(struct chip8_context *ctx); /* CALL addr */
static void op_3xkk(struct chip8_context *ctx); /* SE Vx, byte */
static void op_4xkk(struct chip8_context *ctx); /* SNE Vx, byte */
static void op_5xy0(struct chip8_context *ctx); /* SE Vx, Vy */
static void op_6xkk(struct chip8_context *ctx); /* LD Vx, byte */
static void op_7xkk(struct chip8_context *ctx); /* ADD Vx, byte */

static void op_8_decode(struct chip8_context *ctx);
static void op_8xy0(struct chip8_context *ctx); /* LD Vx, Vy */
static void op_8xy1(struct chip8_context *ctx); /* OR Vx, Vy */
static void op_8xy2(struct chip8_context *ctx); /* AND Vx, Vy */
static void op_8xy3(struct chip8_context *ctx); /* XOR Vx, Vy */
static void op_8xy4(struct chip8_context *ctx); /* ADD Vx, Vy */
static void op_8xy5(struct chip8_context *ctx); /* SUB Vx, Vy */
static void op_8xy6(struct chip8_context *ctx); /* SHR Vx {, Vy} */
static void op_8xy7(struct chip8_context *ctx); /* SUBN Vx, Vy */
static void op_8xyE(struct chip8_context *ctx); /* SHL Vx {, Vy} */

static void op_9xy0(struct chip8_context *ctx); /* SNE Vx, Vy */
static void op_Annn(struct chip8_context *ctx); /* LD I, addr */
static void op_Bnnn(struct chip8_context *ctx); /* JP V0, addr */
static void op_Cxkk(struct chip8_context *ctx); /* RND Vx, byte */
static void op_Dxyn(struct chip8_context *ctx); /* DRW Vx, Vy, nibble */

static void op_E_decode(struct chip8_context *ctx);
static void op_Ex9E(struct chip8_context *ctx); /* SKP Vx */
static void op_ExA1(struct chip8_context *ctx); /* SKNP Vx */

static void op_F_decode(struct chip8_context *ctx);
static void op_Fx07(struct chip8_context *ctx); /* LD Vx, DT */
static void op_Fx0A(struct chip8_context *ctx); /* LD Vx, K */
static void op_Fx15(struct chip8_context *ctx); /* LD DT, Vx */
static void op_Fx18(struct chip8_context *ctx); /* LD ST, Vx */
static void op_Fx1E(struct chip8_context *ctx); /* ADD I, Vx */
static void op_Fx29(struct chip8_context *ctx); /* LD F, Vx */
static void op_Fx33(struct chip8_context *ctx); /* LD B, Vx */
static void op_Fx55(struct chip8_context *ctx); /* LD [I], Vx */
static void op_Fx65(struct chip8_context *ctx); /* LD Vx, [I] */

int chip8_init(struct chip8_context *ctx)
{
    memset(ctx, 0, sizeof(*ctx));

    /* Initialise font sprites */
    uint8_t fonts[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, /* 0 */
        0x20, 0x60, 0x20, 0x20, 0x70, /* 1 */
        0xF0, 0x10, 0xF0, 0x80, 0xF0, /* 2 */
        0xF0, 0x10, 0xF0, 0x10, 0xF0, /* 3 */
        0x90, 0x90, 0xF0, 0x10, 0x10, /* 4 */
        0xF0, 0x80, 0xF0, 0x10, 0xF0, /* 5 */
        0xF0, 0x80, 0xF0, 0x90, 0xF0, /* 6 */
        0xF0, 0x10, 0x20, 0x40, 0x40, /* 7 */
        0xF0, 0x90, 0xF0, 0x90, 0xF0, /* 8 */
        0xF0, 0x90, 0xF0, 0x10, 0xF0, /* 9 */
        0xF0, 0x90, 0xF0, 0x90, 0x90, /* A */
        0xE0, 0x90, 0xE0, 0x90, 0xE0, /* B */
        0xF0, 0x80, 0x80, 0x80, 0xF0, /* C */
        0xE0, 0x90, 0x90, 0x90, 0xE0, /* D */
        0xF0, 0x80, 0xF0, 0x80, 0xF0, /* E */
        0xF0, 0x80, 0xF0, 0x80, 0x80  /* F */
    };

    memcpy(&ctx->mem[0], fonts, sizeof(fonts));

    /* Initialise opcode table */
    ctx->opcode_table[0x0] = &op_0_decode;
    ctx->opcode_table[0x1] = &op_1nnn;
    ctx->opcode_table[0x2] = &op_2nnn;
    ctx->opcode_table[0x3] = &op_3xkk;
    ctx->opcode_table[0x4] = &op_4xkk;
    ctx->opcode_table[0x5] = &op_5xy0;
    ctx->opcode_table[0x6] = &op_6xkk;
    ctx->opcode_table[0x7] = &op_7xkk;
    ctx->opcode_table[0x8] = &op_8_decode;
    ctx->opcode_table[0x9] = &op_9xy0;
    ctx->opcode_table[0xA] = &op_Annn;
    ctx->opcode_table[0xB] = &op_Bnnn;
    ctx->opcode_table[0xC] = &op_Cxkk;
    ctx->opcode_table[0xD] = &op_Dxyn;
    ctx->opcode_table[0xE] = &op_E_decode;
    ctx->opcode_table[0xF] = &op_F_decode;

    ctx->pc = PROGRAM_START;

    return 0;
}

void chip8_loadrom(struct chip8_context *ctx, const char *filepath)
{
    FILE *f = NULL;

    /* Clear current program */
    ctx->pc = PROGRAM_START;
    memset(&ctx->mem[PROGRAM_START], 0, PROGRAM_END - PROGRAM_START);

    if ((f = fopen(filepath, "rb")) != NULL) {
        char buffer[PROGRAM_END - PROGRAM_START];
        size_t bytes_read = 0;

        bytes_read = fread(buffer, 1, sizeof(buffer), f);
        memcpy(&ctx->mem[PROGRAM_START], buffer, bytes_read);

        fclose(f);
    }
}

void chip8_cycle(struct chip8_context *ctx)
{
    if (ctx->pc < PROGRAM_END) {
        ctx->opcode = (ctx->mem[ctx->pc] << 8u) | ctx->mem[ctx->pc + 1];

        /* printf("[%03x] %04x\n", ctx->pc, ctx->opcode); */

        ctx->pc += 2;

        if (ctx->opcode) {
            ctx->opcode_table[ctx->opcode >> 12u](ctx);
        }
    }
}

static void op_invalid(struct chip8_context *ctx)
{
    printf("Unhandled instruction.\n");
}

static void op_0_decode(struct chip8_context *ctx)
{
    switch (ctx->opcode & 0x00FFu) {
    case 0xE0:
        op_00E0(ctx);
        break;

    case 0xEE:
        op_00EE(ctx);
        break;

    default:
        op_invalid(ctx);
    }
}

static void op_0nnn(struct chip8_context *ctx)
{
    /* SYS addr */
}

static void op_00E0(struct chip8_context *ctx)
{
    /* CLS */
    memset(ctx->display, 0, DISPLAY_SIZE * sizeof(uint32_t));
}

static void op_00EE(struct chip8_context *ctx)
{
    /* RET */
    ctx->pc = ctx->stack[--ctx->sp];
}

static void op_1nnn(struct chip8_context *ctx)
{
    /* JP addr */
    ctx->pc = ctx->opcode & 0x0FFFu;
}

static void op_2nnn(struct chip8_context *ctx)
{
    /* CALL addr */
    ctx->stack[ctx->sp++] = ctx->pc;
    ctx->pc = ctx->opcode & 0x0FFFu;
}

static void op_3xkk(struct chip8_context *ctx)
{
    /* SE Vx, byte */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;
    uint8_t byte = ctx->opcode & 0x00FFu;

    if (ctx->registers[x] == byte) {
        ctx->pc += 2;
    }
}

static void op_4xkk(struct chip8_context *ctx)
{
    /* SNE Vx, byte */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;
    uint8_t byte = ctx->opcode & 0x00FFu;

    if (ctx->registers[x] != byte) {
        ctx->pc += 2;
    }
}

static void op_5xy0(struct chip8_context *ctx)
{
    /* SE Vx, Vy */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;
    uint8_t y = (ctx->opcode & 0x00F0u) >> 4u;

    if (ctx->registers[x] == ctx->registers[y]) {
        ctx->pc += 2;
    }
}

static void op_6xkk(struct chip8_context *ctx)
{
    /* LD Vx, byte */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;

    ctx->registers[x] = ctx->opcode & 0x00FFu;
}

static void op_7xkk(struct chip8_context *ctx)
{
    /* ADD Vx, byte */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;

    ctx->registers[x] += ctx->opcode & 0x00FFu;
}

static void op_8_decode(struct chip8_context *ctx)
{
    switch (ctx->opcode & 0x000Fu) {
    case 0x0:
        op_8xy0(ctx);
        break;

    case 0x1:
        op_8xy1(ctx);
        break;

    case 0x2:
        op_8xy2(ctx);
        break;

    case 0x3:
        op_8xy3(ctx);
        break;

    case 0x4:
        op_8xy4(ctx);
        break;

    case 0x5:
        op_8xy5(ctx);
        break;

    case 0x6:
        op_8xy6(ctx);
        break;

    case 0x7:
        op_8xy7(ctx);
        break;

    case 0xE:
        op_8xyE(ctx);
        break;

    default:
        op_invalid(ctx);
        break;
    }
}

static void op_8xy0(struct chip8_context *ctx)
{
    /* LD Vx, Vy */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;
    uint8_t y = (ctx->opcode & 0x00F0u) >> 4u;

    ctx->registers[x] = ctx->registers[y];
}

static void op_8xy1(struct chip8_context *ctx)
{
    /* OR Vx, Vy */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;
    uint8_t y = (ctx->opcode & 0x00F0u) >> 4u;

    ctx->registers[x] |= ctx->registers[y];
}

static void op_8xy2(struct chip8_context *ctx)
{
    /* AND Vx, Vy */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;
    uint8_t y = (ctx->opcode & 0x00F0u) >> 4u;

    ctx->registers[x] &= ctx->registers[y];
}

static void op_8xy3(struct chip8_context *ctx)
{
    /* XOR Vx, Vy */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;
    uint8_t y = (ctx->opcode & 0x00F0u) >> 4u;

    ctx->registers[x] ^= ctx->registers[y];
}

static void op_8xy4(struct chip8_context *ctx)
{
    /* ADD Vx, Vy */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;
    uint8_t y = (ctx->opcode & 0x00F0u) >> 4u;
    uint16_t res = ctx->registers[x] + ctx->registers[y];

    ctx->registers[0xF] = res > 255 ? 1 : 0;
    ctx->registers[x] = res & 0x00FFu;
}

static void op_8xy5(struct chip8_context *ctx)
{
    /* SUB Vx, Vy */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;
    uint8_t y = (ctx->opcode & 0x00F0u) >> 4u;

    ctx->registers[0xF] = x > y ? 1 : 0;
    ctx->registers[x] -= ctx->registers[y];
}

static void op_8xy6(struct chip8_context *ctx)
{
    /* SHR Vx, {, Vy} */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;

    ctx->registers[0xF] = ctx->registers[x] & 0x1 ? 1 : 0;
    ctx->registers[x] >>= 1;
}

static void op_8xy7(struct chip8_context *ctx)
{
    /* SUBN Vx, Vy */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;
    uint8_t y = (ctx->opcode & 0x00F0u) >> 4u;

    ctx->registers[0xF] = ctx->registers[y] > ctx->registers[x] ? 1 : 0;
    ctx->registers[x] = ctx->registers[y] - ctx->registers[x];
}

static void op_8xyE(struct chip8_context *ctx)
{
    /* SHL Vx, {, Vy} */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;
    uint8_t y = (ctx->opcode & 0x00F0u) >> 4u;

    ctx->registers[0xF] = ctx->registers[x] & 128u ? 1 : 0;
    ctx->registers[x] <<= 1;
}

static void op_9xy0(struct chip8_context *ctx)
{
    /* SNE Vx, Vy */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;
    uint8_t y = (ctx->opcode & 0x00F0u) >> 4u;

    if (ctx->registers[x] != ctx->registers[y]) {
        ctx->pc += 2;
    }
}

static void op_Annn(struct chip8_context *ctx)
{
    /* LD I, addr */
    ctx->i = ctx->opcode & 0x0FFFu;
}

static void op_Bnnn(struct chip8_context *ctx)
{
    /* JP V0, addr */
    ctx->pc = (ctx->opcode & 0x0FFFu) + ctx->registers[0x0];
}

static void op_Cxkk(struct chip8_context *ctx)
{
    /* RND Vx, byte */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;

    srand(time(NULL));
    ctx->registers[x] = (uint8_t)(rand()) & (ctx->opcode & 0x00FFu);
}

static void op_Dxyn(struct chip8_context *ctx)
{
    /* DRW Vx, Vy, nibble */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;
    uint8_t y = (ctx->opcode & 0x00F0u) >> 4u;
    uint8_t n = (ctx->opcode & 0x000Fu);

    uint8_t *sprite = &ctx->mem[ctx->i];
    uint8_t x_origin = ctx->registers[x];
    uint8_t y_origin = ctx->registers[y];

    /* TODO: If outside display, wrap around screen */

    for (uint8_t row = 0; row < n; ++row) {
        for (uint8_t col = 0; col < 8; ++col) {
            if (sprite[row] & (128 >> col)) {
                uint32_t *px = &ctx->display[(y_origin + row)
                                             * DISPLAY_WIDTH + x_origin + col];

                if (*px == 0xFFFFFFFF) {
                    ctx->registers[0xF] = 1;
                }

                *px ^= 0xFFFFFFFF;
            }
        }
    }
}

static void op_E_decode(struct chip8_context *ctx)
{
    switch (ctx->opcode & 0x00FFu) {
    case 0x9E:
        op_Ex9E(ctx);
        break;

    case 0xA1:
        op_ExA1(ctx);
        break;

    default:
        op_invalid(ctx);
        break;
    }
}

static void op_Ex9E(struct chip8_context *ctx)
{
    /* SKP Vx */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;

    if (ctx->keys[ctx->registers[x]]) {
        ctx->pc += 2;
    }
}

static void op_ExA1(struct chip8_context *ctx)
{
    /* SKNP Vx */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;

    if (!ctx->keys[ctx->registers[x]]) {
        ctx->pc += 2;
    }
}

static void op_F_decode(struct chip8_context *ctx)
{
    switch (ctx->opcode & 0x00FFu) {
    case 0x07:
        op_Fx07(ctx);
        break;

    case 0x0A:
        op_Fx0A(ctx);
        break;

    case 0x15:
        op_Fx15(ctx);
        break;

    case 0x18:
        op_Fx18(ctx);
        break;

    case 0x1E:
        op_Fx1E(ctx);
        break;

    case 0x29:
        op_Fx29(ctx);
        break;

    case 0x33:
        op_Fx33(ctx);
        break;

    case 0x55:
        op_Fx55(ctx);
        break;

    case 0x65:
        op_Fx65(ctx);
        break;

    default:
        op_invalid(ctx);
        break;
    }
}

static void op_Fx07(struct chip8_context *ctx)
{
    /* LD Vx, DT */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;
    ctx->registers[x] = ctx->delay_timer;
}

static void op_Fx0A(struct chip8_context *ctx)
{
    /* LD Vx, K */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;

    for (int i = 0; i < 0xF + 1; ++i) {
        if (ctx->keys[i]) {
            ctx->registers[x] = i;
            return;
        }
    }

    ctx->pc -= 2;
}

static void op_Fx15(struct chip8_context *ctx)
{
    /* LD DT, Vx */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;
    ctx->delay_timer = ctx->registers[x];
}

static void op_Fx18(struct chip8_context *ctx)
{
    /* LD ST, Vx */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;
    ctx->sound_timer = ctx->registers[x];
}

static void op_Fx1E(struct chip8_context *ctx)
{
    /* ADD I, Vx */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8;
    ctx->i += ctx->registers[x];
}

static void op_Fx29(struct chip8_context *ctx)
{
    /* TODO drawing */
}

static void op_Fx33(struct chip8_context *ctx)
{
    /* TODO drawing */
}

static void op_Fx55(struct chip8_context *ctx)
{
    /* LD [I], Vx */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;

    for (uint8_t j = 0; j < x; ++j) {
        ctx->mem[ctx->i + j] = ctx->registers[j];
    }
}

static void op_Fx65(struct chip8_context *ctx)
{
    /* LD Vx, [I] */
    uint8_t x = (ctx->opcode & 0x0F00u) >> 8u;

    for (uint8_t j = 0; j < x; ++j) {
        ctx->registers[j] = ctx->mem[ctx->i + j];
    }
}
