#include <functional>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <bitset>

#include "cpu.h"
#include "memref.h"
#include "offsetref.h"

CPU::CPU(IMMU& m_mmu_, InterruptState& m_intState_, WORD m_breakpoint_) :
	m_breakpoint{m_breakpoint_},
	m_mmu{m_mmu_},
	m_intState{m_intState_}
{
	m_instructions = {{
		{ 0x00, [](){}, "NOP", 4, 0 },
		{ 0x01, std::bind(&CPU::LD<WORD, WORD>, 	this, std::ref(m_bc), std::cref(nn)), 	"LD BC, nn",	12, 2 },
		{ 0x02, std::bind(&CPU::LD<MemRef, BYTE>,	this, MemRef{m_bc, m_mmu}, std::cref(a)),	"LD (BC), A",	8, 0 },
		{ 0x03, std::bind<void(CPU::*)(WORD&)>(&CPU::INC, this, std::ref(m_bc)),			"INC BC",	8, 0 },
		{ 0x04, std::bind(&CPU::INC<BYTE>,		this, std::ref(b)),			"INC B",	4, 0 },
		{ 0x05, std::bind(&CPU::DEC<BYTE>,		this, std::ref(b)),			"DEC B",	4, 0 },
		{ 0x06, std::bind(&CPU::LD<BYTE, BYTE>, 	this, std::ref(b), std::cref(n)), 	"LD B, n",	8, 1 },
		{ 0x07, std::bind(&CPU::RLCA,			this),					"RLCA",		4, 0 },
		{ 0x08, std::bind(&CPU::LD<MemRef, WORD>,	this, MemRef{nn, m_mmu}, std::cref(m_sp)),	"LD (nn), SP",	20, 2 },
		{ 0x09, std::bind<void(CPU::*)(WORD&, const WORD&)>(&CPU::ADD, this, std::ref(m_hl), std::cref(m_bc)), "ADD HL, BC", 8, 0 },
		{ 0x0a, std::bind(&CPU::LD<BYTE, MemRef>,	this, std::ref(a), MemRef{m_bc, m_mmu}),	"LD A, (BC)",	8, 0 },
		{ 0x0b, std::bind<void(CPU::*)(WORD&)>(&CPU::DEC, this, std::ref(m_bc)), 			"DEC BC", 	8, 0 },
		{ 0x0c, std::bind(&CPU::INC<BYTE>,		this, std::ref(c)),			"INC C",	4, 0 },
		{ 0x0d, std::bind(&CPU::DEC<BYTE>,		this, std::ref(c)),			"DEC C",	4, 0 },
		{ 0x0e, std::bind(&CPU::LD<BYTE, BYTE>, 	this, std::ref(c), std::cref(n)), 	"LD C, n", 	8, 1 },
		{ 0x0f, std::bind(&CPU::RRCA,			this),					"RRCA",		4, 0 },

		{},
		{ 0x11, std::bind(&CPU::LD<WORD, WORD>, 	this, std::ref(m_de), std::cref(nn)), 	"LD DE, nn", 	12, 2 },
		{ 0x12, std::bind(&CPU::LD<MemRef, BYTE>,	this, MemRef{m_de, m_mmu}, std::cref(a)),	"LD (DE), A",	8, 0 },
		{ 0x13, std::bind<void(CPU::*)(WORD&)>(&CPU::INC, this, std::ref(m_de)), 			"INC DE", 	8, 0 },
		{ 0x14, std::bind(&CPU::INC<BYTE>,		this, std::ref(d)),			"INC D",	4, 0 },
		{ 0x15, std::bind(&CPU::DEC<BYTE>,		this, std::ref(d)),			"DEC D",	4, 0 },
		{ 0x16, std::bind(&CPU::LD<BYTE, BYTE>, 	this, std::ref(d), std::cref(n)), 	"LD D, n", 	8, 1 },
		{ 0x17, std::bind(&CPU::RLA,			this),					"RLA",		4, 0 },
		{ 0x18, std::bind(&CPU::JR,			this, true, std::cref(n)),		"JR n",		0, 0 },
		{ 0x19, std::bind<void(CPU::*)(WORD&, const WORD&)>(&CPU::ADD, this, std::ref(m_hl), std::cref(m_de)), "ADD HL, DE", 8, 0 },
		{ 0x1a, std::bind(&CPU::LD<BYTE, MemRef>,	this, std::ref(a), MemRef{m_de, m_mmu}),	"LD A, (DE)",	8, 0 },
		{ 0x1b, std::bind<void(CPU::*)(WORD&)>(&CPU::DEC, this, std::ref(m_de)), 			"DEC DE", 	8, 0 },
		{ 0x1c, std::bind(&CPU::INC<BYTE>,		this, std::ref(e)),			"INC E",	4, 0 },
		{ 0x1d, std::bind(&CPU::DEC<BYTE>,		this, std::ref(e)),			"DEC E",	4, 0 },
		{ 0x1e, std::bind(&CPU::LD<BYTE, BYTE>, 	this, std::ref(e), std::cref(n)), 	"LD E, n", 	8, 1 },
		{ 0x1f, std::bind(&CPU::RRA,			this),					"RRA",		4, 0 },

		{ 0x20, std::bind(&CPU::JRn,			this, m_zeroFlag, std::cref(n)),		"JR NZ, n",	0, 0 },
		{ 0x21, std::bind(&CPU::LD<WORD, WORD>, 	this, std::ref(m_hl), std::cref(nn)), 	"LD HL, nn", 	12, 2 },
		{ 0x22, std::bind(&CPU::LDI<MemRef, BYTE>,	this, MemRef{m_hl, m_mmu}, std::cref(a)),	"LDI (HL+), A", 8, 0 },
		{ 0x23, std::bind<void(CPU::*)(WORD&)>(&CPU::INC, this, std::ref(m_hl)), 			"INC HL", 	8, 0 },
		{ 0x24, std::bind(&CPU::INC<BYTE>,		this, std::ref(h)),			"INC H",	4, 0 },
		{ 0x25, std::bind(&CPU::DEC<BYTE>,		this, std::ref(h)),			"DEC H",	4, 0 },
		{ 0x26, std::bind(&CPU::LD<BYTE, BYTE>, 	this, std::ref(h), std::cref(n)), 	"LD H, n", 	8, 1 },
		{ 0x27, std::bind(&CPU::DAA,			this),					"DAA",		4, 0 },
		{ 0x28, std::bind(&CPU::JR,			this, m_zeroFlag, std::cref(n)),		"JR Z, n",	0, 0 },
		{ 0x29, std::bind<void(CPU::*)(WORD&, const WORD&)>(&CPU::ADD, this, std::ref(m_hl), std::cref(m_hl)), "ADD HL, HL", 8, 0 },
		{ 0x2a, std::bind(&CPU::LDI<BYTE, MemRef>,	this, std::ref(a), MemRef{m_hl, m_mmu}),	"LDI A, (HL+)", 8, 0 },
		{ 0x2b, std::bind<void(CPU::*)(WORD&)>(&CPU::DEC, this, std::ref(m_hl)), 			"DEC HL", 	8, 0 },
		{ 0x2c, std::bind(&CPU::INC<BYTE>,		this, std::ref(l)),			"INC L",	4, 0 },
		{ 0x2d, std::bind(&CPU::DEC<BYTE>,		this, std::ref(l)),			"DEC L",	4, 0 },
		{ 0x2e, std::bind(&CPU::LD<BYTE, BYTE>, 	this, std::ref(l), std::cref(n)), 	"LD L, n", 	8, 1 },
		{ 0x2f, std::bind(&CPU::CPL,			this),					"CPL",		4, 0 },

		{ 0x30, std::bind(&CPU::JRn,			this, m_carryFlag, std::cref(n)),		"JR NC, n",	0, 0 },
		{ 0x31, std::bind(&CPU::LD<WORD, WORD>, 	this, std::ref(m_sp), std::cref(nn)), 	"LD SP, nn", 	12, 2 },
		{ 0x32, std::bind(&CPU::LDD<MemRef, BYTE>,	this, MemRef{m_hl, m_mmu}, std::cref(a)),	"LDD (HL-), A", 8, 0 },
		{ 0x33, std::bind<void(CPU::*)(WORD&)>(&CPU::INC, this, std::ref(m_sp)), 			"INC SP", 	8, 0 },
		{ 0x34, std::bind(&CPU::INC<MemRef>,		this, MemRef{m_hl, m_mmu}),			"INC (HL)",	12, 0 },
		{ 0x35, std::bind(&CPU::DEC<MemRef>,		this, MemRef{m_hl, m_mmu}),			"DEC (HL)",	12, 0 },
		{ 0x36, std::bind(&CPU::LD<MemRef, BYTE>,	this, MemRef{m_hl, m_mmu}, std::cref(n)),	"LD (HL), N",	12, 1 },
		{ 0x37, std::bind(&CPU::SCF,			this),					"SCF",		4, 0 },
		{ 0x38, std::bind(&CPU::JR,			this, m_carryFlag, std::cref(n)),		"JR C, n",	0, 0 },
		{ 0x39, std::bind<void(CPU::*)(WORD&, const WORD&)>(&CPU::ADD, this, std::ref(m_hl), std::cref(m_sp)), "ADD HL, SP", 8, 0 },
		{ 0x3a, std::bind(&CPU::LDD<BYTE, MemRef>,	this, std::ref(a), MemRef{m_hl, m_mmu}),	"LDD A, (HL-)", 8, 0 },
		{ 0x3b, std::bind<void(CPU::*)(WORD&)>(&CPU::DEC, this, std::ref(m_sp)), 			"DEC SP", 	8, 0 },
		{ 0x3c, std::bind(&CPU::INC<BYTE>,		this, std::ref(a)),			"INC A",	4, 0 },
		{ 0x3d, std::bind(&CPU::DEC<BYTE>,		this, std::ref(a)),			"DEC A",	4, 0 },
		{ 0x3e, std::bind(&CPU::LD<BYTE, BYTE>, 	this, std::ref(a), std::cref(n)), 	"LD A, n", 	8, 1 },
		{ 0x3f, std::bind(&CPU::CCF,			this),					"CCF",		4, 0 },

		{ 0x40, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), std::cref(b)),	"LD B, B",	4, 0 },
		{ 0x41, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), std::cref(c)),	"LD B, C",	4, 0 },
		{ 0x42, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), std::cref(d)),	"LD B, D",	4, 0 },
		{ 0x43, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), std::cref(e)),	"LD B, E",	4, 0 },
		{ 0x44, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), std::cref(h)),	"LD B, H",	4, 0 },
		{ 0x45, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), std::cref(l)),	"LD B, L",	4, 0 },
		{ 0x46, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), MemRef{m_hl, m_mmu}),	"LD B, (HL)",	8, 0 },
		{ 0x47, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(b), std::cref(a)),	"LD B, A",	4, 0 },
		{ 0x48, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(c), std::cref(b)),	"LD C, B",	4, 0 },
		{ 0x49, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(c), std::cref(c)),	"LD C, C",	4, 0 },
		{ 0x4a, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(c), std::cref(d)),	"LD C, D",	4, 0 },
		{ 0x4b, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(c), std::cref(e)),	"LD C, E",	4, 0 },
		{ 0x4c, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(c), std::cref(h)),	"LD C, H",	4, 0 },
		{ 0x4d, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(c), std::cref(l)),	"LD C, L",	4, 0 },
		{ 0x4e, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(c), MemRef{m_hl, m_mmu}),	"LD C, (HL)",	8, 0 },
		{ 0x4f, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(c), std::cref(a)),	"LD C, A",	4, 0 },

		{ 0x50, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(d), std::cref(b)),	"LD D, B",	4, 0 },
		{ 0x51, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(d), std::cref(c)),	"LD D, C",	4, 0 },
		{ 0x52, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(d), std::cref(d)),	"LD D, D",	4, 0 },
		{ 0x53, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(d), std::cref(e)),	"LD D, E",	4, 0 },
		{ 0x54, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(d), std::cref(h)),	"LD D, H",	4, 0 },
		{ 0x55, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(d), std::cref(l)),	"LD D, L",	4, 0 },
		{ 0x56, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(d), MemRef{m_hl, m_mmu}),	"LD D, (HL)",	8, 0 },
		{ 0x57, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(d), std::cref(a)),	"LD D, A",	4, 0 },
		{ 0x58, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(e), std::cref(b)),	"LD E, B",	4, 0 },
		{ 0x59, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(e), std::cref(c)),	"LD E, C",	4, 0 },
		{ 0x5a, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(e), std::cref(d)),	"LD E, D",	4, 0 },
		{ 0x5b, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(e), std::cref(e)),	"LD E, E",	4, 0 },
		{ 0x5c, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(e), std::cref(h)),	"LD E, H",	4, 0 },
		{ 0x5d, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(e), std::cref(l)),	"LD E, L",	4, 0 },
		{ 0x5e, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(e), MemRef{m_hl, m_mmu}),	"LD E, (HL)",	8, 0 },
		{ 0x5f, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(e), std::cref(a)),	"LD E, A",	4, 0 },

		{ 0x60, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(h), std::cref(b)),	"LD H, B",	4, 0 },
		{ 0x61, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(h), std::cref(c)),	"LD H, C",	4, 0 },
		{ 0x62, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(h), std::cref(d)),	"LD H, D",	4, 0 },
		{ 0x63, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(h), std::cref(e)),	"LD H, E",	4, 0 },
		{ 0x64, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(h), std::cref(h)),	"LD H, H",	4, 0 },
		{ 0x65, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(h), std::cref(l)),	"LD H, L",	4, 0 },
		{ 0x66, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(h), MemRef{m_hl, m_mmu}),	"LD H, (HL)",	8, 0 },
		{ 0x67, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(h), std::cref(a)),	"LD H, A",	4, 0 },
		{ 0x68, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(l), std::cref(b)),	"LD L, B",	4, 0 },
		{ 0x69, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(l), std::cref(c)),	"LD L, C",	4, 0 },
		{ 0x6a, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(l), std::cref(d)),	"LD L, D",	4, 0 },
		{ 0x6b, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(l), std::cref(e)),	"LD L, E",	4, 0 },
		{ 0x6c, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(l), std::cref(h)),	"LD L, H",	4, 0 },
		{ 0x6d, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(l), std::cref(l)),	"LD L, L",	4, 0 },
		{ 0x6e, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(l), MemRef{m_hl, m_mmu}),	"LD L, (HL)",	8, 0 },
		{ 0x6f, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(l), std::cref(a)),	"LD L, A",	4, 0 },

		{ 0x70, std::bind(&CPU::LD<MemRef, BYTE>, this, MemRef{m_hl, m_mmu}, std::cref(b)),	"LD (HL), B",	8, 0 },
		{ 0x71, std::bind(&CPU::LD<MemRef, BYTE>, this, MemRef{m_hl, m_mmu}, std::cref(c)),	"LD (HL), C",	8, 0 },
		{ 0x72, std::bind(&CPU::LD<MemRef, BYTE>, this, MemRef{m_hl, m_mmu}, std::cref(d)),	"LD (HL), D",	8, 0 },
		{ 0x73, std::bind(&CPU::LD<MemRef, BYTE>, this, MemRef{m_hl, m_mmu}, std::cref(e)),	"LD (HL), E",	8, 0 },
		{ 0x74, std::bind(&CPU::LD<MemRef, BYTE>, this, MemRef{m_hl, m_mmu}, std::cref(h)),	"LD (HL), H",	8, 0 },
		{ 0x75, std::bind(&CPU::LD<MemRef, BYTE>, this, MemRef{m_hl, m_mmu}, std::cref(l)),	"LD (HL), L",	8, 0 },
		{}, // 0x76
		{ 0x77, std::bind(&CPU::LD<MemRef, BYTE>, this, MemRef{m_hl, m_mmu}, std::cref(a)),	"LD (HL), A",	8, 0 },
		{ 0x78, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(a), std::cref(b)),	"LD A, B",	4, 0 },
		{ 0x79, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(a), std::cref(c)),	"LD A, C",	4, 0 },
		{ 0x7a, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(a), std::cref(d)),	"LD A, D",	4, 0 },
		{ 0x7b, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(a), std::cref(e)),	"LD A, E",	4, 0 },
		{ 0x7c, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(a), std::cref(h)),	"LD A, H",	4, 0 },
		{ 0x7d, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(a), std::cref(l)),	"LD A, L",	4, 0 },
		{ 0x7e, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(a), MemRef{m_hl, m_mmu}),	"LD A, (HL)",	8, 0 },
		{ 0x7f, std::bind(&CPU::LD<BYTE, BYTE>,	this, std::ref(a), std::cref(a)),	"LD A, A",	4, 0 },

		{ 0x80, std::bind<void(CPU::*)(const BYTE&)>(&CPU::ADD,	this, std::cref(b)),		"ADD A, B",	4, 0 },
		{ 0x81, std::bind<void(CPU::*)(const BYTE&)>(&CPU::ADD,	this, std::cref(c)),		"ADD A, C",	4, 0 },
		{ 0x82, std::bind<void(CPU::*)(const BYTE&)>(&CPU::ADD,	this, std::cref(d)),		"ADD A, D",	4, 0 },
		{ 0x83, std::bind<void(CPU::*)(const BYTE&)>(&CPU::ADD,	this, std::cref(e)),		"ADD A, E",	4, 0 },
		{ 0x84, std::bind<void(CPU::*)(const BYTE&)>(&CPU::ADD,	this, std::cref(h)),		"ADD A, H",	4, 0 },
		{ 0x85, std::bind<void(CPU::*)(const BYTE&)>(&CPU::ADD,	this, std::cref(l)),		"ADD A, L",	4, 0 },
		{ 0x86, std::bind<void(CPU::*)(const BYTE&)>(&CPU::ADD,	this, MemRef{m_hl, m_mmu}),		"ADD A, (HL)",	8, 0 },
		{ 0x87, std::bind<void(CPU::*)(const BYTE&)>(&CPU::ADD,	this, std::cref(a)),		"ADD A, A",	4, 0 },
		{ 0x88, std::bind(&CPU::ADC,		this, std::cref(b)),			"ADC A, B",	4, 0 },
		{ 0x89, std::bind(&CPU::ADC,		this, std::cref(c)),			"ADC A, C",	4, 0 },
		{ 0x8a, std::bind(&CPU::ADC,		this, std::cref(d)),			"ADC A, D",	4, 0 },
		{ 0x8b, std::bind(&CPU::ADC,		this, std::cref(e)),			"ADC A, E",	4, 0 },
		{ 0x8c, std::bind(&CPU::ADC,		this, std::cref(h)),			"ADC A, H",	4, 0 },
		{ 0x8d, std::bind(&CPU::ADC,		this, std::cref(l)),			"ADC A, L",	4, 0 },
		{ 0x8e, std::bind(&CPU::ADC,		this, MemRef{m_hl, m_mmu}),			"ADC A, (HL)",	8, 0 },
		{ 0x8f, std::bind(&CPU::ADC,		this, std::cref(a)),			"ADC A, A",	4, 0 },

		{ 0x90, std::bind(&CPU::SUB,		this, std::cref(b)),			"SUB A, B",	4, 0 },
		{ 0x91, std::bind(&CPU::SUB,		this, std::cref(c)),			"SUB A, C",	4, 0 },
		{ 0x92, std::bind(&CPU::SUB,		this, std::cref(d)),			"SUB A, D",	4, 0 },
		{ 0x93, std::bind(&CPU::SUB,		this, std::cref(e)),			"SUB A, E",	4, 0 },
		{ 0x94, std::bind(&CPU::SUB,		this, std::cref(h)),			"SUB A, H",	4, 0 },
		{ 0x95, std::bind(&CPU::SUB,		this, std::cref(l)),			"SUB A, L",	4, 0 },
		{ 0x96, std::bind(&CPU::SUB,		this, MemRef{m_hl, m_mmu}),			"SUB A, (HL)",	8, 0 },
		{ 0x97, std::bind(&CPU::SUB,		this, std::cref(a)),			"SUB A, A",	4, 0 },
		{ 0x98, std::bind(&CPU::SBC,		this, std::cref(b)),			"SBC A, B",	4, 0 },
		{ 0x99, std::bind(&CPU::SBC,		this, std::cref(c)),			"SBC A, C",	4, 0 },
		{ 0x9a, std::bind(&CPU::SBC,		this, std::cref(d)),			"SBC A, D",	4, 0 },
		{ 0x9b, std::bind(&CPU::SBC,		this, std::cref(e)),			"SBC A, E",	4, 0 },
		{ 0x9c, std::bind(&CPU::SBC,		this, std::cref(h)),			"SBC A, H",	4, 0 },
		{ 0x9d, std::bind(&CPU::SBC,		this, std::cref(l)),			"SBC A, L",	4, 0 },
		{ 0x9e, std::bind(&CPU::SBC,		this, MemRef{m_hl, m_mmu}),			"SBC A, (HL)",	8, 0 },
		{ 0x9f, std::bind(&CPU::SBC,		this, std::cref(a)),			"SBC A, A",	4, 0 },

		{ 0xa0, std::bind(&CPU::AND,		this, std::cref(b)),			"AND A, B",	4, 0 },
		{ 0xa1, std::bind(&CPU::AND,		this, std::cref(c)),			"AND A, C",	4, 0 },
		{ 0xa2, std::bind(&CPU::AND,		this, std::cref(d)),			"AND A, D",	4, 0 },
		{ 0xa3, std::bind(&CPU::AND,		this, std::cref(e)),			"AND A, E",	4, 0 },
		{ 0xa4, std::bind(&CPU::AND,		this, std::cref(h)),			"AND A, H",	4, 0 },
		{ 0xa5, std::bind(&CPU::AND,		this, std::cref(l)),			"AND A, L",	4, 0 },
		{ 0xa6, std::bind(&CPU::AND,		this, MemRef{m_hl, m_mmu}),			"AND A, (HL)",	8, 0 },
		{ 0xa7, std::bind(&CPU::AND,		this, std::cref(a)),			"AND A, A",	4, 0 },
		{ 0xa8, std::bind(&CPU::XOR,		this, std::cref(b)),			"XOR A, B",	4, 0 },
		{ 0xa9, std::bind(&CPU::XOR,		this, std::cref(c)),			"XOR A, C",	4, 0 },
		{ 0xaa, std::bind(&CPU::XOR,		this, std::cref(d)),			"XOR A, D",	4, 0 },
		{ 0xab, std::bind(&CPU::XOR,		this, std::cref(e)),			"XOR A, E",	4, 0 },
		{ 0xac, std::bind(&CPU::XOR,		this, std::cref(h)),			"XOR A, H",	4, 0 },
		{ 0xad, std::bind(&CPU::XOR,		this, std::cref(l)),			"XOR A, L",	4, 0 },
		{ 0xae, std::bind(&CPU::XOR,		this, MemRef{m_hl, m_mmu}),			"XOR A, (HL)",	8, 0 },
		{ 0xaf, std::bind(&CPU::XOR,		this, std::cref(a)),			"XOR A, A",	4, 0 },

		{ 0xb0, std::bind(&CPU::OR,		this, std::cref(b)),			"OR A, B",	4, 0 },
		{ 0xb1, std::bind(&CPU::OR,		this, std::cref(c)),			"OR A, C",	4, 0 },
		{ 0xb2, std::bind(&CPU::OR,		this, std::cref(d)),			"OR A, D",	4, 0 },
		{ 0xb3, std::bind(&CPU::OR,		this, std::cref(e)),			"OR A, E",	4, 0 },
		{ 0xb4, std::bind(&CPU::OR,		this, std::cref(h)),			"OR A, H",	4, 0 },
		{ 0xb5, std::bind(&CPU::OR,		this, std::cref(l)),			"OR A, L",	4, 0 },
		{ 0xb6, std::bind(&CPU::OR,		this, MemRef{m_hl, m_mmu}),			"OR A, (HL)",	8, 0 },
		{ 0xb7, std::bind(&CPU::OR,		this, std::cref(a)),			"OR A, A",	4, 0 },
		{ 0xb8, std::bind(&CPU::CP,		this, std::cref(b)),			"CP A, B",	4, 0 },
		{ 0xb9, std::bind(&CPU::CP,		this, std::cref(c)),			"CP A, C",	4, 0 },
		{ 0xba, std::bind(&CPU::CP,		this, std::cref(d)),			"CP A, D",	4, 0 },
		{ 0xbb, std::bind(&CPU::CP,		this, std::cref(e)),			"CP A, E",	4, 0 },
		{ 0xbc, std::bind(&CPU::CP,		this, std::cref(h)),			"CP A, H",	4, 0 },
		{ 0xbd, std::bind(&CPU::CP,		this, std::cref(l)),			"CP A, L",	4, 0 },
		{ 0xbe, std::bind(&CPU::CP,		this, MemRef{m_hl, m_mmu}),			"CP A, (HL)",	8, 0 },
		{ 0xbf, std::bind(&CPU::CP,		this, std::cref(a)),			"CP A, A",	4, 0 },
		
		{ 0xc0, std::bind(&CPU::RETncond,	this, m_zeroFlag),			"RET NZ",	0, 0 },
		{ 0xc1, std::bind(&CPU::POP,		this, std::ref(m_bc)),			"POP BC",	12, 0 },
		{ 0xc2, std::bind(&CPU::JPn,		this, m_zeroFlag,	std::cref(nn)),		"JP NZ, nn",	0, 0 },
		{ 0xc3, std::bind(&CPU::JP,		this, true, std::cref(nn)),		"JP nn",	0, 0 },
		{}, // 0xc4
		{ 0xc5, std::bind(&CPU::PUSH,		this, std::cref(m_bc)),			"PUSH BC",	16, 0 },
		{ 0xc6, std::bind<void(CPU::*)(const BYTE&)>(&CPU::ADD,	this, std::cref(n)),		"ADD A, n",	8, 1 },
		{}, // 0xc7
		{ 0xc8, std::bind(&CPU::RETcond,	this, m_zeroFlag),			"RET Z",	0, 0 },
		{ 0xc9, std::bind(&CPU::RET,		this),					"RET",		16, 0 }, // !!!
		{ 0xca, std::bind(&CPU::JP,		this, m_zeroFlag, std::cref(nn)),		"JP Z, nn",	0, 0 },
		{ 0xcb, std::bind(&CPU::CB,		this),					"CB",		4, 1 },
		{}, // 0xcc
		{ 0xcd, std::bind(&CPU::CALL,		this, std::cref(nn)),			"CALL nn",	24, 0 }, // !!!
		{ 0xce, std::bind(&CPU::ADC,		this, std::cref(n)),			"ADC A, n",	8, 1 },
		{ 0xcf, std::bind(&CPU::RST<0x0008>,	this),					"RST 0x0008",	16, 0 }, // !!!
		
		{ 0xd0, std::bind(&CPU::RETncond,	this, m_carryFlag),			"RET NC",	0, 0 },
		{ 0xd1, std::bind(&CPU::POP,		this, std::ref(m_de)),			"POP DE",	12, 0 },
		{ 0xd2, std::bind(&CPU::JPn,		this, m_carryFlag, std::cref(nn)),	"JP NC, nn",	0, 0 },
		{}, // 0xd3
		{}, // 0xd4
		{ 0xd5, std::bind(&CPU::PUSH,		this, std::cref(m_de)),			"PUSH DE",	16, 0 },
		{ 0xd6, std::bind(&CPU::SUB,		this, std::cref(n)),			"SUB A, n",	8, 1 },
		{}, // 0xd7
		{ 0xd8, std::bind(&CPU::RETcond,	this, m_carryFlag),			"RET C",	0, 0 },
		{ 0xd9, std::bind(&CPU::RETI,		this),					"RETI",		16, 0 },
		{ 0xda, std::bind(&CPU::JP,		this, m_carryFlag, std::cref(nn)),	"JP C, nn", 	0, 0 },
		{}, // 0xdb
		{}, // 0xdc
		{}, // 0xdd
		{ 0xde, std::bind(&CPU::SBC,		this, std::cref(n)),			"SBC A, n",	8, 1 },
		{ 0xdf, std::bind(&CPU::RST<0x0018>,	this),					"RST 0x0018",	16, 0 }, // !!!
		
		{ 0xe0, std::bind(&CPU::LD<OffsetRef<0xff00>, BYTE>, this, OffsetRef<0xff00>{n, m_mmu}, std::cref(a)), "LD (N+0xff00), A", 12, 1 },
		{ 0xe1, std::bind(&CPU::POP,		this, std::ref(m_hl)),			"POP HL",	12, 0 },
		{ 0xe2, std::bind(&CPU::LD<OffsetRef<0xff00>, BYTE>, this, OffsetRef<0xff00>{c, m_mmu}, std::cref(a)), "LD (C+0xff00), A", 8, 0 },
		{}, // 0xe3
		{}, // 0xe4
		{ 0xe5, std::bind(&CPU::PUSH,		this, std::cref(m_hl)),			"PUSH HL",	16, 0 },
		{ 0xe6, std::bind(&CPU::AND,		this, std::cref(n)),			"AND A, n",	8, 1 },
		{}, // 0xe7
		{ 0xe8, std::bind<void(CPU::*)()>(&CPU::ADD, this), "ADD SP, n", 16, 1 },
		{ 0xe9, std::bind(&CPU::JP,		this, true, std::cref(m_hl)),		"JP HL",	4, 0 }, // !!! docs say (HL) but this is wrong (and makes little sense)
		{ 0xea, std::bind(&CPU::LD<MemRef, BYTE>, this, MemRef{nn, m_mmu}, std::cref(a)),	"LD (nn), A",	16, 2 },
		{}, // 0xeb
		{}, // 0xec
		{}, // 0xed
		{ 0xee, std::bind(&CPU::XOR,		this, std::cref(n)),			"XOR A, n",	8, 1 },
		{ 0xef, std::bind(&CPU::RST<0x0028>,	this),					"RST 0x0028",	16, 0 }, // !!!

		{ 0xf0, std::bind(&CPU::LD<BYTE, OffsetRef<0xff00>>, this, std::ref(a), OffsetRef<0xff00>{n, m_mmu}), "LD A, (N+0xff00)", 12, 1 },
		{ 0xf1, std::bind(&CPU::POP,		this, std::ref(m_af)),			"POP AF",	12, 0 },
		{ 0xf2, std::bind(&CPU::LD<BYTE, OffsetRef<0xff00>>, this, std::ref(c), OffsetRef<0xff00>{c, m_mmu}), "LD A, (C+0xff00)", 8, 0 },
		{ 0xf3, std::bind(&CPU::DI,		this),					"DI",		4, 0 },
		{}, // 0xf4
		{ 0xf5, std::bind(&CPU::PUSH,		this, std::cref(m_af)),			"PUSH AF",	16, 0 },
		{ 0xf6, std::bind(&CPU::OR,		this, std::cref(n)),			"OR A, n",	8, 1 },
		{}, // 0xf7
		{ 0xf8, std::bind(&CPU::LDadd,		this),					"LD HL, SP+n",	12, 1 },
		{}, // 0xf9
		{ 0xfa, std::bind(&CPU::LD<BYTE, MemRef>, this, std::ref(a), MemRef{nn, m_mmu}),	"LD A, (nn)",	16, 2 },
		{ 0xfb, std::bind(&CPU::EI,		this),					"EI",		4, 0 },
		{}, // 0xfc
		{}, // 0xfd
		{ 0xfe, std::bind(&CPU::CP,		this, std::cref(n)),			"CP A, n",	8, 1 },
		{ 0xff, std::bind(&CPU::RST<0x0038>,	this),					"RST 0x0038",	16, 0 }, // !!!
	}};

	m_extended = {{
		{ 0x00, std::bind(&CPU::RLC<BYTE>, this, std::ref(b)),		"RLC B", 8, 0 },
		{ 0x01, std::bind(&CPU::RLC<BYTE>, this, std::ref(c)),		"RLC C", 8, 0 },
		{ 0x02, std::bind(&CPU::RLC<BYTE>, this, std::ref(d)),		"RLC D", 8, 0 },
		{ 0x03, std::bind(&CPU::RLC<BYTE>, this, std::ref(e)),		"RLC E", 8, 0 },
		{ 0x04, std::bind(&CPU::RLC<BYTE>, this, std::ref(h)),		"RLC H", 8, 0 },
		{ 0x05, std::bind(&CPU::RLC<BYTE>, this, std::ref(l)),		"RLC L", 8, 0 },
		{ 0x06, std::bind(&CPU::RLC<MemRef>, this, MemRef{m_hl, m_mmu}),	"RLC (HL)", 16, 0 },
		{ 0x07, std::bind(&CPU::RLC<BYTE>, this, std::ref(a)),		"RLC A", 8, 0 },
		{ 0x08, std::bind(&CPU::RRC<BYTE>, this, std::ref(b)),		"RRC B", 8, 0 },
		{ 0x09, std::bind(&CPU::RRC<BYTE>, this, std::ref(c)),		"RRC C", 8, 0 },
		{ 0x0a, std::bind(&CPU::RRC<BYTE>, this, std::ref(d)),		"RRC D", 8, 0 },
		{ 0x0b, std::bind(&CPU::RRC<BYTE>, this, std::ref(e)),		"RRC E", 8, 0 },
		{ 0x0c, std::bind(&CPU::RRC<BYTE>, this, std::ref(h)),		"RRC H", 8, 0 },
		{ 0x0d, std::bind(&CPU::RRC<BYTE>, this, std::ref(l)),		"RRC L", 8, 0 },
		{ 0x0e, std::bind(&CPU::RRC<MemRef>, this, MemRef{m_hl, m_mmu}),	"RRC (HL)", 16, 0 },
		{ 0x0f, std::bind(&CPU::RRC<BYTE>, this, std::ref(a)),		"RRC A", 8, 0 },

		{ 0x10, std::bind(&CPU::RL<BYTE>, this, std::ref(b)),		"RL B", 8, 0 },
		{ 0x11, std::bind(&CPU::RL<BYTE>, this, std::ref(c)),		"RL C", 8, 0 },
		{ 0x12, std::bind(&CPU::RL<BYTE>, this, std::ref(d)),		"RL D", 8, 0 },
		{ 0x13, std::bind(&CPU::RL<BYTE>, this, std::ref(e)),		"RL E", 8, 0 },
		{ 0x14, std::bind(&CPU::RL<BYTE>, this, std::ref(h)),		"RL H", 8, 0 },
		{ 0x15, std::bind(&CPU::RL<BYTE>, this, std::ref(l)),		"RL L", 8, 0 },
		{ 0x16, std::bind(&CPU::RL<MemRef>, this, MemRef{m_hl, m_mmu}),	"RL (HL)", 16, 0 },
		{ 0x17, std::bind(&CPU::RL<BYTE>, this, std::ref(a)),		"RL A", 8, 0 },
		{ 0x18, std::bind(&CPU::RR<BYTE>, this, std::ref(b)),		"RR B", 8, 0 },
		{ 0x19, std::bind(&CPU::RR<BYTE>, this, std::ref(c)),		"RR C", 8, 0 },
		{ 0x1a, std::bind(&CPU::RR<BYTE>, this, std::ref(d)),		"RR D", 8, 0 },
		{ 0x1b, std::bind(&CPU::RR<BYTE>, this, std::ref(e)),		"RR E", 8, 0 },
		{ 0x1c, std::bind(&CPU::RR<BYTE>, this, std::ref(h)),		"RR H", 8, 0 },
		{ 0x1d, std::bind(&CPU::RR<BYTE>, this, std::ref(l)),		"RR L", 8, 0 },
		{ 0x1e, std::bind(&CPU::RR<MemRef>, this, MemRef{m_hl, m_mmu}),	"RR (HL)", 16, 0 },
		{ 0x1f, std::bind(&CPU::RR<BYTE>, this, std::ref(a)),		"RR A", 8, 0 },

		{ 0x20, std::bind(&CPU::SLA<BYTE>, this, std::ref(b)),		"SLA B", 8, 0 },
		{ 0x21, std::bind(&CPU::SLA<BYTE>, this, std::ref(c)),		"SLA C", 8, 0 },
		{ 0x22, std::bind(&CPU::SLA<BYTE>, this, std::ref(d)),		"SLA D", 8, 0 },
		{ 0x23, std::bind(&CPU::SLA<BYTE>, this, std::ref(e)),		"SLA E", 8, 0 },
		{ 0x24, std::bind(&CPU::SLA<BYTE>, this, std::ref(h)),		"SLA H", 8, 0 },
		{ 0x25, std::bind(&CPU::SLA<BYTE>, this, std::ref(l)),		"SLA L", 8, 0 },
		{ 0x26, std::bind(&CPU::SLA<MemRef>, this, MemRef{m_hl, m_mmu}),	"SLA (HL)", 16, 0 },
		{ 0x27, std::bind(&CPU::SLA<BYTE>, this, std::ref(a)),		"SLA A", 8, 0 },
		{ 0x28, std::bind(&CPU::SRA<BYTE>, this, std::ref(b)),		"SRA B", 8, 0 },
		{ 0x29, std::bind(&CPU::SRA<BYTE>, this, std::ref(c)),		"SRA C", 8, 0 },
		{ 0x2a, std::bind(&CPU::SRA<BYTE>, this, std::ref(d)),		"SRA D", 8, 0 },
		{ 0x2b, std::bind(&CPU::SRA<BYTE>, this, std::ref(e)),		"SRA E", 8, 0 },
		{ 0x2c, std::bind(&CPU::SRA<BYTE>, this, std::ref(h)),		"SRA H", 8, 0 },
		{ 0x2d, std::bind(&CPU::SRA<BYTE>, this, std::ref(l)),		"SRA L", 8, 0 },
		{ 0x2e, std::bind(&CPU::SRA<MemRef>, this, MemRef{m_hl, m_mmu}),	"SRA (HL)", 16, 0 },
		{ 0x2f, std::bind(&CPU::SRA<BYTE>, this, std::ref(a)),		"SRA A", 8, 0 },

		{ 0x30, std::bind(&CPU::SWAP<BYTE>, this, std::ref(b)),		"SWAP B", 8, 0 },
		{ 0x31, std::bind(&CPU::SWAP<BYTE>, this, std::ref(c)),		"SWAP C", 8, 0 },
		{ 0x32, std::bind(&CPU::SWAP<BYTE>, this, std::ref(d)),		"SWAP D", 8, 0 },
		{ 0x33, std::bind(&CPU::SWAP<BYTE>, this, std::ref(e)),		"SWAP E", 8, 0 },
		{ 0x34, std::bind(&CPU::SWAP<BYTE>, this, std::ref(h)),		"SWAP H", 8, 0 },
		{ 0x35, std::bind(&CPU::SWAP<BYTE>, this, std::ref(l)),		"SWAP L", 8, 0 },
		{ 0x36, std::bind(&CPU::SWAP<MemRef>, this, MemRef{m_hl, m_mmu}),	"SWAP (HL)", 16, 0 },
		{ 0x37, std::bind(&CPU::SWAP<BYTE>, this, std::ref(a)),		"SWAP A", 8, 0 },
		{ 0x38, std::bind(&CPU::SRL<BYTE>, this, std::ref(b)),		"SRL B", 8, 0 },
		{ 0x39, std::bind(&CPU::SRL<BYTE>, this, std::ref(c)),		"SRL C", 8, 0 },
		{ 0x3a, std::bind(&CPU::SRL<BYTE>, this, std::ref(d)),		"SRL D", 8, 0 },
		{ 0x3b, std::bind(&CPU::SRL<BYTE>, this, std::ref(e)),		"SRL E", 8, 0 },
		{ 0x3c, std::bind(&CPU::SRL<BYTE>, this, std::ref(h)),		"SRL H", 8, 0 },
		{ 0x3d, std::bind(&CPU::SRL<BYTE>, this, std::ref(l)),		"SRL L", 8, 0 },
		{ 0x3e, std::bind(&CPU::SRL<MemRef>, this, MemRef{m_hl, m_mmu}),	"SRL (HL)", 16, 0 },
		{ 0x3f, std::bind(&CPU::SRL<BYTE>, this, std::ref(a)),		"SRL A", 8, 0 },

		{ 0x40, std::bind(&CPU::BIT<BitRef<BYTE, 0>>, this, BitRef<BYTE, 0>{b}), "BIT 0, B", 8, 0 },
		{ 0x41, std::bind(&CPU::BIT<BitRef<BYTE, 0>>, this, BitRef<BYTE, 0>{c}), "BIT 0, C", 8, 0 },
		{ 0x42, std::bind(&CPU::BIT<BitRef<BYTE, 0>>, this, BitRef<BYTE, 0>{d}), "BIT 0, D", 8, 0 },
		{ 0x43, std::bind(&CPU::BIT<BitRef<BYTE, 0>>, this, BitRef<BYTE, 0>{e}), "BIT 0, E", 8, 0 },
		{ 0x44, std::bind(&CPU::BIT<BitRef<BYTE, 0>>, this, BitRef<BYTE, 0>{h}), "BIT 0, H", 8, 0 },
		{ 0x45, std::bind(&CPU::BIT<BitRef<BYTE, 0>>, this, BitRef<BYTE, 0>{l}), "BIT 0, L", 8, 0 },
		{ 0x46, std::bind(&CPU::BIT<BitRef<MemRef, 0>>, this, BitRef<MemRef, 0>{MemRef{m_hl, m_mmu}}), "BIT 0, (HL)", 16, 0 },
		{ 0x47, std::bind(&CPU::BIT<BitRef<BYTE, 0>>, this, BitRef<BYTE, 0>{a}), "BIT 0, A", 8, 0 },
		{ 0x48, std::bind(&CPU::BIT<BitRef<BYTE, 1>>, this, BitRef<BYTE, 1>{b}), "BIT 1, B", 8, 0 },
		{ 0x49, std::bind(&CPU::BIT<BitRef<BYTE, 1>>, this, BitRef<BYTE, 1>{c}), "BIT 1, C", 8, 0 },
		{ 0x4a, std::bind(&CPU::BIT<BitRef<BYTE, 1>>, this, BitRef<BYTE, 1>{d}), "BIT 1, D", 8, 0 },
		{ 0x4b, std::bind(&CPU::BIT<BitRef<BYTE, 1>>, this, BitRef<BYTE, 1>{e}), "BIT 1, E", 8, 0 },
		{ 0x4c, std::bind(&CPU::BIT<BitRef<BYTE, 1>>, this, BitRef<BYTE, 1>{h}), "BIT 1, H", 8, 0 },
		{ 0x4d, std::bind(&CPU::BIT<BitRef<BYTE, 1>>, this, BitRef<BYTE, 1>{l}), "BIT 1, L", 8, 0 },
		{ 0x4e, std::bind(&CPU::BIT<BitRef<MemRef, 1>>, this, BitRef<MemRef, 1>{MemRef{m_hl, m_mmu}}), "BIT 1, (HL)", 16, 0 },
		{ 0x4f, std::bind(&CPU::BIT<BitRef<BYTE, 1>>, this, BitRef<BYTE, 1>{a}), "BIT 1, A", 8, 0 },

		{ 0x50, std::bind(&CPU::BIT<BitRef<BYTE, 2>>, this, BitRef<BYTE, 2>{b}), "BIT 2, B", 8, 0 },
		{ 0x51, std::bind(&CPU::BIT<BitRef<BYTE, 2>>, this, BitRef<BYTE, 2>{c}), "BIT 2, C", 8, 0 },
		{ 0x52, std::bind(&CPU::BIT<BitRef<BYTE, 2>>, this, BitRef<BYTE, 2>{d}), "BIT 2, D", 8, 0 },
		{ 0x53, std::bind(&CPU::BIT<BitRef<BYTE, 2>>, this, BitRef<BYTE, 2>{e}), "BIT 2, E", 8, 0 },
		{ 0x54, std::bind(&CPU::BIT<BitRef<BYTE, 2>>, this, BitRef<BYTE, 2>{h}), "BIT 2, H", 8, 0 },
		{ 0x55, std::bind(&CPU::BIT<BitRef<BYTE, 2>>, this, BitRef<BYTE, 2>{l}), "BIT 2, L", 8, 0 },
		{ 0x56, std::bind(&CPU::BIT<BitRef<MemRef, 2>>, this, BitRef<MemRef, 2>{MemRef{m_hl, m_mmu}}), "BIT 2, (HL)", 16, 0 },
		{ 0x57, std::bind(&CPU::BIT<BitRef<BYTE, 2>>, this, BitRef<BYTE, 2>{a}), "BIT 2, A", 8, 0 },
		{ 0x58, std::bind(&CPU::BIT<BitRef<BYTE, 3>>, this, BitRef<BYTE, 3>{b}), "BIT 3, B", 8, 0 },
		{ 0x59, std::bind(&CPU::BIT<BitRef<BYTE, 3>>, this, BitRef<BYTE, 3>{c}), "BIT 3, C", 8, 0 },
		{ 0x5a, std::bind(&CPU::BIT<BitRef<BYTE, 3>>, this, BitRef<BYTE, 3>{d}), "BIT 3, D", 8, 0 },
		{ 0x5b, std::bind(&CPU::BIT<BitRef<BYTE, 3>>, this, BitRef<BYTE, 3>{e}), "BIT 3, E", 8, 0 },
		{ 0x5c, std::bind(&CPU::BIT<BitRef<BYTE, 3>>, this, BitRef<BYTE, 3>{h}), "BIT 3, H", 8, 0 },
		{ 0x5d, std::bind(&CPU::BIT<BitRef<BYTE, 3>>, this, BitRef<BYTE, 3>{l}), "BIT 3, L", 8, 0 },
		{ 0x5e, std::bind(&CPU::BIT<BitRef<MemRef, 3>>, this, BitRef<MemRef, 3>{MemRef{m_hl, m_mmu}}), "BIT 3, (HL)", 16, 0 },
		{ 0x5f, std::bind(&CPU::BIT<BitRef<BYTE, 3>>, this, BitRef<BYTE, 3>{a}), "BIT 3, A", 8, 0 },

		{ 0x60, std::bind(&CPU::BIT<BitRef<BYTE, 4>>, this, BitRef<BYTE, 4>{b}), "BIT 4, B", 8, 0 },
		{ 0x61, std::bind(&CPU::BIT<BitRef<BYTE, 4>>, this, BitRef<BYTE, 4>{c}), "BIT 4, C", 8, 0 },
		{ 0x62, std::bind(&CPU::BIT<BitRef<BYTE, 4>>, this, BitRef<BYTE, 4>{d}), "BIT 4, D", 8, 0 },
		{ 0x63, std::bind(&CPU::BIT<BitRef<BYTE, 4>>, this, BitRef<BYTE, 4>{e}), "BIT 4, E", 8, 0 },
		{ 0x64, std::bind(&CPU::BIT<BitRef<BYTE, 4>>, this, BitRef<BYTE, 4>{h}), "BIT 4, H", 8, 0 },
		{ 0x65, std::bind(&CPU::BIT<BitRef<BYTE, 4>>, this, BitRef<BYTE, 4>{l}), "BIT 4, L", 8, 0 },
		{ 0x66, std::bind(&CPU::BIT<BitRef<MemRef, 4>>, this, BitRef<MemRef, 4>{MemRef{m_hl, m_mmu}}), "BIT 4, (HL)", 16, 0 },
		{ 0x67, std::bind(&CPU::BIT<BitRef<BYTE, 4>>, this, BitRef<BYTE, 4>{a}), "BIT 4, A", 8, 0 },
		{ 0x68, std::bind(&CPU::BIT<BitRef<BYTE, 5>>, this, BitRef<BYTE, 5>{b}), "BIT 5, B", 8, 0 },
		{ 0x69, std::bind(&CPU::BIT<BitRef<BYTE, 5>>, this, BitRef<BYTE, 5>{c}), "BIT 5, C", 8, 0 },
		{ 0x6a, std::bind(&CPU::BIT<BitRef<BYTE, 5>>, this, BitRef<BYTE, 5>{d}), "BIT 5, D", 8, 0 },
		{ 0x6b, std::bind(&CPU::BIT<BitRef<BYTE, 5>>, this, BitRef<BYTE, 5>{e}), "BIT 5, E", 8, 0 },
		{ 0x6c, std::bind(&CPU::BIT<BitRef<BYTE, 5>>, this, BitRef<BYTE, 5>{h}), "BIT 5, H", 8, 0 },
		{ 0x6d, std::bind(&CPU::BIT<BitRef<BYTE, 5>>, this, BitRef<BYTE, 5>{l}), "BIT 5, L", 8, 0 },
		{ 0x6e, std::bind(&CPU::BIT<BitRef<MemRef, 5>>, this, BitRef<MemRef, 5>{MemRef{m_hl, m_mmu}}), "BIT 5, (HL)", 16, 0 },
		{ 0x6f, std::bind(&CPU::BIT<BitRef<BYTE, 5>>, this, BitRef<BYTE, 5>{a}), "BIT 5, A", 8, 0 },

		{ 0x70, std::bind(&CPU::BIT<BitRef<BYTE, 6>>, this, BitRef<BYTE, 6>{b}), "BIT 6, B", 8, 0 },
		{ 0x71, std::bind(&CPU::BIT<BitRef<BYTE, 6>>, this, BitRef<BYTE, 6>{c}), "BIT 6, C", 8, 0 },
		{ 0x72, std::bind(&CPU::BIT<BitRef<BYTE, 6>>, this, BitRef<BYTE, 6>{d}), "BIT 6, D", 8, 0 },
		{ 0x73, std::bind(&CPU::BIT<BitRef<BYTE, 6>>, this, BitRef<BYTE, 6>{e}), "BIT 6, E", 8, 0 },
		{ 0x74, std::bind(&CPU::BIT<BitRef<BYTE, 6>>, this, BitRef<BYTE, 6>{h}), "BIT 6, H", 8, 0 },
		{ 0x75, std::bind(&CPU::BIT<BitRef<BYTE, 6>>, this, BitRef<BYTE, 6>{l}), "BIT 6, L", 8, 0 },
		{ 0x76, std::bind(&CPU::BIT<BitRef<MemRef, 6>>, this, BitRef<MemRef, 6>{MemRef{m_hl, m_mmu}}), "BIT 6, (HL)", 16, 0 },
		{ 0x77, std::bind(&CPU::BIT<BitRef<BYTE, 6>>, this, BitRef<BYTE, 6>{a}), "BIT 6, A", 8, 0 },
		{ 0x78, std::bind(&CPU::BIT<BitRef<BYTE, 7>>, this, BitRef<BYTE, 7>{b}), "BIT 7, B", 8, 0 },
		{ 0x79, std::bind(&CPU::BIT<BitRef<BYTE, 7>>, this, BitRef<BYTE, 7>{c}), "BIT 7, C", 8, 0 },
		{ 0x7a, std::bind(&CPU::BIT<BitRef<BYTE, 7>>, this, BitRef<BYTE, 7>{d}), "BIT 7, D", 8, 0 },
		{ 0x7b, std::bind(&CPU::BIT<BitRef<BYTE, 7>>, this, BitRef<BYTE, 7>{e}), "BIT 7, E", 8, 0 },
		{ 0x7c, std::bind(&CPU::BIT<BitRef<BYTE, 7>>, this, BitRef<BYTE, 7>{h}), "BIT 7, H", 8, 0 },
		{ 0x7d, std::bind(&CPU::BIT<BitRef<BYTE, 7>>, this, BitRef<BYTE, 7>{l}), "BIT 7, L", 8, 0 },
		{ 0x7e, std::bind(&CPU::BIT<BitRef<MemRef, 7>>, this, BitRef<MemRef, 7>{MemRef{m_hl, m_mmu}}), "BIT 7, (HL)", 16, 0 },
		{ 0x7f, std::bind(&CPU::BIT<BitRef<BYTE, 7>>, this, BitRef<BYTE, 7>{a}), "BIT 7, A", 8, 0 },

		{ 0x80, std::bind(&CPU::RES<BitRef<BYTE, 0>>, this, BitRef<BYTE, 0>{b}), "RES 0, B", 8, 0 },
		{ 0x81, std::bind(&CPU::RES<BitRef<BYTE, 0>>, this, BitRef<BYTE, 0>{c}), "RES 0, C", 8, 0 },
		{ 0x82, std::bind(&CPU::RES<BitRef<BYTE, 0>>, this, BitRef<BYTE, 0>{d}), "RES 0, D", 8, 0 },
		{ 0x83, std::bind(&CPU::RES<BitRef<BYTE, 0>>, this, BitRef<BYTE, 0>{e}), "RES 0, E", 8, 0 },
		{ 0x84, std::bind(&CPU::RES<BitRef<BYTE, 0>>, this, BitRef<BYTE, 0>{h}), "RES 0, H", 8, 0 },
		{ 0x85, std::bind(&CPU::RES<BitRef<BYTE, 0>>, this, BitRef<BYTE, 0>{l}), "RES 0, L", 8, 0 },
		{ 0x86, std::bind(&CPU::RES<BitRef<MemRef, 0>>, this, BitRef<MemRef, 0>{MemRef{m_hl, m_mmu}}), "RES 0, (HL)", 16, 0 },
		{ 0x87, std::bind(&CPU::RES<BitRef<BYTE, 0>>, this, BitRef<BYTE, 0>{a}), "RES 0, A", 8, 0 },
		{ 0x88, std::bind(&CPU::RES<BitRef<BYTE, 1>>, this, BitRef<BYTE, 1>{b}), "RES 1, B", 8, 0 },
		{ 0x89, std::bind(&CPU::RES<BitRef<BYTE, 1>>, this, BitRef<BYTE, 1>{c}), "RES 1, C", 8, 0 },
		{ 0x8a, std::bind(&CPU::RES<BitRef<BYTE, 1>>, this, BitRef<BYTE, 1>{d}), "RES 1, D", 8, 0 },
		{ 0x8b, std::bind(&CPU::RES<BitRef<BYTE, 1>>, this, BitRef<BYTE, 1>{e}), "RES 1, E", 8, 0 },
		{ 0x8c, std::bind(&CPU::RES<BitRef<BYTE, 1>>, this, BitRef<BYTE, 1>{h}), "RES 1, H", 8, 0 },
		{ 0x8d, std::bind(&CPU::RES<BitRef<BYTE, 1>>, this, BitRef<BYTE, 1>{l}), "RES 1, L", 8, 0 },
		{ 0x8e, std::bind(&CPU::RES<BitRef<MemRef, 1>>, this, BitRef<MemRef, 1>{MemRef{m_hl, m_mmu}}), "RES 1, (HL)", 16, 0 },
		{ 0x8f, std::bind(&CPU::RES<BitRef<BYTE, 1>>, this, BitRef<BYTE, 1>{a}), "RES 1, A", 8, 0 },

		{ 0x90, std::bind(&CPU::RES<BitRef<BYTE, 2>>, this, BitRef<BYTE, 2>{b}), "RES 2, B", 8, 0 },
		{ 0x91, std::bind(&CPU::RES<BitRef<BYTE, 2>>, this, BitRef<BYTE, 2>{c}), "RES 2, C", 8, 0 },
		{ 0x92, std::bind(&CPU::RES<BitRef<BYTE, 2>>, this, BitRef<BYTE, 2>{d}), "RES 2, D", 8, 0 },
		{ 0x93, std::bind(&CPU::RES<BitRef<BYTE, 2>>, this, BitRef<BYTE, 2>{e}), "RES 2, E", 8, 0 },
		{ 0x94, std::bind(&CPU::RES<BitRef<BYTE, 2>>, this, BitRef<BYTE, 2>{h}), "RES 2, H", 8, 0 },
		{ 0x95, std::bind(&CPU::RES<BitRef<BYTE, 2>>, this, BitRef<BYTE, 2>{l}), "RES 2, L", 8, 0 },
		{ 0x96, std::bind(&CPU::RES<BitRef<MemRef, 2>>, this, BitRef<MemRef, 2>{MemRef{m_hl, m_mmu}}), "RES 2, (HL)", 16, 0 },
		{ 0x97, std::bind(&CPU::RES<BitRef<BYTE, 2>>, this, BitRef<BYTE, 2>{a}), "RES 2, A", 8, 0 },
		{ 0x98, std::bind(&CPU::RES<BitRef<BYTE, 3>>, this, BitRef<BYTE, 3>{b}), "RES 3, B", 8, 0 },
		{ 0x99, std::bind(&CPU::RES<BitRef<BYTE, 3>>, this, BitRef<BYTE, 3>{c}), "RES 3, C", 8, 0 },
		{ 0x9a, std::bind(&CPU::RES<BitRef<BYTE, 3>>, this, BitRef<BYTE, 3>{d}), "RES 3, D", 8, 0 },
		{ 0x9b, std::bind(&CPU::RES<BitRef<BYTE, 3>>, this, BitRef<BYTE, 3>{e}), "RES 3, E", 8, 0 },
		{ 0x9c, std::bind(&CPU::RES<BitRef<BYTE, 3>>, this, BitRef<BYTE, 3>{h}), "RES 3, H", 8, 0 },
		{ 0x9d, std::bind(&CPU::RES<BitRef<BYTE, 3>>, this, BitRef<BYTE, 3>{l}), "RES 3, L", 8, 0 },
		{ 0x9e, std::bind(&CPU::RES<BitRef<MemRef, 3>>, this, BitRef<MemRef, 3>{MemRef{m_hl, m_mmu}}), "RES 3, (HL)", 16, 0 },
		{ 0x9f, std::bind(&CPU::RES<BitRef<BYTE, 3>>, this, BitRef<BYTE, 3>{a}), "RES 3, A", 8, 0 },

		{ 0xa0, std::bind(&CPU::RES<BitRef<BYTE, 4>>, this, BitRef<BYTE, 4>{b}), "RES 4, B", 8, 0 },
		{ 0xa1, std::bind(&CPU::RES<BitRef<BYTE, 4>>, this, BitRef<BYTE, 4>{c}), "RES 4, C", 8, 0 },
		{ 0xa2, std::bind(&CPU::RES<BitRef<BYTE, 4>>, this, BitRef<BYTE, 4>{d}), "RES 4, D", 8, 0 },
		{ 0xa3, std::bind(&CPU::RES<BitRef<BYTE, 4>>, this, BitRef<BYTE, 4>{e}), "RES 4, E", 8, 0 },
		{ 0xa4, std::bind(&CPU::RES<BitRef<BYTE, 4>>, this, BitRef<BYTE, 4>{h}), "RES 4, H", 8, 0 },
		{ 0xa5, std::bind(&CPU::RES<BitRef<BYTE, 4>>, this, BitRef<BYTE, 4>{l}), "RES 4, L", 8, 0 },
		{ 0xa6, std::bind(&CPU::RES<BitRef<MemRef, 4>>, this, BitRef<MemRef, 4>{MemRef{m_hl, m_mmu}}), "RES 4, (HL)", 16, 0 },
		{ 0xa7, std::bind(&CPU::RES<BitRef<BYTE, 4>>, this, BitRef<BYTE, 4>{a}), "RES 4, A", 8, 0 },
		{ 0xa8, std::bind(&CPU::RES<BitRef<BYTE, 5>>, this, BitRef<BYTE, 5>{b}), "RES 5, B", 8, 0 },
		{ 0xa9, std::bind(&CPU::RES<BitRef<BYTE, 5>>, this, BitRef<BYTE, 5>{c}), "RES 5, C", 8, 0 },
		{ 0xaa, std::bind(&CPU::RES<BitRef<BYTE, 5>>, this, BitRef<BYTE, 5>{d}), "RES 5, D", 8, 0 },
		{ 0xab, std::bind(&CPU::RES<BitRef<BYTE, 5>>, this, BitRef<BYTE, 5>{e}), "RES 5, E", 8, 0 },
		{ 0xac, std::bind(&CPU::RES<BitRef<BYTE, 5>>, this, BitRef<BYTE, 5>{h}), "RES 5, H", 8, 0 },
		{ 0xad, std::bind(&CPU::RES<BitRef<BYTE, 5>>, this, BitRef<BYTE, 5>{l}), "RES 5, L", 8, 0 },
		{ 0xae, std::bind(&CPU::RES<BitRef<MemRef, 5>>, this, BitRef<MemRef, 5>{MemRef{m_hl, m_mmu}}), "RES 5, (HL)", 16, 0 },
		{ 0xaf, std::bind(&CPU::RES<BitRef<BYTE, 5>>, this, BitRef<BYTE, 5>{a}), "RES 5, A", 8, 0 },

		{ 0xb0, std::bind(&CPU::RES<BitRef<BYTE, 6>>, this, BitRef<BYTE, 6>{b}), "RES 6, B", 8, 0 },
		{ 0xb1, std::bind(&CPU::RES<BitRef<BYTE, 6>>, this, BitRef<BYTE, 6>{c}), "RES 6, C", 8, 0 },
		{ 0xb2, std::bind(&CPU::RES<BitRef<BYTE, 6>>, this, BitRef<BYTE, 6>{d}), "RES 6, D", 8, 0 },
		{ 0xb3, std::bind(&CPU::RES<BitRef<BYTE, 6>>, this, BitRef<BYTE, 6>{e}), "RES 6, E", 8, 0 },
		{ 0xb4, std::bind(&CPU::RES<BitRef<BYTE, 6>>, this, BitRef<BYTE, 6>{h}), "RES 6, H", 8, 0 },
		{ 0xb5, std::bind(&CPU::RES<BitRef<BYTE, 6>>, this, BitRef<BYTE, 6>{l}), "RES 6, L", 8, 0 },
		{ 0xb6, std::bind(&CPU::RES<BitRef<MemRef, 6>>, this, BitRef<MemRef, 6>{MemRef{m_hl, m_mmu}}), "RES 6, (HL)", 16, 0 },
		{ 0xb7, std::bind(&CPU::RES<BitRef<BYTE, 6>>, this, BitRef<BYTE, 6>{a}), "RES 6, A", 8, 0 },
		{ 0xb8, std::bind(&CPU::RES<BitRef<BYTE, 7>>, this, BitRef<BYTE, 7>{b}), "RES 7, B", 8, 0 },
		{ 0xb9, std::bind(&CPU::RES<BitRef<BYTE, 7>>, this, BitRef<BYTE, 7>{c}), "RES 7, C", 8, 0 },
		{ 0xba, std::bind(&CPU::RES<BitRef<BYTE, 7>>, this, BitRef<BYTE, 7>{d}), "RES 7, D", 8, 0 },
		{ 0xbb, std::bind(&CPU::RES<BitRef<BYTE, 7>>, this, BitRef<BYTE, 7>{e}), "RES 7, E", 8, 0 },
		{ 0xbc, std::bind(&CPU::RES<BitRef<BYTE, 7>>, this, BitRef<BYTE, 7>{h}), "RES 7, H", 8, 0 },
		{ 0xbd, std::bind(&CPU::RES<BitRef<BYTE, 7>>, this, BitRef<BYTE, 7>{l}), "RES 7, L", 8, 0 },
		{ 0xbe, std::bind(&CPU::RES<BitRef<MemRef, 7>>, this, BitRef<MemRef, 7>{MemRef{m_hl, m_mmu}}), "RES 7, (HL)", 16, 0 },
		{ 0xbf, std::bind(&CPU::RES<BitRef<BYTE, 7>>, this, BitRef<BYTE, 7>{a}), "RES 7, A", 8, 0 },

		{ 0xc0, std::bind(&CPU::SET<BitRef<BYTE, 0>>, this, BitRef<BYTE, 0>{b}), "SET 0, B", 8, 0 },
		{ 0xc1, std::bind(&CPU::SET<BitRef<BYTE, 0>>, this, BitRef<BYTE, 0>{c}), "SET 0, C", 8, 0 },
		{ 0xc2, std::bind(&CPU::SET<BitRef<BYTE, 0>>, this, BitRef<BYTE, 0>{d}), "SET 0, D", 8, 0 },
		{ 0xc3, std::bind(&CPU::SET<BitRef<BYTE, 0>>, this, BitRef<BYTE, 0>{e}), "SET 0, E", 8, 0 },
		{ 0xc4, std::bind(&CPU::SET<BitRef<BYTE, 0>>, this, BitRef<BYTE, 0>{h}), "SET 0, H", 8, 0 },
		{ 0xc5, std::bind(&CPU::SET<BitRef<BYTE, 0>>, this, BitRef<BYTE, 0>{l}), "SET 0, L", 8, 0 },
		{ 0xc6, std::bind(&CPU::SET<BitRef<MemRef, 0>>, this, BitRef<MemRef, 0>{MemRef{m_hl, m_mmu}}), "SET 0, (HL)", 16, 0 },
		{ 0xc7, std::bind(&CPU::SET<BitRef<BYTE, 0>>, this, BitRef<BYTE, 0>{a}), "SET 0, A", 8, 0 },
		{ 0xc8, std::bind(&CPU::SET<BitRef<BYTE, 1>>, this, BitRef<BYTE, 1>{b}), "SET 1, B", 8, 0 },
		{ 0xc9, std::bind(&CPU::SET<BitRef<BYTE, 1>>, this, BitRef<BYTE, 1>{c}), "SET 1, C", 8, 0 },
		{ 0xca, std::bind(&CPU::SET<BitRef<BYTE, 1>>, this, BitRef<BYTE, 1>{d}), "SET 1, D", 8, 0 },
		{ 0xcb, std::bind(&CPU::SET<BitRef<BYTE, 1>>, this, BitRef<BYTE, 1>{e}), "SET 1, E", 8, 0 },
		{ 0xcc, std::bind(&CPU::SET<BitRef<BYTE, 1>>, this, BitRef<BYTE, 1>{h}), "SET 1, H", 8, 0 },
		{ 0xcd, std::bind(&CPU::SET<BitRef<BYTE, 1>>, this, BitRef<BYTE, 1>{l}), "SET 1, L", 8, 0 },
		{ 0xce, std::bind(&CPU::SET<BitRef<MemRef, 1>>, this, BitRef<MemRef, 1>{MemRef{m_hl, m_mmu}}), "SET 1, (HL)", 16, 0 },
		{ 0xcf, std::bind(&CPU::SET<BitRef<BYTE, 1>>, this, BitRef<BYTE, 1>{a}), "SET 1, A", 8, 0 },

		{ 0xd0, std::bind(&CPU::SET<BitRef<BYTE, 2>>, this, BitRef<BYTE, 2>{b}), "SET 2, B", 8, 0 },
		{ 0xd1, std::bind(&CPU::SET<BitRef<BYTE, 2>>, this, BitRef<BYTE, 2>{c}), "SET 2, C", 8, 0 },
		{ 0xd2, std::bind(&CPU::SET<BitRef<BYTE, 2>>, this, BitRef<BYTE, 2>{d}), "SET 2, D", 8, 0 },
		{ 0xd3, std::bind(&CPU::SET<BitRef<BYTE, 2>>, this, BitRef<BYTE, 2>{e}), "SET 2, E", 8, 0 },
		{ 0xd4, std::bind(&CPU::SET<BitRef<BYTE, 2>>, this, BitRef<BYTE, 2>{h}), "SET 2, H", 8, 0 },
		{ 0xd5, std::bind(&CPU::SET<BitRef<BYTE, 2>>, this, BitRef<BYTE, 2>{l}), "SET 2, L", 8, 0 },
		{ 0xd6, std::bind(&CPU::SET<BitRef<MemRef, 2>>, this, BitRef<MemRef, 2>{MemRef{m_hl, m_mmu}}), "SET 2, (HL)", 16, 0 },
		{ 0xd7, std::bind(&CPU::SET<BitRef<BYTE, 2>>, this, BitRef<BYTE, 2>{a}), "SET 2, A", 8, 0 },
		{ 0xd8, std::bind(&CPU::SET<BitRef<BYTE, 3>>, this, BitRef<BYTE, 3>{b}), "SET 3, B", 8, 0 },
		{ 0xd9, std::bind(&CPU::SET<BitRef<BYTE, 3>>, this, BitRef<BYTE, 3>{c}), "SET 3, C", 8, 0 },
		{ 0xda, std::bind(&CPU::SET<BitRef<BYTE, 3>>, this, BitRef<BYTE, 3>{d}), "SET 3, D", 8, 0 },
		{ 0xdb, std::bind(&CPU::SET<BitRef<BYTE, 3>>, this, BitRef<BYTE, 3>{e}), "SET 3, E", 8, 0 },
		{ 0xdc, std::bind(&CPU::SET<BitRef<BYTE, 3>>, this, BitRef<BYTE, 3>{h}), "SET 3, H", 8, 0 },
		{ 0xdd, std::bind(&CPU::SET<BitRef<BYTE, 3>>, this, BitRef<BYTE, 3>{l}), "SET 3, L", 8, 0 },
		{ 0xde, std::bind(&CPU::SET<BitRef<MemRef, 3>>, this, BitRef<MemRef, 3>{MemRef{m_hl, m_mmu}}), "SET 3, (HL)", 16, 0 },
		{ 0xdf, std::bind(&CPU::SET<BitRef<BYTE, 3>>, this, BitRef<BYTE, 3>{a}), "SET 3, A", 8, 0 },

		{ 0xe0, std::bind(&CPU::SET<BitRef<BYTE, 4>>, this, BitRef<BYTE, 4>{b}), "SET 4, B", 8, 0 },
		{ 0xe1, std::bind(&CPU::SET<BitRef<BYTE, 4>>, this, BitRef<BYTE, 4>{c}), "SET 4, C", 8, 0 },
		{ 0xe2, std::bind(&CPU::SET<BitRef<BYTE, 4>>, this, BitRef<BYTE, 4>{d}), "SET 4, D", 8, 0 },
		{ 0xe3, std::bind(&CPU::SET<BitRef<BYTE, 4>>, this, BitRef<BYTE, 4>{e}), "SET 4, E", 8, 0 },
		{ 0xe4, std::bind(&CPU::SET<BitRef<BYTE, 4>>, this, BitRef<BYTE, 4>{h}), "SET 4, H", 8, 0 },
		{ 0xe5, std::bind(&CPU::SET<BitRef<BYTE, 4>>, this, BitRef<BYTE, 4>{l}), "SET 4, L", 8, 0 },
		{ 0xe6, std::bind(&CPU::SET<BitRef<MemRef, 4>>, this, BitRef<MemRef, 4>{MemRef{m_hl, m_mmu}}), "SET 4, (HL)", 16, 0 },
		{ 0xe7, std::bind(&CPU::SET<BitRef<BYTE, 4>>, this, BitRef<BYTE, 4>{a}), "SET 4, A", 8, 0 },
		{ 0xe8, std::bind(&CPU::SET<BitRef<BYTE, 5>>, this, BitRef<BYTE, 5>{b}), "SET 5, B", 8, 0 },
		{ 0xe9, std::bind(&CPU::SET<BitRef<BYTE, 5>>, this, BitRef<BYTE, 5>{c}), "SET 5, C", 8, 0 },
		{ 0xea, std::bind(&CPU::SET<BitRef<BYTE, 5>>, this, BitRef<BYTE, 5>{d}), "SET 5, D", 8, 0 },
		{ 0xeb, std::bind(&CPU::SET<BitRef<BYTE, 5>>, this, BitRef<BYTE, 5>{e}), "SET 5, E", 8, 0 },
		{ 0xec, std::bind(&CPU::SET<BitRef<BYTE, 5>>, this, BitRef<BYTE, 5>{h}), "SET 5, H", 8, 0 },
		{ 0xed, std::bind(&CPU::SET<BitRef<BYTE, 5>>, this, BitRef<BYTE, 5>{l}), "SET 5, L", 8, 0 },
		{ 0xee, std::bind(&CPU::SET<BitRef<MemRef, 5>>, this, BitRef<MemRef, 5>{MemRef{m_hl, m_mmu}}), "SET 5, (HL)", 16, 0 },
		{ 0xef, std::bind(&CPU::SET<BitRef<BYTE, 5>>, this, BitRef<BYTE, 5>{a}), "SET 5, A", 8, 0 },

		{ 0xf0, std::bind(&CPU::SET<BitRef<BYTE, 6>>, this, BitRef<BYTE, 6>{b}), "SET 6, B", 8, 0 },
		{ 0xf1, std::bind(&CPU::SET<BitRef<BYTE, 6>>, this, BitRef<BYTE, 6>{c}), "SET 6, C", 8, 0 },
		{ 0xf2, std::bind(&CPU::SET<BitRef<BYTE, 6>>, this, BitRef<BYTE, 6>{d}), "SET 6, D", 8, 0 },
		{ 0xf3, std::bind(&CPU::SET<BitRef<BYTE, 6>>, this, BitRef<BYTE, 6>{e}), "SET 6, E", 8, 0 },
		{ 0xf4, std::bind(&CPU::SET<BitRef<BYTE, 6>>, this, BitRef<BYTE, 6>{h}), "SET 6, H", 8, 0 },
		{ 0xf5, std::bind(&CPU::SET<BitRef<BYTE, 6>>, this, BitRef<BYTE, 6>{l}), "SET 6, L", 8, 0 },
		{ 0xf6, std::bind(&CPU::SET<BitRef<MemRef, 6>>, this, BitRef<MemRef, 6>{MemRef{m_hl, m_mmu}}), "SET 6, (HL)", 16, 0 },
		{ 0xf7, std::bind(&CPU::SET<BitRef<BYTE, 6>>, this, BitRef<BYTE, 6>{a}), "SET 6, A", 8, 0 },
		{ 0xf8, std::bind(&CPU::SET<BitRef<BYTE, 7>>, this, BitRef<BYTE, 7>{b}), "SET 7, B", 8, 0 },
		{ 0xf9, std::bind(&CPU::SET<BitRef<BYTE, 7>>, this, BitRef<BYTE, 7>{c}), "SET 7, C", 8, 0 },
		{ 0xfa, std::bind(&CPU::SET<BitRef<BYTE, 7>>, this, BitRef<BYTE, 7>{d}), "SET 7, D", 8, 0 },
		{ 0xfb, std::bind(&CPU::SET<BitRef<BYTE, 7>>, this, BitRef<BYTE, 7>{e}), "SET 7, E", 8, 0 },
		{ 0xfc, std::bind(&CPU::SET<BitRef<BYTE, 7>>, this, BitRef<BYTE, 7>{h}), "SET 7, H", 8, 0 },
		{ 0xfd, std::bind(&CPU::SET<BitRef<BYTE, 7>>, this, BitRef<BYTE, 7>{l}), "SET 7, L", 8, 0 },
		{ 0xfe, std::bind(&CPU::SET<BitRef<MemRef, 7>>, this, BitRef<MemRef, 7>{MemRef{m_hl, m_mmu}}), "SET 7, (HL)", 16, 0 },
		{ 0xff, std::bind(&CPU::SET<BitRef<BYTE, 7>>, this, BitRef<BYTE, 7>{a}), "SET 7, A", 8, 0 },
	}};
}

