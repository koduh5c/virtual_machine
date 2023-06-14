#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm_constants.h"
#include "vm_prototypes.h"
#include "vm_structs.h"

int32_t header_array[HEADER_ARRAY_SIZE];
uint8_t memory[MEM_SIZE];
uint32_t reg[REG_COUNT];
uint32_t pc = 0;

int main(int argc, char const *argv[]) {
    memset(header_array, -1, sizeof(header_array));
    memset(memory, 0, sizeof(memory));
    memset(reg, 0, sizeof(reg));
    pc = 0;

    FILE *instr_file = fopen(argv[1], "rb");
    if (instr_file == NULL) {
        printf("Error opening file\n");
        exit(1);
    }
    fread(memory, sizeof(uint8_t), INSTR_MEM_SIZE+DATA_MEM_SIZE, instr_file);
    fclose(instr_file);

    uint32_t instr;
    while ((instr = get_val(INSTR_MEM_START + pc, 32))) {
        parse_instruction(instr);
        pc += 4;
    }
}

R_Type parse_r(uint32_t instr) {
    R_Type res;
    res.rd = (instr >> 7) & mask(5);
    res.func3 = (instr >> 12) & mask(3);
    res.rs1 = (instr >> 15) & mask(5);
    res.rs2 = (instr >> 20) & mask(5);
    res.func7 = (instr >> 25) & mask(7);
    return res;
}

I_Type parse_i(uint32_t instr) {
    I_Type res;
    res.rd = (instr >> 7) & mask(5);
    res.func3 = (instr >> 12) & mask(3);
    res.rs1 = (instr >> 15) & mask(5);
    res.imm = (instr >> 20) & mask(12);
    res.imm = sext(res.imm, 12);
    return res;
}

S_Type parse_s(uint32_t instr) {
    S_Type res;
    res.func3 = (instr >> 12) & mask(3);
    res.rs1 = (instr >> 15) & mask(5);
    res.rs2 = (instr >> 20) & mask(5);
    res.imm = (instr >> 25 & mask(7)) << 5;
    res.imm |= instr >> 7 & mask(5);
    res.imm = sext(res.imm, 12);
    return res;
}

SB_Type parse_sb(uint32_t instr) {
    SB_Type res;
    res.func3 = (instr >> 12) & mask(3);
    res.rs1 = (instr >> 15) & mask(5);
    res.rs2 = (instr >> 20) & mask(5);

    uint32_t imm12 = (instr >> 31) & mask(1);
    uint32_t imm11 = (instr >> 7) & mask(1);
    uint32_t imm10_5 = (instr >> 25) & mask(6);
    uint32_t imm4_1 = (instr >> 8) & mask(4);

    res.imm = (imm12 << 11) | (imm11 << 10) | (imm10_5 << 4) | imm4_1;
    res.imm = sext(res.imm, 12);
    return res;
}

U_Type parse_u(uint32_t instr) {
    U_Type res;
    res.rd = (instr >> 7) & mask(5);
    res.imm = (instr >> 12) & mask(20);
    res.imm = sext(res.imm, 20);
    return res;
}

UJ_Type parse_uj(uint32_t instr) {
    UJ_Type res;
    res.rd = (instr >> 7) & mask(5);

    uint32_t imm20 = (instr >> 31) & mask(1);
    uint32_t imm10_1 = (instr >> 21) & mask(10);
    uint32_t imm11 = (instr >> 20) & mask(1);
    uint32_t imm19_12 = (instr >> 12) & mask(8);

    res.imm = (imm20 << 19) | (imm19_12 << 11) | (imm11 << 10) | imm10_1;
    res.imm = sext(res.imm, 20);
    return res;
}

int32_t mask(int n) {
    return (1 << n) - 1;
}

int32_t sext(int32_t value, uint32_t of_bitwidth) {
    int const m = 1U << (of_bitwidth - 1);
    return (value ^ m) - m;
}

int is_instr(uint32_t address, uint8_t bitwidth) {
    return INSTR_MEM_START <= address && address <= INSTR_MEM_END - bitwidth / 8;
}

int is_data(uint32_t address, uint8_t bitwidth) {
    return DATA_MEM_START <= address && address <= DATA_MEM_END - bitwidth / 8;
}

int is_virt(uint32_t address, uint8_t bitwidth) {
    return VIRT_ROUT_START <= address && address <= VIRT_ROUT_END - bitwidth / 8;
}

int is_heap(uint32_t address, uint8_t bitwidth) {
    return HEAP_BANK_START <= address && address <= HEAP_BANK_END - bitwidth / 8;
}

