#pragma once

#include <vector>

#include "types.h"
#include "mapper.h"

class RomOnly : public Mapper {
	public:
		RomOnly(std::vector<BYTE>&&);
		virtual BYTE readByte(WORD) override;
		virtual void writeByte(WORD, BYTE) override;
};
