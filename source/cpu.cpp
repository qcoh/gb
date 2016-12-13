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
		{ 0x00, [](){}, "NOP" },
		{ 0x03, std::bind(&CPU::INC, this, std::ref(bc)), "INC BC" },
		{ 0x13, std::bind(&CPU::INC, this, std::ref(de)), "INC DE" },
		{ 0x23, std::bind(&CPU::INC, this, std::ref(hl)), "INC HL" },
		{ 0x33, std::bind(&CPU::INC, this, std::ref(sp)), "INC SP" },
	}};
}

void CPU::step() {

}

void CPU::INC(WORD& w) {
	w++;
}
