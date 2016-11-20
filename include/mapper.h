#pragma once

#include <memory>
#include <string>
#include <vector>

#include "types.h"

class Mapper {
	public:
		virtual ~Mapper() = default;

		static std::unique_ptr<Mapper> fromFile(const std::string&);
	protected:
		Mapper(std::vector<BYTE>&&);
	private:
		std::vector<BYTE> rom;

};