int32_t check_virtual_routine(uint32_t address, int32_t value) {
    if (address == 0x0800) {
        console_write_character(value);
    } else if (address == 0x0804) {
        console_write_signed_integer(value);
    } else if (address == 0x0808) {
        console_write_unsigned_integer(value);
    } else if (address == 0x080C) {
        halt();
    } else if (address == 0x0812) {
        return console_read_character();
    } else if (address == 0x0816) {
        return console_read_signed_integer();
    } else if (address == 0x0820) {
        dump_pc();
    } else if (address == 0x0824) {
        dump_register_banks();
    } else if (address == 0x0828) {
        dump_memory_word(value);
    } else if (address == 0x0830) {
        vmalloc(value);
    } else if (address == 0x0834) {
        return vfree(value);
    }
    return INT_MIN;
}

uint32_t get_val(uint32_t address, uint8_t bitwidth) {
    if (is_virt(address, bitwidth)) {
        return check_virtual_routine(address, 0);
    }
    if (is_instr(address, bitwidth) || is_data(address, bitwidth) || is_heap(address, bitwidth)) {
        uint32_t res = 0;
        for (int i = 0; i < bitwidth / 8; i++) {
            res |= memory[address + i] << (i * 8);
        }
        return res;
    }
    return 0;
}

int set_val(uint32_t address, int32_t value, uint8_t bitwidth) {
    if (is_virt(address, bitwidth)) {
        check_virtual_routine(address, value);
        return 1;
    }
    if (is_data(address, bitwidth) || is_heap(address, bitwidth)) {
        for (int i = 0; i < bitwidth / 8; i++) {
            memory[address + i] = value >> (i * 8);
        }
        return 1;
    }
    return 0;
}

int set_reg(uint8_t index, int32_t value) {
    if (0 < index && index < REG_COUNT) {
        reg[index] = value;
        return 1;
    }
    return 0;
}

void add(R_Type instr) {
    set_reg(instr.rd, reg[instr.rs1] + reg[instr.rs2]);
}

void addi(I_Type instr) {
    set_reg(instr.rd, reg[instr.rs1] + instr.imm);
}

void sub(R_Type instr) {
    set_reg(instr.rd, reg[instr.rs1] - reg[instr.rs2]);
}

void lui(U_Type instr) {
    set_reg(instr.rd, instr.imm << 12);
}

void xor (R_Type instr) {
    set_reg(instr.rd, reg[instr.rs1] ^ reg[instr.rs2]);
}

    void xori(I_Type instr) {
    set_reg(instr.rd, reg[instr.rs1] ^ instr.imm);
}

void or (R_Type instr) {
    set_reg(instr.rd, reg[instr.rs1] | reg[instr.rs2]);
}

void ori(I_Type instr) {
    set_reg(instr.rd, reg[instr.rs1] | instr.imm);
}

void and (R_Type instr) {
    set_reg(instr.rd, reg[instr.rs1] & reg[instr.rs2]);
}

void andi(I_Type instr) {
    set_reg(instr.rd, reg[instr.rs1] & instr.imm);
}

void sll(R_Type instr) {
    set_reg(instr.rd, reg[instr.rs1] << reg[instr.rs2]);
}

void srl(R_Type instr) {
    set_reg(instr.rd, reg[instr.rs1] >> reg[instr.rs2]);
}

void sra(R_Type instr) {
    srl(instr);
    set_reg(instr.rd, (reg[instr.rd] & mask(1)) << 31 | reg[instr.rd] >> 1);
}

int lb(I_Type instr) {
    uint32_t val = get_val(reg[instr.rs1] + instr.imm, 8);
    if (val == INT_MIN) {
        return 0;
    }
    return set_reg(instr.rd, sext(val, 8));
}

int lh(I_Type instr) {
    uint32_t val = get_val(reg[instr.rs1] + instr.imm, 16);
    if (val == INT_MIN) {
        return 0;
    }
    return set_reg(instr.rd, sext(val, 16));
}

int lw(I_Type instr) {
    uint32_t val = get_val(reg[instr.rs1] + instr.imm, 32);
    if (val == INT_MIN) {
        return 0;
    }
    return set_reg(instr.rd, val);
}

int lbu(I_Type instr) {
    uint32_t val = get_val(reg[instr.rs1] + instr.imm, 8);
    if (val == INT_MIN) {
        return 0;
    }
    return set_reg(instr.rd, val);
}

int lhu(I_Type instr) {
    uint32_t val = get_val(reg[instr.rs1] + instr.imm, 16);
    if (val == INT_MIN) {
        return 0;
    }
    return set_reg(instr.rd, val);
}

