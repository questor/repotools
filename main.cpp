
#define LOGURU_IMPLEMENTATION 1
#include "loguru/loguru.hpp"

#include "eaio/EAFileDirectory.h"
#include "eaio/FnEncode.h"

#include "eastl/vector.h"
#include "eastl/bitvector.h"

#include "workersystem.h"
#include "commands/checkforupdates.h"
#include "jsonio.h"
#include "report.h"

#include "anyoption/anyoption.h"

#include "json/json.hpp"
using json = nlohmann::json;

int main(int argc, char **argv) {

   AnyOption opt;

   opt.addUsage("-h  --help                     prints this help");
   opt.addUsage("    --version                  show version");
   opt.addUsage("-o  --ouput <filename>         output to file; default to console");
   opt.addUsage("-v X                           verbose level");
   opt.addUsage("-t  --type <type>              reporttype(txt, html) default:txt");
   opt.addUsage("");
   opt.addUsage("-d  --details  scan");
   opt.addUsage("               update");
   opt.addUsage("               savestate");

   opt.setFlag("help", 'h');
   opt.setFlag("version");
   opt.setOption('v');
   opt.setFlag("details", 'd');
   opt.setOption("option", 'o');

   opt.processCommandArgs(argc, argv);

   if(!opt.hasOptions()) {
      opt.printUsage();
      return -1;
   }

   loguru::init(argc, (const char **)argv);

   initJobSystem();

   eastl::vector<eastl::string> gitRepositories;
   loadGitRepositoriesFromFile(gitRepositories);

   if(strcmp("scan", opt.getArgv(opt.getArgc()-1)) == 0) {
      LOG_F(0, "doing scan of directories");
      eastl::vector<eastl::string> newGitRepositories;
      eastl::vector<eastl::string> directories;

      EA::IO::EntryFindData efd;
      directories.pushBack(".");
      while(directories.size() != 0) {
         eastl::string currentDirectory = directories[0];
         directories.erase(directories.begin() + 0);

         char16_t path[1024];
         EA::IO::StrlcpyUTF8ToUTF16(&path[0], 1024, currentDirectory.c_str());

         if(EA::IO::entryFindFirst(path, u"*", &efd)) {
            do {
               if(efd.mbIsDirectory) {

                  char8_t tmp[1024];
                  EA::IO::StrlcpyUTF16ToUTF8(&tmp[0], 1024, efd.mName);
                  //TODO: assert that efd.mName is less than 1024!
                  LOG_F(1, "checking directory %s", tmp);

                  if(EA::IO::StrEq16(efd.mName, u".git/")) {
                     LOG_F(1, "found git repo in %s", currentDirectory.c_str());
                     newGitRepositories.pushBack(currentDirectory);
                  } else {
                     if(!EA::IO::StrEq16(efd.mName, u"./") && !EA::IO::StrEq16(efd.mName, u"../")) {
                        directories.pushBack(tmp);                        
                     }
                  }
               }
            } while(EA::IO::entryFindNext(&efd));
            EA::IO::entryFindFinish(&efd);
         }         
      }

      if(opt.getFlag('d') || opt.getFlag("details")) {
         //if file is already present we can try to find new/deleted repositories
         json reportData;

         json reposArray = json::array();
         for(int i=0; i<newGitRepositories.size(); ++i) {
            reposArray.push_back(newGitRepositories[i].c_str());
         }
         reportData["repositories"] = reposArray;

         eastl::bitvector<> newRepoBits;
         newRepoBits.resize(newGitRepositories.size(), false);
         eastl::bitvector<> oldRepoBits;
         oldRepoBits.resize(gitRepositories.size(), false);

         for(int i=0; i<gitRepositories.size(); ++i) {
            for(int j=0; j<newGitRepositories.size(); ++j) {
               if(gitRepositories[i].compare(newGitRepositories[j]) == 0) {
                  newRepoBits[j] = true;
                  oldRepoBits[i] = true;
                  break;
               }
            }
         }

         json addedArray = json::array();
         for(int i=0; i<newGitRepositories.size(); ++i) {
            if(newRepoBits[i] == false) {
               addedArray.push_back(newGitRepositories[i].c_str());
            }
         }
         reportData["addedRepositories"] = addedArray;

         json deletedArray = json::array();
         for(int i=0; i<gitRepositories.size(); ++i) {
            if(oldRepoBits[i] == false) {
               deletedArray.push_back(gitRepositories[i].c_str());
            }
         }
         reportData["deletedRepositories"] = deletedArray;

         generateAndOutputReport(opt, "repoupdate", reportData);
      }

      gitRepositories = newGitRepositories;
      saveGitRepositoriesToFile(gitRepositories);
      shutdownJobSystem();
      return 0;
   }

   if(strcmp("check", opt.getArgv(opt.getArgc()-1)) == 0) {
      LOG_F(0, "checking all directories for updates");
      checkForUpdates(opt, gitRepositories);

   }

   shutdownJobSystem();

   return 0;
}
