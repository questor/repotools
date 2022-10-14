
#include "savestate.h"
#include "callexecutable.h"
#include "workersystem.h"
#include "concurrentqueue/concurrentqueue.h"

#include "report.h"
#include "repostateio.h"

#include "loguru/loguru.hpp"

class SaveStateParameters : public WorkerParams {
public:
   eastl::string repositoryToCheck;
   eastl::string revision;       //this is the output parameter!
};
moodycamel::ConcurrentQueue<SaveStateParameters*> resultsSaveState;


void saveStateSingleRepo(WorkerParams *params) {
   SaveStateParameters *saveParams = (SaveStateParameters*)params;

   CallParams callParams;
   callParams.workingDir = saveParams->repositoryToCheck.c_str();
   callParams.process = "git";
   callParams.arguments.pushBack("rev-parse");
   callParams.arguments.pushBack("@{0}");
   callExecutable(&callParams);

   callParams.output = removeNewlines(callParams.output);

   saveParams->revision = callParams.output;
   resultsSaveState.enqueue(saveParams);
}

void saveState(AnyOption &options, eastl::vector<eastl::string> &repos) {
   for(int i=0; i<repos.size(); ++i) {
      SaveStateParameters *params = new SaveStateParameters();
      params->repositoryToCheck = repos[i];
      addJob(saveStateSingleRepo, params);
   }

   waitForAllJobsFinished();

   json reportData = json::array();
   SaveStateParameters *result;
   while(resultsSaveState.try_dequeue(result)) {
      json oneResult;
      oneResult["repoPath"] = result->repositoryToCheck.c_str();
      oneResult["revision"] = result->revision.c_str();
      reportData.push_back(oneResult);
      delete result;
   }

   saveState(options.getArgv(options.getArgc()-1), reportData);

//   generateAndOutputReport(options, "savestate", reportData);
}
