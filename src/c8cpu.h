#ifndef C8CPU_H
#define C8CPU_H
#include <stdint.h>
#include <stdio.h>

struct Instruction;
typedef struct Instruction Instruction;

signed int load_program(FILE *program);

/**
 * Initialize all CPU dependencies and start main loop.
 * @return The exit code of the program (the value stored in V0)
 */
int run_cpu();

/**
 * Fetches the next instruction from memory. That is, gets the 
 * instruction stored in the address pointed to by the program counter.
 * @return The next instruction
 */
uint16_t fetch();

/**
 * Decodes an instruction read from memory and splits it into
 * several useful pieces.
 * @param op The operation in binary read from the program in memory
 * @return A structure encapsulating useful information about the operation
 */
Instruction decode(uint16_t op);

/**
 * Takes a decoded instruction and simulates it running on the CHIP-8 VM.
 * @param instruction A pointer to the instruction to execute
 */
void execute(Instruction *instruction);

#endif