#pragma once

#include "ref.h"
#include "types.h"
#include "immu.h"

class MemRef : public Ref<BYTE>, public Ref<WORD> {
	public:
		MemRef(const WORD&, IMMU&);

		MemRef(const MemRef&) = default;
		MemRef& operator=(const MemRef&) = delete;
		virtual ~MemRef() = default;

		virtual operator BYTE () const override;
		virtual operator WORD () const override;

		virtual void operator=(BYTE) override;
		virtual void operator=(WORD) override;
	private:
		const WORD& addr;
		IMMU& mmu;
};
