#include "../include/vm.h"

#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/image_loader.h"
#include "../include/operations.h"


#ifdef __linux__
#include <sys/termios.h>
#include <unistd.h>
struct termios original_tio;
#elif _WIN32
#include <Windows.h>
HANDLE hStdin = INVALID_HANDLE_VALUE;
DWORD fdwMode, fdwOldMode;
#endif

uint16_t* memory = NULL;
uint16_t* regs = NULL;

void disable_input_buffering() {
  #ifdef __linux__
  tcgetattr(STDIN_FILENO, &original_tio);
  struct termios new_tio = original_tio;
  new_tio.c_lflag &= (unsigned int)(~ICANON & ~ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
  #elif _WIN32
    hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdin, &fdwOldMode);     /* save old mode */
    fdwMode = fdwOldMode ^ ENABLE_ECHO_INPUT /* no input echo */
              ^ ENABLE_LINE_INPUT;           /* return when one or
                                                more characters are available
                                                */
    SetConsoleMode(hStdin, fdwMode);         /* set new mode */
    FlushConsoleInputBuffer(hStdin);         /* clear buffer */
  #endif
}

void restore_input_buffering() {
  #ifdef __linux__
  tcsetattr(STDIN_FILENO, TCSANOW, &original_tio);
  #elif _WIN32
    SetConsoleMode(hStdin, fdwOldMode);
  #endif
}

void clean_up(void) {
  if (memory) free(memory);
  if (regs) free(regs);
}

void handle_interrupt(int signal) {
  restore_input_buffering();
  clean_up();
  printf("interrupted with signal %d\n", signal);
  exit(signal);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    fprintf(stderr, "%s\n", "missing image file");
    return EXIT_FAILURE;
  }
  
  // setup initial memory 'segment' and registers
  memory = calloc(UINT16_MAX, sizeof *memory);
  if (!memory) {
    fprintf(stderr, "%s\n", "memory allocation failure");
    return EXIT_FAILURE;
  }
  regs = calloc(R_COUNT, sizeof *regs);
  if (!regs) {
    fprintf(stderr, "%s\n", "registers allocation failure");
    clean_up();
    return EXIT_FAILURE;
  }

  // set the PC register to the starting position which is 0x3000 by default
  enum { PC_START = 0x3000 };
  regs[R_PC] = PC_START;
  regs[R_COND] = FL_ZRO;

  for (int i = 1; i < argc; i++) {
    if (!read_image(argv[i], memory)) {
      fprintf(stderr, "failed to load image %s\n", argv[i]);
      clean_up();
      return EXIT_FAILURE;
    }
  }

  signal(SIGINT, handle_interrupt);
  disable_input_buffering();

  int run = 1;  
  while (run) {
    // fetch instruction from 'memory'
    uint16_t instruction = mem_read(regs[R_PC]++, memory);
    uint16_t op = instruction >> 12;

    switch (op) {
      case OP_ADD:
        add(instruction, regs);
        break;
      case OP_AND:
        bitwise_and(instruction, regs);
        break;
      case OP_BR:
        branch(instruction, regs);
        break;
      case OP_JMP:
        jump(instruction, regs);
        break;
      case OP_JSR:
        jump_register(instruction, regs);
        break;
      case OP_LD:
        load(instruction, regs, memory);
        break;
      case OP_LDI:
        load_indirect(instruction, regs, memory);
        break;
      case OP_LDR:
        load_register(instruction, regs, memory);
        break;
      case OP_LEA:
        load_effective_address(instruction, regs);
        break;
      case OP_NOT:
        bitwise_not(instruction, regs);
        break;
      case OP_ST:
        store(instruction, regs, memory);
        break;
      case OP_STI:
        store_indirect(instruction, regs, memory);
        break;
      case OP_STR:
        store_register(instruction, regs, memory);
        break;
      case OP_TRAP:
        trap(instruction, regs, memory, &run);
        break;
      case OP_RES:
      case OP_RTI:
      default:
        fprintf(stderr, "unsupported opcode %x\n", op);
        break;
    }
  }

  // shutdown
  clean_up();
  restore_input_buffering();
  return EXIT_SUCCESS;
}