DWORD CPU::step() {
	if (m_pc == m_breakpoint) {
		m_debugMode = true;
	}
	auto rb = m_mmu.readByte(m_pc++);
	auto& op = m_instructions[rb];
	if (op.opcode != rb) {
		std::cout << "Missing instruction: 0x" << std::hex << +rb << " (0x" << std::hex << +op.opcode << ")\n";
		throw std::runtime_error{"Missing instruction"};
	}
	n = m_mmu.readByte(m_pc);
	nn  = m_mmu.readWord(m_pc);

	if (m_debugMode) {
		std::cout << "PC: 0x" << std::setfill('0') << std::setw(4) << std::hex << +(m_pc-1) << '\n';
		std::cout << "SP: 0x" << std::setfill('0') << std::setw(4) << std::hex << +m_sp << '\n';
		std::cout << "AF: 0x" << std::setfill('0') << std::setw(4) << std::hex << +m_af << " == 0b" << std::bitset<16>(m_af) << " = [f: " << std::bitset<8>(f) << "][a: " << std::bitset<8>(a) << "]\n";
		std::cout << "BC: 0x" << std::setfill('0') << std::setw(4) << std::hex << +m_bc << " == 0b" << std::bitset<16>(m_bc) << " = [c: " << std::bitset<8>(c) << "][b: " << std::bitset<8>(b) << "]\n";
		std::cout << "DE: 0x" << std::setfill('0') << std::setw(4) << std::hex << +m_de << " == 0b" << std::bitset<16>(m_de) << " = [e: " << std::bitset<8>(e) << "][d: " << std::bitset<8>(d) << "]\n";
		std::cout << "HL: 0x" << std::setfill('0') << std::setw(4) << std::hex << +m_hl << " == 0b" << std::bitset<16>(m_hl) << " = [l: " << std::bitset<8>(l) << "][h: " << std::bitset<8>(h) << "]\n";
		std::cout << "NN: 0x" << std::setfill('0') << std::setw(4) << std::hex << +nn << " == 0b" << std::bitset<16>(nn) << '\n';
		std::cout << "N:  0x" << std::setfill('0') << std::setw(2) << std::hex << +n << " == 0b" << std::bitset<8>(n) << '\n';
		std::cout << "Instruction: 0x" << std::hex << +rb << " = " << op.description;
		if (rb == 0xcb) {
			std::cout << " " << m_extended[n].description;
		}
		std::cout << "\n----\n\n";
		std::cin.get();
	}
	op.f();
	// note: some instructions have variable length cycles. these instructions have op.cylces == 0 and set the correct values themselves.
	if (op.cycles != 0) {
		m_cycles = op.cycles;
	}
	m_pc += op.offset;
	return m_cycles;
}

