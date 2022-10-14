
#ifndef __COMMANDS_SAVESTATE_H__
#define __COMMANDS_SAVESTATE_H__

#include "eastl/vector.h"
#include "eastl/string.h"

class AnyOption;

void saveState(AnyOption &options, eastl::vector<eastl::string> &repos);

#endif

