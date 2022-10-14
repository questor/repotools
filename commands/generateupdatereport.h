
#ifndef __COMMANDS_GENERATEUPDATEREPORT_H__
#define __COMMANDS_GENERATEUPDATEREPORT_H__

#include "eastl/vector.h"
#include "eastl/string.h"

class AnyOption;

void generateUpdateReport(AnyOption &options, eastl::vector<eastl::string> &repos);

#endif

