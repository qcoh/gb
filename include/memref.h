#pragma once

#include "types.h"
#include "immu.h"

class MemRef {
	public:
		MemRef(const WORD&, IMMU&);

		MemRef(const MemRef&) = default;
		MemRef& operator=(const MemRef&) = delete;
		virtual ~MemRef() = default;

		operator BYTE () const;

		void operator=(BYTE);
		void operator=(WORD);
	private:
		const WORD& m_addr;
		IMMU& m_mmu;
};
