
#include "pullrepositories.h"
#include "callexecutable.h"
#include "workersystem.h"
#include "concurrentqueue/concurrentqueue.h"

#include "report.h"

#include "loguru/loguru.hpp"

class PullRepositoryParameters : public WorkerParams {
public:
   eastl::string repositoryToPull;
   eastl::string result;       //this is the output parameter!
};
moodycamel::ConcurrentQueue<PullRepositoryParameters*> resultsPull;

void pullSingleRepo(WorkerParams *params) {
   PullRepositoryParameters *pullParams = (PullRepositoryParameters*)params;

   CallParams callParams;
   callParams.workingDir = pullParams->repositoryToPull.c_str();
   callParams.process = "git";
   callParams.arguments.pushBack("pull");
   callExecutable(&callParams);

   callParams.output = removeNewlines(callParams.output);

   pullParams->result = callParams.output;
   resultsPull.enqueue(pullParams);
}

void pullRepositories(AnyOption &options, eastl::vector<eastl::string> &repos) {

   //prepare to have fast fail it template is not available
   prepareOutputReport(options, "pullrepositories");

   for(int i=0; i<repos.size(); ++i) {
      PullRepositoryParameters *params = new PullRepositoryParameters();
      params->repositoryToPull = repos[i];
      addJob(pullSingleRepo, params);
   }

   waitForAllJobsFinished();

   json reportData;
   json pulledRepos = json::array();
   json alreadyUpToDateRepos = json::array();

   PullRepositoryParameters *result;
   while(resultsPull.try_dequeue(result)) {
      if(result->result.compare("Already up to date.") == 0) {
         alreadyUpToDateRepos.push_back(result->repositoryToPull.c_str());
      } else {
         json pullDescr;
         pullDescr["repository"] = result->repositoryToPull.c_str();
         pullDescr["output"] = result->result.c_str();
         pulledRepos.push_back(pullDescr);
      }
      delete result;
   }

   reportData["pulledRepos"] = pulledRepos;
   reportData["alreadyUpToDateRepos"] = alreadyUpToDateRepos;

   generateAndOutputReport(options, reportData);
}
