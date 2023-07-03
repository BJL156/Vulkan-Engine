#include "Application.hpp"

#include <stdexcept>
#include <iostream>

int main() {
	ve::Application application;
	try {
		application.run();
	} catch (std::exception& exception) {
		std::cerr << exception.what() << std::endl;
		std::cin.get();

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}