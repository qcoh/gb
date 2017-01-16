#include <stdexcept>
#include <iostream>
#include <type_traits>
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
		default:
			break;
		}
		std::cout << "LCD register read: " << std::hex << +addr << '\n';
		throw std::runtime_error{"LCD registers not implemented"};
	default:
		throw std::runtime_error{"Out of bounds"};
	}
}

void GPU::renderScanline() {
	if (m_bgDisplay) {
		renderTiles();
	}
	if (m_objDisplayEnable) {
		renderSprites();
	}
}

void GPU::renderTiles() {
	// see: http://www.codeslinger.co.uk/pages/projects/gameboy/graphics.html
	bool unsig = true;
	bool useWindow = m_windowDisplayEnable && (m_wY <= m_lY);


	WORD tileData = 0;
	if (m_bgwinTileDataSelect) {
		tileData = 0x8000;
	} else {
		tileData = 0x8800;
		unsig = false;
	}
	
	WORD bgMemory = 0;
	if (useWindow) {
		bgMemory = (m_bgTileMapDisplaySelect) ? 0x9c00 : 0x9800;
	} else {
		bgMemory = (m_windowTileMapDisplaySelect) ? 0x9c00 : 0x9800;
	}

	BYTE ypos = 0;
	if (!useWindow) {
		ypos = m_scY + m_lY;
	} else {
		ypos = m_lY - m_scY;
	}

	WORD tileRow = static_cast<WORD>(((ypos >> 3) & 0xff) << 5);

	for (int pixel = 0; pixel < 160; pixel++) {
		BYTE xpos = static_cast<BYTE>(pixel + m_scX);

		if (useWindow) {
			if (pixel >= m_wX) {
				xpos = static_cast<BYTE>(pixel - m_wX);
			}
		}

		WORD tileColumn = xpos >> 3;
		WORD tileAddr = bgMemory + tileRow + tileColumn;
		WORD tileLocation = tileData;
		
		int16_t tileNo = 0;

		if (unsig) {
			tileNo = readByte(tileAddr);
			tileLocation += tileNo * 16;
		} else {
			tileNo = static_cast<int8_t>(readByte(tileAddr));
			tileLocation += (tileNo + 128) * 16;
		}

		BYTE currentLine = static_cast<BYTE>((ypos & 0x7) << 1);
		BYTE lineData0 = readByte(static_cast<WORD>(tileLocation + currentLine));
		BYTE lineData1 = readByte(static_cast<WORD>(tileLocation + currentLine + 1));

		int colorBit = ((xpos & 0x7) - 7) * (-1);
		int color = (((lineData1 >> colorBit) << 1) & 0x2) | ((lineData0 >> colorBit) & 0x1);

		if (m_lY > 143 || pixel > 159) {
			continue;
			throw std::runtime_error{"overflowing image"};
		}

		m_pixelArray[static_cast<DWORD>(pixel + 160 * m_lY)] = paletteColor(color);
	}

	// printf debugging
	//for (unsigned i = 0; i < 160; i++) {
	//	std::cout << std::hex << +m_display[160 * m_lY + i] << " ";
	//}
	//std::cout << '\n';
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
