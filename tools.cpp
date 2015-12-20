#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <netinet/in.h>
#include "tools.h"
#include <arpa/inet.h>


std::string getError(int error) {
	return std::string(strerror(error));
}

std::vector<std::string> split(std::string str, char delimiter) {
	auto size = str.size();
	std::vector<std::string> tokens;
	unsigned int prev = 0;
	for (unsigned int i = 0; i < size; ++i) {
		if (str[i] == delimiter) {
			auto token = str.substr(prev, i - prev);
			if (!token.empty()) {
				tokens.push_back(token);
			}
			prev = i + 1;
		}
	}
	auto token = str.substr(prev, size - prev);
	if (!token.empty()) {
		tokens.push_back(token);
	}
	return tokens;
}

std::string getIp(sockaddr_in * socketAddress) {
	void *addr = &(socketAddress->sin_addr);
	char buf[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, addr, buf, sizeof buf);
	return std::string(buf);
}