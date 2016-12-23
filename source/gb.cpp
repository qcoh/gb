#include <iostream>
#include <exception>

#include "cxxopts.hpp"
#include "mapper.h"
#include "cpu.h"

int main(int argc, char *argv[]) {
	cxxopts::Options options{"gb", "A GameBoy emulator"};
	options.add_options()
		("d,debug", "Enable debugging")
		("f,file", "File name", cxxopts::value<std::string>());
	options.parse(argc, argv);

	try {
		auto debug = options["d"].as<bool>();
		(void)debug;
		auto filename = options["f"].as<std::string>();
		if (filename == "") {
			throw std::runtime_error{"No rom"};
		}

	} catch (std::exception& e) {
		std::cerr << e.what() << '\n';
	}
}

