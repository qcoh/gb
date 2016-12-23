#include "immu.h"

WORD IMMU::readWord(WORD addr) {
	return static_cast<WORD>(readByte(addr)) | static_cast<WORD>(readByte(addr+1) << 8);
}

void IMMU::writeWord(WORD addr, WORD v) {
	writeByte(addr, static_cast<BYTE>(v));
	writeByte(addr+1, static_cast<BYTE>(v >> 8));
}
