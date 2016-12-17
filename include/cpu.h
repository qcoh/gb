#pragma once

#include "mmu.h"
#include "types.h"
#include "instruction.h"
#include "bitref.h"

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

		BYTE& a;
		BYTE& f;
		BYTE& b;
		BYTE& c;
		BYTE& d;
		BYTE& e;
		BYTE& h;
		BYTE& l;
		
		BitRef<BYTE, 7> zeroFlag;
		BitRef<BYTE, 6> negFlag;
		BitRef<BYTE, 5> halfFlag;
		BitRef<BYTE, 4> carryFlag;

		// immediate byte/word
		BYTE n;
		WORD nn;

		unsigned cycles;

		std::array<Instruction, 256> instructions;
		std::array<Instruction, 256> extended;

		void INC(WORD&);
		void DEC(WORD&);

		template <typename T>
		void LD(T& target, const T& source) {
			target = source;
		}

		void JR(const bool&, const BYTE&);
		void JRn(const bool&, const BYTE&);
		void JP(const bool&, const WORD&);
		void JPn(const bool&, const WORD&);

		void ADD(const BYTE&);
		void ADC(const BYTE&);
		void SUB(const BYTE&);
};
