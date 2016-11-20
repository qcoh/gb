#include <iostream>
#include <exception>

#include "mapper.h"

int main(int args, char *argv[]) {
	if (args != 2) {
		std::cout << "gb [rom]\n";
		return 0;
	}
	try {
		auto mapper = Mapper::fromFile(argv[1]);
		(void)mapper;
	} catch (std::exception& e) {
		std::cerr << e.what() << '\n';
	}
}
