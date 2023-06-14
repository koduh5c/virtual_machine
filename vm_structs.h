#ifndef VM_DATA_STRUCTURES_H
#define VM_DATA_STRUCTURES_H

#include <stdint.h>

typedef struct {
    uint8_t rd : 7;
    uint8_t func3 : 3;
    uint8_t rs1 : 5;
    uint8_t rs2 : 5;
    uint8_t func7 : 7;
} R_Type;

typedef struct {
    uint8_t rd : 7;
    uint8_t func3 : 3;
    uint8_t rs1 : 5;
    int32_t imm : 12;
} I_Type;

typedef struct {
    uint8_t func3 : 3;
    uint8_t rs1 : 5;
    uint8_t rs2 : 5;
    int32_t imm : 12;
} S_Type;

typedef struct {
    uint8_t func3 : 3;
    uint8_t rs1 : 5;
    uint8_t rs2 : 5;
    int32_t imm : 13;
} SB_Type;

typedef struct {
    uint8_t rd : 7;
    int32_t imm : 32;
} U_Type;

typedef struct {
    uint8_t rd : 7;
    int32_t imm : 21;
} UJ_Type;

#endif /* VM_DATA_STRUCTURES_H */
