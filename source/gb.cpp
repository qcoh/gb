#include <iostream>
#include <exception>

#include "cxxopts.hpp"
#include "mapper.h"
#include "cpu.h"

void debugRepl(CPU&);

int main(int argc, char *argv[]) {
	cxxopts::Options options{"gb", "A GameBoy emulator"};
	options.add_options()
		("d,debug", "Enable debugging")
		("f,file", "File name", cxxopts::value<std::string>());
	options.parse(argc, argv);

	try {
		auto debug = options["d"].as<bool>();
		auto filename = options["f"].as<std::string>();
		if (filename == "") {
			throw std::runtime_error{"No rom"};
		}

		CPU cpu{Mapper::fromFile(filename)};
		if (debug) {
			debugRepl(cpu);
		}
	} catch (std::exception& e) {
		std::cerr << e.what() << '\n';
	}
}

void debugRepl(CPU& cpu) {
	(void)cpu;
	std::string s{};
	for (;;) {
		std::cout << "> ";
		std::getline(std::cin, s);
		std::cout << "  " << s << '\n';
	}
}
