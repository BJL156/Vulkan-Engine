#include "Application.h"

#include <stdexcept>

int main(void) {
	eng::Application application{};
	try {
		application.run();
	} catch (const std::exception& exception) {
		std::cerr << exception.what() << '\n';
		std::cin.get();

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}