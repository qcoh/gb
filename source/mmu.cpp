#include "mmu.h"

MMU::MMU(std::unique_ptr<Mapper> mapper) : 
	mapper{std::move(mapper)},
	bios{},
	biosMode{true}
{
	// unused error
	(void)bios;
	(void)biosMode;
}

BYTE MMU::readByte(WORD addr) {
	// TODO
	(void)addr;
	return 0;
}

WORD MMU::readWord(WORD addr) {
	return static_cast<WORD>(readByte(addr) | readByte(addr+1) << 8);
}

void MMU::writeByte(WORD addr, BYTE v) {
	// TODO
	(void)addr;
	(void)v;
}

void MMU::writeWord(WORD addr, WORD v) {
	// TODO
	(void)addr;
	(void)v;
}
