
#include "scandirectories.h"

#include "cachedrepositoryio.h"
#include "report.h"

#include "eastl/bitvector.h"

#include "loguru/loguru.hpp"

#include "eastl/extra/path.h"

void scanDirectories(AnyOption &options, eastl::vector<eastl::string> &gitRepositories) {
   eastl::vector<eastl::string> newGitRepositories;
   eastl::vector<eastl::string> directories;

   eastl::string workingDirectory(".");
   if(options.getValue("work") != nullptr) {
      workingDirectory = options.getValue("work");
   } else if(options.getValue('w') != nullptr) {
      workingDirectory = options.getValue('w');
   }

   directories.pushBack(workingDirectory);
   while(directories.size() != 0) {
      eastl::string currentDirectory = directories[0];
      directories.erase(directories.begin() + 0);

      LOG_F(2, "scan folder %s", currentDirectory.c_str());

      if(currentDirectory.back() != '/')
         currentDirectory += "/";

      for(const filesystem::directory_entry & dirEntry : filesystem::directory_iterator(currentDirectory.c_str())) {
         if(dirEntry.path().is_directory() == true) {
            eastl::string folder = dirEntry.path().str();
            eastl::string last4Characters = folder.substr(folder.length()-5, 5);
            if(strcmp(last4Characters.c_str(), "\\.git") == 0) {        //FIXME: Linux!
                  newGitRepositories.pushBack(currentDirectory);
                  //abort checking further folders in this one?
            } else {
               bool defaultFolder = false;
               eastl::string last2Characters = folder.substr(folder.length()-2, 2);
               eastl::string last3Characters = folder.substr(folder.length()-3, 3);
               if(strcmp(last2Characters.c_str(), "\\.") == 0)          // FIXME: what about linux?
                  defaultFolder = true;
               if(strcmp(last3Characters.c_str(), "\\..") == 0)
                  defaultFolder = true;
               if(defaultFolder == false) {
                  directories.pushBack(folder);
               }
            }
         }
      } 
   }

   if(options.getFlag('d') || options.getFlag("details")) {
      //if file is already present we can try to find new/deleted repositories
      json reportData;

      prepareOutputReport(options, "repoupdate");


      json reposArray = json::array();
      for(int i=0; i<newGitRepositories.size(); ++i) {
         reposArray.push_back(removeNewlines(newGitRepositories[i]).c_str());
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
            addedArray.push_back(removeNewlines(newGitRepositories[i]).c_str());
         }
      }
      reportData["addedRepositories"] = addedArray;

      json deletedArray = json::array();
      for(int i=0; i<gitRepositories.size(); ++i) {
         if(oldRepoBits[i] == false) {
            deletedArray.push_back(removeNewlines(gitRepositories[i]).c_str());
         }
      }
      reportData["deletedRepositories"] = deletedArray;

      generateAndOutputReport(options, reportData);
   }

   saveGitRepositoriesToFile(newGitRepositories);
}