void CPU::handleInterrupts() {
	if (!m_intState.ime) {
		return;
	}
	if (m_intState.vBlankReq && m_intState.vBlank) {
		RST_INT<0x0040, 0b00000001>();
	} else if (m_intState.lcdStatReq && m_intState.lcdStat) {
		throw std::runtime_error{"LCD stat interrupt"};
	} else if (m_intState.timerReq && m_intState.timer) {
		throw std::runtime_error{"Timer interrupt"};
	} else if (m_intState.serialReq && m_intState.serial) {
		throw std::runtime_error{"Serial interrupt"};
	} else if (m_intState.joypadReq && m_intState.joypad) {
		throw std::runtime_error{"Joypad interrupt"};
	}
}

void CPU::JR(const bool& cond, const BYTE& offset) {
	if (cond) {
		m_pc += static_cast<int8_t>(offset);
		m_cycles += 4;
	}
	m_pc += 1;
	m_cycles += 8;
}

void CPU::JRn(const bool& cond, const BYTE& offset) {
	if (!cond) {
		m_pc += static_cast<int8_t>(offset);
		m_cycles += 4;
	}
	m_pc += 1;
	m_cycles += 8;
}

void CPU::JP(const bool& cond, const WORD& addr) {
	if (cond) {
		m_pc = addr;
		m_cycles += 4;
	} else {
		m_pc += 2;
	}
	m_cycles += 12;
}

