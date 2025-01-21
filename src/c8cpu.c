#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "c8cpu.h"

// Magic Number Definitions
#define MEM_SIZE        4096
#define NUM_REGISTERS   0x10
#define PROGRAM_START   0x200
#define NUM_OPCODES     0x20
#define OP_SIZE         0x02

static const int SCALING_FACTOR = 10;
static const int DISPLAY_WIDTH = 64;
static const int DISPLAY_HEIGHT = 32;


typedef struct Instruction {
    uint8_t opcode;
    uint16_t operands;
    uint16_t raw;
} Instruction;

typedef enum {
    KEY_W = (1 << 8),
    KEY_A = (1 << 4),
    KEY_S = (1 << 6),
    KEY_D = (1 << 2)
} KeyCodes;

// I typedef'd a pointer
typedef void (*op_function)(Instruction*);

// CPU registers and memory
static _Bool running = (_Bool) 1;
static uint8_t MEMORY[MEM_SIZE];
static uint8_t REGISTERS[NUM_REGISTERS];
static SDL_Window *DISPLAY = NULL;
static SDL_Renderer *RENDERER = NULL;
static uint16_t KEYS = 0;
static uint16_t PC = PROGRAM_START;
static uint16_t SP = PROGRAM_START - 1;
static uint16_t I  = 0;

/**
 * Opcode: 0x1
 * Jumps to a given address.
 */
static void jmp_addr(Instruction *instruction) {
    uint16_t address = instruction->operands;
    PC = address;
}

/**
 * Opcode: 0x2
 * Stores the current program counter on the stack and jumps to the given address.
 */
static void call_subroutine(Instruction *instruction) {
    uint16_t address = instruction->operands;
    MEMORY[SP--] = PC;
    PC = address;
}

/**
 * Opcode: 0x3
 * Skips the next instruction if Vn is equal to a given 4-bit constant
 */
static void skip_eq_constant(Instruction *instruction) {
    uint8_t register_number = (instruction->operands & 0x0f00) >> 8;
    uint8_t constant = instruction->operands & 0x00ff;

    if (REGISTERS[register_number] == constant) {
        PC += OP_SIZE;
    }
}

/**
 * Opcode: 0x4
 * Skips the next instruction if Vn is not equal to a given 4-bit constant
 */
static void skip_neq_constant(Instruction *instruction) {
    uint8_t register_number = (instruction->operands & 0x0f00) >> 8;
    uint8_t constant = instruction->operands & 0x00ff;

    if (REGISTERS[register_number] != constant) {
        PC += OP_SIZE;
    }
}

/**
 * Opcode: 0x5
 * Skips the next instruction if Vx is equal to Vy
 */
static void skip_eq_register(Instruction *instruction) {
    uint8_t register_1 = (instruction->operands & 0x0f00) >> 8;
    uint8_t register_2 = (instruction->operands & 0x00f0) >> 4;

    if (REGISTERS[register_1] == REGISTERS[register_2]) {
        PC += OP_SIZE;
    }
}

/**
 * Opcode: 0x6
 * Stores a given constant in Vx
 */
static void mov_constant(Instruction *instruction) {
    uint8_t register_number = (instruction->operands & 0x0f00) >> 8;
    uint8_t constant = instruction->operands & 0x00ff;
    REGISTERS[register_number] = constant;
}

/**
 * Opcode: 0x7
 * Adds a constant to a register Vx
 */
static void add_constant(Instruction *instruction) {
    uint8_t register_number = (instruction->operands & 0x0f00) >> 8;
    uint8_t constant = instruction->operands & 0x00ff;
    REGISTERS[register_number] += constant;
}

/**
 * Opcode: 0x8
 * Handles the following cases involving two registers:
 * `Vx = Vy`      (8XY0)
 * `Vx |= Vy`     (8XY1)
 * `Vx &= Vy`     (8XY2)
 * `Vx ^= Vy`     (8XY3)
 * `Vx += Vy`     (8XY4)
 * `Vx -= Vy`     (8XY5)
 * `Vx >>= Vy`    (8XY6)
 * `Vx = Vy - Vx` (8XY7)
 * `Vx <<= Vy`    (8XYE)
 */
