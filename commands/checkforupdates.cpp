
#include "checkforupdates.h"
#include "callexecutable.h"
#include "workersystem.h"
#include "concurrentqueue/concurrentqueue.h"

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
   CallParams callParams;
   callParams.workingDir = checkParams->repositoryToCheck.c_str();
   callParams.process = "git";
   callParams.arguments.pushBack("status");
   callParams.arguments.pushBack("-s");
   callExecutable(&callParams);
   results.enqueue(callParams.output);
}

void checkForUpdates(AnyOption &options, eastl::vector<eastl::string> &repos) {
   for(int i=0; i<repos.size(); ++i) {
      CheckUpdateParameters *params = new CheckUpdateParameters();
      params->repositoryToCheck = repos[i];
      addJob(checkSingleRepo, params);
   }

   waitForAllJobsFinished();

   eastl::string result;
   while(results.try_dequeue(result)) {
      printf("result: %s\n", result.c_str());
   }
}