void CPU::JPn(const bool& cond, const WORD& addr) {
	if (!cond) {
		m_pc = addr;
		m_cycles += 4;
	} else {
		m_pc += 2;
	}
	m_cycles += 12;
}

void CPU::ADD(const BYTE& source) {
	m_halfFlag = ((((a & 0xf) + (source & 0xf)) & 0xf0) != 0);
	WORD temp = static_cast<WORD>(a) + static_cast<WORD>(source);
	a = static_cast<BYTE>(temp);
	m_carryFlag = ((temp & 0xf00) != 0);
	m_zeroFlag = (a == 0);
	m_negFlag = false;
}

void CPU::ADC(const BYTE& source) {
	m_halfFlag = ((((a & 0xf) + (source & 0xf) + m_carryFlag) & 0xf0) != 0);
	WORD temp = static_cast<WORD>(a) + static_cast<WORD>(source) + m_carryFlag;
	a = static_cast<BYTE>(temp);
	m_carryFlag = ((temp & 0xf00) != 0);
	m_zeroFlag = (a == 0);
	m_negFlag = false;
}

void CPU::SUB(const BYTE& source) {
	m_halfFlag = ((a & 0xf) < (source & 0xf));
	int temp = a - source;
	a = static_cast<BYTE>(temp);
	m_zeroFlag = (a == 0);
	m_carryFlag = (temp < 0);
	m_negFlag = true;
}

