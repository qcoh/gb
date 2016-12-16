#include <functional>

#include "catch.hpp"
#include "bitref.h"
#include "types.h"

SCENARIO("Verify bitref", "[bitref]") {
	GIVEN("BYTE (1)") {
		BYTE foo = 0x10; // 16 = 0b0001000

		WHEN("bitref of foo (at 1 location)") {
			BitRef<BYTE, 4> br{foo};

			THEN("implicit conversion operator yields true") {
				REQUIRE(br == true);
			}
		}
		WHEN("bitref of foo (at 0 location)") {
			BitRef<BYTE, 5> br{foo};

			THEN("implicit conversion operator yields false") {
				REQUIRE(br == false);
			}
		}
	}

	GIVEN("WORD (1), function bound to bitref") {
		WORD foo = 0xffff;
		auto l = [](bool b) -> bool { return b; };
		auto f = std::bind(l, BitRef<WORD, 10>{foo});

		WHEN("Modifying foo (1)") {
			foo = 0x00ff;

			THEN("Return value of f == false") {
				REQUIRE(f() == false);
			}
		}
		WHEN("Modifying foo (2)") {
			foo = 0xff00;

			THEN("Return value of f == true") {
				REQUIRE(f() == true);
			}
		}
	}

	GIVEN("BYTE (2)") {
		BYTE foo = 0;
		BitRef<BYTE, 0> b0{foo};
		BitRef<BYTE, 1> b1{foo};
		BitRef<BYTE, 2> b2{foo};
		BitRef<BYTE, 3> b3{foo};
		BitRef<BYTE, 4> b4{foo};
		BitRef<BYTE, 5> b5{foo};
		BitRef<BYTE, 6> b6{foo};
		BitRef<BYTE, 7> b7{foo};


		WHEN("reassign foo") {
			foo = 0xaa; // 0b10101010

			THEN("even bits == 0, odd bits == 1") {
				REQUIRE(b0 == false);
				REQUIRE(b2 == false);
				REQUIRE(b4 == false);
				REQUIRE(b6 == false);
				REQUIRE(b1 == true);
				REQUIRE(b3 == true);
				REQUIRE(b5 == true);
				REQUIRE(b7 == true);
			}
		}
	}

	GIVEN("BYTE (3)") {
		BYTE foo = 0;
		BitRef<BYTE, 1> rf{foo};

		WHEN("setting the 2nd bit") {
			rf = true;

			THEN("foo == 0b0000 0010") {
				REQUIRE(foo == 0x02);
			}
		}
	}
	
	GIVEN("BYTE (4)") {
		BYTE foo = 0xff;
		BitRef<BYTE, 7> rf{foo};

		WHEN("unsetting the 7th bit") {
			rf = false;

			THEN("foo == 0b0111 1111") {
				REQUIRE(foo == 0x7f);
			}
		}
	}
}

