
#ifndef __COMMANDS_PULLREPOSITORIES_H__
#define __COMMANDS_PULLREPOSITORIES_H__

#include "eastl/vector.h"
#include "eastl/string.h"

class AnyOption;

void pullRepositories(AnyOption &options, eastl::vector<eastl::string> &repos);

#endif