void CPU::SBC(const BYTE& source) {
	m_halfFlag = ((a & 0xf) < ((source & 0xf) + m_carryFlag));
	int temp = a - source - m_carryFlag;
	a = static_cast<BYTE>(temp);
	m_zeroFlag = (a == 0);
	m_carryFlag = (temp < 0);
	m_negFlag = true;
}

void CPU::AND(const BYTE& source) {
	a &= source;
	m_zeroFlag = (a == 0);
	m_halfFlag = true;
	m_negFlag = false;
	m_carryFlag = false;
}

void CPU::XOR(const BYTE& source) {
	a ^= source;
	m_zeroFlag = (a == 0);
	m_halfFlag = false;
	m_negFlag = false;
	m_carryFlag = false;
}

void CPU::OR(const BYTE& source) {
	a |= source;
	m_zeroFlag = (a == 0);
	m_halfFlag = false;
	m_negFlag = false;
	m_carryFlag = false;
}

void CPU::CP(const BYTE& source) {
	int temp = a - source;
	m_halfFlag = ((a & 0xf) < (source & 0xf));
	m_carryFlag = (temp < 0);
	m_negFlag = true;
	m_zeroFlag = (temp == 0);
}

void CPU::CB() {
	auto& op = m_extended[n];
	if (op.opcode != n) {
		std::cout << "Missing extended instruction: 0x" << std::ios::hex << n << '\n';
		throw std::runtime_error{"Missing instruction"};
	}
	op.f();
	m_cycles += op.cycles;
}

