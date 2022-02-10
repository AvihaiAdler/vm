#ifndef VM_H_
#define VM_H_
#include <stdint.h>

#ifdef _WIN32
extern HANDLE hStdin;
#endif

enum registers {
  R_R0,
  R_R1,
  R_R2,
  R_R3,
  R_R4,
  R_R5,
  R_R6,
  R_R7,
  R_PC,
  R_COND,
  R_COUNT
};

enum special_registers {
  MR_KBSR = 0xfe00, //keyboard status (whether a key get pressed)
  MR_KBDR = 0xfe02  //keyboard data (which key was pressed)
};

enum opcodes {
  OP_BR,  // branch
  OP_ADD,
  OP_LD,   // load
  OP_ST,   // store
  OP_JSR,  // jump register
  OP_AND,
  OP_LDR,  // load register
  OP_STR,  // store register
  OP_RTI,  // unused
  OP_NOT,
  OP_LDI,  // load indirect
  OP_STI,  // store indirect
  OP_JMP,
  OP_RES,  // reserved (unused)
  OP_LEA,  // load affective address
  OP_TRAP  // execute trap
};

enum flags { 
  FL_POS = 0x1, // P
  FL_ZRO = 0x2, // Z
  FL_NEG = 0x4  // N
};

enum traps {
  // get a char from the keyboard, not echo'd onto the terminal
  TRAP_GETC = 0x20,    
  TRAP_OUT,    // output a char
  TRAP_PUTS,   // output a word string
  TRAP_IN,     // get a char from the keyboard, echo'd onto the terminal
  TRAP_PUTSP,  // output a byte string
  TRAP_HALT    // halt the program
};

#endif
