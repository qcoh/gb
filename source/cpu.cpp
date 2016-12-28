#include <functional>
#include <stdexcept>
#include <iostream>
#include <cstdint>
#include "cpu.h"
#include "memref.h"

CPU::CPU(IMMU& mmu_) :
	mmu{mmu_},
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
	instructions = {{
		{ 0x00, [](){}, "NOP", 4, 1 },
		{ 0x01, std::bind(&CPU::LD<WORD, WORD>, 	this, std::ref(bc), std::cref(nn)), 	"LD BC, nn",	12, 3 },
		{ 0x02, std::bind(&CPU::LD<MemRef, BYTE>,	this, MemRef{bc, mmu}, std::cref(a)),	"LD (BC), A",	8, 1 },
		{ 0x03, std::bind(&CPU::INC,			this, std::ref(bc)),			"INC BC",	8, 1 },
		{ 0x04, std::bind(&CPU::INCb,			this, std::ref(b)),			"INC B",	4, 1 },
		{ 0x05, std::bind(&CPU::DECb,			this, std::ref(b)),			"DEC B",	4, 1 },
		{ 0x06, std::bind(&CPU::LD<BYTE, BYTE>, 	this, std::ref(b), std::cref(n)), 	"LD B, n",	8, 2 },
		{ 0x08, std::bind(&CPU::LD<MemRef, WORD>,	this, MemRef{nn, mmu}, std::cref(sp)),	"LD (nn), SP",	20, 3 },
		{ 0x0a, std::bind(&CPU::LD<BYTE, MemRef>,	this, std::ref(a), MemRef{bc, mmu}),	"LD A, (BC)",	8, 1 },
		{ 0x0b, std::bind(&CPU::DEC,			this, std::ref(bc)), 			"DEC BC", 	8, 1 },
		{ 0x0c, std::bind(&CPU::INCb,			this, std::ref(c)),			"INC C",	4, 1 },
		{ 0x0d, std::bind(&CPU::DECb,			this, std::ref(c)),			"DEC C",	4, 1 },
		{ 0x0e, std::bind(&CPU::LD<BYTE, BYTE>, 	this, std::ref(c), std::cref(n)), 	"LD C, n", 	8, 2 },
		{ 0x11, std::bind(&CPU::LD<WORD, WORD>, 	this, std::ref(de), std::cref(nn)), 	"LD DE, nn", 	12, 3 },
		{ 0x12, std::bind(&CPU::LD<MemRef, BYTE>,	this, MemRef{de, mmu}, std::cref(a)),	"LD (DE), A",	8, 1 },
		{ 0x13, std::bind(&CPU::INC, 			this, std::ref(de)), 			"INC DE", 	8, 1 },
		{ 0x14, std::bind(&CPU::INCb, 			this, std::ref(d)),			"INC D",	4, 1 },
		{ 0x15, std::bind(&CPU::DECb, 			this, std::ref(d)),			"DEC D",	4, 1 },
		{ 0x16, std::bind(&CPU::LD<BYTE, BYTE>, 	this, std::ref(d), std::cref(n)), 	"LD D, n", 	8, 2 },
		{ 0x18, std::bind(&CPU::JR,			this, true, std::cref(n)),		"JR n",		0, 2 },
		{ 0x1a, std::bind(&CPU::LD<BYTE, MemRef>,	this, std::ref(a), MemRef{de, mmu}),	"LD A, (DE)",	8, 1 },
		{ 0x1b, std::bind(&CPU::DEC, 			this, std::ref(de)), 			"DEC DE", 	8, 1 },
		{ 0x1c, std::bind(&CPU::INCb,			this, std::ref(e)),			"INC E",	4, 1 },
		{ 0x1d, std::bind(&CPU::DECb,			this, std::ref(e)),			"DEC E",	4, 1 },
		{ 0x1e, std::bind(&CPU::LD<BYTE, BYTE>, 	this, std::ref(e), std::cref(n)), 	"LD E, n", 	8, 2 },
		{ 0x20, std::bind(&CPU::JRn,			this, zeroFlag, std::cref(n)),		"JR NZ, n",	0, 2 },
		{ 0x21, std::bind(&CPU::LD<WORD, WORD>, 	this, std::ref(hl), std::cref(nn)), 	"LD HL, nn", 	12, 3 },
		{ 0x23, std::bind(&CPU::INC, 			this, std::ref(hl)), 			"INC HL", 	8, 1 },
		{ 0x24, std::bind(&CPU::INCb, 			this, std::ref(h)),			"INC H",	4, 1 },
		{ 0x25, std::bind(&CPU::DECb, 			this, std::ref(h)),			"DEC H",	4, 1 },
		{ 0x26, std::bind(&CPU::LD<BYTE, BYTE>, 	this, std::ref(h), std::cref(n)), 	"LD H, n", 	8, 2 },
		{ 0x28, std::bind(&CPU::JR,			this, zeroFlag, std::cref(n)),		"JR Z, n",	0, 2 },
		{ 0x2b, std::bind(&CPU::DEC, 			this, std::ref(hl)), 			"DEC HL", 	8, 1 },
		{ 0x2c, std::bind(&CPU::INCb,			this, std::ref(l)),			"INC L",	4, 1 },
		{ 0x2d, std::bind(&CPU::DECb,			this, std::ref(l)),			"DEC L",	4, 1 },
		{ 0x2e, std::bind(&CPU::LD<BYTE, BYTE>, 	this, std::ref(l), std::cref(n)), 	"LD L, n", 	8, 2 },
		{ 0x30, std::bind(&CPU::JRn,			this, carryFlag, std::cref(n)),		"JR NC, n",	0, 2 },
		{ 0x31, std::bind(&CPU::LD<WORD, WORD>, 	this, std::ref(sp), std::cref(nn)), 	"LD SP, nn", 	12, 3 },
		{ 0x33, std::bind(&CPU::INC, 			this, std::ref(sp)), 			"INC SP", 	8, 1 },
		//{ 0x34, std::bind(&CPU::INCb, 			this, MemRef{hl, mmu}),			"INC (HL)",	12, 1 },
		//{ 0x35, std::bind(&CPU::DECb, 			this, MemRef{hl, mmu}),			"DEC (HL)",	12, 1 },
		{ 0x38, std::bind(&CPU::JR,			this, carryFlag, std::cref(n)),		"JR C, n",	0, 2 },
		{ 0x3b, std::bind(&CPU::DEC,			this, std::ref(sp)), 			"DEC SP", 	8, 1 },
		{ 0x3c, std::bind(&CPU::INCb,			this, std::ref(a)),			"INC A",	4, 1 },
		{ 0x3d, std::bind(&CPU::DECb,			this, std::ref(a)),			"DEC A",	4, 1 },
		{ 0x3e, std::bind(&CPU::LD<BYTE, BYTE>, 	this, std::ref(a), std::cref(n)), 	"LD A, n", 	8, 2 },
		{ 0x40, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), std::cref(b)),	"LD B, B",	4, 1 },
		{ 0x41, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), std::cref(c)),	"LD B, C",	4, 1 },
		{ 0x42, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), std::cref(d)),	"LD B, D",	4, 1 },
		{ 0x43, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), std::cref(e)),	"LD B, E",	4, 1 },
		{ 0x44, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), std::cref(h)),	"LD B, H",	4, 1 },
		{ 0x45, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), std::cref(l)),	"LD B, L",	4, 1 },
		{ 0x46, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), MemRef{hl, mmu}),	"LD B, (HL)",	8, 1 },
		{ 0x47, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), std::cref(a)),	"LD B, A",	4, 1 },
		
		{ 0x48, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(c), std::cref(b)),	"LD C, B",	4, 1 },
		{ 0x49, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(c), std::cref(c)),	"LD C, C",	4, 1 },
		{ 0x4a, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(c), std::cref(d)),	"LD C, D",	4, 1 },
		{ 0x4b, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(c), std::cref(e)),	"LD C, E",	4, 1 },
		{ 0x4c, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(c), std::cref(h)),	"LD C, H",	4, 1 },
		{ 0x4d, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(c), std::cref(l)),	"LD C, L",	4, 1 },
		{ 0x4e, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(c), MemRef{hl, mmu}),	"LD C, (HL)",	8, 1 },
		{ 0x4f, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(c), std::cref(a)),	"LD C, A",	4, 1 },


		{ 0x50, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(d), std::cref(b)),	"LD D, B",	4, 1 },
		{ 0x51, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(d), std::cref(c)),	"LD D, C",	4, 1 },
		{ 0x52, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(d), std::cref(d)),	"LD D, D",	4, 1 },
		{ 0x53, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(d), std::cref(e)),	"LD D, E",	4, 1 },
		{ 0x54, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(d), std::cref(h)),	"LD D, H",	4, 1 },
		{ 0x55, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(d), std::cref(l)),	"LD D, L",	4, 1 },
		{ 0x56, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(d), MemRef{hl, mmu}),	"LD D, (HL)",	8, 1 },
		{ 0x57, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(d), std::cref(a)),	"LD D, A",	4, 1 },
		
		{ 0x58, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(e), std::cref(b)),	"LD E, B",	4, 1 },
		{ 0x59, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(e), std::cref(c)),	"LD E, C",	4, 1 },
		{ 0x5a, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(e), std::cref(d)),	"LD E, D",	4, 1 },
		{ 0x5b, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(e), std::cref(e)),	"LD E, E",	4, 1 },
		{ 0x5c, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(e), std::cref(h)),	"LD E, H",	4, 1 },
		{ 0x5d, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(e), std::cref(l)),	"LD E, L",	4, 1 },
		{ 0x5e, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(e), MemRef{hl, mmu}),	"LD E, (HL)",	8, 1 },
		{ 0x5f, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(e), std::cref(a)),	"LD E, A",	4, 1 },


		{ 0x60, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(h), std::cref(b)),	"LD H, B",	4, 1 },
		{ 0x61, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(h), std::cref(c)),	"LD H, C",	4, 1 },
		{ 0x62, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(h), std::cref(d)),	"LD H, D",	4, 1 },
		{ 0x63, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(h), std::cref(e)),	"LD H, E",	4, 1 },
		{ 0x64, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(h), std::cref(h)),	"LD H, H",	4, 1 },
		{ 0x65, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(h), std::cref(l)),	"LD H, L",	4, 1 },
		{ 0x66, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(h), MemRef{hl, mmu}),	"LD H, (HL)",	8, 1 },
		{ 0x67, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(h), std::cref(a)),	"LD H, A",	4, 1 },
		
		{ 0x68, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(l), std::cref(b)),	"LD L, B",	4, 1 },
		{ 0x69, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(l), std::cref(c)),	"LD L, C",	4, 1 },
		{ 0x6a, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(l), std::cref(d)),	"LD L, D",	4, 1 },
		{ 0x6b, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(l), std::cref(e)),	"LD L, E",	4, 1 },
		{ 0x6c, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(l), std::cref(h)),	"LD L, H",	4, 1 },
		{ 0x6d, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(l), std::cref(l)),	"LD L, L",	4, 1 },
		{ 0x6e, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(l), MemRef{hl, mmu}),	"LD L, (HL)",	8, 1 },
		{ 0x6f, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(l), std::cref(a)),	"LD L, A",	4, 1 },


		//{ 0x70, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), std::cref(b)),	"LD B, B",	4, 1 },
		//{ 0x71, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), std::cref(c)),	"LD B, C",	4, 1 },
		//{ 0x72, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), std::cref(d)),	"LD B, D",	4, 1 },
		//{ 0x73, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), std::cref(e)),	"LD B, E",	4, 1 },
		//{ 0x74, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), std::cref(h)),	"LD B, H",	4, 1 },
		//{ 0x75, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), std::cref(l)),	"LD B, L",	4, 1 },
		// HALT
		//{ 0x77, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), std::cref(a)),	"LD B, A",	4, 1 },
	
		{ 0x78, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(a), std::cref(b)),	"LD A, B",	4, 1 },
		{ 0x79, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(a), std::cref(c)),	"LD A, C",	4, 1 },
		{ 0x7a, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(a), std::cref(d)),	"LD A, D",	4, 1 },
		{ 0x7b, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(a), std::cref(e)),	"LD A, E",	4, 1 },
		{ 0x7c, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(a), std::cref(h)),	"LD A, H",	4, 1 },
		{ 0x7d, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(a), std::cref(l)),	"LD A, L",	4, 1 },
		{ 0x7e, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(a), MemRef{hl, mmu}),	"LD A, (HL)",	8, 1 },
		{ 0x7f, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(a), std::cref(a)),	"LD A, A",	4, 1 },

		{ 0x80, std::bind(&CPU::ADD,		this, std::cref(b)),			"ADD A, B",	4, 1 },
		{ 0x81, std::bind(&CPU::ADD,		this, std::cref(c)),			"ADD A, C",	4, 1 },
		{ 0x82, std::bind(&CPU::ADD,		this, std::cref(d)),			"ADD A, D",	4, 1 },
		{ 0x83, std::bind(&CPU::ADD,		this, std::cref(e)),			"ADD A, E",	4, 1 },
		{ 0x84, std::bind(&CPU::ADD,		this, std::cref(h)),			"ADD A, H",	4, 1 },
		{ 0x85, std::bind(&CPU::ADD,		this, std::cref(l)),			"ADD A, L",	4, 1 },
		{ 0x86, std::bind(&CPU::ADD,		this, MemRef{hl, mmu}),			"ADD A, (HL)",	8, 1 },
		{ 0x87, std::bind(&CPU::ADD,		this, std::cref(a)),			"ADD A, A",	4, 1 },

		{ 0x88, std::bind(&CPU::ADC,		this, std::cref(b)),			"ADC A, B",	4, 1 },
		{ 0x89, std::bind(&CPU::ADC,		this, std::cref(c)),			"ADC A, C",	4, 1 },
		{ 0x8a, std::bind(&CPU::ADC,		this, std::cref(d)),			"ADC A, D",	4, 1 },
		{ 0x8b, std::bind(&CPU::ADC,		this, std::cref(e)),			"ADC A, E",	4, 1 },
		{ 0x8c, std::bind(&CPU::ADC,		this, std::cref(h)),			"ADC A, H",	4, 1 },
		{ 0x8d, std::bind(&CPU::ADC,		this, std::cref(l)),			"ADC A, L",	4, 1 },
		{ 0x8e, std::bind(&CPU::ADC,		this, MemRef{hl, mmu}),			"ADC A, (HL)",	8, 1 },
		{ 0x8f, std::bind(&CPU::ADC,		this, std::cref(a)),			"ADC A, A",	4, 1 },

		{ 0x90, std::bind(&CPU::SUB,		this, std::cref(b)),			"SUB A, B",	4, 1 },
		{ 0x91, std::bind(&CPU::SUB,		this, std::cref(c)),			"SUB A, C",	4, 1 },
		{ 0x92, std::bind(&CPU::SUB,		this, std::cref(d)),			"SUB A, D",	4, 1 },
		{ 0x93, std::bind(&CPU::SUB,		this, std::cref(e)),			"SUB A, E",	4, 1 },
		{ 0x94, std::bind(&CPU::SUB,		this, std::cref(h)),			"SUB A, H",	4, 1 },
		{ 0x95, std::bind(&CPU::SUB,		this, std::cref(l)),			"SUB A, L",	4, 1 },
		{ 0x96, std::bind(&CPU::SUB,		this, MemRef{hl, mmu}),			"SUB A, (HL)",	8, 1 },
		{ 0x97, std::bind(&CPU::SUB,		this, std::cref(a)),			"SUB A, A",	4, 1 },

		{ 0x98, std::bind(&CPU::SBC,		this, std::cref(b)),			"SBC A, B",	4, 1 },
		{ 0x99, std::bind(&CPU::SBC,		this, std::cref(c)),			"SBC A, C",	4, 1 },
		{ 0x9a, std::bind(&CPU::SBC,		this, std::cref(d)),			"SBC A, D",	4, 1 },
		{ 0x9b, std::bind(&CPU::SBC,		this, std::cref(e)),			"SBC A, E",	4, 1 },
		{ 0x9c, std::bind(&CPU::SBC,		this, std::cref(h)),			"SBC A, H",	4, 1 },
		{ 0x9d, std::bind(&CPU::SBC,		this, std::cref(l)),			"SBC A, L",	4, 1 },
		{ 0x9e, std::bind(&CPU::SBC,		this, MemRef{hl, mmu}),			"SBC A, (HL)",	8, 1 },
		{ 0x9f, std::bind(&CPU::SBC,		this, std::cref(a)),			"SBC A, A",	4, 1 },

		{ 0xa0, std::bind(&CPU::AND,		this, std::cref(b)),			"ADD A, B",	4, 1 },
		{ 0xa1, std::bind(&CPU::AND,		this, std::cref(c)),			"ADD A, C",	4, 1 },
		{ 0xa2, std::bind(&CPU::AND,		this, std::cref(d)),			"ADD A, D",	4, 1 },
		{ 0xa3, std::bind(&CPU::AND,		this, std::cref(e)),			"ADD A, E",	4, 1 },
		{ 0xa4, std::bind(&CPU::AND,		this, std::cref(h)),			"ADD A, H",	4, 1 },
		{ 0xa5, std::bind(&CPU::AND,		this, std::cref(l)),			"ADD A, L",	4, 1 },
		{ 0xa6, std::bind(&CPU::AND,		this, MemRef{hl, mmu}),			"ADD A, (HL)",	8, 1 },
		{ 0xa7, std::bind(&CPU::AND,		this, std::cref(a)),			"ADD A, A",	4, 1 },

		{ 0xa8, std::bind(&CPU::XOR,		this, std::cref(b)),			"XOR A, B",	4, 1 },
		{ 0xa9, std::bind(&CPU::XOR,		this, std::cref(c)),			"XOR A, C",	4, 1 },
		{ 0xaa, std::bind(&CPU::XOR,		this, std::cref(d)),			"XOR A, D",	4, 1 },
		{ 0xab, std::bind(&CPU::XOR,		this, std::cref(e)),			"XOR A, E",	4, 1 },
		{ 0xac, std::bind(&CPU::XOR,		this, std::cref(h)),			"XOR A, H",	4, 1 },
		{ 0xad, std::bind(&CPU::XOR,		this, std::cref(l)),			"XOR A, L",	4, 1 },
		{ 0xae, std::bind(&CPU::XOR,		this, MemRef{hl, mmu}),			"XOR A, (HL)",	8, 1 },
		{ 0xaf, std::bind(&CPU::XOR,		this, std::cref(a)),			"XOR A, A",	4, 1 },

		{ 0xb0, std::bind(&CPU::OR,		this, std::cref(b)),			"OR A, B",	4, 1 },
		{ 0xb1, std::bind(&CPU::OR,		this, std::cref(c)),			"OR A, C",	4, 1 },
		{ 0xb2, std::bind(&CPU::OR,		this, std::cref(d)),			"OR A, D",	4, 1 },
		{ 0xb3, std::bind(&CPU::OR,		this, std::cref(e)),			"OR A, E",	4, 1 },
		{ 0xb4, std::bind(&CPU::OR,		this, std::cref(h)),			"OR A, H",	4, 1 },
		{ 0xb5, std::bind(&CPU::OR,		this, std::cref(l)),			"OR A, L",	4, 1 },
		{ 0xb6, std::bind(&CPU::OR,		this, MemRef{hl, mmu}),			"OR A, (HL)",	8, 1 },
		{ 0xb7, std::bind(&CPU::OR,		this, std::cref(a)),			"OR A, A",	4, 1 },

		{ 0xb8, std::bind(&CPU::CP,		this, std::cref(b)),			"CP A, B",	4, 1 },
		{ 0xb9, std::bind(&CPU::CP,		this, std::cref(c)),			"CP A, C",	4, 1 },
		{ 0xba, std::bind(&CPU::CP,		this, std::cref(d)),			"CP A, D",	4, 1 },
		{ 0xbb, std::bind(&CPU::CP,		this, std::cref(e)),			"CP A, E",	4, 1 },
		{ 0xbc, std::bind(&CPU::CP,		this, std::cref(h)),			"CP A, H",	4, 1 },
		{ 0xbd, std::bind(&CPU::CP,		this, std::cref(l)),			"CP A, L",	4, 1 },
		{ 0xbe, std::bind(&CPU::CP,		this, MemRef{hl, mmu}),			"CP A, (HL)",	8, 1 },
		{ 0xbf, std::bind(&CPU::CP,		this, std::cref(a)),			"CP A, A",	4, 1 },
		
		{ 0xc2, std::bind(&CPU::JPn,		this, zeroFlag,	std::cref(nn)),		"JP NZ, nn",	0, 3 },
		{ 0xc3, std::bind(&CPU::JP,		this, true, std::cref(nn)),		"JP nn",	0, 3 },

		{ 0xc6, std::bind(&CPU::ADD,		this, std::cref(n)),			"ADD A, n",	8, 2 },

		{ 0xca, std::bind(&CPU::JP,		this, zeroFlag, std::cref(nn)),		"JP Z, nn",	0, 3 },
		{ 0xcb, std::bind(&CPU::CB,		this),					"CB",		4, 2 },

		{ 0xce, std::bind(&CPU::ADC,		this, std::cref(n)),			"ADC A, n",	8, 2 },

		{ 0xd2, std::bind(&CPU::JPn,		this, carryFlag, std::cref(nn)),	"JP NC, nn",	0, 3 },

		{ 0xd6, std::bind(&CPU::SUB,		this, std::cref(n)),			"SUB A, n",	8, 2 },

		{ 0xda, std::bind(&CPU::JP,		this, carryFlag, std::cref(nn)),	"JP C, nn", 	0, 3 },

		{ 0xde, std::bind(&CPU::SBC,		this, std::cref(n)),			"SBC A, n",	8, 2 },

		{ 0xe6, std::bind(&CPU::AND,		this, std::cref(n)),			"AND A, n",	8, 2 },

		{ 0xee, std::bind(&CPU::XOR,		this, std::cref(n)),			"XOR A, n",	8, 2 },

		{ 0xf6, std::bind(&CPU::OR,		this, std::cref(n)),			"OR A, n",	8, 2 },

		{ 0xfe, std::bind(&CPU::CP,		this, std::cref(n)),			"CP A, n",	8, 2 },
	}};

	extended = {{
		{ 0x00, std::bind(&CPU::RLC<BYTE>, this, std::ref(b)),		"RLC B", 8, 0 },
		{ 0x01, std::bind(&CPU::RLC<BYTE>, this, std::ref(c)),		"RLC B", 8, 0 },
		{ 0x02, std::bind(&CPU::RLC<BYTE>, this, std::ref(d)),		"RLC B", 8, 0 },
		{ 0x03, std::bind(&CPU::RLC<BYTE>, this, std::ref(e)),		"RLC B", 8, 0 },
		{ 0x04, std::bind(&CPU::RLC<BYTE>, this, std::ref(h)),		"RLC B", 8, 0 },
		{ 0x05, std::bind(&CPU::RLC<BYTE>, this, std::ref(l)),		"RLC B", 8, 0 },
		{ 0x06, std::bind(&CPU::RLC<MemRef>, this, MemRef{hl, mmu}),	"RLC B", 8, 0 },
		{ 0x07, std::bind(&CPU::RLC<BYTE>, this, std::ref(a)),		"RLC B", 8, 0 },

	}};
}

