#pragma once

#include <memory>
#include <array>

#include "immu.h"
#include "mapper.h"
#include "types.h"
#include "gpu.h"

class MMU : public IMMU {
	public:
		MMU(std::unique_ptr<Mapper>&&, GPU&);

		virtual BYTE readByte(WORD) override;
		virtual void writeByte(WORD, BYTE) override;

	private:
		// ROM/BIOS: 0x0000 to 0x7fff
		std::unique_ptr<Mapper> mapper;
		static std::array<BYTE, 256> bios;

		GPU& gpu;

		bool biosMode;

		BYTE interruptFlag;
		BYTE interruptEnable;

		std::array<BYTE, 127> hram;

		std::array<BYTE, 4096> wram0;
		std::array<BYTE, 4096> wram1;
};