int sb(S_Type instr) {
    return set_val(reg[instr.rs1] + instr.imm, sext(reg[instr.rs2], 8), 8);
}

int sh(S_Type instr) {
    return set_val(reg[instr.rs1] + instr.imm, sext(reg[instr.rs2], 16), 16);
}

int sw(S_Type instr) {
    return set_val(reg[instr.rs1] + instr.imm, sext(reg[instr.rs2], 32), 32);
}

void slt(R_Type instr) {
    set_reg(instr.rd, (reg[instr.rs1] < reg[instr.rs2]) ? 1 : 0);
}

void slti(I_Type instr) {
    set_reg(instr.rd, (reg[instr.rs1] < instr.imm) ? 1 : 0);
}

void sltu(R_Type instr) {
    set_reg(instr.rd, ((uint32_t)reg[instr.rs1] < (uint32_t)reg[instr.rs2]) ? 1 : 0);
}

void sltiu(I_Type instr) {
    set_reg(instr.rd, ((uint32_t)reg[instr.rs1] < (uint32_t)instr.imm) ? 1 : 0);
}

void beq(SB_Type instr) {
    if (reg[instr.rs1] == reg[instr.rs2]) {
        pc += (instr.imm << 1) - 4;
    }
}

void bne(SB_Type instr) {
    if (reg[instr.rs1] != reg[instr.rs2]) {
        pc += (instr.imm << 1) - 4;
    }
}

void blt(SB_Type instr) {
    if (reg[instr.rs1] < reg[instr.rs2]) {
        pc += (instr.imm << 1) - 4;
    }
}

void bltu(SB_Type instr) {
    if ((uint32_t)reg[instr.rs1] < (uint32_t)reg[instr.rs2]) {
        pc += (instr.imm << 1) - 4;
    }
}

void bge(SB_Type instr) {
    if (reg[instr.rs1] >= reg[instr.rs2]) {
        pc += (instr.imm << 1) - 4;
    }
}

void bgeu(SB_Type instr) {
    if ((uint32_t)reg[instr.rs1] >= (uint32_t)reg[instr.rs2]) {
        pc += (instr.imm << 1) - 4;
    }
}

void jal(UJ_Type instr) {
    set_reg(instr.rd, pc + 4);
    pc += (instr.imm << 1) - 4;
}

void jalr(I_Type instr) {
    set_reg(instr.rd, pc + 4);
    pc = reg[instr.rs1] + instr.imm - 4;
}

void console_write_character(int32_t value) {
    printf("%c", value);
}

void console_write_signed_integer(int32_t value) {
    printf("%d", value);
}

void console_write_unsigned_integer(int32_t value) {
    printf("%x", (uint32_t)value);
}

void halt() {
    printf("CPU Halt Requested\n");
    exit(0);
}

char console_read_character() {
    char input;
    scanf("%c", &input);
    return input;
}

int32_t console_read_signed_integer() {
    int32_t input;
    scanf("%d", &input);
    return input;
}

void dump_pc() {
    printf("%x", pc);
}

void dump_register_banks() {
    printf("PC = 0x%08x;\n", pc);
    for (int i = 0; i < REG_COUNT; i++) {
        printf("R[%d] = 0x%08x;\n", i, reg[i]);
    }
}

void dump_memory_word(int32_t value) {
    printf("%x", memory[value]);
}

void vmalloc(uint32_t size) {
    int32_t header_index;
    for (int i = HEAP_BANK_START; i < HEAP_BANK_END; i += HEAP_BANK_SIZE) {
        header_index = i;
        int found_space = 1;
        for (int j = i; j < i + size; j += HEAP_BANK_SIZE) {
            if (header_array[j / HEAP_BANK_SIZE]) {
                found_space = 0;
                break;
            }
        }
        if (found_space) {
            for (int j = i; j < i + size; j += HEAP_BANK_SIZE) {
                header_array[j / HEAP_BANK_SIZE] = header_index;
            }
            reg[28] = header_index;
            return;
        }
    }
    reg[28] = 0;
}

int vfree(uint32_t address) {
    int32_t header_index = (address - HEAP_BANK_START) / HEAP_BANK_SIZE;
    if (header_array[header_index] == -1) {
        return 0;
    }
    for (int i = 0; i < (HEAP_BANK_END - HEAP_BANK_START) / HEAP_BANK_SIZE; i++) {
        if (header_array[i] == header_index) {
            for (int j = 0; j < HEAP_BANK_SIZE; j++) {
                memory[HEAP_BANK_START + i + j] = 0;
            }
            header_array[i] = -1;
        }
    }
    return 1;
}

