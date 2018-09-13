
#ifndef __REPOSTATEIO_H__
#define __REPOSTATEIO_H__

#include "json/json.hpp"

/* json format expected:
   json reportData = json::array();
   while(resultsSaveState.try_dequeue(result)) {
      oneResult["repoPath"] = result->repositoryToCheck.c_str();
      oneResult["revision"] = result->revision.c_str();
      reportData.push_back(oneResult);
   }*/
void saveState(char *filename, nlohmann::json &jsondata);

nlohmann::json loadState(char *filename);

#endif
