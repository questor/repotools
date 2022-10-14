
#include "mergelist.h"
#include "callexecutable.h"
#include "workersystem.h"
#include "concurrentqueue/concurrentqueue.h"

#include "report.h"
#include "repostateio.h"

#include "eastl/string.h"
#include "eastl/bitvector.h"

#include "loguru/loguru.hpp"

#include "cachedrepositoryio.h"

class MergeListParameters : public WorkerParams {
public:
   eastl::string sourceUrl;
   eastl::string directory;
};

void createRepo(WorkerParams *params) {
   MergeListParameters *saveParams = (MergeListParameters*)params;

   //when a folder (or folder-structure) is specified as clone-destination the folders will
   // be created by automatically, but the project-name needs to be appended in that case

   //extract project name
   size_t lastFolderSymbol = saveParams->sourceUrl.findLastOf("/");
   size_t extensionStart = saveParams->sourceUrl.size()-lastFolderSymbol-4;   //cut ".git" from path
   eastl::string projectName = saveParams->sourceUrl.substr(lastFolderSymbol+1, extensionStart-1);

   printf("syncing %s (%s)\n", projectName.c_str(), saveParams->sourceUrl.c_str());

   eastl::string destDir = saveParams->directory + projectName;

   CallParams callParams;
   callParams.workingDir = ".";
//   callParams.workingDir = saveParams->directory.c_str();
   callParams.process = "git";
   callParams.arguments.pushBack("clone");
   callParams.arguments.pushBack(saveParams->sourceUrl);
   callParams.arguments.pushBack(destDir.c_str());
   callExecutable(&callParams);

//   callParams.output = removeNewlines(callParams.output);

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
         repos.pushBack(toMergeRepositories[i].directory);
      }
   }

   waitForAllJobsFinished();

   //SAVE NEW LIST
   //saveGitRepositoriesToFile(repos);
}
