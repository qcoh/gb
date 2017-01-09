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

		enum class Mode {
			AccessingOAM,
			AccessingVRAM,
			HBlank,
			VBlank,
		};

		static const WORD LCD_CONTROL = 0xff40;
		static const WORD LCD_STAT = 0xff41;
		static const WORD LCD_SCY = 0xff42;
		static const WORD LCD_SCX = 0xff43;
		static const WORD LCD_LY = 0xff44;
		static const WORD LCD_LYC = 0xff45;
		static const WORD LCD_DMA = 0xff46;
		static const WORD LCD_BGP = 0xff47;
		static const WORD LCD_OBP0 = 0xff48;
		static const WORD LCD_OBP1 = 0xff49;
		static const WORD LCD_WY = 0xff4a;
		static const WORD LCD_WX = 0xff4b;
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

		// Scroll registers
		BYTE scX;
		BYTE scY;
};
