# VM - 16-bit CPU Simulator

A minimal 16-bit CPU simulator written in C for educational purposes. This project implements a simple fetch-decode-execute pipeline with a custom ISA.

## Instruction Set Architecture

The CPU has 8 general-purpose registers (R0-R7), a 16-bit program counter (PC), a 16-bit instruction register (IR), and 64KB of byte-addressable memory. The first 16KB are reserved for the text segment (instructions), and the remainder for data.

### Instructions

| Opcode | Mnemonic | Description                                      |
|--------|----------|--------------------------------------------------|
| 0x0    | HALT     | Stop execution (sets ON to 0)                    |
| 0x1    | ADDI     | Add immediate: `RT = RS + IMM`                   |
| 0x2    | SW       | Store word: `MEM[RS + IMM] = RT` (big-endian)    |
| 0x3    | LW       | Load word: `RT = MEM[RS + IMM]` (big-endian)     |

### Instruction Format (16 bits)

```
15 14 13 12 | 11 10 9 | 8 7 6 | 5 4 3 2 1 0
   opcode   |   RS    |  RT   |    IMM
```

- **opcode** (4 bits) - instruction type
- **RS** (3 bits) - source register
- **RT** (3 bits) - destination / second source register
- **IMM** (6 bits) - immediate value (0-63)

## Building

```sh
make        # build the simulator
make run    # build and run with the test program
make clean  # remove the binary
```

## Pipeline

The CPU executes instructions in a 3-stage pipeline:

1. **Fetch** - reads a 16-bit instruction from `MEM[PC]` into IR, increments PC by 2
2. **Decode** - extracts opcode, RS, RT, and IMM from the instruction
3. **Execute** - performs the operation based on the opcode

R0 is hardwired to 0 and is reset after every instruction.

## License

MIT
