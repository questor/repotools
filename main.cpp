
#define LOGURU_IMPLEMENTATION 1
#include "loguru/loguru.hpp"

#include "eastl/vector.h"
#include "eastl/string.h"

#include "workersystem.h"
#include "commands/checkforupdates.h"
#include "commands/scandirectories.h"
#include "jsonio.h"

#include "anyoption/anyoption.h"

int main(int argc, char **argv) {

   AnyOption opt;

   opt.addUsage("-h  --help                     prints this help");
   opt.addUsage("    --version                  show version");
   opt.addUsage("-o  --ouput <filename>         output to file; default to console");
   opt.addUsage("-v X                           verbose level");
   opt.addUsage("-t  --type <type>              reporttype(txt, html) default:txt");
   opt.addUsage("-w  --work <directory>         set working directory(default .");
   opt.addUsage("");
   opt.addUsage("-d  --details  scan");
   opt.addUsage("               update");
   opt.addUsage("               savestate");

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

   loguru::init(argc, (const char **)argv);

   initJobSystem();

   eastl::vector<eastl::string> gitRepositories;
   loadGitRepositoriesFromFile(gitRepositories);

   if(strcmp("scan", opt.getArgv(opt.getArgc()-1)) == 0) {
      LOG_F(0, "doing scan of directories");
      scanDirectories(opt, gitRepositories);
   }

   if(strcmp("check", opt.getArgv(opt.getArgc()-1)) == 0) {
      LOG_F(0, "checking all directories for updates");
      checkForUpdates(opt, gitRepositories);
   }

   shutdownJobSystem();
   return 0;
}
