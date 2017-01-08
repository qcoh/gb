#pragma once

#include <array>
#include "types.h"
#include "bitref.h"

class GPU {
	public:
		GPU();
		void step(DWORD);
		void writeByte(WORD, BYTE);
		BYTE readByte(WORD);

		BYTE& lcdControl();

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

		// LCD Control register
		BYTE lcdc;
		BitRef<BYTE, 7> lcdEnable;
		BitRef<BYTE, 6> windowTileSelect;
		BitRef<BYTE, 5> windowEnable;
		BitRef<BYTE, 4> tileDataSelect;
		BitRef<BYTE, 3> bgTileSelect;
		BitRef<BYTE, 2> objSize;
		BitRef<BYTE, 1> objEnable;
		BitRef<BYTE, 0> bgDisplay;


};
