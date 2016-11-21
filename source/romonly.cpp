#include "romonly.h"

RomOnly::RomOnly(std::vector<BYTE>&& rom) : Mapper{std::move(rom)} {
}

BYTE RomOnly::readByte(WORD addr) {
	return rom[addr];
}
