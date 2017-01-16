#include <stdexcept>
#include <iostream>
#include "gpu.h"

GPU::GPU(Display& display_, InterruptState& intState_) :
	m_pixelArray{{0}},
	m_display{display_},
	m_intState{intState_},
	m_vram{{0}},
	m_oam{{0}}
{
	(void)m_dma;
	(void)m_lYC;
	(void)m_wX;
}

// See: http://imrannazar.com/GameBoy-Emulation-in-JavaScript:-GPU-Timings
void GPU::step(DWORD cycles) {
	m_cycleCount += cycles;

	switch (m_lcdStat & 0b11) {
	case ACCESSING_OAM:
		if (m_cycleCount >= 80) {
			m_cycleCount = 0;
			m_lcdStat = (m_lcdStat & 0b11111100) | ACCESSING_VRAM;
		}
		break;
	case ACCESSING_VRAM:
		if (m_cycleCount >= 172) {
			m_cycleCount = 0;
			m_lcdStat = (m_lcdStat & 0b11111100) | HBLANK;

			//throw std::runtime_error{"Scanline"};
			renderScanline();
		}
		break;
	case HBLANK:
		if (m_cycleCount >= 204) {
			m_cycleCount = 0;
			m_lY++;

			// TODO: 144 or 143???
			if (m_lY == 144) {
				m_lcdStat = (m_lcdStat & 0b11111100) | VBLANK;
				m_intState.vBlank = true;
				m_display.render(m_pixelArray);
			} else {
				m_lcdStat = (m_lcdStat & 0b11111100) | ACCESSING_OAM;
			}
		}
		break;
	case VBLANK:
		if (m_cycleCount >= 456) {
			m_cycleCount = 0;
			m_lY++;

			if (m_lY > 153) {
				m_lcdStat = (m_lcdStat & 0b11111100) | ACCESSING_OAM;
				m_lY = 0;
			}
		}
		break;
	}
}

void GPU::writeByte(WORD addr, BYTE v) {
	switch (addr & 0xf000) {
	case 0x8000:
	case 0x9000:
		m_vram[addr - 0x8000] = v;
		if (addr < 0x9800) {
			updateTiles(addr, v);
		}
		return;
	case 0xf000:
		if ((addr & 0xff00) == 0xfe00) {
			// OAM
			m_oam[addr - 0xfe00] = v;
			return;
		}
		switch (addr) {
		case LCD_CONTROL:
			m_lcdControl = v;
			return;
		case LCD_STAT:
			m_lcdStat = v;
			return;
		case LCD_SCY:
			m_scY = v;
			return;
		case LCD_SCX:
			m_scX = v;
			return;
		case LCD_LY:
			m_lY = v;
			return;
		case LCD_LYC:
		case LCD_DMA:
			break;
		case LCD_BGP:
			m_bgp = v;
			return;
		case LCD_OBP0:
			m_obp0 = v;
			return;
		case LCD_OBP1:
			m_obp1 = v;
			return;
		case LCD_WY:
			m_wY = v;
			return;
		case LCD_WX:
			m_wX = v;
			return;
		case LCD_VBK:
			// gbc, ignore
			return;
		default:
			break;
		}
		std::cout << "LCD register write: (" << std::hex << +addr << ") = 0x" << +v << '\n';
		throw std::runtime_error{"LCD registers not implemented"};
	default:
		std::cout << std::hex << +addr << '\n';
		throw std::runtime_error{"Out of bounds"};
	}
}

BYTE GPU::readByte(WORD addr) {
	switch (addr & 0xf000) {
	case 0x8000:
	case 0x9000:
		return m_vram[addr - 0x8000];
	case 0xf000:
		if ((addr & 0xff00) == 0xfe00) {
			// OAM
			return m_oam[addr - 0xfe00];
		}
		switch (addr) {
		case LCD_CONTROL:
			return m_lcdControl;
		case LCD_STAT:
			return m_lcdStat;
		case LCD_SCY:
			return m_scY;
		case LCD_SCX:
			return m_scX;
		case LCD_LY:
			return m_lY;
		case LCD_LYC:
		case LCD_DMA:
			break;
		case LCD_BGP:
			return m_bgp;
		case LCD_OBP0:
			return m_obp0;
		case LCD_OBP1:
			return m_obp1;
		case LCD_WY:
			return m_wY;
		case LCD_WX:
			return m_wX;
		case LCD_VBK:
			// gbc, ignore
			return 0;
		default:
			break;
		}
		std::cout << "LCD register read: " << std::hex << +addr << '\n';
		throw std::runtime_error{"LCD registers not implemented"};
	default:
		std::cout << std::hex << +addr << '\n';
		throw std::runtime_error{"Out of bounds"};
	}
}

