#include <fstream>
#include <iterator>
#include <stdexcept>

#include "mapper.h"
#include "romonly.h"

Mapper::Mapper(std::vector<BYTE>&& rom_) : rom{std::move(rom_)} {
}

std::unique_ptr<Mapper> Mapper::fromFile(const std::string& path) {
	std::vector<BYTE> rom{};
	std::ifstream f{path, std::ios::in|std::ios::binary};
	std::copy(std::istreambuf_iterator<char>{f}, {}, std::back_inserter(rom));

	// TODO: different mappers
	return std::make_unique<RomOnly>(std::move(rom));
}
