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

	instructions = {
		{{0, {}, "test"}},
	};
}

void CPU::step() {

}
