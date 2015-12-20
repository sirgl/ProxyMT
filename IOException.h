#pragma once

#include "ProxyException.h"

class IOException : public ProxyException {

public:
	IOException(const std::string &__arg) : ProxyException(__arg) { }
};
