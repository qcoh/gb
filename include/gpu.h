#pragma once

#include <array>
#include "types.h"

class GPU {
	public:
		GPU();
		void step(DWORD);
		void writeByte(WORD, BYTE);
		BYTE readByte(WORD);

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

		std::array<BYTE, 0x2000> vram;
};
