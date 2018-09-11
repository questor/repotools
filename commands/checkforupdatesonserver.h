
#ifndef __CHECKFORUPDATESONSERVER_H__
#define __CHECKFORUPDATESONSERVER_H__

#include "eastl/vector.h"
#include "eastl/string.h"

class AnyOption;

void checkForUpdatesOnServer(AnyOption &options, eastl::vector<eastl::string> &repos);

#endif
