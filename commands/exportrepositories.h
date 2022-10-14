
#ifndef __EXPORTREPOSITORIES_H__
#define __EXPORTREPOSITORIES_H__

#include "eastl/vector.h"
#include "eastl/string.h"

class AnyOption;

void exportRepositories(AnyOption &options, eastl::vector<eastl::string> &repos);

#endif
