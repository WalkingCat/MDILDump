#pragma once

#include "mdil_data.h"

class mdil_parser
{
public:
	std::string parse(const wchar_t* filename, mdil_data& data);
};