void CPU::RLCA() {
	// slightly different than RLC: m_zeroFlag is always false
	m_carryFlag = ((a >> 7) != 0);
	a = static_cast<BYTE>((a << 1) | m_carryFlag);
	m_zeroFlag = false;
	m_halfFlag = false;
	m_negFlag = false;
}

void CPU::RRCA() {
	// slightly different than RRC: m_zeroFlag is always false
	m_carryFlag = ((a & 0x1) != 0);
	a = static_cast<BYTE>((a >> 1) | (m_carryFlag << 7));
	m_zeroFlag = false;
	m_halfFlag = false;
	m_negFlag = false;
}

void CPU::RLA() {
	// slightly different than RL: m_zeroFlag is always false
	bool temp = m_carryFlag;
	m_carryFlag = ((a & 0b10000000) != 0);
	a = static_cast<BYTE>((a << 1) | temp);
	m_zeroFlag = false;
	m_halfFlag = false;
	m_negFlag = false;
}

void CPU::RRA() {
	// slightly different than RR: m_zeroFlag is always false
	bool temp = m_carryFlag;
	m_carryFlag = ((a & 0x1) != 0);
	a = static_cast<BYTE>((a >> 1) | (temp << 7));
	m_zeroFlag = false;
	m_halfFlag = false;
	m_negFlag = false;
}

