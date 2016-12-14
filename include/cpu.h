#pragma once

#include "mmu.h"
#include "types.h"
#include "instruction.h"

class CPU {
	public:
		CPU(MMU&&);

		void step();
	protected:
		MMU mmu;

		WORD pc;
		WORD sp;

		WORD af;
		WORD bc;
		WORD de;
		WORD hl;

		BYTE* a;
		BYTE* f;
		BYTE* b;
		BYTE* c;
		BYTE* d;
		BYTE* e;
		BYTE* h;
		BYTE* l;

		std::array<Instruction, 256> instructions;
		std::array<Instruction, 256> extended;

		void INC(WORD&);
		void DEC(WORD&);
};
