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

class TestInstructionCPU : public CPU {
	public:
		TestInstructionCPU(MMU&& mmu_) : CPU(std::move(mmu_)) {
		}

		void call(BYTE op) {
			for (auto& in : instructions) {
				if (in.opcode == op) {
					in.f();
				}
			}
		}

		WORD getBC() {
			return bc;
		}
};

SCENARIO("Testing instructions", "[cpu]") {
	GIVEN("CPU-derivative with BC accessor") {
		auto ptr = std::make_unique<RomOnly>(std::vector<BYTE>{});
		TestInstructionCPU cpu{MMU{std::move(ptr)}};

		WHEN("incrementing bc") {
			cpu.call(0x03);

			THEN("bc == 1") {
				REQUIRE(cpu.getBC() == 1);
			}
		}
	}
}