void CPU::ADD(WORD& target, const WORD& source) {
	int temp = target + source;
	m_carryFlag = (temp > 0xffff);
	m_negFlag = false;
	m_halfFlag = ((((target & 0x0fff) + (source & 0x0fff)) & 0xf000) != 0);
	target = static_cast<WORD>(temp);
}

void CPU::ADD() {
	// see: http://forums.nesdev.com/viewtopic.php?p=42143#p42143
	m_zeroFlag = false;
	m_negFlag = false;
	m_halfFlag = ((((m_sp & 0xf) + (n & 0xf)) & 0xf0) != 0);
	m_carryFlag = ((((m_sp & 0xff) + n) & 0xf00) != 0);
	m_sp = static_cast<WORD>(m_sp + static_cast<char>(n));
}

void CPU::LDadd() {
	// see: http://forums.nesdev.com/viewtopic.php?p=42143#p42143
	m_zeroFlag = false;
	m_negFlag = false;
	m_halfFlag = ((((m_sp & 0xf) + (n & 0xf)) & 0xf0) != 0);
	m_carryFlag = ((((m_sp & 0xff) + n) & 0xf00) != 0);
	m_hl = static_cast<WORD>(m_sp + static_cast<char>(n));
}

void CPU::CCF() {
	m_carryFlag = !m_carryFlag;
	m_negFlag = false;
	m_halfFlag = false;
}

