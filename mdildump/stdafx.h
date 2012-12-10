#pragma once

#include "targetver.h"

#include <stdio.h>

#pragma warning(push)
#pragma warning(disable : 4345 )
#include <memory>
#pragma warning(pop)

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <map>
#include <functional>

#define VC_EXTRALEAN
#include <windows.h>
#include <atlbase.h>

#include <CorHdr.h>
#include <cor.h>
#include <metahost.h>