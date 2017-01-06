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
		auto mapper = Mapper::fromFile(argv[1]);
		MMU mmu{std::move(mapper)};
		CPU cpu{mmu, true};
		GPU gpu{};

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
