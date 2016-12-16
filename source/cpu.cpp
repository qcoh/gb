#include <functional>
#include <stdexcept>
#include <iostream>
#include "cpu.h"

CPU::CPU(MMU&& mmu_) :
	mmu{std::move(mmu_)},
	pc{0},
	sp{0},
	af{0},
	bc{0},
	de{0},
	hl{0},
	a{*(static_cast<BYTE*>(static_cast<void*>(&af))+1)},
	f{*static_cast<BYTE*>(static_cast<void*>(&af))},
	b{*(static_cast<BYTE*>(static_cast<void*>(&bc))+1)},
	c{*static_cast<BYTE*>(static_cast<void*>(&bc))},
	d{*(static_cast<BYTE*>(static_cast<void*>(&de))+1)},
	e{*static_cast<BYTE*>(static_cast<void*>(&de))},
	h{*(static_cast<BYTE*>(static_cast<void*>(&hl))+1)},
	l{*static_cast<BYTE*>(static_cast<void*>(&hl))},
	zeroFlag{f},
	negFlag{f},
	halfFlag{f},
	carryFlag{f},
	n{0},
	nn{0}
{
	(void)zeroFlag;
	(void)negFlag;
	(void)halfFlag;
	(void)carryFlag;

	instructions = {{
		{ 0x00, [](){}, "NOP", 4, 1 },
		{ 0x01, std::bind(&CPU::LD<WORD>, 	this, std::ref(bc), std::cref(nn)), 	"LD BC, nn",	12, 3 },
		{ 0x03, std::bind(&CPU::INC,		this, std::ref(bc)),			"INC BC",	8, 1 },
		{ 0x06, std::bind(&CPU::LD<BYTE>, 	this, std::ref(b), std::cref(n)), 	"LD B, n",	8, 2 },
		{ 0x0b, std::bind(&CPU::DEC,		this, std::ref(bc)), 			"DEC BC", 	8, 1 },
		{ 0x0e, std::bind(&CPU::LD<BYTE>, 	this, std::ref(c), std::cref(n)), 	"LD C, n", 	8, 2 },
		{ 0x11, std::bind(&CPU::LD<WORD>, 	this, std::ref(de), std::cref(nn)), 	"LD DE, nn", 	12, 3 },
		{ 0x13, std::bind(&CPU::INC, 		this, std::ref(de)), 			"INC DE", 	8, 1 },
		{ 0x16, std::bind(&CPU::LD<BYTE>, 	this, std::ref(d), std::cref(n)), 	"LD D, n", 	8, 2 },
		{ 0x1b, std::bind(&CPU::DEC, 		this, std::ref(de)), 			"DEC DE", 	8, 1 },
		{ 0x1e, std::bind(&CPU::LD<BYTE>, 	this, std::ref(e), std::cref(n)), 	"LD E, n", 	8, 2 },
		{ 0x21, std::bind(&CPU::LD<WORD>, 	this, std::ref(hl), std::cref(nn)), 	"LD HL, nn", 	12, 3 },
		{ 0x23, std::bind(&CPU::INC, 		this, std::ref(hl)), 			"INC HL", 	8, 1 },
		{ 0x26, std::bind(&CPU::LD<BYTE>, 	this, std::ref(h), std::cref(n)), 	"LD H, n", 	8, 2 },
		{ 0x2b, std::bind(&CPU::DEC, 		this, std::ref(hl)), 			"DEC HL", 	8, 1 },
		{ 0x2e, std::bind(&CPU::LD<BYTE>, 	this, std::ref(l), std::cref(n)), 	"LD L, n", 	8, 2 },
		{ 0x31, std::bind(&CPU::LD<WORD>, 	this, std::ref(sp), std::cref(nn)), 	"LD SP, nn", 	12, 3 },
		{ 0x33, std::bind(&CPU::INC, 		this, std::ref(sp)), 			"INC SP", 	8, 1 },
		{ 0x3b, std::bind(&CPU::DEC,		this, std::ref(sp)), 			"DEC SP", 	8, 1 },
		{ 0x3e, std::bind(&CPU::LD<BYTE>, 	this, std::ref(a), std::cref(n)), 	"LD A, n", 	8, 2 },

		{ 0x40, std::bind(&CPU::LD<BYTE>,	this, std::ref(b), std::cref(b)),	"LD B, B",	4, 1 },
		{ 0x41, std::bind(&CPU::LD<BYTE>,	this, std::ref(b), std::cref(c)),	"LD B, C",	4, 1 },
		{ 0x42, std::bind(&CPU::LD<BYTE>,	this, std::ref(b), std::cref(d)),	"LD B, D",	4, 1 },
		{ 0x43, std::bind(&CPU::LD<BYTE>,	this, std::ref(b), std::cref(e)),	"LD B, E",	4, 1 },
		{ 0x44, std::bind(&CPU::LD<BYTE>,	this, std::ref(b), std::cref(h)),	"LD B, H",	4, 1 },
		{ 0x45, std::bind(&CPU::LD<BYTE>,	this, std::ref(b), std::cref(l)),	"LD B, L",	4, 1 },
		//{ 0x46, std::bind(&CPU::LD<BYTE>,	this, std::ref(b), std::cref(b)),	"LD B, B",	4, 1 },
		{ 0x47, std::bind(&CPU::LD<BYTE>,	this, std::ref(b), std::cref(a)),	"LD B, A",	4, 1 },
		
		{ 0x48, std::bind(&CPU::LD<BYTE>,	this, std::ref(c), std::cref(b)),	"LD C, B",	4, 1 },
		{ 0x49, std::bind(&CPU::LD<BYTE>,	this, std::ref(c), std::cref(c)),	"LD C, C",	4, 1 },
		{ 0x4a, std::bind(&CPU::LD<BYTE>,	this, std::ref(c), std::cref(d)),	"LD C, D",	4, 1 },
		{ 0x4b, std::bind(&CPU::LD<BYTE>,	this, std::ref(c), std::cref(e)),	"LD C, E",	4, 1 },
		{ 0x4c, std::bind(&CPU::LD<BYTE>,	this, std::ref(c), std::cref(h)),	"LD C, H",	4, 1 },
		{ 0x4d, std::bind(&CPU::LD<BYTE>,	this, std::ref(c), std::cref(l)),	"LD C, L",	4, 1 },
		//{ 0x4e, std::bind(&CPU::LD<BYTE>,	this, std::ref(c), std::cref(b)),	"LD C, B",	4, 1 },
		{ 0x4f, std::bind(&CPU::LD<BYTE>,	this, std::ref(c), std::cref(a)),	"LD C, A",	4, 1 },


		{ 0x50, std::bind(&CPU::LD<BYTE>,	this, std::ref(d), std::cref(b)),	"LD D, B",	4, 1 },
		{ 0x51, std::bind(&CPU::LD<BYTE>,	this, std::ref(d), std::cref(c)),	"LD D, C",	4, 1 },
		{ 0x52, std::bind(&CPU::LD<BYTE>,	this, std::ref(d), std::cref(d)),	"LD D, D",	4, 1 },
		{ 0x53, std::bind(&CPU::LD<BYTE>,	this, std::ref(d), std::cref(e)),	"LD D, E",	4, 1 },
		{ 0x54, std::bind(&CPU::LD<BYTE>,	this, std::ref(d), std::cref(h)),	"LD D, H",	4, 1 },
		{ 0x55, std::bind(&CPU::LD<BYTE>,	this, std::ref(d), std::cref(l)),	"LD D, L",	4, 1 },
		//{ 0x56, std::bind(&CPU::LD<BYTE>,	this, std::ref(d), std::cref(b)),	"LD D, B",	4, 1 },
		{ 0x57, std::bind(&CPU::LD<BYTE>,	this, std::ref(d), std::cref(a)),	"LD D, A",	4, 1 },
		
		{ 0x58, std::bind(&CPU::LD<BYTE>,	this, std::ref(e), std::cref(b)),	"LD E, B",	4, 1 },
		{ 0x59, std::bind(&CPU::LD<BYTE>,	this, std::ref(e), std::cref(c)),	"LD E, C",	4, 1 },
		{ 0x5a, std::bind(&CPU::LD<BYTE>,	this, std::ref(e), std::cref(d)),	"LD E, D",	4, 1 },
		{ 0x5b, std::bind(&CPU::LD<BYTE>,	this, std::ref(e), std::cref(e)),	"LD E, E",	4, 1 },
		{ 0x5c, std::bind(&CPU::LD<BYTE>,	this, std::ref(e), std::cref(h)),	"LD E, H",	4, 1 },
		{ 0x5d, std::bind(&CPU::LD<BYTE>,	this, std::ref(e), std::cref(l)),	"LD E, L",	4, 1 },
		//{ 0x5e, std::bind(&CPU::LD<BYTE>,	this, std::ref(e), std::cref(b)),	"LD E, B",	4, 1 },
		{ 0x5f, std::bind(&CPU::LD<BYTE>,	this, std::ref(e), std::cref(a)),	"LD E, A",	4, 1 },


		{ 0x60, std::bind(&CPU::LD<BYTE>,	this, std::ref(h), std::cref(b)),	"LD H, B",	4, 1 },
		{ 0x61, std::bind(&CPU::LD<BYTE>,	this, std::ref(h), std::cref(c)),	"LD H, C",	4, 1 },
		{ 0x62, std::bind(&CPU::LD<BYTE>,	this, std::ref(h), std::cref(d)),	"LD H, D",	4, 1 },
		{ 0x63, std::bind(&CPU::LD<BYTE>,	this, std::ref(h), std::cref(e)),	"LD H, E",	4, 1 },
		{ 0x64, std::bind(&CPU::LD<BYTE>,	this, std::ref(h), std::cref(h)),	"LD H, H",	4, 1 },
		{ 0x65, std::bind(&CPU::LD<BYTE>,	this, std::ref(h), std::cref(l)),	"LD H, L",	4, 1 },
		//{ 0x66, std::bind(&CPU::LD<BYTE>,	this, std::ref(h), std::cref(b)),	"LD H, B",	4, 1 },
		{ 0x67, std::bind(&CPU::LD<BYTE>,	this, std::ref(h), std::cref(a)),	"LD H, A",	4, 1 },
		
		{ 0x68, std::bind(&CPU::LD<BYTE>,	this, std::ref(l), std::cref(b)),	"LD L, B",	4, 1 },
		{ 0x69, std::bind(&CPU::LD<BYTE>,	this, std::ref(l), std::cref(c)),	"LD L, C",	4, 1 },
		{ 0x6a, std::bind(&CPU::LD<BYTE>,	this, std::ref(l), std::cref(d)),	"LD L, D",	4, 1 },
		{ 0x6b, std::bind(&CPU::LD<BYTE>,	this, std::ref(l), std::cref(e)),	"LD L, E",	4, 1 },
		{ 0x6c, std::bind(&CPU::LD<BYTE>,	this, std::ref(l), std::cref(h)),	"LD L, H",	4, 1 },
		{ 0x6d, std::bind(&CPU::LD<BYTE>,	this, std::ref(l), std::cref(l)),	"LD L, L",	4, 1 },
		//{ 0x6e, std::bind(&CPU::LD<BYTE>,	this, std::ref(l), std::cref(b)),	"LD L, B",	4, 1 },
		{ 0x6f, std::bind(&CPU::LD<BYTE>,	this, std::ref(l), std::cref(a)),	"LD L, A",	4, 1 },


		//{ 0x70, std::bind(&CPU::LD<BYTE>,	this, std::ref(b), std::cref(b)),	"LD B, B",	4, 1 },
		//{ 0x71, std::bind(&CPU::LD<BYTE>,	this, std::ref(b), std::cref(c)),	"LD B, C",	4, 1 },
		//{ 0x72, std::bind(&CPU::LD<BYTE>,	this, std::ref(b), std::cref(d)),	"LD B, D",	4, 1 },
		//{ 0x73, std::bind(&CPU::LD<BYTE>,	this, std::ref(b), std::cref(e)),	"LD B, E",	4, 1 },
		//{ 0x74, std::bind(&CPU::LD<BYTE>,	this, std::ref(b), std::cref(h)),	"LD B, H",	4, 1 },
		//{ 0x75, std::bind(&CPU::LD<BYTE>,	this, std::ref(b), std::cref(l)),	"LD B, L",	4, 1 },
		// HALT
		//{ 0x77, std::bind(&CPU::LD<BYTE>,	this, std::ref(b), std::cref(a)),	"LD B, A",	4, 1 },
	
		{ 0x78, std::bind(&CPU::LD<BYTE>,	this, std::ref(a), std::cref(b)),	"LD A, B",	4, 1 },
		{ 0x79, std::bind(&CPU::LD<BYTE>,	this, std::ref(a), std::cref(c)),	"LD A, C",	4, 1 },
		{ 0x7a, std::bind(&CPU::LD<BYTE>,	this, std::ref(a), std::cref(d)),	"LD A, D",	4, 1 },
		{ 0x7b, std::bind(&CPU::LD<BYTE>,	this, std::ref(a), std::cref(e)),	"LD A, E",	4, 1 },
		{ 0x7c, std::bind(&CPU::LD<BYTE>,	this, std::ref(a), std::cref(h)),	"LD A, H",	4, 1 },
		{ 0x7d, std::bind(&CPU::LD<BYTE>,	this, std::ref(a), std::cref(l)),	"LD A, L",	4, 1 },
		//{ 0x7e, std::bind(&CPU::LD<BYTE>,	this, std::ref(a), std::cref(b)),	"LD A, B",	4, 1 },
		{ 0x7f, std::bind(&CPU::LD<BYTE>,	this, std::ref(a), std::cref(a)),	"LD A, A",	4, 1 },
	}};
}

void CPU::step() {
	auto rb = mmu.readByte(pc);
	auto& op = instructions[rb];
	if (op.opcode == rb) {
		std::cout << "Missing instruction: 0x" << std::ios::hex << rb << '\n';
		throw std::runtime_error{"Missing instruction"};
	}
	op.f();
	cycles += op.cycles;
	pc += op.offset;
}

void CPU::INC(WORD& w) {
	w++;
}

void CPU::DEC(WORD& w) {
	w--;
}
