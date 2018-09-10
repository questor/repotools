
#ifndef __CHECKFORUPDATES_H__
#define __CHECKFORUPDATES_H__

#include "eastl/vector.h"
#include "eastl/string.h"

class AnyOption;

void checkForUpdates(AnyOption &options, eastl::vector<eastl::string> &repos);

#endif
