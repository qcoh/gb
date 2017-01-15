#pragma once

#include <string>
#include <functional>
#include "types.h"

struct Instruction {
	BYTE opcode = 0;
	std::function<void(void)> f;
	std::string description;
	BYTE cycles = 0;
	BYTE offset = 0;
};
