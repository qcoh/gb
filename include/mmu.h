#pragma once

#include <memory>
#include <array>

#include "mapper.h"
#include "types.h"

class MMU {
	public:
		MMU(std::unique_ptr<Mapper>);

		BYTE readByte(WORD);
		WORD readWord(WORD);
		void writeByte(WORD, BYTE);
		void writeWord(WORD, WORD);
	private:
		std::unique_ptr<Mapper> mapper;
		static std::array<BYTE, 256> bios;
		bool biosMode;
};
