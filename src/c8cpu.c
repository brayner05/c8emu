#include <string.h>
#include "c8cpu.h"

#define MEM_SIZE        4096
#define NUM_REGISTERS   0x10
#define PROGRAM_START   0x200
#define NUM_OPCODES     0x20

// typedef enum {

// } Operation;

typedef struct Instruction {
    uint8_t opcode;
    uint16_t operands;
} Instruction;

typedef void (*op_function)(void);

static uint8_t MEMORY[MEM_SIZE];
static uint8_t REGISTERS[NUM_REGISTERS];
static uint16_t PC = PROGRAM_START;

static op_function OPERATIONS[NUM_OPCODES] = {
    [1] = NULL,
};

void load_program(FILE *program) {
    // fseek(program, 0L, SEEK_END);
    // size_t program_size = ftell(program);
    // rewind(program);
    // fread(&MEMORY[PC], sizeof(uint8_t), program_size, program);
    MEMORY[PROGRAM_START] = 0x12;
    MEMORY[PROGRAM_START + 1] = 0xff;
}

void run_cpu() {
    while (1) {
        uint16_t op = fetch();
        if (op == 0) break;
        Instruction instruction = decode(op);
        execute(instruction);
    }
}

uint16_t fetch() {
    uint16_t op = (MEMORY[PC] << 8) | MEMORY[PC + 1];
    PC += 2;
    return op;
}

Instruction decode(uint16_t op) {
    Instruction instruction;
    instruction.opcode = (op & 0xf000) >> 12;
    instruction.operands = op & 0x0fff;
    return instruction;
}

void execute(Instruction instruction) {
    printf("%u %x\n", instruction.opcode, instruction.operands);
}