/*
 The architecture:
 	This is a simple 16 bit CPU simulator, that has 8 register (R0-R7), 16-bit instruction register (IR), 16-bit program counter,
	and 64KB memory.

	|| INSTRUCTIONS ||

	The ISA will initially consist of 4 simple instructions:
	ADDI (Add Immediate)
	LW (Load Word) 
	SW (Store Word)
	HALT (Halt)

	|| ISA ARCHITECTURE ||
	
	We have 2 different instruction type:
	R-Type:
	
	... 
	 

	|| OPCODES ||

	HALT = 0x0
	ADDI = 0x1
	SW = 0x2
	LW = 0x3
	

	NOTE: in the current implementation, IMM is only 6 bits, meaning using raw addi sw or lw you can
	only add numbers up to 63, ori can solve this problem which will be implemented in a later version


 */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

enum { MEM_SIZE = 65536, SIZE_TEXT = 16384 };


typedef struct {
	uint8_t opcode;
	uint8_t RT;
	uint8_t RS;
	uint8_t IMM;
} I_INSTR;
	// NOTE: in the current implementation, IMM is only 6 bits, meaning using raw addi sw or lw you can
			// only add numbers up to 63, ori can solve this problem which will be implemented in a later version


typedef struct {
	uint16_t REG[8];
	uint16_t PC;
	uint16_t IR; // Instruction register
	uint8_t ON; // is the cpu running?
	

	I_INSTR curr_instr;
	
	uint8_t MEM[MEM_SIZE]; // Text memory
} CPU;


typedef enum {
	OP_HALT = 0x0,
	OP_ADDI = 0x1,
	OP_SW = 0x2,
	OP_LW = 0x3
} OPCODES;

typedef enum {
	RG0 = 0x0,
	RG1 = 0x1,
	RG2 = 0x2,
	RG3 = 0x3,
	RG4 = 0x4,
	RG5 = 0x5,
	RG6 = 0x6,
	RG7 = 0x7,
} REGS;

void fetch(CPU *cpu) {
	uint16_t instr = (cpu->MEM[cpu->PC] << 8) | cpu->MEM[cpu->PC + 1];

	cpu->PC += 2;

	cpu->IR = instr;
}

uint8_t extract_bits(uint16_t val,  uint8_t x, uint8_t y) {
	uint8_t width = (x - y) + 1;
	uint16_t mask = (1 << width) - 1;

	return (val >> y) & mask;
}

void decode(CPU *cpu) {
	const uint16_t raw_instr = cpu->IR;
	I_INSTR *curr_instr = &cpu->curr_instr;

	uint8_t opcode = (raw_instr >> 12) & 0x0F;
	uint8_t RS = extract_bits(raw_instr, 0xB, 0x9);
	uint8_t RT = extract_bits(raw_instr , 0x8, 0x6);
	uint8_t IMM = extract_bits(raw_instr, 0x5, 0x0);
	
	curr_instr->opcode = opcode;
	curr_instr->RS = RS;
	curr_instr->RT = RT;
	curr_instr->IMM = IMM;
}

void execute(CPU *cpu) {
	I_INSTR *curr_instr = &cpu->curr_instr;

	uint8_t curr_opcode = curr_instr->opcode;

	switch (curr_opcode) {
		case OP_ADDI: {
			// NOTE: in the current implementation, IMM is only 6 bits, meaning using raw addi sw or lw you can
			// only add numbers up to 63, ori can solve this problem which will be implemented in a later version
			uint8_t rt = curr_instr->RT;
			uint8_t rs = curr_instr->RS;
			uint8_t imm = curr_instr->IMM;
			cpu->REG[rt] = cpu->REG[rs] + imm;
			break;
		}
		case OP_SW: {
			uint8_t rt = curr_instr->RT;
			uint8_t rs = curr_instr->RS;
			uint8_t imm = curr_instr->IMM;
			
			uint16_t new_addr = cpu->REG[rs] + imm;
			uint16_t val = cpu->REG[rt];

			cpu->MEM[new_addr] = (val >> 8) & 0xFF;

			cpu->MEM[new_addr + 1] = val & 0xFF;
			break;
		}
		case OP_LW: {
			uint8_t rt = curr_instr->RT;
			uint8_t rs = curr_instr->RS;
			uint8_t imm = curr_instr->IMM;

			uint16_t addr = cpu->REG[rs] + imm;

			uint16_t val = (cpu->MEM[addr] << 8) | (cpu->MEM[addr + 1] & 0xFF);
			
			cpu->REG[rt] = val;
			break;
		}
		case OP_HALT: {
			cpu->ON = 0x00;
			break;
		}
	}
	cpu->REG[0] = 0x00;
}



void load_program(const uint8_t *program, uint8_t *memory, size_t size) {
    if (program == NULL || memory == NULL || size == 0) {
        return; // Safety guard rails
    }
    memcpy(memory, program, size);
}


