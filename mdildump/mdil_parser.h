#pragma once

#include "mdil_data.h"

class mdil_parser
{
public:
	std::string parse(const char* filename, mdil_data& data);
};

