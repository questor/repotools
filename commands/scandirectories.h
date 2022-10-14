
#ifndef __COMMANDS_SCANDIRECTORIES_H__
#define __COMMANDS_SCANDIRECTORIES_H__

#include "eastl/vector.h"
#include "eastl/string.h"

class AnyOption;

void scanDirectories(AnyOption &options, eastl::vector<eastl::string> &repos);

#endif
