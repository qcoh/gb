#pragma once

#include <memory>
#include <string>
#include <vector>

#include "types.h"

class Mapper {
	public:
		virtual ~Mapper() = default;
		virtual BYTE readByte(WORD) = 0;

		static std::unique_ptr<Mapper> fromFile(const std::string&);
	protected:
		Mapper(std::vector<BYTE>&&);
		std::vector<BYTE> rom;
};
