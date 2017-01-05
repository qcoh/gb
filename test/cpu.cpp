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

		void setC(BYTE c_) {
			c = c_;
		}

		BYTE getC() {
			return c;
		}

		void setHL(WORD hl_) {
			hl = hl_;
		}

		WORD getHL() {
			return hl;
		}

		void setSP(WORD sp_) {
			sp = sp_;
		}

		WORD getSP() {
			return sp;
		}

		void setPC(WORD pc_) {
			pc = pc_;
		}

		WORD getPC() {
			return pc;
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
		TestMMU(std::array<BYTE, 0x10000>& data_) : data{data_} {}

		BYTE readByte(WORD addr) override {
			return data[addr];
		}

		void writeByte(WORD addr, BYTE v) override {
			data[addr] = v;
		}

		std::array<BYTE, 0x10000>& data;
};

SCENARIO("WORD registers should have correct endianness", "[cpu]") {
	GIVEN("CPU-derivative with BC and B accessors") {
		std::array<BYTE, 0x10000> data = {{ 0 }};
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
		std::array<BYTE, 0x10000> data = {{ 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0b10101010, 0b01010101 }};
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
		WHEN("setting (HL)") {
			cpu.setHL(0);
			cpu.setN(0xc6);
			cpu.call(0xcb);

			cpu.setN(0xce);
			cpu.call(0xcb);

			cpu.setN(0xd6);
			cpu.call(0xcb);

			cpu.setN(0xde);
			cpu.call(0xcb);

			cpu.setN(0xe6);
			cpu.call(0xcb);

			cpu.setN(0xee);
			cpu.call(0xcb);

			cpu.setN(0xf6);
			cpu.call(0xcb);

			cpu.setN(0xfe);
			cpu.call(0xcb);

			THEN("(0) == 0b11111111") {
				REQUIRE(data[0] == 0b11111111);
			}
		}
		WHEN("lding to (HL)") {
			cpu.setHL(0);
			cpu.setB(0xff);
			cpu.call(0x70);

			THEN("(0) == 0xff") {
				REQUIRE(data[0] == 0xff);
			}
		}
		WHEN("ldi-ing to (hl)") {
			cpu.setHL(0);
			cpu.setA(0x44);
			cpu.call(0x22);

			THEN("(0) == 0x44, hl == 1") {
				REQUIRE(data[0] == 0x44);
				REQUIRE(cpu.getHL() == 1);
			}
		}
		WHEN("ldding to (hl)") {
			cpu.setHL(1);
			cpu.setA(0x55);
			cpu.call(0x32);

			THEN("(1) == 0x55, hl == 0") {
				REQUIRE(data[1] == 0x55);
				REQUIRE(cpu.getHL() == 0);
			}
		}
		WHEN("ldi-ing from (hl)") {
			cpu.setHL(0);
			cpu.call(0x2a);

			THEN("a == 0xaa, hl == 1") {
				REQUIRE(cpu.getA() == 0xaa);
				REQUIRE(cpu.getHL() == 1);
			}
		}
		WHEN("ldd-ing from (hl)") {
			cpu.setHL(1);
			cpu.call(0x3a);

			THEN("a == 0xbb, hl == 0") {
				REQUIRE(cpu.getA() == 0xbb);
				REQUIRE(cpu.getHL() == 0);
			}
		}
	}
}

SCENARIO("Testing OffsetRef", "[cpu]") {
	GIVEN("CPU-derivative") {
		std::array<BYTE, 0x10000> data = {{ 0 }};
		TestMMU mmu{data};
		TestCPU cpu{mmu};

		WHEN("Writing to c+0xff00") {
			cpu.setA(0x74);
			cpu.setC(0x05);
			cpu.call(0xe2);
			
			// read to b from 0xff05
			cpu.setHL(0xff05);
			cpu.call(0x46);

			THEN("b == 0x74") {
				REQUIRE(cpu.getB() == 0x74);
			}
		}
		WHEN("Reading from c+0xff00") {
			cpu.setHL(0xff23);
			cpu.setA(0x89);
			cpu.call(0x77);

			cpu.setC(0x23);
			cpu.call(0xf2);

			THEN("c == 0x89") {
				REQUIRE(cpu.getC() == 0x89);
			}
		}
		WHEN("Writing to n+0xff00") {
			cpu.setN(0x11);
			cpu.setA(0x66);
			cpu.call(0xe0);

			cpu.setHL(0xff11);
			cpu.call(0x46);

			THEN("b == 0x66") {
				REQUIRE(cpu.getB() == 0x66);
			}
		}
		WHEN("Reading from n+0xff00") {
			cpu.setHL(0xff34);
			cpu.setB(0x12);
			cpu.call(0x70);

			cpu.setN(0x34);
			cpu.call(0xf0);

			THEN("a == 0x12") {
				REQUIRE(cpu.getA() == 0x12);
			}
		}
	}
}

