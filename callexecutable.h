
#ifndef __CALLEXECUTABLE_H__
#define __CALLEXECUTABLE_H__

#include "eastl/string.h"
#include "eastl/vector.h"

typedef struct {
   eastl::string workingDir;
   eastl::string process;
   eastl::vector<eastl::string> arguments;

   uint32_t returnValue;
   eastl::string output;
} CallParams;

bool callExecutable(CallParams *params);

#endif   //#ifndef __CALLEXECUTABLE_H__

