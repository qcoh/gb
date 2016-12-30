#include "catch.hpp"
#include "cpu.h"
#include "romonly.h"
#include "mmu.h"

class TestCPU : public CPU {
	public:
		TestCPU(IMMU& mmu_) : CPU{mmu_} {
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

		void setHL(WORD hl_) {
			hl = hl_;
		}

		void setSP(WORD sp_) {
			sp = sp_;
		}

		auto getCarry() -> decltype(carryFlag)& {
			return carryFlag;
		}

		auto getHalf() -> decltype(halfFlag)& {
			return halfFlag;
		}

		auto getZero() -> decltype(zeroFlag)& {
			return zeroFlag;
		}

		auto getNeg() -> decltype(negFlag)& {
			return negFlag;
		}
};

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

SCENARIO("WORD registers should have correct endianness", "[cpu]") {
	GIVEN("CPU-derivative with BC and B accessors") {
		std::array<BYTE, 1024> data = {{ 0 }};
		TestMMU mmu{data};
		TestCPU cpu{mmu};

		WHEN("setting bc to 0xff00") {
			cpu.setBC(0xff00);

			THEN("b == 0xff") {
				auto b = cpu.getB();
				REQUIRE(b == 0xff);
			}
		}
	}
}


SCENARIO("Testing MemRef", "[cpu]") {
	GIVEN("CPU-derivative") {
		std::array<BYTE, 1024> data = {{ 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0b10101010, 0b01010101 }};
		TestMMU mmu{data};
		TestCPU cpu{mmu};

		WHEN("setting hl = 1, reading (hl) to a") {
			cpu.setHL(1);
			cpu.call(0x7e);

			THEN("a == 0xbb") {
				REQUIRE(cpu.getA() == 0xbb);
			}
		}
		WHEN("lding sp to (nn), lding (nn) to a, (nn+1) to b") {
			cpu.setNN(0x12);
			cpu.setSP(0xffee);
			cpu.call(0x08);

			cpu.setHL(0x12);
			cpu.call(0x7e);
			cpu.setHL(0x13);
			cpu.call(0x46);

			THEN("a == 0xee, b == 0xff") {
				REQUIRE(cpu.getA() == 0xee);
				REQUIRE(cpu.getB() == 0xff);
			}
		}
		WHEN("bitting (HL) (1)") {
			cpu.setHL(1);
			cpu.setN(0x46);
			cpu.call(0xcb);

			THEN("zeroFlag == false") {
				REQUIRE(cpu.getZero() == false);
			}
		}
		WHEN("bitting (HL) (2)") {
			cpu.setHL(0);
			cpu.setN(0x46);
			cpu.call(0xcb);

			THEN("zeroFlag == true") {
				REQUIRE(cpu.getZero() == true);
			}
		}
		WHEN("bitting (HL) (3)") {
			cpu.setHL(6);
			cpu.setN(0x7e);
			cpu.call(0xcb);

			THEN("zeroFlag == false") {
				REQUIRE(cpu.getZero() == false);
			}
		}
		WHEN("bitting (HL) (4)") {
			cpu.setHL(7);
			cpu.setN(0x7e);
			cpu.call(0xcb);

			THEN("zeroFlag == true") {
				REQUIRE(cpu.getZero() == true);
			}
		}
		WHEN("resetting (HL)") {
			cpu.setHL(0);
			cpu.setN(0x86);
			cpu.call(0xcb);

			cpu.setN(0x8e);
			cpu.call(0xcb);

			cpu.setN(0x96);
			cpu.call(0xcb);

			cpu.setN(0x9e);
			cpu.call(0xcb);

			cpu.setN(0xa6);
			cpu.call(0xcb);

			cpu.setN(0xae);
			cpu.call(0xcb);

			cpu.setN(0xb6);
			cpu.call(0xcb);

			cpu.setN(0xbe);
			cpu.call(0xcb);

			THEN("(0) == 0") {
				REQUIRE(data[0] == 0);
			}
		}
	}
}

SCENARIO("Testing instructions", "[cpu]") {
	GIVEN("CPU-derivative") {
		std::array<BYTE, 1024> data = {{ 0 }};
		TestMMU mmu{data};
		TestCPU cpu{mmu};

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
		WHEN("anding (no zero)") {
			cpu.setA(0xff);
			cpu.setB(0x22);
			cpu.call(0xa0);

			THEN("a == 0x22, zeroFlag == false") {
				REQUIRE(cpu.getA() == 0x22);
				REQUIRE(cpu.getZero() == false);
			}
		}
		WHEN("anding (zero)") {
			cpu.setA(0xf0);
			cpu.setB(0x0f);
			cpu.call(0xa0);

			THEN("a == 0x00, zeroFlag == true") {
				REQUIRE(cpu.getA() == 0x00);
				REQUIRE(cpu.getZero() == true);
			}
		}
		WHEN("xoring (no zero)") {
			cpu.setA(0xf0);
			cpu.setB(0x00);
			cpu.call(0xa8);

			THEN("a == 0xf0, zeroFlag == false") {
				REQUIRE(cpu.getA() == 0xf0);
				REQUIRE(cpu.getZero() == false);
			}
		}
		WHEN("xoring (zero)") {
			cpu.setA(0xaa);
			cpu.setB(0xaa);
			cpu.call(0xa8);

			THEN("a == 0x00, zeroFlag == true") {
				REQUIRE(cpu.getA() == 0x00);
				REQUIRE(cpu.getZero() == true);
			}
		}
		WHEN("oring (no zero)") {
			cpu.setA(0xb0);
			cpu.setB(0x0a);
			cpu.call(0xb0);

			THEN("a == 0xba, zeroFlag == false") {
				REQUIRE(cpu.getA() == 0xba);
				REQUIRE(cpu.getZero() == false);
			}
		}
		WHEN("oring (zero)") {
			cpu.setA(0x00);
			cpu.setB(0x00);
			cpu.call(0xb0);

			THEN("a == 0x00, zeroFlag == true") {
				REQUIRE(cpu.getA() == 0x00);
				REQUIRE(cpu.getZero() == true);
			}
		}
		WHEN("cping (less)") {
			cpu.setA(0x00);
			cpu.setB(0x01);
			cpu.call(0xb8);

			THEN("zeroFlag == false, halfFlag == true, carryFlag == true") {
				REQUIRE(cpu.getZero() == false);
				REQUIRE(cpu.getHalf() == true);
				REQUIRE(cpu.getCarry() == true);
			}
		}
		WHEN("cping (more)") {
			cpu.setA(0xff);
			cpu.setB(0x0f);
			cpu.call(0xb8);

			THEN("zeroFlag == false, halfFlag == false, carryFlag == false") {
				REQUIRE(cpu.getZero() == false);
				REQUIRE(cpu.getHalf() == false);
				REQUIRE(cpu.getCarry() == false);
			}
		}
		WHEN("cping (equal)") {
			cpu.setA(0x55);
			cpu.setB(0x55);
			cpu.call(0xb8);

			THEN("zeroFlag == true, halfFlag == false, carryFlag == false") {
				REQUIRE(cpu.getZero() == true);
				REQUIRE(cpu.getHalf() == false);
				REQUIRE(cpu.getCarry() == false);
			}
		}
		WHEN("inc a (overflow)") {
			cpu.setA(0xff);
			cpu.call(0x3c);

			THEN("a == 0x00, zeroFlag == true, halfFlag == true") {
				REQUIRE(cpu.getA() == 0x00);
				REQUIRE(cpu.getZero() == true);
				REQUIRE(cpu.getHalf() == true);
			}
		}
		WHEN("inc a (halfCarry, no overflow)") {
			cpu.setA(0x0f);
			cpu.call(0x3c);

			THEN("a == 0x10, zeroFlag == false, halfFlag == true") {
				REQUIRE(cpu.getA() == 0x10);
				REQUIRE(cpu.getZero() == false);
				REQUIRE(cpu.getHalf() == true);
			}
		}
		WHEN("inc a (no overflow)") {
			cpu.setA(0x00);
			cpu.call(0x3c);

			THEN("a == 0x01, zeroFlag == false, halfFlag == false") {
				REQUIRE(cpu.getA() == 0x01);
				REQUIRE(cpu.getZero() == false);
				REQUIRE(cpu.getHalf() == false);
			}
		}
	}
}

SCENARIO("Testing extended instructions", "[cpu]") {
	GIVEN("CPU-derivative") {
		std::array<BYTE, 1024> data = {{ 0 }};
		TestMMU mmu{data};
		TestCPU cpu{mmu};

		WHEN("rlcing a") {
			cpu.setN(0x07);
			cpu.setA(0b11111111);
			cpu.call(0xcb);

			THEN("a == 0b11111111, carryFlag == true") {
				REQUIRE(cpu.getA() == 0b11111111);
				REQUIRE(cpu.getCarry() == true);
			}
		}
		WHEN("rrcing a") {
			cpu.setN(0x0f);
			cpu.setA(0b10011001);
			cpu.call(0xcb);

			THEN("a == 0b11001100, carryFlag == true") {
				REQUIRE(cpu.getA() == 0b11001100);
				REQUIRE(cpu.getCarry() == true);
			}
		}
		WHEN("rrcing a (without carry)") {
			cpu.setN(0x0f);
			cpu.setA(0b11110000);
			cpu.call(0xcb);

			THEN("a == 0b01111000, carryFlag == false") {
				REQUIRE(cpu.getA() == 0b01111000);
				REQUIRE(cpu.getCarry() == false);
			}
		}
		WHEN("rling a (1)") {
			cpu.setN(0x17);
			cpu.setA(0);
			cpu.getCarry() = true;
			cpu.call(0xcb);

			THEN("a == 0b0000001, carryFlag == false") {
				REQUIRE(cpu.getA() == 1);
				REQUIRE(cpu.getCarry() == false);
			}
		}
		WHEN("rling a (2)") {
			cpu.setN(0x17);
			cpu.setA(0b10101010);
			cpu.getCarry() = false;
			cpu.call(0xcb);

			THEN("a == 0b01010100, carryFlag == true") {
				REQUIRE(cpu.getA() == 0b01010100);
				REQUIRE(cpu.getCarry() == true);
			}
		}
		WHEN("rring a (1)") {
			cpu.setN(0x1f);
			cpu.setA(0);
			cpu.getCarry() = true;
			cpu.call(0xcb);

			THEN("a == 0b10000000, carryFlag == false") {
				REQUIRE(cpu.getA() == 0b10000000);
				REQUIRE(cpu.getCarry() == false);
			}
		}
		WHEN("rring a (2)") {
			cpu.setN(0x1f);
			cpu.setA(0b10101010);
			cpu.getCarry() = true;
			cpu.call(0xcb);

			THEN("a == 0b11010101, carryFlag == false") {
				REQUIRE(cpu.getA() == 0b11010101);
				REQUIRE(cpu.getCarry() == false);
			}
		}
		WHEN("slaing a (1)") {
			cpu.setN(0x27);
			cpu.setA(0b01111111);
			cpu.call(0xcb);

			THEN("a == 0b11111110, carryFlag == false") {
				REQUIRE(cpu.getA() == 0b11111110);
				REQUIRE(cpu.getCarry() == false);
			}
		}
		WHEN("slaing a (2)") {
			cpu.setN(0x27);
			cpu.setA(0b10000000);
			cpu.call(0xcb);

			THEN("a == 0, carryFlag == true, zeroFlag == true") {
				REQUIRE(cpu.getA() == 0);
				REQUIRE(cpu.getCarry() == true);
				REQUIRE(cpu.getZero() == true);
			}
		}
		WHEN("sraing a (1)") {
			cpu.setN(0x2f);
			cpu.setA(0b10000001);
			cpu.call(0xcb);

			THEN("a == 0b11000000, carryFlag == true") {
				REQUIRE(cpu.getA() == 0b11000000);
				REQUIRE(cpu.getCarry() == true);
			}
		}
		WHEN("sraing a (2)") {
			cpu.setN(0x2f);
			cpu.setA(1);
			cpu.call(0xcb);

			THEN("a == 0, carryFlag == true, zeroFlag == true") {
				REQUIRE(cpu.getA() == 0);
				REQUIRE(cpu.getCarry() == true);
				REQUIRE(cpu.getZero() == true);
			}
		}
		WHEN("swapping a (1)") {
			cpu.setN(0x37);
			cpu.setA(0b11110000);
			cpu.call(0xcb);

			THEN("a == 0b00001111, zeroFlag == false") {
				REQUIRE(cpu.getA() == 0b00001111);
				REQUIRE(cpu.getZero() == false);
			}
		}
		WHEN("swapping a (2)") {
			cpu.setN(0x37);
			cpu.setA(0);
			cpu.call(0xcb);

			THEN("a == 0, zeroFlag == true") {
				REQUIRE(cpu.getA() == 0);
				REQUIRE(cpu.getZero() == true);
			}
		}
		WHEN("srling a (1)") {
			cpu.setN(0x3f);
			cpu.setA(0b11111111);
			cpu.call(0xcb);

			THEN("a == 0b01111111, carryFlag == true") {
				REQUIRE(cpu.getA() == 0b01111111);
				REQUIRE(cpu.getCarry() == true);
			}
		}
		WHEN("srling a (2)") {
			cpu.setN(0x3f);
			cpu.setA(0b11111110);
			cpu.call(0xcb);

			THEN("a == 0b01111111, carryFlag == false") {
				REQUIRE(cpu.getA() == 0b01111111);
				REQUIRE(cpu.getCarry() == false);
			}
		}
		WHEN("bitting a (1)") {
			cpu.setN(0x47);
			cpu.setA(1);
			cpu.call(0xcb);

			THEN("zeroFlag == false") {
				REQUIRE(cpu.getZero() == false);
			}
		}
		WHEN("bitting a (2)") {
			cpu.setN(0x47);
			cpu.setA(0b11111110);
			cpu.call(0xcb);

			THEN("zeroFlag == true") {
				REQUIRE(cpu.getZero() == true);
			}
		}
		WHEN("resetting a") {
			cpu.setN(0x87);
			cpu.setA(1);
			cpu.call(0xcb);

			THEN("a == 0") {
				REQUIRE(cpu.getA() == 0);
			}
		}
	}
}
