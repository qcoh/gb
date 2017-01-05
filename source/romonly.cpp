#include "romonly.h"

RomOnly::RomOnly(std::vector<BYTE>&& rom_) : Mapper{std::move(rom_)} {
}

BYTE RomOnly::readByte(WORD addr) {
	return rom[addr];
}

void RomOnly::writeByte(WORD addr, BYTE v) {
	(void)addr;
	(void)v;

	// TODO: memory bank controller
}
