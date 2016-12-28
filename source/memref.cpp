#include "memref.h"

MemRef::MemRef(const WORD& addr_, IMMU& mmu_) : addr{addr_}, mmu{mmu_} {
}

MemRef::operator BYTE () const {
	return mmu.readByte(addr);
}

void MemRef::operator=(BYTE rhs) {
	mmu.writeByte(addr, rhs);
}

void MemRef::operator=(WORD rhs) {
	mmu.writeWord(addr, rhs);
}
