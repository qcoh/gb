#pragma once

#include "types.h"

class GPU {
	public:
		GPU();
		void step(DWORD);

		enum class Mode {
			AccessingOAM,
			AccessingVRAM,
			HBlank,
			VBlank,
		};
	private:
		DWORD cycleCount;
		Mode mode;
		BYTE line;
};
