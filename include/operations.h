#ifndef OPERATIONS_H_
#define OPERATIONS_H_
#define DEBUG 0
#include <stdint.h>

void logger(const char* operation, uint16_t* const regs);

uint16_t sign_extend(uint16_t num, int bit_count);

void update_flags(uint16_t reg, uint16_t* const regs);

void add(uint16_t instruction, uint16_t* const regs);

void bitwise_and(uint16_t instruction, uint16_t* const regs);

void branch(uint16_t instruction, uint16_t* const regs);

void jump(uint16_t insturction, uint16_t* const regs);

void jump_register(uint16_t instruction, uint16_t* const regs);

void load(uint16_t instruction, uint16_t* const regs, uint16_t* const memory);

void load_indirect(uint16_t instruction, uint16_t* const regs,
                   uint16_t* const memory);

void load_register(uint16_t instruction, uint16_t* const regs,
                   uint16_t* const memory);

void load_effective_address(uint16_t instruction, uint16_t* const regs);

void bitwise_not(uint16_t instruction, uint16_t* const regs);

void store(uint16_t instruction, uint16_t* const regs, uint16_t* const memory);

void store_indirect(uint16_t instruction, uint16_t* const regs,
                    uint16_t* const memory);

void store_register(uint16_t instruction, uint16_t* const regs,
                    uint16_t* const memory);

void trap(uint16_t instruction, uint16_t* const regs, uint16_t* const memory,
          int* run);

void mem_write(uint16_t address, uint16_t* const memory, uint16_t data);

uint16_t mem_read(uint16_t address, uint16_t* const memory);

#endif
