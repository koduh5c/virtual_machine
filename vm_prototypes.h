#ifndef VM_PROTOTYPES_H
#define VM_PROTOTYPES_H

#include <stdint.h>

// Constants
#include "vm_constants.h"

// Data structures
#include "vm_structs.h"

// Utility functions
int32_t mask(int n);
int32_t sext(int32_t value, uint32_t of_bitwidth);
uint32_t get_val(uint32_t address, uint8_t bitwidth);
int set_val(uint32_t address, int32_t value, uint8_t bitwidth);
int set_reg(uint8_t index, int32_t value);
int is_instr(uint32_t address, uint8_t bitwidth);
int is_data(uint32_t address, uint8_t bitwidth);
int is_virt(uint32_t address, uint8_t bitwidth);
int is_heap(uint32_t address, uint8_t bitwidth);

// Instruction functions
void add(R_Type instr);
void addi(I_Type instr);
void sub(R_Type instr);
void lui(U_Type instr);
void xor (R_Type instr);
void xori(I_Type instr);
void or (R_Type instr);
void ori(I_Type instr);
void and (R_Type instr);
void andi(I_Type instr);
void sll(R_Type instr);
void srl(R_Type instr);
void sra(R_Type instr);
int lb(I_Type instr);
int lh(I_Type instr);
int lw(I_Type instr);
int lbu(I_Type instr);
int lhu(I_Type instr);
int sb(S_Type instr);
int sh(S_Type instr);
int sw(S_Type instr);
void slt(R_Type instr);
void slti(I_Type instr);
void sltu(R_Type instr);
void sltiu(I_Type instr);
void beq(SB_Type instr);
void bne(SB_Type instr);
void blt(SB_Type instr);
void bltu(SB_Type instr);
void bge(SB_Type instr);
void bgeu(SB_Type instr);
void jal(UJ_Type instr);
void jalr(I_Type instr);

// Virtual routines
void console_write_character(int32_t value);
void console_write_signed_integer(int32_t value);
void console_write_unsigned_integer(int32_t value);
void halt();
char console_read_character();
int32_t console_read_signed_integer();
void vmalloc(uint32_t size);
int vfree(uint32_t address);
void dump_pc();
void dump_register_banks();
void dump_memory_word(int32_t value);
int32_t check_virtual_routine(uint32_t address, int32_t value);

// Instruction parsing functions
R_Type parse_r(uint32_t instr);
I_Type parse_i(uint32_t instr);
S_Type parse_s(uint32_t instr);
SB_Type parse_sb(uint32_t instr);
U_Type parse_u(uint32_t instr);
UJ_Type parse_uj(uint32_t instr);
void parse_instruction(uint32_t instr);

#endif
