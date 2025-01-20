#include "c8cpu.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s [CHIP-8 ROM]\n", argv[0]);
        return -1;
    }

    // FILE *program = fopen(argv[1], "rb");
    load_program(NULL);
    // fclose(program);

    run_cpu();
}