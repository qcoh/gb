#pragma once

#include <memory>
#include <array>

#include "immu.h"
#include "mapper.h"
#include "types.h"

class MMU : public IMMU {
	public:
		MMU(std::unique_ptr<Mapper>&&);

		virtual BYTE readByte(WORD) override;
		virtual void writeByte(WORD, BYTE) override;

	private:
		// ROM/BIOS: 0x0000 to 0x7fff
		std::unique_ptr<Mapper> mapper;
		static std::array<BYTE, 256> bios;

		// VRAM; 0x8000 to 0x9fff
		std::array<BYTE, 0x2000> vram;

		bool biosMode;

		BYTE interruptFlag;
		BYTE interruptEnable;
};
