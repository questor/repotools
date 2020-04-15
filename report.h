
#ifndef __REPORT_H__
#define __REPORT_H__

#include "eastl/string.h"

#include "anyoption/anyoption.h"

#include "inja/src/inja.hpp"
using json = nlohmann::json;

//helper functions for strings
eastl::string removeNewlines(eastl::string &source);

void prepareOutputReport(AnyOption &options, eastl::string reportFilename);
void generateAndOutputReport(AnyOption &options, json reportData);

#endif
