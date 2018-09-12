
#include "generateupdatereport.h"
#include "callexecutable.h"
#include "workersystem.h"
#include "concurrentqueue/concurrentqueue.h"

#include "report.h"

#include "loguru/loguru.hpp"

class GenerateUpdateReportParameters : public WorkerParams {
public:
   eastl::string repositoryToCheck;
   eastl::string oldRevision;
   eastl::string updatesDone;
};
moodycamel::ConcurrentQueue<GenerateUpdateReportParameters*> resultsUpdateReport;


void updateReportSingleRepo(WorkerParams *params) {
   GenerateUpdateReportParameters *saveParams = (GenerateUpdateReportParameters*)params;

   CallParams callParams;
   callParams.workingDir = saveParams->repositoryToCheck.c_str();
   callParams.process = "git";
   callParams.arguments.pushBack("rev-parse");
   callParams.arguments.pushBack("@{0}");
   callExecutable(&callParams);

   callParams.output = removeNewlines(callParams.output);

   GenerateUpdateReportParameters->revision = callParams.output;
   resultsUpdateReport.enqueue(saveParams);
}

void generateUpdateReport(AnyOption &options, eastl::vector<eastl::string> &repos) {

   //TODO: read in file with old content

   for(int i=0; i<repos.size(); ++i) {
      GenerateUpdateReportParameters *params = new GenerateUpdateReportParameters();
      params->repositoryToCheck = repos[i];
      addJob(updateReportSingleRepo, params);
   }

   waitForAllJobsFinished();

   json reportData = json::array();
   GenerateUpdateReportParameters *result;
   while(resultsSaveState.try_dequeue(result)) {
      json oneResult;
      oneResult["repoPath"] = result->repositoryToCheck.c_str();
      oneResult["revision"] = result->revision.c_str();
      reportData.push_back(oneResult);
      delete result;
   }

//   generateAndOutputReport(options, "savestate", reportData);
}
