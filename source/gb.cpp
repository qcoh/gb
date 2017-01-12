#include <iostream>
#include <exception>
#include <cstdio>
#include <memory>
#include <SDL2/SDL.h>

#include "mapper.h"
#include "mmu.h"
#include "cpu.h"
#include "gpu.h"

template <typename Fun>
struct ScopeGuard {
	ScopeGuard(Fun f_) : f{std::move(f_)} {}
	~ScopeGuard() { f(); }
	Fun f;
};

template <typename Fun>
ScopeGuard<Fun> guard(Fun&& f) {
	return ScopeGuard<Fun>{std::move(f)};
}

int main(int argc, char *argv[]) {
	(void)argc;

	bool quit = false;
	
	// TODO: error handling
	SDL_Init(SDL_INIT_VIDEO);
	auto g = guard([](){ SDL_Quit(); });
	SDL_Event ev = { 0 };

	try {
		Display display{};
		GPU gpu{display};
		auto mapper = Mapper::fromFile(argv[1]);
		MMU mmu{std::move(mapper), gpu};
		CPU cpu{mmu, 0x0281};

		while (!quit) {
			cpu.interrupt();
			DWORD cycles = cpu.step();
			gpu.step(cycles);
			//std::cin.get();
			
			SDL_PollEvent(&ev);
			switch (ev.type) {
			case SDL_QUIT:
				quit = true;
				break;
			}
		}
	} catch (std::exception& e) {
		std::cerr << e.what() << '\n';
	}

	SDL_Quit();
}
