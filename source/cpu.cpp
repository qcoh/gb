#include <functional>
#include "cpu.h"

CPU::CPU(MMU&& mmu_) : mmu{std::move(mmu_)} {
	(void)pc;
	(void)sp;

	instructions = {
		{{0, {}, "test"}},
	};
}

void CPU::step() {

}
