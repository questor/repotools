
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
   callParamsLocal.arguments.pushBack("rev-parse");
   callParamsLocal.arguments.pushBack("@{0}");
   callExecutable(&callParamsLocal);

   CallParams callParamsRemote;
   callParamsRemote.workingDir = checkParams->repositoryToCheck.c_str();
   callParamsRemote.process = "git";
   callParamsRemote.arguments.pushBack("ls-remote");
   callParamsRemote.arguments.pushBack("origin");
   callParamsRemote.arguments.pushBack("-h");
   callParamsRemote.arguments.pushBack("refs/heads/master");
   callExecutable(&callParamsRemote);

   callParamsLocal.output = removeNewlines(callParamsLocal.output);
   callParamsRemote.output = removeNewlines(callParamsRemote.output);

   eastl::string result;
   if(callParamsRemote.output.find(callParamsLocal.output) == 0) {
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
