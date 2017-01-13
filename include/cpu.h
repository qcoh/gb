#pragma once

#include "immu.h"
#include "types.h"
#include "instruction.h"
#include "bitref.h"

class CPU {
	public:
		CPU(IMMU&, WORD = 0);

		DWORD step();
		void interrupt();
	protected:
		WORD breakpoint;
		bool debugMode;

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

		// interrupt master enable flag
		bool ime;

		// number of cycles of last instruction
		DWORD cycles;

		std::array<Instruction, 256> instructions;
		std::array<Instruction, 256> extended;

		// loads
		template <typename T, typename S>
		void LD(T& target, const S& source) {
			target = source;
		}
		template <typename T, typename S>
		void LDI(T& target, const S& source) {
			target = source;
			hl++;
		}
		template <typename T, typename S>
		void LDD(T& target, const S& source) {
			target = source;
			hl--;
		}

		void POP(WORD&);
		void PUSH(const WORD&);

		// controlflow
		void JR(const bool&, const BYTE&);
		void JRn(const bool&, const BYTE&);
		void JP(const bool&, const WORD&);
		void JPn(const bool&, const WORD&);

		void CALL(const WORD&);
		void RET();
		void RETcond(const bool&);
		void RETncond(const bool&);

		template <WORD addr>
		void RST() {
			mmu.writeByte(sp-1, static_cast<BYTE>(pc >> 8));
			mmu.writeByte(sp-2, static_cast<BYTE>(pc));
			pc = addr;
			sp -= 2;
		}
		template <WORD addr, BYTE mask>
		void RST_INT() {
			ime = false;
			mmu.writeByte(sp-1, static_cast<BYTE>(pc >> 8));
			mmu.writeByte(sp-2, static_cast<BYTE>(pc));
			pc = addr;
			sp -= 2;

			// disable flag
			BYTE intF = mmu.readByte(0xff0f);
			intF ^= mask;
			mmu.writeByte(0xff0f, intF);
		}

		// 8bit arithmetic
		void ADD(const BYTE&);
		void ADC(const BYTE&);
		void SUB(const BYTE&);
		void SBC(const BYTE&);
		void AND(const BYTE&);
		void XOR(const BYTE&);
		void OR(const BYTE&);
		void CP(const BYTE&);

		void CCF();
		void SCF();
		void CPL();
		void DAA();

		// 16bit arithmetic
		void ADD(WORD&, const WORD&);

		// mixed
		void ADD(WORD&, const BYTE&);

		template <typename T>
		void INC(T& target) {
			halfFlag = ((((target & 0xf) + 1) & 0xf0) != 0);
			target++;
			zeroFlag = (target == 0);
			negFlag = false;
		}
		// Highly annoying: Can't specialize in class scope (for no apparent reason: https://cplusplus.github.io/EWG/ewg-active.html#41)
		// template <>
		void INC(WORD& target) {
			target++;
		}

		template <typename T>
		void DEC(T& target) {
			halfFlag = ((target & 0xf) == 0);
			target--;
			zeroFlag = (target == 0);
			negFlag = true;
		}
		// template <>
		void DEC(WORD& target) {
			target--;
		}

		// shift and rotate
		void RLCA();
		void RRCA();
		void RLA();
		void RRA();

		// misc
		void CB();
		void EI();
		void DI();

		// extended instruction set
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

		template <typename T>
		void RL(T& target) {
			bool temp = carryFlag;
			carryFlag = ((target >> 7) != 0);
			target = static_cast<BYTE>((target << 1) | temp);
			zeroFlag = (target == 0);
			halfFlag = false;
			negFlag = false;
		}

		template <typename T>
		void RR(T& target) {
			bool temp = carryFlag;
			carryFlag = ((target & 0x1) != 0);
			target = static_cast<BYTE>((target >> 1) | (temp << 7));
			zeroFlag = (target == 0);
			halfFlag = false;
			negFlag = false;
		}

		template <typename T>
		void SLA(T& target) {
			carryFlag = ((target >> 7) != 0);
			target = static_cast<BYTE>(target << 1);
			zeroFlag = (target == 0);
			halfFlag = false;
			negFlag = false;
		}

		template <typename T>
		void SRA(T& target) {
			carryFlag = ((target & 0x1) != 0);
			target = static_cast<BYTE>((target >> 1) | (target & 0b10000000));
			zeroFlag = (target == 0);
			halfFlag = false;
			negFlag = false;
		}

		template <typename T>
		void SWAP(T& target) {
			target = static_cast<BYTE>((target >> 4) | (target << 4));
			zeroFlag = (target == 0);
			carryFlag = false;
			halfFlag = false;
			negFlag = false;
		}

		template <typename T>
		void SRL(T& target) {
			carryFlag = ((target & 0x1) != 0);
			target = static_cast<BYTE>(target >> 1);
			zeroFlag = (target == 0);
			halfFlag = false;
			negFlag = false;
		}

		template <typename T>
		void BIT(const T& target) {
			zeroFlag = !target;
			negFlag = false;
			halfFlag = true;
		}

		template <typename T>
		void RES(T& target) {
			target = false;
		}

		template <typename T>
		void SET(T& target) {
			target = true;
		}
};
