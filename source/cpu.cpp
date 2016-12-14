#include <functional>
#include "cpu.h"

CPU::CPU(MMU&& mmu_) :
	mmu{std::move(mmu_)},
	pc{0},
	sp{0},
	af{0},
	bc{0},
	de{0},
	hl{0},
	a{static_cast<BYTE*>(static_cast<void*>(&af))+1},
	f{static_cast<BYTE*>(static_cast<void*>(&af))},
	b{static_cast<BYTE*>(static_cast<void*>(&bc))+1},
	c{static_cast<BYTE*>(static_cast<void*>(&bc))},
	d{static_cast<BYTE*>(static_cast<void*>(&de))+1},
	e{static_cast<BYTE*>(static_cast<void*>(&de))},
	h{static_cast<BYTE*>(static_cast<void*>(&hl))+1},
	l{static_cast<BYTE*>(static_cast<void*>(&hl))}
{
	(void)pc;
	(void)sp;
	(void)af;
	(void)bc;
	(void)de;
	(void)hl;
	(void)a;
	(void)f;
	(void)b;
	(void)c;
	(void)d;
	(void)e;
	(void)h;
	(void)l;

	instructions = {{
		{ 0x00, [](){}, "NOP", 4, 1 },
		{ 0x01, std::bind(&CPU::LD, this, std::ref(bc), std::cref(nn)), "LD BC, nn", 12, 3 },

		{ 0x03, std::bind(&CPU::INC, this, std::ref(bc)), "INC BC", 8, 1 },
		{ 0x0b, std::bind(&CPU::DEC, this, std::ref(bc)), "DEC BC", 8, 1 },

		{ 0x11, std::bind(&CPU::LD, this, std::ref(de), std::cref(nn)), "LD DE, nn", 12, 3 },

		{ 0x13, std::bind(&CPU::INC, this, std::ref(de)), "INC DE", 8, 1 },
		{ 0x1b, std::bind(&CPU::DEC, this, std::ref(de)), "DEC DE", 8, 1 },

		{ 0x21, std::bind(&CPU::LD, this, std::ref(hl), std::cref(nn)), "LD HL, nn", 12, 3 },

		{ 0x23, std::bind(&CPU::INC, this, std::ref(hl)), "INC HL", 8, 1 },
		{ 0x2b, std::bind(&CPU::DEC, this, std::ref(hl)), "DEC HL", 8, 1 },

		{ 0x31, std::bind(&CPU::LD, this, std::ref(sp), std::cref(nn)), "LD SP, nn", 12, 3 },

		{ 0x33, std::bind(&CPU::INC, this, std::ref(sp)), "INC SP", 8, 1 },
		{ 0x3b, std::bind(&CPU::DEC, this, std::ref(sp)), "DEC SP", 8, 1 },
	}};
}

void CPU::step() {

}

void CPU::INC(WORD& w) {
	w++;
}

void CPU::DEC(WORD& w) {
	w--;
}

void CPU::LD(WORD& target, const WORD& source) {
	target = source;
}
