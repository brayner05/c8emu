#include <SDL2/SDL.h>
#include "c8cpu.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s [CHIP-8 ROM]\n", argv[0]);
        return -1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        return -1;
    }

    FILE *program = fopen(argv[1], "rb");
    if (program == NULL) {
        perror("Error");
        return -1;
    }

    if (load_program(program) < 0) {
        puts("File is to large");
        return -1;
    }

    fclose(program);
    int exit_status = run_cpu();
    SDL_Quit();

    printf("Exited with status %d.\n", exit_status);
}