static void compute_operation(Instruction *instruction) {
    uint8_t ending = instruction->operands & 0x0f;  
    uint8_t vx = (instruction->operands & 0x0f00) >> 8;
    uint8_t vy = (instruction->operands & 0x00f0) >> 4;

    switch (ending) {
        case 0x0: REGISTERS[vx] = REGISTERS[vy]; break;
        case 0x1: REGISTERS[vx] |= REGISTERS[vy]; break;
        case 0x2: REGISTERS[vx] &= REGISTERS[vy]; break;
        case 0x3: REGISTERS[vx] ^= REGISTERS[vy]; break;
        case 0x4: REGISTERS[vx] += REGISTERS[vy]; break;
        case 0x5: REGISTERS[vx] -= REGISTERS[vy]; break;
        case 0x6: REGISTERS[vx] >>= REGISTERS[vy]; break;
        case 0x7: REGISTERS[vx] = REGISTERS[vy] - REGISTERS[vx]; break;
        case 0xe: REGISTERS[vx] <<= REGISTERS[vy]; break;
    }
}

/**
 * Opcode: 0x9
 * Skips the next instruction if Vx = Vy
 */
static void skip_neq_register(Instruction *instruction) {
    uint8_t vx = (instruction->operands & 0x0f00) >> 8;
    uint8_t vy = (instruction->operands & 0x00f0) >> 4;

    if (REGISTERS[vx] != REGISTERS[vy]) {
        PC += OP_SIZE;
    }
}

/**
 * Opcode: 0xa
 * Sets the I register to a given address.
 */
static void set_i_register(Instruction *instruction) {
    uint16_t address = instruction->operands;
    I = address;
}

/**
 * Opcode: 0xb
 * Jumps to the address stored in V0 + a given offset
 */
static void jmp_v0_offset(Instruction *instruction) {
    uint16_t address = instruction->operands;
    PC = REGISTERS[0] + address;
}

/**
 * Opcode: 0xc
 * Generates a random number, performs a bitwise and with a given constant,
 * and then stores the result in the given register.
 */
static void generate_random(Instruction *instruction) {
    uint8_t vx = (instruction->operands & 0x0f00) >> 8;
    uint8_t constant = instruction->operands & 0x00ff;
    uint8_t random_number = (uint8_t) rand();
    REGISTERS[vx] = random_number & constant;
}

/**
 * Opcode: 0xd
 * Draws a sprite at (Vx, Vy) with a width of 8 pixels and a height of
 * a given number.
 */
