#pragma once

#include "immu.h"
#include "types.h"
#include "instruction.h"
#include "bitref.h"

class CPU {
	public:
		CPU(IMMU&);

		void step();
	protected:
		IMMU& mmu;

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
		void INCb(BYTE&);
		void DECb(BYTE&);

		template <typename T, typename S>
		void LD(T& target, const S& source) {
			target = source;
		}

		void JR(const bool&, const BYTE&);
		void JRn(const bool&, const BYTE&);
		void JP(const bool&, const WORD&);
		void JPn(const bool&, const WORD&);

		void ADD(const BYTE&);
		void ADC(const BYTE&);
		void SUB(const BYTE&);
		void SBC(const BYTE&);
		void AND(const BYTE&);
		void XOR(const BYTE&);
		void OR(const BYTE&);
		void CP(const BYTE&);

		void CB();

		template <typename T>
		void RLC(T& target) {
			carryFlag = ((target >> 7) != 0);
			target = static_cast<BYTE>(static_cast<BYTE>(target << 1) | carryFlag);
			zeroFlag = (target == 0);
			halfFlag = false;
			negFlag = false;
		}

		template <typename T>
		void RRC(T& target) {
			carryFlag = ((target & 0x1) != 0);
			target = static_cast<BYTE>(static_cast<BYTE>(target >> 1) | (carryFlag << 7));
			zeroFlag = (target == 0);
			halfFlag = false;
			negFlag = false;
		}
};
