#pragma once

#include "mmu.h"
#include "types.h"

class CPU {
	public:
		CPU(MMU&&);

		void step();
	private:
		MMU mmu;

		WORD pc;
		WORD sp;
};
