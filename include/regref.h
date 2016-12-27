#pragma once

#include "types.h"
#include "ref.h"

class RegRef : public Ref<BYTE> {
	public:
		RegRef(BYTE&);
		RegRef(const RegRef&) = default;
		RegRef& operator=(const RegRef&) = delete;
		virtual ~RegRef() = default;

		virtual operator BYTE () const override;
		virtual void operator=(BYTE) override;
	private:
		BYTE& reg;
};
