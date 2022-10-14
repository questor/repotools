
#define LOGURU_IMPLEMENTATION 1
#include "loguru/loguru.hpp"

#include "eastl/vector.h"
#include "eastl/string.h"

#include "workersystem.h"
#include "commands/checkforupdatesonserver.h"
#include "commands/scandirectories.h"
#include "commands/savestate.h"
#include "commands/pullrepositories.h"
#include "commands/generateupdatereport.h"
#include "commands/exportrepositories.h"
#include "commands/mergelist.h"
#include "cachedrepositoryio.h"

#include "anyoption/anyoption.h"

#if defined(__linux)
#include <strings.h>
#endif
#ifdef _MSC_VER
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#endif

#include "rang/include/rang.hpp"

int safeStrcasecmp(const char *str1, const char *str2) {
   if(str1 == nullptr)
      return -1;
   if(str2 == nullptr)
      return -1;
   return strcasecmp(str1, str2);
}


int main(int argc, char **argv) {
   AnyOption opt;

   opt.addUsage("-h  --help                     prints this help");
   opt.addUsage("    --version                  show version");
   opt.addUsage("-o  --ouput <filename>         output to file(default to console)");
   opt.addUsage("-v X                           verbose level");
   opt.addUsage("-t  --type <type>              reporttype:txt, html(default:txt)");
   opt.addUsage("-p  --path <path>              path to the reportfolder(default:.)");
   opt.addUsage("");
   opt.addUsage("Suboptions for scan:");
   opt.addUsage("-d  --details    - print details for scanning");
   opt.addUsage("-w --work <path> - working folder");
   opt.addUsage("               scan");
   opt.addUsage("               checkForUpdates");
   opt.addUsage("               savestate <statefile>");
   opt.addUsage("               pull");
   opt.addUsage("               genUpdateReport <oldState> [<newState>]");
   opt.addUsage("               exportList <filename>");
   opt.addUsage("               mergeList <filename>");
   opt.addUsage("");
//   opt.addUsage("               findChanges(TODO!)");
//   opt.addUsage("               findAhead(TODO!)");

//find changes: git diff-index --name-only --ignore-submodules HEAD --
//   finds locally changed files not submitted

//find ahead: git status -sb => ahead in the string?
//   finds commits submited locally, but not pushed

   opt.addUsage("detailed explanation:");
   opt.addUsage("scan - scans directory and it's subdirectories for git repositories");
   opt.addUsage("checkForUpdates - checks all repositories for updates on the server");
   opt.addUsage("savestate - stores each latest commit of each repository to a file");
   opt.addUsage("pull - pull all repositories");
   opt.addUsage("genUpdateReport - generate list of changes for each repository");
   opt.addUsage("exportList - exports a list of all repositories with directory and git source url");
   opt.addUsage("mergeList - import list and create new repositories where needed");

   opt.setFlag("help", 'h');
   opt.setFlag("version");
   opt.setOption('v');
   opt.setFlag("details", 'd');
   opt.setOption("output", 'o');
   opt.setOption("path", 'p');
   opt.setOption("work", 'w');

   opt.processCommandArgs(argc, argv);

   if(!opt.hasOptions() || opt.getValue('h')) {
      opt.printUsage();
      return 0;
   }

   if(opt.getValue('v') != nullptr)
      loguru::init(argc, (const char **)argv);

   LOG_F(1, "start job system");
   initJobSystem();

   LOG_F(1, "read cache into memory");
   eastl::vector<eastl::string> gitRepositories;
   loadGitRepositoriesFromFile(".repotool.cache", gitRepositories);


   if(safeStrcasecmp("scan", opt.getArgv(opt.getArgc()-1)) == 0) {
      LOG_F(1, "doing scan of directories");
      scanDirectories(opt, gitRepositories);
   } else {
      //if the command is "mergelist" allow no prefilled repository list as we might do the initial merge
      if(opt.getArgv(opt.getArgc()-2) != nullptr) {
         if(safeStrcasecmp("mergelist", opt.getArgv(opt.getArgc()-2)) != 0) {
            if(gitRepositories.size() == 0) {
               printf("no repository list found! please first scan and create the list.\n");
               return -1;
            }
         }
      }
   }

   if(safeStrcasecmp("checkForUpdates", opt.getArgv(opt.getArgc()-1)) == 0) {
      LOG_F(1, "checking all directories for updates");
      checkForUpdatesOnServer(opt, gitRepositories);
   }

   if(safeStrcasecmp("saveState", opt.getArgv(opt.getArgc()-2)) == 0) {
      LOG_F(1, "save state of current directories");
      saveState(opt, gitRepositories);
   }

   if(safeStrcasecmp("pull", opt.getArgv(opt.getArgc()-1)) == 0) {
         LOG_F(1, "pulling all repositories");
      pullRepositories(opt, gitRepositories);
   }

   if((safeStrcasecmp("genUpdateReport", opt.getArgv(opt.getArgc()-2)) == 0) || 
      (safeStrcasecmp("genUpdateReport", opt.getArgv(opt.getArgc()-3)) == 0)) {
      LOG_F(1, "generate update report");
      generateUpdateReport(opt, gitRepositories);
   }

   if(safeStrcasecmp("exportlist", opt.getArgv(opt.getArgc()-2)) == 0) {
      LOG_F(1, "export list of all repositories with their sources");
      exportRepositories(opt, gitRepositories);
   }

   if(safeStrcasecmp("mergelist", opt.getArgv(opt.getArgc()-2)) == 0) {
      LOG_F(1, "merge list and clone new ones");
      mergeList(opt, gitRepositories);
   }

   shutdownJobSystem();
   return 0;
}
