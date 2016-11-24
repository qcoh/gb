#include "cpu.h"

CPU::CPU(MMU&& mmu) : mmu{std::move(mmu)}
{
	(void)pc;
	(void)sp;
}

void CPU::step() {

}