static int all_passed;

static void dump_cpu_state(CPU *cpu, const char *name, int step,
                    uint16_t exp_regs[8], uint16_t exp_pc, uint8_t exp_on,
                    int check_mem, uint16_t exp_mem_addr, uint16_t exp_mem_val)
{
	printf("========================================================================\n");
	printf("  STEP %d: %s\n", step, name);
	printf("========================================================================\n");
	printf("  Field        | Expected     | Actual       | Status\n");
	printf("  -------------+--------------+--------------+--------\n");

	for (int i = 0; i < 8; i++) {
		if (exp_regs[i] != 0 || cpu->REG[i] != 0) {
			const char *status = (cpu->REG[i] == exp_regs[i]) ? "PASS" : "FAIL";
			if (cpu->REG[i] != exp_regs[i]) all_passed = 0;
			printf("  R[%d]         |   0x%04X      |   0x%04X      | %s\n",
			       i, exp_regs[i], cpu->REG[i], status);
		}
	}
	printf("  PC            |   0x%04X      |   0x%04X      | %s\n",
	       exp_pc, cpu->PC, cpu->PC == exp_pc ? "PASS" : "FAIL");
	if (cpu->PC != exp_pc) all_passed = 0;

	printf("  ON            |   0x%02X        |   0x%02X        | %s\n",
	       exp_on, cpu->ON, cpu->ON == exp_on ? "PASS" : "FAIL");
	if (cpu->ON != exp_on) all_passed = 0;

	if (check_mem) {
		uint16_t actual = (cpu->MEM[exp_mem_addr] << 8) | cpu->MEM[exp_mem_addr + 1];
		printf("  MEM[0x%04X]   |   0x%04X      |   0x%04X      | %s\n",
		       exp_mem_addr, exp_mem_val, actual,
		       actual == exp_mem_val ? "PASS" : "FAIL");
		if (actual != exp_mem_val) all_passed = 0;
	}

	printf("\n");
}

int main(void)
{
	CPU my_cpu;
	memset(&my_cpu, 0, sizeof(my_cpu));

	my_cpu.ON = 0x01;
	all_passed = 1;

	uint8_t program[] = {
		0x10, 0x60,  // ADDI R1, R0, 0x20        R1 = 0 + 32 = 0x0020
		0x10, 0x8F,  // ADDI R2, R0, 0x0F        R2 = 0 + 15 = 0x000F
		0x12, 0xC5,  // ADDI R3, R1, 0x05        R3 = 0x0020 + 5 = 0x0025
		0x22, 0x80,  // SW   R1, R2, 0x00        MEM[0x0020] = 0x000F
		0x33, 0x00,  // LW   R4, R1, 0x00        R4 = MEM[0x0020] = 0x000F
		0x00, 0x00,  // HALT                      ON = 0
	};

	load_program(program, my_cpu.MEM, sizeof(program));

	struct {
		const char *name;
		uint16_t exp_regs[8];
		uint16_t exp_pc;
		uint8_t  exp_on;
		int      check_mem;
		uint16_t exp_mem_addr;
		uint16_t exp_mem_val;
	} steps[] = {
		{"ADDI R1, R0, 0x20", {0x0000, 0x0020, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}, 0x0002, 0x01, 0, 0, 0},
		{"ADDI R2, R0, 0x0F", {0x0000, 0x0020, 0x000F, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000}, 0x0004, 0x01, 0, 0, 0},
		{"ADDI R3, R1, 0x05", {0x0000, 0x0020, 0x000F, 0x0025, 0x0000, 0x0000, 0x0000, 0x0000}, 0x0006, 0x01, 0, 0, 0},
		{"SW   R1, R2, 0x00", {0x0000, 0x0020, 0x000F, 0x0025, 0x0000, 0x0000, 0x0000, 0x0000}, 0x0008, 0x01, 1, 0x0020, 0x000F},
		{"LW   R4, R1, 0x00", {0x0000, 0x0020, 0x000F, 0x0025, 0x000F, 0x0000, 0x0000, 0x0000}, 0x000A, 0x01, 0, 0, 0},
		{"HALT",              {0x0000, 0x0020, 0x000F, 0x0025, 0x000F, 0x0000, 0x0000, 0x0000}, 0x000C, 0x00, 0, 0, 0},
	};

	int i = 0;

	while (my_cpu.ON) {
		fetch(&my_cpu);
		decode(&my_cpu);
		execute(&my_cpu);

		dump_cpu_state(&my_cpu, steps[i].name, i,
		               steps[i].exp_regs, steps[i].exp_pc, steps[i].exp_on,
		               steps[i].check_mem, steps[i].exp_mem_addr, steps[i].exp_mem_val);
		i++;
	}

	printf("========================================================================\n");
	printf("  %s\n", all_passed ? "ALL TESTS PASSED" : "SOME TESTS FAILED");
	printf("========================================================================\n");

	return all_passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
