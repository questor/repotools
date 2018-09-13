
#include "scandirectories.h"

#include "cachedrepositoryio.h"
#include "report.h"

#include "eastl/bitvector.h"

#include "eaio/EAFileDirectory.h"
#include "eaio/FnEncode.h"

#include "loguru/loguru.hpp"


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

      if(currentDirectory.back() != '/')
         currentDirectory += "/";

      char16_t path[1024];
      EA::IO::StrlcpyUTF8ToUTF16(&path[0], 1024, currentDirectory.c_str());

      EA::IO::EntryFindData efd;
      if(EA::IO::entryFindFirst(path, u"*", &efd)) {
         do {
            char8_t tmp[1024];
            EA::IO::StrlcpyUTF16ToUTF8(&tmp[0], 1024, efd.mName);
            //TODO: assert that efd.mName is less than 1024!

            if(efd.mbIsDirectory) {
               if(EA::IO::StrEq16(efd.mName, u".git/")) {
                  newGitRepositories.pushBack(currentDirectory);
               } else {
                  if(!EA::IO::StrEq16(efd.mName, u"./") && !EA::IO::StrEq16(efd.mName, u"../")) {
                     directories.pushBack(currentDirectory + tmp);                        
                  }
               }
            }
         } while(EA::IO::entryFindNext(&efd));
         EA::IO::entryFindFinish(&efd);
      } else {
         //Directory is empty, normal usecase
      }
   }

   if(options.getFlag('d') || options.getFlag("details")) {
      //if file is already present we can try to find new/deleted repositories
      json reportData;

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

      generateAndOutputReport(options, "repoupdate", reportData);
   }

   saveGitRepositoriesToFile(newGitRepositories);
}
