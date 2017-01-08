#include <iostream>
#include <exception>
#include <cstdio>

#include "mapper.h"
#include "mmu.h"
#include "cpu.h"
#include "gpu.h"

int main(int argc, char *argv[]) {
	(void)argc;
	try {
		GPU gpu{};
		auto mapper = Mapper::fromFile(argv[1]);
		MMU mmu{std::move(mapper), gpu};
		CPU cpu{mmu, true};

		while (true) {
			cpu.interrupt();
			DWORD cycles = cpu.step();
			gpu.step(cycles);
			//std::cin.get();
		}
	} catch (std::exception& e) {
		std::cerr << e.what() << '\n';
	}
}