SCENARIO("Testing instructions", "[cpu]") {
	GIVEN("CPU-derivative") {
		std::array<BYTE, 0x10000> data = {{ 0 }};
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
		WHEN("adding with carry and halfCarry (daa error)") {
			cpu.setA(0x98);
			cpu.setB(0x97);
			cpu.call(0x80);

			THEN("a == 0x2f, carryFlag == true, halfFlag == false") {
				REQUIRE(cpu.getA() == 0x2f);
				REQUIRE(cpu.getCarry() == true);
				REQUIRE(cpu.getHalf() == false);
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
		WHEN("rlcaing (1)") {
			cpu.setA(0b10000000);
			cpu.call(0x07);

			THEN("a == 0b0000001, carryFlag == true") {
				REQUIRE(cpu.getA() == 0b00000001);
				REQUIRE(cpu.getCarry() == true);
			}
		}
		WHEN("rlcaing (2)") {
			cpu.setA(0);
			cpu.call(0x07);

			THEN("a == 0, carryFlag == false, zeroFlag == false") {
				REQUIRE(cpu.getA() == 0);
				REQUIRE(cpu.getCarry() == false);
				REQUIRE(cpu.getZero() == false);
			}
		}
		WHEN("rrcaing (1)") {
			cpu.setA(1);
			cpu.call(0x0f);

			THEN("a == 0b10000000, carryFlag == true") {
				REQUIRE(cpu.getA() == 0b10000000);
				REQUIRE(cpu.getCarry() == true);
			}
		}
		WHEN("rrcaing (2)") {
			cpu.setA(2);
			cpu.call(0x0f);

			THEN("a == 1, carryFlag == false") {
				REQUIRE(cpu.getA() == 1);
				REQUIRE(cpu.getCarry() == false);
			}
		}
		WHEN("rlaing (1)") {
			cpu.setA(0b01000000);
			cpu.getCarry() = true;
			cpu.call(0x17);

			THEN("a == 0b10000001, carryFlag == false") {
				REQUIRE(cpu.getA() == 0b10000001);
				REQUIRE(cpu.getCarry() == false);
			}
		}
		WHEN("rlaing (2)") {
			cpu.setA(0xff);
			cpu.call(0x17);

			THEN("a == 0b11111110, carryFlag == true") {
				REQUIRE(cpu.getA() == 0b11111110);
				REQUIRE(cpu.getCarry() == true);
			}
		}
		WHEN("rraing (1)") {
			cpu.setA(0xff);
			cpu.call(0x1f);
			
			THEN("a == 0b01111111, carryFlag == true") {
				REQUIRE(cpu.getA() == 0b01111111);
				REQUIRE(cpu.getCarry() == true);
			}
		}
		WHEN("rraing (2)") {
			cpu.setA(0);
			cpu.getCarry() = true;
			cpu.call(0x1f);

			THEN("a == 0b10000000, carryFlag == false") {
				REQUIRE(cpu.getA() == 0b10000000);
				REQUIRE(cpu.getCarry() == false);
			}
		}
		WHEN("adding words (1)") {
			cpu.setHL(0x00ff);
			cpu.setBC(0x00ff);
			cpu.call(0x09);

			THEN("hl == 0x01fe, carryFlag == false, halfFlag == false") {
				REQUIRE(cpu.getHL() == 0x01fe);
				REQUIRE(cpu.getCarry() == false);
				REQUIRE(cpu.getHalf() == false);
			}
		}
		WHEN("adding words (2)") {
			cpu.setHL(0x0f00);
			cpu.setBC(0x0f00);
			cpu.call(0x09);

			THEN("hl == 0x1e00, carryFlag == false, halfFlag == true") {
				REQUIRE(cpu.getHL() == 0x1e00);
				REQUIRE(cpu.getCarry() == false);
				REQUIRE(cpu.getHalf() == true);
			}
		}
		WHEN("adding words (3)") {
			cpu.setHL(0xf000);
			cpu.setBC(0xf000);
			cpu.call(0x09);

			THEN("hl == 0xe000, carryFlag == true, halfFlag == false") {
				REQUIRE(cpu.getHL() == 0xe000);
				REQUIRE(cpu.getCarry() == true);
				REQUIRE(cpu.getHalf() == false);
			}
		}
		WHEN("adding words (4)") {
			cpu.setHL(0x1111);
			cpu.setBC(0x2222);
			cpu.call(0x09);

			THEN("hl == 0x3333, carryFlag == false, halfFlag == false") {
				REQUIRE(cpu.getHL() == 0x3333);
				REQUIRE(cpu.getCarry() == false);
				REQUIRE(cpu.getHalf() == false);
			}
		}
		WHEN("cpling") {
			cpu.setA(0b10101010);
			cpu.call(0x2f);

			THEN("a == 0b01010101") {
				REQUIRE(cpu.getA() == 0b01010101);
			}
		}
		WHEN("ccfing") {
			cpu.getCarry() = true;
			cpu.call(0x3f);

			THEN("carryFlag == false") {
				REQUIRE(cpu.getCarry() == false);
			}
		}
		WHEN("scfing") {
			cpu.call(0x37);
			
			THEN("carryFlag == true") {
				REQUIRE(cpu.getCarry() == true);
			}
		}
	}
}

BYTE dec2bcd(BYTE in) {
	return static_cast<BYTE>(((in / 10) << 4) | (in % 10));
}

SCENARIO("Testing DAA instruction", "[cpu]") {
	GIVEN("CPU-derivative") {
		std::array<BYTE, 0x10000> data = {{ 0 }};
		TestMMU mmu{data};
		TestCPU cpu{mmu};

		WHEN("adding two bcds (without carry, halfCarry") {
			cpu.setA(0x55);
			cpu.setB(0x11);
			cpu.call(0x80);
			cpu.call(0x27);

			THEN("a == 0x66, zeroFlag == false, halfFlag == false, carryFlag == false") {
				REQUIRE(cpu.getA() == dec2bcd(66));
				REQUIRE(cpu.getZero() == false);
				REQUIRE(cpu.getHalf() == false);
				REQUIRE(cpu.getCarry() == false);
			}
		}
		WHEN("adding two bcds (with halfCarry, without Carry") {
			cpu.setA(0x18);
			cpu.setB(0x04);
			cpu.call(0x80);
			cpu.call(0x27);

			THEN("a == 0x22, zeroFlag == false, halfFlag == false, carryFlag == false") {
				REQUIRE(cpu.getA() == dec2bcd(22));
				REQUIRE(cpu.getZero() == false);
				REQUIRE(cpu.getHalf() == false);
				REQUIRE(cpu.getCarry() == false);
			}
		}
		WHEN("adding two bcds (with carry, without halfCarry)") {
			cpu.setA(0x99);
			cpu.setB(0x02);
			cpu.call(0x80);
			cpu.call(0x27);

			THEN("a == 0x01, zeroFlag == false, carryFlag == true") {
				REQUIRE(cpu.getA() == dec2bcd(1));
				REQUIRE(cpu.getZero() == false);
				REQUIRE(cpu.getCarry() == true);
			}
		}
		WHEN("adding two bcds (with carry, with halfCarry)") {
			cpu.setA(0x98);
			cpu.setB(0x97);
			cpu.call(0x80);
			cpu.call(0x27);

			THEN("a == 0x95, carryFlag == true, zeroFlag == false") {
				REQUIRE(cpu.getA() == dec2bcd(95));
				REQUIRE(cpu.getZero() == false);
				REQUIRE(cpu.getCarry() == true);
			}
		}
		WHEN("adding two bcds to 0 (1)") {
			cpu.setA(0x23);
			cpu.setB(0x77);
			cpu.call(0x80);
			cpu.call(0x27);

			THEN("a == 0, carryFlag == true, zeroFlag == true") {
				REQUIRE(cpu.getA() == dec2bcd(0));
				REQUIRE(cpu.getCarry() == true);
				REQUIRE(cpu.getZero() == true);
			}
		}
		WHEN("adding two bcds to 0 (2)") {
			cpu.setA(0x50);
			cpu.setB(0x50);
			cpu.call(0x80);
			cpu.call(0x27);

			THEN("a == 0, carryFlag == true, zeroFlag == true") {
				REQUIRE(cpu.getA() == dec2bcd(0));
				REQUIRE(cpu.getCarry() == true);
				REQUIRE(cpu.getZero() == true);
			}
		}
		WHEN("adding two bcds to 0 (3)") {
			cpu.setA(0x99);
			cpu.setB(0x01);
			cpu.call(0x80);
			cpu.call(0x27);

			THEN("a == 0, carryFlag == true, zeroFlag == true") {
				REQUIRE(cpu.getA() == dec2bcd(0));
				REQUIRE(cpu.getCarry() == true);
				REQUIRE(cpu.getZero() == true);
			}
		}
		WHEN("adcing (without carry, halfCarry)") {
			cpu.setA(0x14);
			cpu.setB(0x41);
			cpu.getCarry() = true;
			cpu.call(0x88);
			cpu.call(0x27);

			THEN("a == 0x56, carryFlag == false, halfFlag == false, zeroFlag == false") {
				REQUIRE(cpu.getA() == dec2bcd(56));
				REQUIRE(cpu.getCarry() == false);
				REQUIRE(cpu.getHalf() == false);
				REQUIRE(cpu.getZero() == false);
			}
		}
		WHEN("adcing (without carry, with halfCarry)") {
			cpu.setA(0x08);
			cpu.setB(0x01);
			cpu.getCarry() = true;
			cpu.call(0x88);
			cpu.call(0x27);

			THEN("a == 0x10, carryFlag == false") {
				REQUIRE(cpu.getA() == dec2bcd(10));
				REQUIRE(cpu.getCarry() == false);
			}
		}
		WHEN("adcing (with carry, without halfCarry") {
			cpu.setA(0x99);
			cpu.setB(0);
			cpu.getCarry() = true;
			cpu.call(0x88);
			cpu.call(0x27);

			THEN("a == 0, carryFlag == true") {
				REQUIRE(cpu.getA() == dec2bcd(0));
				REQUIRE(cpu.getCarry() == true);
			}
		}
		WHEN("adcing (with carry, with halfCarry)") {
			cpu.setA(0x99);
			cpu.setB(0x99);
			cpu.getCarry() = true;
			cpu.call(0x88);
			cpu.call(0x27);

			THEN("a == 0x99, carryFlag == true") {
				REQUIRE(cpu.getA() == dec2bcd(99));
				REQUIRE(cpu.getCarry() == true);
			}
		}
		WHEN("adcing to 0") {
			cpu.setA(0x50);
			cpu.setB(0x49);
			cpu.getCarry() = true;
			cpu.call(0x88);
			cpu.call(0x27);

			THEN("a == 0, carryFlag == true") {
				REQUIRE(cpu.getA() == dec2bcd(0));
				REQUIRE(cpu.getCarry() == true);
			}
		}
		WHEN("incing a (1)") {
			cpu.setA(0x29);
			cpu.call(0x3c);
			cpu.call(0x27);

			THEN("a == 0x30, carryFlag == false") {
				REQUIRE(cpu.getA() == dec2bcd(30));
				REQUIRE(cpu.getCarry() == false);
			}
		}
		WHEN("incing a (2)") {
			cpu.setA(0x99);
			cpu.call(0x3c);
			cpu.call(0x27);

			THEN("a == 0, carryFlag == true") {
				REQUIRE(cpu.getA() == dec2bcd(0));
				REQUIRE(cpu.getCarry() == true);
			}
		}
		WHEN("popping") {
			data[0] = 0x34;
			data[1] = 0x12;
			cpu.setSP(0);
			cpu.call(0xc1);

			THEN("bc == 0x1234, sp == 0x2") {
				REQUIRE(cpu.getBC() == 0x1234);
				REQUIRE(cpu.getSP() == 2);
			}
		}
		WHEN("pushing") {
			cpu.setSP(0xffff);
			cpu.setBC(0x5678);
			cpu.call(0xc5);

			THEN("(0xfffe) == 0x56, (0xfffd) == 0x78") {
				REQUIRE(data[0xfffe] == 0x56);
				REQUIRE(data[0xfffd] == 0x78);
			}
		}
	}
}

SCENARIO("Testing extended instructions", "[cpu]") {
	GIVEN("CPU-derivative") {
		std::array<BYTE, 0x10000> data = {{ 0 }};
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
		WHEN("setting a") {
			cpu.setN(0xc7);
			cpu.setA(0);
			cpu.call(0xcb);

			THEN("a == 1") {
				REQUIRE(cpu.getA() == 1);
			}
		}
	}
}

SCENARIO("Testing control-flow instructions", "[cpu]") {
	GIVEN("CPU-derivative") {
		std::array<BYTE, 0x10000> data = {{ 0 }};
		TestMMU mmu{data};
		TestCPU cpu{mmu};

		WHEN("Calling function") {
			cpu.setPC(0x1234);
			cpu.setSP(0xffff);
			cpu.setNN(0x7788);

			cpu.call(0xcd);

			THEN("pc == 0x7788, sp == 0xfffd, (0xfffe) == 0x12, (0xfffd) == 0x34") {
				REQUIRE(cpu.getPC() == 0x7788);
				REQUIRE(cpu.getSP() == 0xfffd);
				REQUIRE(data[0xfffe] == 0x12);
				REQUIRE(data[0xfffd] == 0x34);
			}
		}
	}
}
