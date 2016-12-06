#include "catch.hpp"
#include "cpu.h"
#include "romonly.h"

class TestCPU : public CPU {
	public:
		TestCPU(MMU&& mmu_) : CPU(std::move(mmu_)) {
		}
		void setBC(WORD bc_) {
			bc = bc_;
		}
		BYTE getB() {
			return *b;
		}
};

SCENARIO("WORD registers should have correct endianness", "[cpu]") {
	GIVEN("CPU-derivative with BC and B accessors") {
		auto ptr = std::make_unique<RomOnly>(std::vector<BYTE>{});
		TestCPU cpu{MMU{std::move(ptr)}};

		WHEN("setting bc to 0xff00") {
			cpu.setBC(0xff00);

			THEN("b == 0xff") {
				auto b = cpu.getB();
				REQUIRE(b == 0xff);
			}
		}
	}
}
