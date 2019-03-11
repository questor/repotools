
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
   eastl::string sourceUrl;
   eastl::string directory;
};

void createRepo(WorkerParams *params) {
   MergeListParameters *saveParams = (MergeListParameters*)params;

//TODO: create directory path one below the git repo?

   EA::IO::Path::PathString8 path(saveParams->directory.c_str());
   EA::IO::Path::PathString8::iterator lastFolder = EA::IO::Path::getPathComponentStart(path.begin(), path.end(), -1);

   EA::IO::Path::PathString8 frontPath(path.begin(), lastFolder);
   EA::IO::Path::PathString8 dirPath(lastFolder, path.end());

   CallParams callParams;
   callParams.workingDir = saveParams->directory.c_str();
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

   FILE *fp = fopen(options.getArgv(options.getArgc()-1), "rb");
   if(!fp) {
      //TODO: error handling!
      printf("could not open input file to merge the list!\n");
      return;
   }
   fseek(fp, 0, SEEK_END);
   int contentLength = ftell(fp);
   fseek(fp, 0, SEEK_SET);
   char *content = new char[contentLength+10];
   memset(content, 0, contentLength+10);
   fread(content, sizeof(char), contentLength, fp);
   fclose(fp);

   json toMergeList = json::parse(content);
   delete[] content;

   typedef struct {
      eastl::string directory;
      eastl::string sourceUrl;
   } Item;
   eastl::vector<Item> toMergeRepositories;

   for(json::iterator it = toMergeList.begin(); it != toMergeList.end(); ++it) {
      json element = *it;

      Item i;
      i.directory = (*it)["directory"].get<std::string>().c_str();
      i.sourceUrl = (*it)["sourceurl"].get<std::string>().c_str();
      toMergeRepositories.pushBack(i);
   }

   //MERGE WITH CURRENT ONE AND REMOVE ALREADY EXISTING ENTRIES
   eastl::bitvector<> newRepoBits;
   newRepoBits.resize(toMergeRepositories.size(), true);

   for(int i=0; i<toMergeRepositories.size(); ++i) {
      for(int j=0; j<repos.size(); ++j) {
         if(toMergeRepositories[i].directory.compare(repos[j]) == 0) {
            newRepoBits[i] = false;
            break;
         }
      }
   }

   //CREATE NEW REPOSITORIES BY CLONING
   for(int i=0; i<toMergeRepositories.size(); ++i) {
      if(newRepoBits[i] == true) {
         MergeListParameters *params = new MergeListParameters();
         params->sourceUrl = toMergeRepositories[i].sourceUrl;
         params->directory = toMergeRepositories[i].directory;
         addJob(createRepo, params);
         repos.pushBack(repos[i]);
      }
   }

   waitForAllJobsFinished();

   //SAVE NEW LIST
   //saveGitRepositoriesToFile(repos);
}
