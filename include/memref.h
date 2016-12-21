#pragma once

#include "types.h"
#include "mmu.h"

class MemRef {
	public:
		MemRef(const WORD&, MMU&);

		MemRef(const MemRef&) = default;
		MemRef& operator=(const MemRef&) = delete;

		operator BYTE () const;
		operator WORD () const;

		void operator=(BYTE);
		void operator=(WORD);
	private:
		const WORD& addr;
		MMU& mmu;
};
