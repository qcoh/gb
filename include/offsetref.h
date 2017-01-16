#pragma once

#include "types.h"
#include "immu.h"

template <WORD offset>
class OffsetRef {
	public:
		OffsetRef(const BYTE& addr, IMMU& mmu) : m_addr{addr}, m_mmu{mmu} {}

		OffsetRef(const OffsetRef&) = default;
		OffsetRef& operator=(const OffsetRef&) = delete;
		virtual ~OffsetRef() = default;

		operator BYTE () const {
			return m_mmu.readByte(static_cast<WORD>(m_addr + offset));
		}

		void operator=(BYTE rhs) {
			m_mmu.writeByte(static_cast<WORD>(m_addr + offset), rhs);
		}
	private:
		const BYTE& m_addr;
		IMMU& m_mmu;
};