static void draw(Instruction *instruction) {
    int8_t x_pos = (instruction->operands & 0x0f00) >> 8;
    int8_t y_pos = (instruction->operands & 0x00f0) >> 4;
    int8_t height = instruction->operands & 0x000f;

    SDL_Rect sprite = (SDL_Rect) {
        .x = x_pos,
        .y = y_pos,
        .w = 8 * SCALING_FACTOR,
        .h = height * SCALING_FACTOR
    };

    SDL_SetRenderDrawColor(RENDERER, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderFillRect(RENDERER, &sprite);
    SDL_RenderPresent(RENDERER);
}

/**
 * Opcode: 0xe
 * Skips the next instruction if the key stored in Vx is pressed.
 */
static void skip_on_key_event(Instruction *instruction) {
    uint8_t vx = (instruction->operands & 0x0f00) >> 8;
    uint8_t ending = instruction->operands & 0xff;
    
    switch (ending) {
        // Skip if key in Vx is pressed
        case 0x9e:
            if ((KEYS & (1 << REGISTERS[vx])) != 0) {
                PC += OP_SIZE;
            }
            break;

        // Skip if key in Vx is not pressed
        case 0xa1:
            if ((KEYS & (1 << REGISTERS[vx])) == 0) {
                PC += OP_SIZE;
            }
            break;
    }
}

/**
 * Opcode: 0xf
 * Stores the value of the delay timer in Vx
 */
static void get_delay(Instruction *instruction) {

}

// Map opcodes to their respective handlers
static op_function OPERATIONS[NUM_OPCODES] = {
    [0x1] = jmp_addr,
    [0x2] = call_subroutine,
    [0x3] = skip_eq_constant,
    [0x4] = skip_neq_constant,
    [0x5] = skip_eq_register,
    [0x6] = mov_constant,
    [0x7] = add_constant,
    [0x8] = compute_operation,
    [0x9] = skip_neq_register,
    [0xa] = set_i_register,
    [0xb] = jmp_v0_offset,
    [0xc] = generate_random,
    [0xd] = draw,
    [0xe] = skip_on_key_event,
    [0xf] = get_delay
};

/**
 * 
 */
signed int load_program(FILE *program) {
    fseek(program, 0L, SEEK_END);
    size_t program_size = ftell(program);
    rewind(program);

    if (program_size > (MEM_SIZE - PROGRAM_START + 1)) {
        return -1;
    }
    
    fread(&MEMORY[PC], sizeof(uint8_t), program_size, program);
    return 0;
}

static void print_vm_debug_info() {
    printf("PC: %lu, SP: %lu, (SP): %u\n", PC, SP, MEMORY[SP]);
    puts("Registers\n==============");
    for (size_t i = 0; i < NUM_REGISTERS; ++i) {
        printf("  %lu ", (size_t) REGISTERS[i]);
    }
    puts("");
    for (size_t i = 0; i < NUM_REGISTERS; ++i) {
        printf(" v%lu ", i);
    }
    puts("\n");
}

static int init_display() {
    return SDL_CreateWindowAndRenderer(
        DISPLAY_WIDTH * SCALING_FACTOR,
        DISPLAY_HEIGHT * SCALING_FACTOR, 
        SDL_WINDOW_SHOWN, 
        &DISPLAY, 
        &RENDERER
    );
}

/**
 * Runs whenever a key is pressed
 */
static void handle_key_down(SDL_Event *key_event) {
    SDL_KeyCode key_code = key_event->key.keysym.sym;
    if (key_code == SDLK_w) KEYS |= KEY_W;
    if (key_code == SDLK_a) KEYS |= KEY_A;
    if (key_code == SDLK_s) KEYS |= KEY_S;
    if (key_code == SDLK_d) KEYS |= KEY_D;
}

/**
 * Runs whenever a key is released
 */
static void handle_key_up(SDL_Event *key_event) {
    SDL_KeyCode key_code = key_event->key.keysym.sym;
    if (key_code == SDLK_w) KEYS &= ~KEY_W;
    if (key_code == SDLK_a) KEYS &= ~KEY_A;
    if (key_code == SDLK_s) KEYS &= ~KEY_S;
    if (key_code == SDLK_d) KEYS &= ~KEY_D;
}

/**
 * Handle external events such as exiting, and key eventsOpcode: 
 */
static void process_events() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = (_Bool) 0;
        }
        if (event.type == SDL_KEYDOWN) {
            handle_key_down(&event);
        }
        if (event.type == SDL_KEYUP) {
            handle_key_up(&event);
        }
    }
}

/**
 * Initialize all CPU dependencies and start main loop.
 */
void run_cpu() {
    srand(time(NULL));
    if (init_display() < 0) {
        fputs("Failed to initialize display.", stderr);
        return;
    }

    while (running) {
        process_events();
        uint16_t op = fetch();
        if (op == 0) break;
        Instruction instruction = decode(op);
        execute(&instruction);
    }

    SDL_DestroyWindow(DISPLAY);
    SDL_DestroyRenderer(RENDERER);
}

/**
 * Fetch the next instruction from memory.
 */
uint16_t fetch() {
    uint16_t op = (MEMORY[PC] << 8) | MEMORY[PC + 1];
    PC += OP_SIZE;
    return op;
}

/**
 * Decode raw binary into an instruction.
 */
Instruction decode(uint16_t op) {
    Instruction instruction = (Instruction) {
        .opcode = (op & 0xf000) >> 12,
        .operands = op & 0x0fff,
        .raw = op
    };
    return instruction;
}

/**
 * Execute a decoded instruction.
 */
void execute(Instruction *instruction) {
    // Clear screen
    if (instruction->raw == 0x00e0) {
        SDL_SetRenderDrawColor(RENDERER, 0, 0, 0, 0xff);
        SDL_RenderClear(RENDERER);
        return;
    }

    // return
    if (instruction->raw == 0x00ee) {
        PC = MEMORY[SP++];
        return;
    }

    op_function op_handler = OPERATIONS[instruction->opcode];
    op_handler(instruction);
}