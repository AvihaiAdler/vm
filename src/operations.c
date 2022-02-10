#include "../include/operations.h"

#include <stdio.h>

#include "../include/vm.h"

#ifdef __linux__
#include <sys/select.h>
#include <unistd.h>
#elif _WIN32
#include <Windows.h>
#include <conio.h>
#endif

void logger(const char* operation, uint16_t* const regs) {
  printf("\n----------\n");
  printf("op: %s:\n", operation);
  for(uint16_t i = 0; i < R_COUNT; i++) {
    const char* reg;
    switch (i) {
    case 0:
    reg = "R_R0";
      break;
    case 1:
      reg = "R_R1";
      break;
    case 2:
      reg = "R_R2";
      break;
    case 3:
      reg = "R_R3";
      break;
    case 4:
      reg = "R_R4";
      break;
    case 5:
      reg = "R_R5";
      break;
    case 6:
      reg = "R_R6";
      break;
    case 7:
      reg = "R_R7";
      break;
    case 8:
      reg = "R_PC";
      break;
    case 9:
      reg = "R_COND";
      break;
    }
    printf("register: %s : %x\n", reg, regs[i]);
  }
  printf("\n----------\n");
}

uint16_t check_key(void) {
  #ifdef __linux__
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(STDIN_FILENO, &readfds);

  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 0;
  return select(1, &readfds, NULL, NULL, &timeout) != 0;
  #elif _WIN32
    return WaitForSingleObject(hStdin, 1000) == WAIT_OBJECT_0 && _kbhit();
  #endif
}

uint16_t get_register(uint16_t instruction, int offset) {
  return (uint16_t)((instruction >> offset) & 0x7);
}

/*
  extend a 5 bit number into a int16_t (2's complement) number
*/
uint16_t sign_extend(uint16_t num, int bit_count) {
  if ((num >> (bit_count - 1)) & 1) {  // if num is negative (MSB is 1)
    // fill in all bits higher than bit_count with 1's
    num |= (uint16_t)(0xffff << bit_count);
  }
  return num;
}

void update_flags(uint16_t reg, uint16_t* const regs) {
  if (reg == 0) {
    regs[R_COND] = FL_ZRO;
  } else {
    if (reg >> 15)  // negative
      regs[R_COND] = FL_NEG;
    else  // positive
      regs[R_COND] = FL_POS;
  }
}

void add(uint16_t instruction, uint16_t* const regs) {
  uint16_t r0 = get_register(instruction, 9);  // get the DR register
  uint16_t r1 = get_register(instruction, 6);  // get the SR1 register

  // whether we're in immediate mode or not
  uint8_t mode = (instruction >> 5) & 1;

  if (mode) {  // immediate mode
    uint16_t imm = sign_extend(instruction & 0x1f, 5);
    regs[r0] = (uint16_t)(regs[r1] + imm);
  } else {
    uint16_t r2 = get_register(instruction, 0);
    regs[r0] = (uint16_t)(regs[r1] + regs[r2]);
  }
  update_flags(regs[r0], regs);

  if (DEBUG) logger("add", regs);
}

void bitwise_and(uint16_t instruction, uint16_t* const regs) {
  uint16_t r0 = get_register(instruction, 9);  // get the DR register
  uint16_t r1 = get_register(instruction, 6);  // get the SR1 register

  // whether we're in immediate mode or not
  uint8_t mode = (instruction >> 5) & 1;

  if (mode) {  // immediate
    uint16_t imm = sign_extend(instruction & 0x1f, 5);
    regs[r0] = regs[r1] & imm;
  } else {
    uint16_t r2 = get_register(instruction, 0);
    regs[r0] = regs[r1] & regs[r2];
  }

  update_flags(regs[r0], regs);

  if (DEBUG) logger("and", regs);
}

void branch(uint16_t instruction, uint16_t* const regs) {
  uint16_t offset = sign_extend(instruction & 0x1ff, 9);
  uint8_t flag = (instruction >> 9) & 0x7;

  if (flag & regs[R_COND]) {
    regs[R_PC] = (uint16_t) (regs[R_PC] + offset);
  }

  if (DEBUG) logger("br", regs);
}

void jump(uint16_t insturction, uint16_t* const regs) {
  uint16_t r0 = get_register(insturction, 6);
  regs[R_PC] = regs[r0];

  if (DEBUG) logger("jmp", regs);
}

void jump_register(uint16_t instruction, uint16_t* const regs) {
  regs[R_R7] = regs[R_PC];
  uint8_t mode = (instruction >> 11) & 1;

  if (mode) {  // JSR
    uint16_t offset = sign_extend(instruction & 0x7ff, 11);
    regs[R_PC] = (uint16_t)(regs[R_PC] + offset);
  } else {  // JSRR
    uint16_t base_r = get_register(instruction, 6);
    regs[R_PC] = regs[base_r];
  }

  if (DEBUG) logger("jsr", regs);
}

