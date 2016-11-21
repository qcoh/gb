#include <memory>

#include "catch.hpp"
#include "romonly.h"
#include "mmu.h"

SCENARIO("reading a WORD should have correct endianness", "[mmu]") {
	GIVEN("a MMU containing a RomOnly containing a BYTE vector") {
		std::vector<BYTE> v = {0x0, 0xff, 0xaa, 0xbb};
		auto ptr = std::make_unique<RomOnly>(std::move(v));
		MMU mmu{std::move(ptr)};

		WHEN("reading a WORD") {
			WORD w = mmu.readWord(0);
			REQUIRE(w == 0xff00);
		}
	}
}