void CPU::SCF() {
	m_carryFlag = true;
	m_negFlag = false;
	m_halfFlag = false;
}

void CPU::CPL() {
	a ^= 0xff;
	m_negFlag = true;
	m_halfFlag = true;
}

void CPU::DAA() {
	// see: http://www.worldofspectrum.org/faq/reference/z80reference.htm#DAA
	//BYTE oldA = a;
	int temp = a;
	BYTE correction = 0x00;

	if (a > 0x99 || m_carryFlag) {
		correction |= 0x60;
		m_carryFlag = true;
	}
	//nop:
	//else {
	//	correction |= 0x00;
	//	m_carryFlag = false;
	//}
	
	if ((a & 0x0f) > 0x9 || m_halfFlag) {
		correction |= 0x06;
	}
	// nop:
	// else {
	// 	correction |= 0x00;
	// }
	
	temp = (m_negFlag) ? (temp-correction) : (temp+correction);

	// m_halfFlag is always false in gameboy cpu
	// m_halfFlag = (((oldA ^ a) & 0b0001000) != 0);
	m_halfFlag = false;
	a = static_cast<BYTE>(temp);
	m_zeroFlag = (a == 0);
}

void CPU::CALL(const WORD& addr) {
	WORD newpc = m_pc + 2;
	m_mmu.writeByte(m_sp-1, newpc >> 8);
	m_mmu.writeByte(m_sp-2, newpc & 0xff);
	m_sp -= 2;
	m_pc = addr;
}

void CPU::POP(WORD& reg) {
	reg = static_cast<WORD>(m_mmu.readByte(m_sp) + (m_mmu.readByte(m_sp+1) << 8));
	m_sp += 2;
}

void CPU::PUSH(const WORD& reg) {
	m_mmu.writeByte(m_sp-1, static_cast<BYTE>(reg >> 8));
	m_mmu.writeByte(m_sp-2, static_cast<BYTE>(reg));
	m_sp -= 2;
}

void CPU::EI() {
	m_intState.ime = true;
}

void CPU::DI() {
	m_intState.ime = false;
}

void CPU::RET() {
	BYTE low = m_mmu.readByte(m_sp);
	BYTE high = m_mmu.readByte(m_sp+1);
	m_pc = static_cast<WORD>((high << 8) + low);
	m_sp += 2;
}

void CPU::RETcond(const bool& cond) {
	if (cond) {
		RET();
		m_cycles += 12;
	}
	m_cycles += 8;
}

void CPU::RETncond(const bool& cond) {
	if (!cond) {
		RET();
		m_cycles += 12;
	}
	m_cycles += 8;
}

void CPU::RETI() {
	m_intState.ime = true;
	RET();
}
