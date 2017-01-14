#pragma once

#include "types.h"
#include "bitref.h"

struct InterruptState {
	// Interrupt Master Enable
	bool ime = false;

	// Interrupt Flag (0xff0f)
	BYTE intFlag = 0;
	BitRef<BYTE, 0> vBlankReq{intFlag};
	BitRef<BYTE, 1> lcdStatReq{intFlag};
	BitRef<BYTE, 2> timerReq{intFlag};
	BitRef<BYTE, 3> serialReq{intFlag};
	BitRef<BYTE, 4> joypadReq{intFlag};

	// Interrupt Enable (0xffff)
	BYTE intEnable = 0;
	BitRef<BYTE, 0> vBlank{intEnable};
	BitRef<BYTE, 1> lcdStat{intEnable};
	BitRef<BYTE, 2> timer{intEnable};
	BitRef<BYTE, 3> serial{intEnable};
	BitRef<BYTE, 4> joypad{intEnable};
};
