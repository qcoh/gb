#include <array>

#include "catch.hpp"
#include "immu.h"

class TestMMU : public IMMU {
	public:
		TestMMU(std::array<BYTE, 1024>& data_) : data{data_} {}
		BYTE readByte(WORD addr) override {
			return data[addr];
		}
		void writeByte(WORD addr, BYTE v) override {
			data[addr] = v;
		}
		std::array<BYTE, 1024>& data;
};

SCENARIO("reading a WORD (from bios) should have correct endianness", "[mmu]") {
	GIVEN("a MMU containing an emptry RomOnly") {
		std::array<BYTE, 1024> data = {{ 0xfe, 0xff }};
		TestMMU mmu{data};

		WHEN("reading a WORD") {
			WORD w = mmu.readWord(0);

			THEN("WORD has correct endianess") {
				REQUIRE(w == 0xfffe);
			}
		}
	}
}
