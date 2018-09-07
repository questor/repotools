
#ifndef __CHECKFORUPDATES_H__
#define __CHECKFORUPDATES_H__

#include "docopt.cpp/docopt.h"

#include "eastl/vector.h"
#include "eastl/string.h"

void checkForUpdates(std::map<std::string, docopt::value> &args, eastl::vector<eastl::string> &repos);

#endif
