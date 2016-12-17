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

		auto getHalf() -> decltype(halfFlag) {
			return halfFlag;
		}

		auto getZero() -> decltype(zeroFlag) {
			return zeroFlag;
		}

		auto getNeg() -> decltype(negFlag) {
			return negFlag;
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
		WHEN("adding with half-carry") {
			cpu.setA(0x0f);
			cpu.setB(0x0f);
			cpu.call(0x80);

			THEN("a == 0x1e, halfFlag == true") {
				REQUIRE(cpu.getA() == 0x1e);
				REQUIRE(cpu.getHalf() == true);
			}
		}
		WHEN("adding with carry") {
			cpu.setA(0x80);
			cpu.setB(0x80);
			cpu.call(0x80);

			THEN("a == 0x00, carryFlag = true, zeroFlag == true") {
				REQUIRE(cpu.getA() == 0x00);
				REQUIRE(cpu.getCarry() == true);
				REQUIRE(cpu.getZero() == true);
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
		WHEN("adcing with half-carry and carry") {
			cpu.setA(0xff);
			cpu.setB(0x00);
			auto c = cpu.getCarry();
			c = true;
			cpu.call(0x88);

			THEN("a == 0x00, zeroFlag == true, halfFlag == true, carryFlag == true") {
				REQUIRE(cpu.getA() == 0x00);
				REQUIRE(cpu.getHalf() == true);
				REQUIRE(cpu.getZero() == true);
				REQUIRE(cpu.getCarry() == true);
			}
		}
		WHEN("subtracting without carry, halfcarry") {
			cpu.setA(0x0f);
			cpu.setB(0x0e);
			cpu.call(0x90);

			THEN("a == 0x01, zeroFlag = false, halfFlag = false, carryFlag == false, negFlag == true") {
				REQUIRE(cpu.getA() == 0x01);
				REQUIRE(cpu.getZero() == false);
				REQUIRE(cpu.getHalf() == false);
				REQUIRE(cpu.getCarry() == false);
				REQUIRE(cpu.getNeg() == true);
			}
		}
		WHEN("subtracting with carry, halfcarry") {
			cpu.setA(0xee);
			cpu.setB(0xff);
			cpu.call(0x90);

			THEN("a == 0xef, carryFlag == true, halfFlag == true, zeroFlag == false") {
				REQUIRE(cpu.getA() == 0xef);
				REQUIRE(cpu.getHalf() == true);
				REQUIRE(cpu.getCarry() == true);
				REQUIRE(cpu.getZero() == false);
			}
		}
		WHEN("sbcing with carry, halfcarry") {
			cpu.setA(0x01);
			cpu.setB(0x01);
			auto c = cpu.getCarry();
			c = true;
			cpu.call(0x98);

			THEN("a == 0xff, carryFlag == true, halfFlag == true, zeroFlag == false") {
				REQUIRE(cpu.getA() == 0xff);
				REQUIRE(cpu.getHalf() == true);
				REQUIRE(cpu.getCarry() == true);
				REQUIRE(cpu.getZero() == false);
			}
		}
	}
}
