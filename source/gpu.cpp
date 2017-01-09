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

GPU::GPU() :
	cycleCount{0},
	mode{Mode::HBlank},
	line{0},
	vram{{0}},
	lcdc{0},
	lcdEnable{lcdc},
	windowTileSelect{lcdc},
	windowEnable{lcdc},
	tileDataSelect{lcdc},
	bgTileSelect{lcdc},
	objSize{lcdc},
	objEnable{lcdc},
	bgDisplay{lcdc},
	scX{0},
	scY{0}
{
}

// See: http://imrannazar.com/GameBoy-Emulation-in-JavaScript:-GPU-Timings
void GPU::step(DWORD cycles) {
	cycleCount += cycles;

	std::cout << "Cycle count: " << std::dec << cycleCount << '\n';
	std::cout << "Mode: " << mode << '\n';

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

			// TODO: scanline
			throw std::runtime_error{"Scanline"};
		}
		break;
	case Mode::HBlank:
		if (cycleCount >= 204) {
			cycleCount = 0;
			line++;

			if (line == 143) {
				mode = Mode::VBlank;
				// draw to screen
			} else {
				mode = Mode::AccessingOAM;
			}
		}
		break;
	case Mode::VBlank:
		if (cycleCount >= 456) {
			cycleCount = 0;
			line++;

			if (line > 153) {
				mode = Mode::AccessingOAM;
				line = 0;
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
			lcdc = v;
			return;
		case LCD_STAT:
		case LCD_SCY:
			scY = v;
			return;
		case LCD_SCX:
			scX = v;
			return;
		case LCD_LY:
		case LCD_LYC:
		case LCD_DMA:
		case LCD_BGP:
		case LCD_OBP0:
		case LCD_OBP1:
		case LCD_WY:
		case LCD_WX:
		default:
			throw std::runtime_error{"LCD registers not implemented"};
		}
		return;
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
			return lcdc;
		case LCD_STAT:
		case LCD_SCY:
			return scY;
		case LCD_SCX:
			return scX;
		case LCD_LY:
		case LCD_LYC:
		case LCD_DMA:
		case LCD_BGP:
		case LCD_OBP0:
		case LCD_OBP1:
		case LCD_WY:
		case LCD_WX:
		default:
			break;
		}
		throw std::runtime_error{"LCD registers not implemented"};
	default:
		throw std::runtime_error{"Out of bounds"};
	}
}

