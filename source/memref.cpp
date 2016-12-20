#include "memref.h"

MemRef::MemRef(const WORD& addr_, MMU& mmu_) : addr{addr_}, mmu{mmu_} {
}

MemRef::operator BYTE () {
	return mmu.readByte(addr);
}

MemRef::operator WORD () {
	return mmu.readWord(addr);
}

void MemRef::operator=(BYTE rhs) {
	mmu.writeByte(addr, rhs);
}

void MemRef::operator=(WORD rhs) {
	mmu.writeWord(addr, rhs);
}
