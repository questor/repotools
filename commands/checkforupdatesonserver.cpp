
#include "checkforupdatesonserver.h"
#include "callexecutable.h"
#include "workersystem.h"
#include "concurrentqueue/concurrentqueue.h"

#include "report.h"

#include "loguru/loguru.hpp"

class CheckUpdateParameters : public WorkerParams {
public:
   eastl::string repositoryToCheck;
};
moodycamel::ConcurrentQueue<eastl::string> results;


void checkSingleRepo(WorkerParams *params) {
   CheckUpdateParameters *checkParams = (CheckUpdateParameters*)params;
#if 0
   typedef struct {
      eastl::string workingDir;
      eastl::string process;
      eastl::vector<eastl::string> arguments;

      uint32_t returnValue;
      eastl::string output;
   } CallParams;
   bool callExecutable(CallParams *params);
#endif
   CallParams callParamsLocal;
   callParamsLocal.workingDir = checkParams->repositoryToCheck.c_str();
   callParamsLocal.process = "git";
   callParamsLocal.arguments.pushBack("rev-list");
   callParamsLocal.arguments.pushBack("--count");
   callParamsLocal.arguments.pushBack("--left-right");
   callParamsLocal.arguments.pushBack("@{upstream}...HEAD");
   callExecutable(&callParamsLocal);

   callParamsLocal.output = removeNewlines(callParamsLocal.output);

   //  0    0 -> none
   //  x    0 -> behind
   //  0    x -> ahead
   //  other  -> diverged

TODO!

   eastl::string result;
   if(callParamsLocal.output.find(callParamsLocal.output) == 0) {
      //is up to date
      result = "u" + checkParams->repositoryToCheck;     //marker for "upToDate"
   } else {
      result = "n" + checkParams->repositoryToCheck;     //marker for "needsUpdate"
   }

   results.enqueue(result);
   delete checkParams;
}

void checkForUpdatesOnServer(AnyOption &options, eastl::vector<eastl::string> &repos) {
   for(int i=0; i<repos.size(); ++i) {
      CheckUpdateParameters *params = new CheckUpdateParameters();
      params->repositoryToCheck = repos[i];
      addJob(checkSingleRepo, params);
   }

   waitForAllJobsFinished();

   json reportData;
   json reposUpToDate = json::array();
   json reposNeedUpdate = json::array();
   eastl::string result;
   while(results.try_dequeue(result)) {
      if(result[0] == 'u') {
         reposUpToDate.push_back(result.substr(1).c_str());
      } else {
         reposNeedUpdate.push_back(result.substr(1).c_str());
      }
   }
   reportData["reposUpToDate"] = reposUpToDate;
   reportData["reposNeedUpdate"] = reposNeedUpdate;

   generateAndOutputReport(options, "checkforupdatesonserver", reportData);
}
