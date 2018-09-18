
#include "mergelist.h"
#include "callexecutable.h"
#include "workersystem.h"
#include "concurrentqueue/concurrentqueue.h"

#include "report.h"
#include "repostateio.h"

#include "eastl/bitvector.h"
#include "eaio/PathString.h"

#include "loguru/loguru.hpp"

#include "cachedrepositoryio.h"

class MergeListParameters : public WorkerParams {
public:
   eastl::string repositoryToClone;
};

void createRepo(WorkerParams *params) {
   MergeListParameters *saveParams = (MergeListParameters*)params;


   EA::IO::Path::PathString8 path(saveParams->repositoryToClone.c_str());
   EA::IO::Path::PathString8::iterator lastFolder = EA::IO::Path::getPathComponentStart(path.begin(), path.end(), -1);

   EA::IO::Path::PathString8 frontPath(path.begin(), lastFolder);
   EA::IO::Path::PathString8 dirPath(lastFolder, path.end());

   CallParams callParams;
   callParams.workingDir = saveParams->repositoryToClone.c_str();
   callParams.process = "git";
   callParams.arguments.pushBack("clone");
   callParams.arguments.pushBack("@{0}");
//   callExecutable(&callParams);

   printf("frontPath: %s\n", frontPath.c_str());
   printf("dirPath: %s\n", dirPath.c_str());

   callParams.output = removeNewlines(callParams.output);

   delete saveParams;
}

void mergeList(AnyOption &options, eastl::vector<eastl::string> &repos) {

   //LOAD NEW LIST
   eastl::vector<eastl::string> toMergeRepositories;
   loadGitRepositoriesFromFile(options.getArgv(options.getArgc()-1), toMergeRepositories);

   //MERGE WITH CURRENT ONE AND REMOVE ALREADY EXISTING ENTRIES
   eastl::bitvector<> newRepoBits;
   newRepoBits.resize(toMergeRepositories.size(), true);

   for(int i=0; i<toMergeRepositories.size(); ++i) {
      for(int j=0; j<repos.size(); ++j) {
         if(toMergeRepositories[i].compare(repos[j]) == 0) {
            newRepoBits[i] = false;
            break;
         }
      }
   }

   //CREATE NEW REPOSITORIES BY CLONING
   for(int i=0; i<toMergeRepositories.size(); ++i) {
      if(newRepoBits[i] == true) {
         MergeListParameters *params = new MergeListParameters();
         params->repositoryToClone = repos[i];
         addJob(createRepo, params);
         repos.pushBack(repos[i]);
      }
   }

   waitForAllJobsFinished();

   //SAVE NEW LIST
   saveGitRepositoriesToFile(repos);
}
