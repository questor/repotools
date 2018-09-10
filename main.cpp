
#define LOGURU_IMPLEMENTATION 1
#include "loguru/loguru.hpp"

#include "eaio/EAFileDirectory.h"
#include "eaio/FnEncode.h"

#include "eastl/vector.h"

#include "workersystem.h"
#include "commands/checkforupdates.h"
#include "jsonio.h"

#include "anyoption/anyoption.h"

int main(int argc, char **argv) {

   AnyOption opt;

   opt.addUsage("-h  --help                     prints this help");
   opt.addUsage("    --version                  show version");
   opt.addUsage("-v X                           verbose level");
   opt.addUsage("");
   opt.addUsage("-d  --details  scan");
   opt.addUsage("               update");
   opt.addUsage("               savestate");

   opt.setFlag("help", 'h');
   opt.setFlag("version");
   opt.setOption('v');
   opt.setFlag("details", 'd');

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
         //compare to original file!
      }

      gitRepositories = newGitRepositories;
      saveGitRepositoriesToFile(gitRepositories);
      shutdownJobSystem();
      return 0;
   }

   loadGitRepositoriesFromFile(gitRepositories);

   if(strcmp("check", opt.getArgv(opt.getArgc()-1)) == 0) {
      LOG_F(0, "checking all directories for updates");
      checkForUpdates(opt, gitRepositories);

   }

   shutdownJobSystem();

   return 0;
}