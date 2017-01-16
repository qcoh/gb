#pragma once

#include "mmu.h"
#include "types.h"
#include "instruction.h"
#include "bitref.h"
#include "interruptstate.h"

class CPU {
	public:
		CPU(IMMU&, InterruptState&, WORD = 0);

		DWORD step();
		void handleInterrupts();
	protected:
		WORD m_breakpoint = 0;
		bool m_debugMode = false;

		IMMU& m_mmu;
		InterruptState& m_intState;

		WORD m_pc = 0;
		WORD m_sp = 0;

		WORD m_af = 0;
		WORD m_bc = 0;
		WORD m_de = 0;
		WORD m_hl = 0;

		BYTE& a{*(static_cast<BYTE*>(static_cast<void*>(&m_af)) + 1)};
		BYTE& f{*(static_cast<BYTE*>(static_cast<void*>(&m_af)))};
		BYTE& b{*(static_cast<BYTE*>(static_cast<void*>(&m_bc)) + 1)};
		BYTE& c{*(static_cast<BYTE*>(static_cast<void*>(&m_bc)))};
		BYTE& d{*(static_cast<BYTE*>(static_cast<void*>(&m_de)) + 1)};
		BYTE& e{*(static_cast<BYTE*>(static_cast<void*>(&m_de)))};
		BYTE& h{*(static_cast<BYTE*>(static_cast<void*>(&m_hl)) + 1)};
		BYTE& l{*(static_cast<BYTE*>(static_cast<void*>(&m_hl)))};
		
		BitRef<BYTE, 7> m_zeroFlag{f};
		BitRef<BYTE, 6> m_negFlag{f};
		BitRef<BYTE, 5> m_halfFlag{f};
		BitRef<BYTE, 4> m_carryFlag{f};

		// immediate byte/word
		BYTE n = 0;
		WORD nn = 0;

		// number of cycles of last instruction
		DWORD m_cycles = 0;

		std::array<Instruction, 256> m_instructions;
		std::array<Instruction, 256> m_extended;

		// loads
		template <typename T, typename S>
		void LD(T& target, const S& source) {
			target = source;
		}
		template <typename T, typename S>
		void LDI(T& target, const S& source) {
			target = source;
			m_hl++;
		}
		template <typename T, typename S>
		void LDD(T& target, const S& source) {
			target = source;
			m_hl--;
		}
		void LDadd();

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
		void RETI();

		template <WORD addr>
		void RST() {
			m_mmu.writeByte(m_sp-1, static_cast<BYTE>(m_pc >> 8));
			m_mmu.writeByte(m_sp-2, static_cast<BYTE>(m_pc));
			m_pc = addr;
			m_sp -= 2;
		}
		template <WORD addr, BYTE mask>
		void RST_INT() {
			m_intState.ime = false;
			m_mmu.writeByte(m_sp-1, static_cast<BYTE>(m_pc >> 8));
			m_mmu.writeByte(m_sp-2, static_cast<BYTE>(m_pc));
			m_pc = addr;
			m_sp -= 2;

			// disable flag
			m_intState.intFlag ^= mask;
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
		void ADD();

		template <typename T>
		void INC(T& target) {
			m_halfFlag = ((((target & 0xf) + 1) & 0xf0) != 0);
			target = static_cast<BYTE>(target + 1);
			m_zeroFlag = (target == 0);
			m_negFlag = false;
		}
		// Highly annoying: Can't specialize in class scope (for no apparent reason: https://cplusplus.github.io/EWG/ewg-active.html#41)
		// template <>
		void INC(WORD& target) {
			target++;
		}

		template <typename T>
		void DEC(T& target) {
			m_halfFlag = ((target & 0xf) == 0);
			target = static_cast<BYTE>(target - 1);
			m_zeroFlag = (target == 0);
			m_negFlag = true;
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
			m_carryFlag = ((target >> 7) != 0);
			target = static_cast<BYTE>(static_cast<BYTE>(target << 1) | m_carryFlag);
			m_zeroFlag = (target == 0);
			m_halfFlag = false;
			m_negFlag = false;
		}

		template <typename T>
		void RRC(T& target) {
			m_carryFlag = ((target & 0x1) != 0);
			target = static_cast<BYTE>(static_cast<BYTE>(target >> 1) | (m_carryFlag << 7));
			m_zeroFlag = (target == 0);
			m_halfFlag = false;
			m_negFlag = false;
		}

		template <typename T>
		void RL(T& target) {
			bool temp = m_carryFlag;
			m_carryFlag = ((target >> 7) != 0);
			target = static_cast<BYTE>((target << 1) | temp);
			m_zeroFlag = (target == 0);
			m_halfFlag = false;
			m_negFlag = false;
		}

		template <typename T>
		void RR(T& target) {
			bool temp = m_carryFlag;
			m_carryFlag = ((target & 0x1) != 0);
			target = static_cast<BYTE>((target >> 1) | (temp << 7));
			m_zeroFlag = (target == 0);
			m_halfFlag = false;
			m_negFlag = false;
		}

		template <typename T>
		void SLA(T& target) {
			m_carryFlag = ((target >> 7) != 0);
			target = static_cast<BYTE>(target << 1);
			m_zeroFlag = (target == 0);
			m_halfFlag = false;
			m_negFlag = false;
		}

		template <typename T>
		void SRA(T& target) {
			m_carryFlag = ((target & 0x1) != 0);
			target = static_cast<BYTE>((target >> 1) | (target & 0b10000000));
			m_zeroFlag = (target == 0);
			m_halfFlag = false;
			m_negFlag = false;
		}

		template <typename T>
		void SWAP(T& target) {
			target = static_cast<BYTE>((target >> 4) | (target << 4));
			m_zeroFlag = (target == 0);
			m_carryFlag = false;
			m_halfFlag = false;
			m_negFlag = false;
		}

		template <typename T>
		void SRL(T& target) {
			m_carryFlag = ((target & 0x1) != 0);
			target = static_cast<BYTE>(target >> 1);
			m_zeroFlag = (target == 0);
			m_halfFlag = false;
			m_negFlag = false;
		}

		template <typename T>
		void BIT(const T& target) {
			m_zeroFlag = !target;
			m_negFlag = false;
			m_halfFlag = true;
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
