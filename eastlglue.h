#ifndef __EASTLGLUE_H__
#define __EASTLGLUE_H__

#include "eastl/EABase/eabase.h"

#include <stdlib.h>  //malloc
#include <stdarg.h>  //va_list

void* operator new[](size_t size, const char* pName, int flags, unsigned debugFlags, const char* file, int line);

void* operator new[](size_t size, size_t alignment, size_t alignmentOffset,
    const char* pName, int flags, unsigned debugFlags, const char* file, int line);

int Vsnprintf8(char8_t* pDestination, size_t n, const char8_t* pFormat, va_list arguments);

#endif
