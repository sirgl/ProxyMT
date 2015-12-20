#include <iostream>
#include "Proxy.h"
#include "ProxyException.h"

int main(int argc, char** argv) {
	if(argc < 2) {
		std::cerr << "Usage: Proxy <port>" << std::endl;
		return EXIT_FAILURE;
	}

	try {
		Proxy* proxy = new Proxy(argv[1]);
		proxy->start();
	} catch (ProxyException& ex) {
		std::cerr << "Exception: " << ex.what() << std::endl;
	}

	return EXIT_SUCCESS;
}