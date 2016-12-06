#pragma once

#include "mmu.h"
#include "types.h"
#include "instruction.h"

class CPU {
	public:
		CPU(MMU&&);

		void step();
	private:
		MMU mmu;

		WORD pc;
		WORD sp;

		std::array<Instruction, 256> instructions;
		std::array<Instruction, 256> extended;
};
