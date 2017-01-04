#pragma once

#include "types.h"
#include "immu.h"

template <WORD offset>
class OffsetRef {
	public:
		OffsetRef(const BYTE& addr_, IMMU& mmu_) : addr{addr_}, mmu{mmu_} {}

		OffsetRef(const OffsetRef&) = default;
		OffsetRef& operator=(const OffsetRef&) = delete;
		virtual ~OffsetRef() = default;

		operator BYTE () const {
			return mmu.readByte(static_cast<WORD>(addr + offset));
		}

		void operator=(BYTE rhs) {
			mmu.writeByte(static_cast<WORD>(addr + offset), rhs);
		}
	private:
		const BYTE& addr;
		IMMU& mmu;
};
