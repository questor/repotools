
#ifndef __COMMANDS_MERGELIST_H__
#define __COMMANDS_MERGELIST_H__

#include "eastl/vector.h"
#include "eastl/string.h"

class AnyOption;

void mergeList(AnyOption &options, eastl::vector<eastl::string> &repos);

#endif

