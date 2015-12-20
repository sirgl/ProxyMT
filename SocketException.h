#pragma once

#include "IOException.h"

class SocketException : public IOException {
public:
	SocketException(const std::string &__arg) : IOException(__arg) { }
};