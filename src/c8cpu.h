#ifndef C8CPU_H
#define C8CPU_H
#include <stdint.h>
#include <stdio.h>

struct Instruction;
typedef struct Instruction Instruction;

signed int load_program(FILE *program);
void run_cpu();
uint16_t fetch();
Instruction decode(uint16_t op);
void execute(Instruction *instruction);

#endif