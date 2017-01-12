#pragma once

#include <array>
#include "types.h"
#include "bitref.h"
#include "display.h"

class GPU {
	public:
		GPU(Display&);
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
		static const int WIDTH = 160;
		static const int HEIGHT = 144;
		std::array<DWORD, WIDTH * HEIGHT> pixelArray;
		Display& display;

		void renderScanline();
		void renderTiles();
		void renderSprites();
		DWORD paletteColor(int);

		DWORD cycleCount;
		Mode mode;

		std::array<BYTE, 0x2000> vram;
		std::array<BYTE, 0xa0> oam;

		// 0xff40: LCD Control register
		BYTE lcdControl;
		BitRef<BYTE, 7> lcdEnable;
		BitRef<BYTE, 6> windowTileSelect;
		BitRef<BYTE, 5> windowEnable;
		BitRef<BYTE, 4> tileDataSelect;
		BitRef<BYTE, 3> bgTileSelect;
		BitRef<BYTE, 2> objSize;
		BitRef<BYTE, 1> objEnable;
		BitRef<BYTE, 0> bgDisplay;

		// 0xff41: LCD STAT
		BYTE lcdStat;

		// 0xff42: SCY Scroll Y
		BYTE scY;

		// 0xff43: SCX Scroll X
		BYTE scX;

		// 0xff44: LY current Y position
		BYTE lY;

		// 0xff45: LYC LY compare
		BYTE lYC;

		// 0xff46: DMA transfer
		BYTE dma;

		// 0xff47: BGP background palette data
		BYTE bgp;

		// 0xff48: OBP0 object palette 0 data
		BYTE obp0;

		// 0xff49: OBP1 object palette 1 data
		BYTE obp1;

		// 0xff4a: WY window Y position
		BYTE wY;

		// 0xff4b: WX window X position
		BYTE wX;
};