void CPU::step() {
	auto rb = mmu.readByte(pc);
	auto& op = instructions[rb];
	if (op.opcode != rb) {
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

void CPU::JR(const bool& cond, const BYTE& offset) {
	if (cond) {
		pc += static_cast<int8_t>(offset);
		cycles += 4;
	}
	cycles += 8;
}

void CPU::JRn(const bool& cond, const BYTE& offset) {
	if (!cond) {
		pc += static_cast<int8_t>(offset);
		cycles += 4;
	}
	cycles += 8;
}

void CPU::JP(const bool& cond, const WORD& addr) {
	if (cond) {
		pc = addr;
		cycles += 4;
	}
	cycles += 12;
}

void CPU::JPn(const bool& cond, const WORD& addr) {
	if (!cond) {
		pc = addr;
		cycles += 4;
	}
	cycles += 12;
}

void CPU::ADD(const BYTE& source) {
	halfFlag = ((((a & 0xf) + (source & 0xf)) & 0xf0) != 0);
	WORD temp = static_cast<WORD>(a) + static_cast<WORD>(source);
	a = static_cast<BYTE>(temp);
	carryFlag = ((temp & 0xf00) != 0);
	zeroFlag = (a == 0);
	negFlag = false;
}

void CPU::ADC(const BYTE& source) {
	halfFlag = ((((a & 0xf) + (source & 0xf) + carryFlag) & 0xf0) != 0);
	WORD temp = static_cast<WORD>(a) + static_cast<WORD>(source) + carryFlag;
	a = static_cast<BYTE>(temp);
	carryFlag = ((temp & 0xf00) != 0);
	zeroFlag = (a == 0);
	negFlag = false;
}

void CPU::SUB(const BYTE& source) {
	halfFlag = ((a & 0xf) < (source & 0xf));
	int temp = a - source;
	a = static_cast<BYTE>(temp);
	zeroFlag = (a == 0);
	carryFlag = (temp < 0);
	negFlag = true;
}

void CPU::SBC(const BYTE& source) {
	halfFlag = ((a & 0xf) < ((source & 0xf) + carryFlag));
	int temp = a - source - carryFlag;
	a = static_cast<BYTE>(temp);
	zeroFlag = (a == 0);
	carryFlag = (temp < 0);
	negFlag = true;
}

void CPU::AND(const BYTE& source) {
	a &= source;
	zeroFlag = (a == 0);
	halfFlag = true;
	negFlag = false;
	carryFlag = false;
}

void CPU::XOR(const BYTE& source) {
	a ^= source;
	zeroFlag = (a == 0);
	halfFlag = false;
	negFlag = false;
	carryFlag = false;
}

void CPU::OR(const BYTE& source) {
	a |= source;
	zeroFlag = (a == 0);
	halfFlag = false;
	negFlag = false;
	carryFlag = false;
}

void CPU::CP(const BYTE& source) {
	int temp = a - source;
	halfFlag = ((a & 0xf) < (source & 0xf));
	carryFlag = (temp < 0);
	negFlag = true;
	zeroFlag = (temp == 0);
}

void CPU::INCb(BYTE& target) {
	halfFlag = ((((target & 0xf) + 1) & 0xf0) != 0);
	target++;
	zeroFlag = (target == 0);
	negFlag = false;
}

void CPU::DECb(BYTE& target) {
	halfFlag = ((target & 0xf) == 0);
	target--;
	zeroFlag = (target == 0);
	negFlag = true;
}

void CPU::CB() {
	auto& op = extended[n];
	if (op.opcode != n) {
		std::cout << "Missing extended instruction: 0x" << std::ios::hex << n << '\n';
		throw std::runtime_error{"Missing instruction"};
	}
	op.f();
	cycles += op.cycles;
}
