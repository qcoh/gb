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

		template<class Writer, class Reader>
		void LD(Writer& w, Reader& r) {
			w.write(r.read());
		}

};
