#pragma once
#include <stdexcept>

class ProxyException : public std::runtime_error {

public:
	ProxyException(const std::string &__arg) : runtime_error(__arg) { }
};