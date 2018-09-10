
#include "docopt.cpp/docopt.h"

#define LOGURU_IMPLEMENTATION 1
#include "loguru/loguru.hpp"

#include "eaio/EAFileDirectory.h"
#include "eaio/FnEncode.h"

#include "eastl/vector.h"

#include "workersystem.h"
#include "commands/checkforupdates.h"
#include "jsonio.h"

int main(int argc, const char **argv) {

   static const char USAGE[] =
   R"(RepoTool!
       Usage:
         repotool (-h | --help)
         repotool --version
         repotool [-v <nr>] [-d|--details] scan
         repotool [-v <nr>] check
         repotool [-v <nr>] update
         repotool [-v <nr>] savestate <filename>
    
       Options:
         -d --details      show details (new/deleted repos)
   )";

   std::map<std::string, docopt::value> args
      = docopt::docopt(USAGE, { argv + 1, argv + argc },
                       true,               // show help if requested
                       "repotool 0.2");  // version string

   loguru::init(argc, argv);

   initJobSystem();

   eastl::vector<eastl::string> gitRepositories;
   loadGitRepositoriesFromFile(gitRepositories);

   if(args["scan"] || true) {
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
      //TODO: save to file

      if(args["details"]) {
         //if file is already present we can try to find new/deleted repositories
         //compare to original file!
      }

      gitRepositories = newGitRepositories;
      saveGitRepositoriesToFile(gitRepositories);
   } 

   if(args["check"]) {
      LOG_F(0, "checking all directories for updates");
      checkForUpdates(args, gitRepositories);

   }

   shutdownJobSystem();

   return 0;
}