void GPU::renderScanline() {
	if (m_bgDisplay) {
		renderTiles();
	}
	//if (m_objDisplayEnable) {
	//	renderSprites();
	//}
}

void GPU::renderTiles() {
	// http://www.codeslinger.co.uk/pages/projects/gameboy/graphics.html
	// http://bgb.bircd.org/pandocs.htm
	// http://imrannazar.com/GameBoy-Emulation-in-JavaScript:-Graphics
	
	WORD tileMapAddr = m_bgTileMapDisplaySelect ? 0x9c00 : 0x9800;

	BYTE tileX = m_scX >> 3;
	BYTE tileY = static_cast<BYTE>(((m_scY + m_lY) & 0xff) >> 3);
	BYTE pixelOffsetX = m_scX & 0x7;
	BYTE pixelOffsetY = static_cast<BYTE>((m_scY + m_lY) & 0x7);

	WORD tileMapIndex = static_cast<WORD>(tileMapAddr + tileX + tileY * 32);
	WORD tileDataIndex = readByte(tileMapIndex);

	if (m_bgwinTileDataSelect) {
		tileDataIndex = static_cast<BYTE>(static_cast<int8_t>(tileDataIndex) + 0x100);
	}

	for (BYTE pixel = 0; pixel < 160; pixel++) {
		Row current = tileMap[tileDataIndex][pixelOffsetY];

		int mask = (7 - (pixelOffsetX & 0x7));
		BYTE colorIndex = static_cast<BYTE>(((current[0] >> mask) & 0x1) + (((current[1] >> mask) & 0x1) << 1));

		m_pixelArray[pixel + 160 * m_lY] = paletteColor(colorIndex);

		pixelOffsetX = static_cast<BYTE>(pixelOffsetX + 1);
		if (pixelOffsetX == 8) {
			pixelOffsetX = 0;
			// next tile
			tileMapIndex = static_cast<WORD>(tileMapIndex + 1);
			tileDataIndex = readByte(tileMapIndex);

			if (m_bgwinTileDataSelect) {
				tileDataIndex = static_cast<BYTE>(static_cast<int8_t>(tileDataIndex) + 0x100);
			}
		}
	}
}

DWORD GPU::paletteColor(int c) {
	c *= 2;
	switch ((m_bgp >> c) & 0x3) {
	case 0: return 0xffffffff;
	case 1: return 0xffc0c0c0;
	case 2: return 0xff606060;
	default: return 0xff000000;
	}
}

void GPU::renderSprites() {
	for (BYTE i = 0; i < 40; i++) {
		BYTE spriteIndex = static_cast<BYTE>(i << 2);
		BYTE ypos = static_cast<BYTE>(m_oam[spriteIndex] - 16);
		BYTE xpos = static_cast<BYTE>(m_oam[spriteIndex + 1] - 8);
		BYTE tileLocation = m_oam[spriteIndex + 2];
		BYTE attributes = m_oam[spriteIndex + 3];

		BYTE height = (m_objSize) ? 16 : 8;
		if ((m_lY >= ypos) && (m_lY < (ypos + height))) {
			int line = m_lY - ypos;

			if (attributes & 0b01000000) {
				line -= height;
				line *= -1;
			}
			line *= 2;

			WORD dataAddr = static_cast<WORD>(0x8000 + (tileLocation * 16) + line);
			BYTE data0 = readByte(dataAddr);
			BYTE data1 = readByte(static_cast<WORD>(dataAddr + 1));

			for (int pixel = 7; pixel >= 0; pixel--) {
				int colorBit = pixel;
				
				if (attributes & 0b00100000) {
					colorBit -= 7;
					colorBit *= -1;
				}

				int colorNum = (((data1 >> colorBit) << 1) & 0x2) | ((data0 >> colorBit) & 0x1);
				DWORD color = paletteColor(colorNum);

				if (color == 0xffffffff) {
					// white == transparent
					continue;
				}
				
				int pixelPos = xpos + 7 - pixel;

				if (m_lY < 0 || m_lY > 143 || pixelPos < 0 || pixelPos > 159) {
					throw std::runtime_error{"out of bounds"};
				}
				m_pixelArray[static_cast<DWORD>(pixelPos + 160 * m_lY)] = color;
			}
		}
	}
}

void GPU::updateTiles(WORD addr, BYTE v) {
	WORD tileIndex = (addr & 0x1fff) >> 4;
	BYTE rowIndex = static_cast<BYTE>((addr >> 1) & 0x7);
	BYTE bitIndex = static_cast<BYTE>(addr & 0x1);

	tileMap[tileIndex][rowIndex][bitIndex] = v;
}
