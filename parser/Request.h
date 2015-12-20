#pragma once

#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include "../tools.h"

struct Request{
	std::string protocol;
	std::string method;
	std::string url;
	std::unordered_map<std::string, std::string> headers;
	std::string data;

	void print() {
		std::cout << method << " " << url << " " << protocol << std::endl;
		for (auto header : headers) {
			std::cout << header.first << ":" << header.second << std::endl;
		}
		std::cout << std::endl;
		std::cout << data.data() << std::endl;
	}

	std::string getHost() {
		auto tokens = split(url, '/');
		return tokens[1];
	}
};