void parse_instruction(uint32_t instr) {
    uint32_t opcode = instr & mask(7);
    int no_error = 1;
    if (opcode == 0b0010011) {
        I_Type parsed_instr = parse_i(instr);
        if (parsed_instr.func3 == 0b000) {
            addi(parsed_instr);
        } else if (parsed_instr.func3 == 0b100) {
            xori(parsed_instr);
        } else if (parsed_instr.func3 == 0b110) {
            ori(parsed_instr);
        } else if (parsed_instr.func3 == 0b111) {
            andi(parsed_instr);
        } else if (parsed_instr.func3 == 0b010) {
            slti(parsed_instr);
        } else if (parsed_instr.func3 == 0b011) {
            sltiu(parsed_instr);
        } else {
            no_error = 0;
        }
    } else if (opcode == 0b0000011) {
        I_Type parsed_instr = parse_i(instr);
        if (parsed_instr.func3 == 0b000) {
            no_error = lb(parsed_instr);
        } else if (parsed_instr.func3 == 0b001) {
            no_error = lh(parsed_instr);
        } else if (parsed_instr.func3 == 0b010) {
            no_error = lw(parsed_instr);
        } else if (parsed_instr.func3 == 0b100) {
            no_error = lbu(parsed_instr);
        } else if (parsed_instr.func3 == 0b101) {
            no_error = lhu(parsed_instr);
        } else {
            no_error = 0;
        }
    } else if (opcode == 0b1100111) {
        I_Type parsed_instr = parse_i(instr);
        if (parsed_instr.func3 == 0b000) {
            jalr(parsed_instr);
        } else {
            no_error = 0;
        }
    } else if (opcode == 0b0110011) {
        R_Type parsed_instr = parse_r(instr);
        if (parsed_instr.func3 == 0b000 && parsed_instr.func7 == 0b0000000) {
            add(parsed_instr);
        } else if (parsed_instr.func3 == 0b000 && parsed_instr.func7 == 0b0100000) {
            sub(parsed_instr);
        } else if (parsed_instr.func3 == 0b100 && parsed_instr.func7 == 0b0000000) {
            xor(parsed_instr);
        } else if (parsed_instr.func3 == 0b110 && parsed_instr.func7 == 0b0000000) {
            or (parsed_instr);
        } else if (parsed_instr.func3 == 0b111 && parsed_instr.func7 == 0b0000000) {
            and(parsed_instr);
        } else if (parsed_instr.func3 == 0b001 && parsed_instr.func7 == 0b0000000) {
            sll(parsed_instr);
        } else if (parsed_instr.func3 == 0b101 && parsed_instr.func7 == 0b0000000) {
            srl(parsed_instr);
        } else if (parsed_instr.func3 == 0b101 && parsed_instr.func7 == 0b0100000) {
            sra(parsed_instr);
        } else if (parsed_instr.func3 == 0b010 && parsed_instr.func7 == 0b0000000) {
            slt(parsed_instr);
        } else if (parsed_instr.func3 == 0b011 && parsed_instr.func7 == 0b0000000) {
            sltu(parsed_instr);
        } else {
            no_error = 0;
        }
    } else if (opcode == 0b0100011) {
        S_Type parsed_instr = parse_s(instr);
        if (parsed_instr.func3 == 0b000) {
            no_error = sb(parsed_instr);
        } else if (parsed_instr.func3 == 0b001) {
            no_error = sh(parsed_instr);
        } else if (parsed_instr.func3 == 0b010) {
            no_error = sw(parsed_instr);
        } else {
            no_error = 0;
        }
    } else if (opcode == 0b1100011) {
        SB_Type parsed_instr = parse_sb(instr);
        if (parsed_instr.func3 == 0b000) {
            beq(parsed_instr);
        } else if (parsed_instr.func3 == 0b001) {
            bne(parsed_instr);
        } else if (parsed_instr.func3 == 0b100) {
            blt(parsed_instr);
        } else if (parsed_instr.func3 == 0b110) {
            bltu(parsed_instr);
        } else if (parsed_instr.func3 == 0b101) {
            bge(parsed_instr);
        } else if (parsed_instr.func3 == 0b111) {
            bgeu(parsed_instr);
        } else {
            no_error = 0;
        }
    } else if (opcode == 0b0110111) {
        U_Type parsed_instr = parse_u(instr);
        lui(parsed_instr);
    } else if (opcode == 0b1101111) {
        UJ_Type parsed_instr = parse_uj(instr);
        jal(parsed_instr);
    } else {
        no_error = 0;
    }
    if (!no_error) {
        printf("Instruction Not Implemented: 0x%08x\n", instr);
        dump_register_banks();
        exit(1);
    }
}
