
#ifndef __REPORT_H__
#define __REPORT_H__

#include "eastl/string.h"

#include "anyoption/anyoption.h"

#include "inja/src/inja.hpp"
using json = nlohmann::json;

void generateAndOutputReport(AnyOption &options, eastl::string reportFilename, json reportData);

#endif
