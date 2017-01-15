#include <stdexcept>
#include <iostream>
#include <type_traits>
#include "gpu.h"

GPU::GPU(Display& display_, InterruptState& intState_) :
	pixelArray{{0}},
	display{display_},
	intState{intState_},
	vram{{0}},
	oam{{0}}
{
	(void)dma;
	(void)lYC;
	(void)wX;
}

// See: http://imrannazar.com/GameBoy-Emulation-in-JavaScript:-GPU-Timings
void GPU::step(DWORD cycles) {
	cycleCount += cycles;

	switch (lcdStat & 0b11) {
	case ACCESSING_OAM:
		if (cycleCount >= 80) {
			cycleCount = 0;
			lcdStat = (lcdStat & 0b11111100) | ACCESSING_VRAM;
		}
		break;
	case ACCESSING_VRAM:
		if (cycleCount >= 172) {
			cycleCount = 0;
			lcdStat = (lcdStat & 0b11111100) | HBLANK;

			//throw std::runtime_error{"Scanline"};
			renderScanline();
		}
		break;
	case HBLANK:
		if (cycleCount >= 204) {
			cycleCount = 0;
			lY++;

			// TODO: 144 or 143???
			if (lY == 144) {
				lcdStat = (lcdStat & 0b11111100) | VBLANK;
				intState.vBlank = true;
				display.render(pixelArray);
			} else {
				lcdStat = (lcdStat & 0b11111100) | ACCESSING_OAM;
			}
		}
		break;
	case VBLANK:
		if (cycleCount >= 456) {
			cycleCount = 0;
			lY++;

			if (lY > 153) {
				lcdStat = (lcdStat & 0b11111100) | ACCESSING_OAM;
				lY = 0;
			}
		}
		break;
	}
}

void GPU::writeByte(WORD addr, BYTE v) {
	switch (addr & 0xf000) {
	case 0x8000:
	case 0x9000:
		vram[addr - 0x8000] = v;
		return;
	case 0xf000:
		if ((addr & 0xff00) == 0xfe00) {
			// OAM
			oam[addr - 0xfe00] = v;
			return;
		}
		switch (addr) {
		case LCD_CONTROL:
			lcdControl = v;
			return;
		case LCD_STAT:
			lcdStat = v;
			return;
		case LCD_SCY:
			scY = v;
			return;
		case LCD_SCX:
			scX = v;
			return;
		case LCD_LY:
			lY = v;
			return;
		case LCD_LYC:
		case LCD_DMA:
			break;
		case LCD_BGP:
			bgp = v;
			return;
		case LCD_OBP0:
			obp0 = v;
			return;
		case LCD_OBP1:
			obp1 = v;
			return;
		case LCD_WY:
			wY = v;
			return;
		case LCD_WX:
			wX = v;
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
		return vram[addr - 0x8000];
	case 0xf000:
		if ((addr & 0xff00) == 0xfe00) {
			// OAM
			return oam[addr - 0xfe00];
		}
		switch (addr) {
		case LCD_CONTROL:
			return lcdControl;
		case LCD_STAT:
			return lcdStat;
		case LCD_SCY:
			return scY;
		case LCD_SCX:
			return scX;
		case LCD_LY:
			return lY;
		case LCD_LYC:
		case LCD_DMA:
			break;
		case LCD_BGP:
			return bgp;
		case LCD_OBP0:
			return obp0;
		case LCD_OBP1:
			return obp1;
		case LCD_WY:
			return wY;
		case LCD_WX:
			return wX;
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
	if (bgDisplay) {
		renderTiles();
	}
	if (objEnable) {
		renderSprites();
	}
}

void GPU::renderTiles() {
	// see: http://www.codeslinger.co.uk/pages/projects/gameboy/graphics.html
	bool unsig = true;
	bool useWindow = windowEnable && (wY <= lY);


	WORD tileData = 0;
	if (tileDataSelect) {
		tileData = 0x8000;
	} else {
		tileData = 0x8800;
		unsig = false;
	}
	
	WORD bgMemory = 0;
	if (useWindow) {
		bgMemory = (bgTileSelect) ? 0x9c00 : 0x9800;
	} else {
		bgMemory = (windowTileSelect) ? 0x9c00 : 0x9800;
	}

	BYTE ypos = 0;
	if (!useWindow) {
		ypos = scY + lY;
	} else {
		ypos = lY - scY;
	}

	WORD tileRow = static_cast<WORD>(((ypos >> 3) & 0xff) << 5);

	for (int pixel = 0; pixel < 160; pixel++) {
		BYTE xpos = static_cast<BYTE>(pixel + scX);

		if (useWindow) {
			if (pixel >= wX) {
				xpos = static_cast<BYTE>(pixel - wX);
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

		if (lY > 143 || pixel > 159) {
			throw std::runtime_error{"overflowing image"};
		}

		pixelArray[static_cast<DWORD>(pixel + 160 * lY)] = paletteColor(color);
	}

	// printf debugging
	//for (unsigned i = 0; i < 160; i++) {
	//	std::cout << std::hex << +display[160 * lY + i] << " ";
	//}
	//std::cout << '\n';
}

DWORD GPU::paletteColor(int c) {
	c *= 2;
	switch ((bgp >> c) & 0x3) {
	case 0: return 0xffffffff;
	case 1: return 0xffc0c0c0;
	case 2: return 0xff606060;
	default: return 0xff000000;
	}
}

void GPU::renderSprites() {
	for (BYTE i = 0; i < 40; i++) {
		BYTE spriteIndex = static_cast<BYTE>(i << 2);
		BYTE ypos = static_cast<BYTE>(oam[spriteIndex] - 16);
		BYTE xpos = static_cast<BYTE>(oam[spriteIndex + 1] - 8);
		BYTE tileLocation = oam[spriteIndex + 2];
		BYTE attributes = oam[spriteIndex + 3];

		BYTE height = (objSize) ? 16 : 8;
		if ((lY >= ypos) && (lY < (ypos + height))) {
			int line = lY - ypos;

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

				if (lY < 0 || lY > 143 || pixelPos < 0 || pixelPos > 159) {
					throw std::runtime_error{"out of bounds"};
				}
				pixelArray[static_cast<DWORD>(pixelPos + 160 * lY)] = color;
			}
		}
	}
}
