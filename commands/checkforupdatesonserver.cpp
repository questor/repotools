
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

   int local = -1;
   int remote = -1;

   int numberArgsFound = sscanf(callParamsLocal.output.c_str(), "%d %d", &remote, &local);

   eastl::string result;
   if(numberArgsFound == 2) {
      if(remote == 0 && local == 0) {
         result = "u" + checkParams->repositoryToCheck;     //marker for "upToDate"
      } else if(remote != 0 && local == 0) {
         result = "n" + checkParams->repositoryToCheck;     //marker for "needsUpdate"
      } else if(remote == 0 && local != 0) {
         result = "a" + checkParams->repositoryToCheck;     //marker for "ahead"
      } else {
         result = "d" + checkParams->repositoryToCheck;     //marker for "diverged"         
      }
   } else {
      result = "f" + checkParams->repositoryToCheck;     //marker for "failed to get status"      
   }

   results.enqueue(result);
   delete checkParams;
}

void checkForUpdatesOnServer(AnyOption &options, eastl::vector<eastl::string> &repos) {

   //prepare to have fast fail it template is not available
   prepareOutputReport(options, "checkforupdatesonserver");
 
   for(int i=0; i<repos.size(); ++i) {
      CheckUpdateParameters *params = new CheckUpdateParameters();
      params->repositoryToCheck = repos[i];
      addJob(checkSingleRepo, params);
   }

   waitForAllJobsFinished();

   LOG_F(1, "finished checking for updates");

   json reportData;
   json reposUpToDate = json::array();
   json reposNeedUpdate = json::array();
   json reposAhead = json::array();
   json reposDiverged = json::array();
   json reposFailed = json::array();
   eastl::string result;
   while(results.try_dequeue(result)) {
      if(result[0] == 'u') {
         reposUpToDate.push_back(result.substr(1).c_str());
      } else if(result[0] == 'n') {
         reposNeedUpdate.push_back(result.substr(1).c_str());
      } else if(result[0] == 'a') {
         reposAhead.push_back(result.substr(1).c_str());
      } else if(result[0] == 'd') {
         reposDiverged.push_back(result.substr(1).c_str());
      } else if(result[0] == 'f') {
         reposFailed.push_back(result.substr(1).c_str());
      } else {
         LOG_F(1, "NO MARKER FOUND WHAT TO DO?!");
      }

   }
   reportData["reposUpToDate"] = reposUpToDate;
   reportData["reposNeedUpdate"] = reposNeedUpdate;
   reportData["reposAhead"] = reposAhead;
   reportData["reposDiverged"] = reposDiverged;
   reportData["reposFailed"] = reposFailed;

   generateAndOutputReport(options, reportData);
}