void load(uint16_t instruction, uint16_t* const regs, uint16_t* const memory) {
  uint16_t r0 = get_register(instruction, 9);
  uint16_t offset = sign_extend(instruction & 0x1ff, 9);

  regs[r0] = mem_read((uint16_t)(regs[R_PC] + offset), memory);

  update_flags(regs[r0], regs);

  if (DEBUG) logger("ld", regs);
}

void load_indirect(uint16_t instruction, uint16_t* const regs,
                   uint16_t* const memory) {
  uint16_t r0 = get_register(instruction, 9);  // DR register
  uint16_t offset = sign_extend(instruction & 0x1ff, 9);
  regs[r0] =
      mem_read(mem_read((uint16_t)(regs[R_PC] + offset), memory), memory);

  update_flags(regs[r0], regs);

  if (DEBUG) logger("ldi", regs);
}

void load_register(uint16_t instruction, uint16_t* const regs,
                   uint16_t* const memory) {
  uint16_t r0 = get_register(instruction, 9);
  uint16_t r1 = get_register(instruction, 6);
  uint16_t offset = sign_extend(instruction & 0x3f, 6);

  regs[r0] = mem_read((uint16_t)(regs[r1] + offset), memory);

  update_flags(regs[r0], regs);

  if (DEBUG) logger("ldr", regs);
}

void load_effective_address(uint16_t instruction, uint16_t* const regs) {
  uint16_t r0 = get_register(instruction, 9);
  uint16_t offset = sign_extend(instruction & 0x1ff, 9);

  regs[r0] = (uint16_t)(regs[R_PC] + offset);

  update_flags(regs[r0], regs);

  if (DEBUG) logger("lea", regs);
}

void bitwise_not(uint16_t instruction, uint16_t* const regs) {
  uint16_t r0 = get_register(instruction, 9);
  uint16_t r1 = get_register(instruction, 6);

  regs[r0] = (uint16_t)~regs[r1];

  update_flags(regs[r0], regs);

  if (DEBUG) logger("not", regs);
}

void store(uint16_t instruction, uint16_t* const regs, uint16_t* const memory) {
  uint16_t r0 = get_register(instruction, 9);
  uint16_t offset = sign_extend(instruction & 0x1ff, 9);

  mem_write((uint16_t)(regs[R_PC] + offset), memory, regs[r0]);

  if (DEBUG) logger("st", regs);
}

void store_indirect(uint16_t instruction, uint16_t* const regs,
                    uint16_t* const memory) {
  uint16_t r0 = get_register(instruction, 9);
  uint16_t offset = sign_extend(instruction & 0x1ff, 9);

  mem_write(mem_read((uint16_t)(regs[R_PC] + offset), memory), memory,
            regs[r0]);

  if (DEBUG) logger("sti", regs);
}

void store_register(uint16_t instruction, uint16_t* const regs,
                    uint16_t* const memory) {
  uint16_t r0 = get_register(instruction, 9);
  uint16_t r1 = get_register(instruction, 6);
  uint16_t offset = sign_extend(instruction & 0x3f, 6);

  mem_write((uint16_t)(regs[r1] + offset), memory, regs[r0]);

  if (DEBUG) logger("str", regs);
}

void trap(uint16_t instruction, uint16_t* const regs, uint16_t* const memory,
          int* run) {
  regs[R_R7] = regs[R_PC];
  regs[R_PC] = (uint16_t)(instruction & 0xff);

  switch (regs[R_PC]) {
    case TRAP_GETC:
      regs[R_R0] = (uint16_t)fgetc(stdin);
      break;
    case TRAP_OUT:
      fputc((char)regs[R_R0], stdout);
      fflush(stdout);
      break;
    case TRAP_PUTS:
      for (uint16_t* c = memory + regs[R_R0]; *c; c++) {
        fputc((char)*c, stdout);
      }
      fflush(stdout);
      break;
    case TRAP_IN:
      printf("enter a character: ");
      char in = (char)fgetc(stdin);
      fputc(in, stdout);
      fflush(stdout);
      regs[R_R0] = (uint16_t)in;
      break;
    case TRAP_PUTSP:
      for (uint16_t* c = memory + regs[R_R0]; *c; c++) {
        fputc((char)(*c) & 0xff, stdout);
        char second = (char)(*c >> 8);
        if (second) fputc(second, stdout);
      }
      fflush(stdout);
      break;
    case TRAP_HALT:
      printf("halt\n");
      *run = 0;
      break;
    default:
      fprintf(stderr, "unsupported trap vector %x\n", regs[R_PC]);
      break;
  }

  if (DEBUG) logger("trap", regs);
  regs[R_PC] = regs[R_R7];
}

void mem_write(uint16_t address, uint16_t* const memory, uint16_t data) {
  memory[address] = data;
}

uint16_t mem_read(uint16_t address, uint16_t* const memory) {
  if (address == MR_KBSR) {
    if (check_key()) {
      memory[MR_KBSR] = (1 << 15);
      memory[MR_KBDR] = (uint16_t)fgetc(stdin);
    } else {
      memory[MR_KBSR] = 0;
    }
  }
  return memory[address];
}
