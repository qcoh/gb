#pragma once

#include <string>
#include <functional>
#include "types.h"

struct Instruction {
	BYTE opcode;
	std::function<void(void)> f;
	std::string description;
};
