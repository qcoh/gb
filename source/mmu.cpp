#include <stdexcept>
#include "mmu.h"

std::array<BYTE, 256> MMU::bios{{
	0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32, 0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
	0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3, 0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
	0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A, 0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
	0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06, 0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
	0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99, 0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
	0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64, 0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
	0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90, 0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
	0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62, 0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
	0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xF2, 0xF0, 0x42, 0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
	0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04, 0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
	0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9, 0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
	0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D, 0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
	0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99, 0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
	0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E, 0x3c, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x4C,
	0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13, 0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
	0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20, 0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50,
}};

MMU::MMU(std::unique_ptr<Mapper>&& mapper_) : 
	mapper{std::move(mapper_)},
	biosMode{true}
{
}

BYTE MMU::readByte(WORD addr) {
	if (addr <= 0x7fff) {
		// ROM and BIOS
		if (biosMode && addr < 0x100) {
			return bios[addr];
		} else {
			return mapper->readByte(addr);
		}
	} else if (0x8000 <= addr && addr <= 0x9fff) {
		// Video RAM
		throw std::runtime_error{"Read from VRAM"};
	} else if (0xa000 <= addr && addr <= 0xbfff) {
		// Cartridge RAM
		throw std::runtime_error{"Read from CartRAM"};
	} else if (0xc000 <= addr && addr <= 0xcfff) {
		// Work RAM (0)
		throw std::runtime_error{"Read from WRAM (0)"};
	} else if (0xd000 <= addr && addr <= 0xdfff) {
		// Work RAM (1)
		throw std::runtime_error{"Read from WRAM (1)"};
	} else if (0xe000 <= addr && addr <= 0xfdff) {
		// Echo RAM
		throw std::runtime_error{"Read from ERAM"};
	} else if (0xfe00 <= addr && addr <= 0xfe9f) {
		// Object Attribute Memory
		throw std::runtime_error{"Read from OAM"};
	} else if (0xfea0 <= addr && addr <= 0xfeff) {
		// Not usable
		throw std::runtime_error{"Read from unusable memory"};
	} else if (0xff00 <= addr && addr <= 0xff7f) {
		// IO registers
		throw std::runtime_error{"Read from IO registers"};
	} else if (0xff80 <= addr && addr <= 0xfffe) {
		// High RAM
		throw std::runtime_error{"Read from HRAM"};
	} else /* 0xffff */ {
		// Interrupt enable
		throw std::runtime_error{"Read from interrupt-enable register"};
	}
	// TODO:
	//return mapper->readByte(addr);
}

void MMU::writeByte(WORD addr, BYTE v) {
	if (addr <= 0x7fff) {
		mapper->writeByte(addr, v);
	} else if (0x8000 <= addr && addr <= 0x9fff) {
		// Video RAM
		throw std::runtime_error{"Write to VRAM"};
	} else if (0xa000 <= addr && addr <= 0xbfff) {
		// Cartridge RAM
		throw std::runtime_error{"Write to CartRAM"};
	} else if (0xc000 <= addr && addr <= 0xcfff) {
		// Work RAM (0)
		throw std::runtime_error{"Write to WRAM (0)"};
	} else if (0xd000 <= addr && addr <= 0xdfff) {
		// Work RAM (1)
		throw std::runtime_error{"Write to WRAM (1)"};
	} else if (0xe000 <= addr && addr <= 0xfdff) {
		// Echo RAM
		throw std::runtime_error{"Write to ERAM"};
	} else if (0xfe00 <= addr && addr <= 0xfe9f) {
		// Object Attribute Memory
		throw std::runtime_error{"Write to OAM"};
	} else if (0xfea0 <= addr && addr <= 0xfeff) {
		// Not usable
		throw std::runtime_error{"Write to unusable memory"};
	} else if (0xff00 <= addr && addr <= 0xff7f) {
		// IO registers
		throw std::runtime_error{"Write to IO registers"};
	} else if (0xff80 <= addr && addr <= 0xfffe) {
		// High RAM
		throw std::runtime_error{"Write to HRAM"};
	} else /* 0xffff */ {
		// Interrupt enable
		throw std::runtime_error{"Write to interrupt-enable register"};
	}
}
