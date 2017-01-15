#pragma once

#include <memory>
#include <array>

#include "immu.h"
#include "mapper.h"
#include "types.h"
#include "gpu.h"
#include "interruptstate.h"

class MMU : public IMMU {
	public:
		MMU(std::unique_ptr<Mapper>&&, GPU&, InterruptState&);

		virtual BYTE readByte(WORD) override;
		virtual void writeByte(WORD, BYTE) override;

	private:
		// ROM/BIOS: 0x0000 to 0x7fff
		std::unique_ptr<Mapper> mapper;
		static std::array<BYTE, 256> bios;

		GPU& gpu;
		InterruptState& intState;

		bool biosMode = true;

		std::array<BYTE, 127> hram = {{ 0 }};

		std::array<BYTE, 4096> wram0 = {{ 0 }};
		std::array<BYTE, 4096> wram1 = {{ 0 }};
};
