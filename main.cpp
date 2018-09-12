
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
#include "jsonio.h"

#include "anyoption/anyoption.h"

#include "strings.h"

int main(int argc, char **argv) {
   AnyOption opt;

   opt.addUsage("-h  --help                     prints this help");
   opt.addUsage("    --version                  show version");
   opt.addUsage("-o  --ouput <filename>         output to file(default to console)");
   opt.addUsage("-v X                           verbose level");
   opt.addUsage("-t  --type <type>              reporttype:txt, html(default:txt)");
   opt.addUsage("-w  --work <directory>         set working directory(default '.'");
   opt.addUsage("");
   opt.addUsage("-d  --details  scan");
   opt.addUsage("               checkForUpdates");
   opt.addUsage("               savestate <statefile>");
   opt.addUsage("               pull");
   opt.addUsage("               genUpdateReport <oldState> [<newState>]");
   opt.addUsage("");
   opt.addUsage("               findChanges(TODO!)");
   opt.addUsage("               findAhead(TODO!)");

//find changes: git diff-index --name-only --ignore-submodules HEAD --
//   finds locally changed files not submitted

//find ahead: git status -sb => ahead in the string?
//   finds commits submited locally, but not pushed

   opt.setFlag("help", 'h');
   opt.setFlag("version");
   opt.setOption('v');
   opt.setFlag("details", 'd');
   opt.setOption("option", 'o');
   opt.setOption("work", 'w');

   opt.processCommandArgs(argc, argv);

   if(!opt.hasOptions()) {
      opt.printUsage();
      return -1;
   }

   if(opt.getValue('v') != nullptr)
      loguru::init(argc, (const char **)argv);

   initJobSystem();

   eastl::vector<eastl::string> gitRepositories;
   loadGitRepositoriesFromFile(gitRepositories);

   if(strcasecmp("scan", opt.getArgv(opt.getArgc()-1)) == 0) {
      LOG_F(1, "doing scan of directories");
      scanDirectories(opt, gitRepositories);
   } else {
      if(gitRepositories.size() == 0) {
         printf("no repositorie list found! please first scan and create the list.\n");
         return -1;
      }
   }

   if(strcasecmp("checkForUpdates", opt.getArgv(opt.getArgc()-1)) == 0) {
      LOG_F(1, "checking all directories for updates");
      checkForUpdatesOnServer(opt, gitRepositories);
   }

   if(strcasecmp("saveState", opt.getArgv(opt.getArgc()-2)) == 0) {
      LOG_F(1, "save state of current directories");
      saveState(opt, gitRepositories);
   }

   if(strcasecmp("pull", opt.getArgv(opt.getArgc()-1)) == 0) {
      LOG_F(1, "pulling all repositories");
      pullRepositories(opt, gitRepositories);
   }

   if(strcasecmp(("genUpdateReport", opt.getArgv(opt.getArgc()-2)) == 0) || ("genUpdateReport", opt.getArgv(opt.getArgc()-3)) == 0) {
      LOG_F(1, "generate update report");
      generateUpdateReport(opt, gitRepositories);
   }
   shutdownJobSystem();
   return 0;
}
