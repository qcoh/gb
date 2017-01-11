#include <stdexcept>
#include <iostream>
#include <type_traits>
#include "gpu.h"

std::ostream& operator<<(std::ostream& os, const GPU::Mode& mode) {
	switch (mode) {
	case GPU::Mode::AccessingOAM: os << "AccesingOAM"; return os;
	case GPU::Mode::AccessingVRAM: os << "AccesingVRAM"; return os;
	case GPU::Mode::HBlank: os << "HBlank"; return os;
	case GPU::Mode::VBlank: os << "VBlank"; return os;
	}
}

GPU::GPU(Display& display_) :
	pixelArray{{0}},
	display{display_},
	cycleCount{0},
	mode{Mode::HBlank},
	vram{{0}},
	lcdControl{0},
	lcdEnable{lcdControl},
	windowTileSelect{lcdControl},
	windowEnable{lcdControl},
	tileDataSelect{lcdControl},
	bgTileSelect{lcdControl},
	objSize{lcdControl},
	objEnable{lcdControl},
	bgDisplay{lcdControl},
	lcdStat{0},
	scY{0},
	scX{0},
	lY{0},
	lYC{0},
	dma{0},
	bgp{0},
	obp0{0},
	obp1{0},
	wY{0},
	wX{0}
{
	(void)lcdStat;
	(void)lYC;
	(void)dma;
	(void)obp0;
	(void)obp1;
	(void)wX;
	(void)bgp;
}

// See: http://imrannazar.com/GameBoy-Emulation-in-JavaScript:-GPU-Timings
void GPU::step(DWORD cycles) {
	cycleCount += cycles;

	//std::cout << "Cycle count: " << std::dec << cycleCount << '\n';
	//std::cout << "Mode: " << mode << '\n';

	switch (mode) {
	case Mode::AccessingOAM:
		if (cycleCount >= 80) {
			cycleCount = 0;
			mode = Mode::AccessingVRAM;
		}
		break;
	case Mode::AccessingVRAM:
		if (cycleCount >= 172) {
			cycleCount = 0;
			mode = Mode::HBlank;

			//throw std::runtime_error{"Scanline"};
			renderScanline();
		}
		break;
	case Mode::HBlank:
		if (cycleCount >= 204) {
			cycleCount = 0;
			lY++;

			// TODO: 144 or 143???
			if (lY == 144) {
				mode = Mode::VBlank;
				display.render(pixelArray);
			} else {
				mode = Mode::AccessingOAM;
			}
		}
		break;
	case Mode::VBlank:
		if (cycleCount >= 456) {
			cycleCount = 0;
			lY++;

			if (lY > 153) {
				mode = Mode::AccessingOAM;
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
		switch (addr) {
		case LCD_CONTROL:
			lcdControl = v;
			return;
		case LCD_STAT:
			lcdStat = v;
			break;
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
		case LCD_OBP1:
		case LCD_WY:
		case LCD_WX:
		default:
			break;
		}
		std::cout << "LCD register write: " << std::hex << +addr << '\n';
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
		case LCD_OBP1:
		case LCD_WY:
		case LCD_WX:
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
}