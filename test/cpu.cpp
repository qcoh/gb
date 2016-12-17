#include "catch.hpp"
#include "cpu.h"
#include "romonly.h"

class TestCPU : public CPU {
	public:
		TestCPU(MMU&& mmu_) : CPU(std::move(mmu_)) {
		}

		void call(BYTE op) {
			for (auto& in : instructions) {
				if (in.opcode == op) {
					in.f();
				}
			}
		}

		void setBC(WORD bc_) {
			bc = bc_;
		}

		WORD getBC() {
			return bc;
		}

		void setNN(WORD nn_) {
			nn = nn_;
		}
		
		void setN(BYTE n_) {
			n = n_;
		}

		void setB(BYTE b_) {
			b = b_;
		}

		BYTE getB() {
			return b;
		}

		void setA(BYTE a_) {
			a = a_;
		}

		BYTE getA() {
			return a;
		}

		auto getCarry() -> decltype(carryFlag) {
			return carryFlag;
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

SCENARIO("Testing instructions", "[cpu]") {
	GIVEN("CPU-derivative") {
		auto ptr = std::make_unique<RomOnly>(std::vector<BYTE>{});
		TestCPU cpu{MMU{std::move(ptr)}};

		WHEN("incrementing bc") {
			cpu.call(0x03); // INC BC

			THEN("bc == 1") {
				REQUIRE(cpu.getBC() == 1);
			}
		}
		WHEN("decrementing bc") {
			cpu.call(0x0b); // DEC BC
			THEN("bc == 0xffff") {
				REQUIRE(cpu.getBC() == 0xffff);
			}
		}
		WHEN("loading immediate word") {
			cpu.setNN(0xf0f0);
			cpu.call(0x01); // LD BC, nn

			THEN("bc == 0xf0f0") {
				REQUIRE(cpu.getBC() == 0xf0f0);
			}
		}
		WHEN("loading immediate byte") {
			cpu.setN(0xaa);
			cpu.call(0x06); // LD B, n

			THEN("b == 0xaa") {
				REQUIRE(cpu.getB() == 0xaa);
			}
		}
		WHEN("loading register from register") {
			cpu.setA(0xbb);
			cpu.call(0x47); // LD B, A

			THEN("b == 0xbb") {
				REQUIRE(cpu.getB() == 0xbb);
			}
		}
		WHEN("adding to a") {
			cpu.setA(0x0f);
			cpu.setB(0xf0);
			cpu.call(0x80);

			THEN("a == 0xff") {
				REQUIRE(cpu.getA() == 0xff);
			}
		}
		WHEN("adcing to a") {
			cpu.setA(0x0e);
			cpu.setB(0xf0);
			auto c = cpu.getCarry();
			c = true;
			cpu.call(0x88);

			THEN("a == 0xff") {
				REQUIRE(cpu.getA() == 0xff);
			}
		}
	}
}
