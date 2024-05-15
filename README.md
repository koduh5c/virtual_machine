# Virtual Machine Project

This project is a virtual machine implementation written in C. It emulates a simple RISC-V architecture, executing instructions loaded from a binary file.

## Features

- Supports basic RISC-V instructions.
- Implements various instruction types including R, I, S, SB, U, and UJ.
- Provides virtual memory management.
- Includes console I/O functionalities.
- Supports dynamic memory allocation.

## Getting Started

### Prerequisites

- C compiler (e.g., GCC)
- Operating system with a C development environment

### Installation

1. Clone the repository:

2. Compile the code:


### Usage

Execute the virtual machine with an instruction file as an argument:

./vm_executable instruction_file.bin


## Instructions

The virtual machine supports various RISC-V instructions, including arithmetic, logical, memory, and control flow instructions.

### Supported Instructions

- Arithmetic: ADD, SUB, XOR, OR, AND
- Logical Shift: SLL, SRL, SRA
- Comparison: SLT, SLTU
- Immediate Operations: ADDI, XORI, ORI, ANDI, SLTI, SLTIU
- Memory Access: LB, LH, LW, LBU, LHU, SB, SH, SW
- Control Flow: BEQ, BNE, BLT, BLTU, BGE, BGEU, JAL, JALR
- Others: LUI
