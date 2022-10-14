
#include "generateupdatereport.h"
#include "callexecutable.h"
#include "workersystem.h"
#include "concurrentqueue/concurrentqueue.h"

#include "report.h"

#include "loguru/loguru.hpp"

#include "json/json.hpp"
using json = nlohmann::json;

#include "repostateio.h"

#if defined(__linux)
#include <strings.h>
#endif
#ifdef _MSC_VER
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif


class GenerateUpdateReportParameters : public WorkerParams {
public:
   eastl::string repositoryToCheck;
   eastl::string oldRevision;
   eastl::string newRevision;
   eastl::string updateLog;
};
moodycamel::ConcurrentQueue<GenerateUpdateReportParameters*> resultsUpdateReport;


// generate report what was pulled in the last pull:
//   git diff --name-status ORIG_HEAD..

// git log --oneline REV..HEAD

void updateReportSingleRepo(WorkerParams *params) {
   GenerateUpdateReportParameters *saveParams = (GenerateUpdateReportParameters*)params;

   CallParams callParams;
   callParams.workingDir = saveParams->repositoryToCheck.c_str();
   callParams.process = "git";
   callParams.arguments.pushBack("log");
   callParams.arguments.pushBack("--oneline");
   callParams.arguments.pushBack(saveParams->oldRevision+".."+saveParams->newRevision);
   callExecutable(&callParams);

   callParams.output = removeNewlines(callParams.output);

   saveParams->updateLog = callParams.output;
   resultsUpdateReport.enqueue(saveParams);
}

void generateUpdateReport(AnyOption &options, eastl::vector<eastl::string> &repos) {
   int oldFilenameArgcIndex = 1;
   int newFilenameArgcIndex = -1;

   if(strcasecmp("genUpdateReport", options.getArgv(options.getArgc()-3)) == 0) {
      oldFilenameArgcIndex = 2;
      newFilenameArgcIndex = 1;
   }

   nlohmann::json oldState = loadState(options.getArgv(options.getArgc()-oldFilenameArgcIndex));

   nlohmann::json newState;
   if(newFilenameArgcIndex != -1) {
      newState = loadState(options.getArgv(options.getArgc()-newFilenameArgcIndex));
   }

   //prepare to have fast fail it template is not available
   prepareOutputReport(options, "updatereport");

   for(int i=0; i<repos.size(); ++i) {
      GenerateUpdateReportParameters *params = new GenerateUpdateReportParameters();
      params->repositoryToCheck = repos[i];

      bool oldFound = false;
      for(json::iterator it = oldState.begin(); it != oldState.end(); ++it) {
         json element = *it;
         if(repos[i].compare(element["repoPath"].get<std::string>().c_str()) == 0) {
            params->oldRevision = element["revision"].get<std::string>().c_str();
            oldFound = true;
            break;
         }
      }
      assert(oldFound);

      if(newFilenameArgcIndex != -1) {
         bool newFound = false;
         for(json::iterator it = newState.begin(); it != newState.end(); ++it) {
            json element = *it;
            if(repos[i].compare(element["repoPath"].get<std::string>().c_str()) == 0) {
               params->newRevision = element["revision"].get<std::string>().c_str();
               newFound = true;
               break;
            }
         }
         assert(newFound);
      } else {
         params->newRevision = "HEAD";
      }

      //check if oldRevision == newRevision?

      addJob(updateReportSingleRepo, params);
   }

   waitForAllJobsFinished();

   json updateLogs = json::array();
   GenerateUpdateReportParameters *result;
   while(resultsUpdateReport.try_dequeue(result)) {
      if(result->updateLog.size() != 0) {
         json oneResult;
         oneResult["repoPath"] = result->repositoryToCheck.c_str();
         oneResult["updateLog"] = result->updateLog.c_str();
         updateLogs.push_back(oneResult);
      }
      delete result;
   }

   json reportData;
   reportData["updatelogs"] = updateLogs;

   generateAndOutputReport(options, reportData);
}
