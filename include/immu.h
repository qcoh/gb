#pragma once

#include <memory>
#include <array>

#include "mapper.h"
#include "types.h"

class IMMU {
	public:
		virtual BYTE readByte(WORD) = 0;
		virtual void writeByte(WORD, BYTE) = 0;
		virtual ~IMMU() = default;

		WORD readWord(WORD);
		void writeWord(WORD, WORD);
};
