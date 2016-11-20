#include "romonly.h"

RomOnly::RomOnly(std::vector<BYTE>&& rom) : Mapper{std::move(rom)} {
}
