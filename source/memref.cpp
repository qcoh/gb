#include "memref.h"

MemRef::MemRef(const WORD& addr, IMMU& mmu) : m_addr{addr}, m_mmu{mmu} {
}

MemRef::operator BYTE () const {
	return m_mmu.readByte(m_addr);
}

void MemRef::operator=(BYTE rhs) {
	m_mmu.writeByte(m_addr, rhs);
}

void MemRef::operator=(WORD rhs) {
	m_mmu.writeWord(m_addr, rhs);
}
