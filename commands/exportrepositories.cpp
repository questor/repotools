
#include "exportrepositories.h"
#include "callexecutable.h"
#include "workersystem.h"
#include "concurrentqueue/concurrentqueue.h"

#include "report.h"

#include "loguru/loguru.hpp"

class ExportParameters : public WorkerParams {
public:
   eastl::string repositoryToCheck;
   eastl::string sourceUrl;
};
moodycamel::ConcurrentQueue<ExportParameters*> exportResults;


void exportSingleRepo(WorkerParams *params) {
   ExportParameters *checkParams = (ExportParameters*)params;

   CallParams callParamsLocal;
   callParamsLocal.workingDir = checkParams->repositoryToCheck.c_str();
   callParamsLocal.process = "git";
   callParamsLocal.arguments.pushBack("config");
   callParamsLocal.arguments.pushBack("--get");
   callParamsLocal.arguments.pushBack("remote.origin.url");
   callExecutable(&callParamsLocal);

   callParamsLocal.output = removeNewlines(callParamsLocal.output);
   checkParams->sourceUrl = callParamsLocal.output;

   exportResults.enqueue(checkParams);
}

void exportRepositories(AnyOption &options, eastl::vector<eastl::string> &repos) {
   for(int i=0; i<repos.size(); ++i) {
      ExportParameters *params = new ExportParameters();
      params->repositoryToCheck = repos[i];
      addJob(exportSingleRepo, params);
   }

   waitForAllJobsFinished();

   json repolist = json::array();
   ExportParameters *exportParams;
   while(exportResults.try_dequeue(exportParams)) {
      json item;
      item["directory"] = exportParams->repositoryToCheck.c_str();
      item["sourceurl"] = exportParams->sourceUrl.c_str();
      repolist.push_back(item);
      delete exportParams;
   }

   FILE *fp = fopen(options.getArgv(options.getArgc()-1), "wb");
   if(!fp) {
      //TODO: error handling!
      printf("could not open output file to export the list!\n");
      return;
   }
   std::string dump = repolist.dump();
   fwrite(dump.c_str(), sizeof(char), dump.length(), fp);
   fclose(fp);
}
