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
		std::array<DWORD, WIDTH * HEIGHT> m_pixelArray;
		Display& m_display;
		InterruptState& m_intState;

		void renderScanline();
		void renderTiles();
		void renderSprites();
		DWORD paletteColor(int);
		void updateTiles(WORD, BYTE);

		DWORD m_cycleCount = 0;

		std::array<BYTE, 0x2000> m_vram;
		std::array<BYTE, 0xa0> m_oam;

		using Row = std::array<BYTE, 2>;
		using Tile = std::array<Row, 8>;
		std::array<Tile, 384> tileMap;

		// 0xff40: LCD Control register
		BYTE m_lcdControl = 0;
		BitRef<BYTE, 7> m_displayEnable{m_lcdControl};
		BitRef<BYTE, 6> m_windowTileMapDisplaySelect{m_lcdControl};
		BitRef<BYTE, 5> m_windowDisplayEnable{m_lcdControl};
		BitRef<BYTE, 4> m_bgwinTileDataSelect{m_lcdControl};
		BitRef<BYTE, 3> m_bgTileMapDisplaySelect{m_lcdControl};
		BitRef<BYTE, 2> m_objSize{m_lcdControl};
		BitRef<BYTE, 1> m_objDisplayEnable{m_lcdControl};
		BitRef<BYTE, 0> m_bgDisplay{m_lcdControl};

		// 0xff41: LCD STAT
		BYTE m_lcdStat = 0;
		BitRef<BYTE, 6> m_coincidenceInt{m_lcdStat};
		BitRef<BYTE, 5> m_oamInt{m_lcdStat};
		BitRef<BYTE, 4> m_vBlankInt{m_lcdStat};
		BitRef<BYTE, 3> m_hBlankInt{m_lcdStat};
		BitRef<BYTE, 2> m_coincidenceFlag{m_lcdStat};

		// 0xff42: SCY Scroll Y
		BYTE m_scY = 0;

		// 0xff43: SCX Scroll X
		BYTE m_scX = 0;

		// 0xff44: LY current Y position
		BYTE m_lY = 0;

		// 0xff45: LYC LY compare
		BYTE m_lYC = 0;

		// 0xff46: DMA transfer
		BYTE m_dma = 0;

		// 0xff47: BGP background palette data
		BYTE m_bgp = 0;

		// 0xff48: OBP0 object palette 0 data
		BYTE m_obp0 = 0;

		// 0xff49: OBP1 object palette 1 data
		BYTE m_obp1 = 0;

		// 0xff4a: WY window Y position
		BYTE m_wY = 0;

		// 0xff4b: WX window X position
		BYTE m_wX = 0;
};
