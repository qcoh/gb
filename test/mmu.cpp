#include <memory>

#include "catch.hpp"
#include "romonly.h"
#include "mmu.h"

SCENARIO("reading a WORD (from bios) should have correct endianness", "[mmu]") {
	GIVEN("a MMU containing an emptry RomOnly") {
		std::vector<BYTE> data = { 0xfe, 0xff };
		auto ptr = std::make_unique<RomOnly>(std::move(data));
		MMU mmu{std::move(ptr)};

		WHEN("reading a WORD") {
			WORD w = mmu.readWord(0);

			THEN("WORD has correct endianess") {
				REQUIRE(w == 0xfffe);
			}
		}
	}
}
