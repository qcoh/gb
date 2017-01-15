#pragma once

#include <array>
#include "types.h"
#include "bitref.h"
#include "display.h"
#include "interruptstate.h"

class GPU {
	public:
		GPU(Display&, InterruptState&);
		void step(DWORD);
		void writeByte(WORD, BYTE);
		BYTE readByte(WORD);

		static const BYTE ACCESSING_OAM = 0b10;
		static const BYTE ACCESSING_VRAM = 0b11;
		static const BYTE HBLANK = 0b00;
		static const BYTE VBLANK = 0b01;

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
		InterruptState& intState;

		void renderScanline();
		void renderTiles();
		void renderSprites();
		DWORD paletteColor(int);

		DWORD cycleCount = 0;

		std::array<BYTE, 0x2000> vram;
		std::array<BYTE, 0xa0> oam;

		// 0xff40: LCD Control register
		BYTE lcdControl = 0;
		BitRef<BYTE, 7> lcdEnable{lcdControl};
		BitRef<BYTE, 6> windowTileSelect{lcdControl};
		BitRef<BYTE, 5> windowEnable{lcdControl};
		BitRef<BYTE, 4> tileDataSelect{lcdControl};
		BitRef<BYTE, 3> bgTileSelect{lcdControl};
		BitRef<BYTE, 2> objSize{lcdControl};
		BitRef<BYTE, 1> objEnable{lcdControl};
		BitRef<BYTE, 0> bgDisplay{lcdControl};

		// 0xff41: LCD STAT
		BYTE lcdStat = 0;
		BitRef<BYTE, 6> coincidenceInt{lcdStat};
		BitRef<BYTE, 5> oamInt{lcdStat};
		BitRef<BYTE, 4> vBlankInt{lcdStat};
		BitRef<BYTE, 3> hBlankInt{lcdStat};
		BitRef<BYTE, 2> coincidenceFlag{lcdStat};

		// 0xff42: SCY Scroll Y
		BYTE scY = 0;

		// 0xff43: SCX Scroll X
		BYTE scX = 0;

		// 0xff44: LY current Y position
		BYTE lY = 0;

		// 0xff45: LYC LY compare
		BYTE lYC = 0;

		// 0xff46: DMA transfer
		BYTE dma = 0;

		// 0xff47: BGP background palette data
		BYTE bgp = 0;

		// 0xff48: OBP0 object palette 0 data
		BYTE obp0 = 0;

		// 0xff49: OBP1 object palette 1 data
		BYTE obp1 = 0;

		// 0xff4a: WY window Y position
		BYTE wY = 0;

		// 0xff4b: WX window X position
		BYTE wX = 0;
};
