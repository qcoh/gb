#pragma once

#include <memory>
#include <array>

#include "immu.h"
#include "mapper.h"
#include "types.h"

class MMU : public IMMU {
	public:
		MMU(std::unique_ptr<Mapper>);

		virtual BYTE readByte(WORD) override;
		virtual void writeByte(WORD, BYTE) override;
	private:
		std::unique_ptr<Mapper> mapper;
		static std::array<BYTE, 256> bios;
		bool biosMode;
};
