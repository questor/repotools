
#include "cachedrepositoryio.h"

#include <string>

#include "json/json.hpp"
using json = nlohmann::json;

void loadGitRepositoriesFromFile(const char *filename, eastl::vector<eastl::string> &gitRepositories) {
   gitRepositories.clear();
   FILE *fp = fopen(filename, "rb");
   if(!fp) {
      //TODO: error handling?
      return;
   }
   fseek(fp, 0, SEEK_END);
   int filelen = ftell(fp);
   fseek(fp, 0, SEEK_SET);
   char *filebuffer = new char[filelen+1];
   memset(filebuffer, 0, filelen+1);
   fread(filebuffer, sizeof(char), filelen, fp);
   fclose(fp);

   json j = json::parse(filebuffer);
   delete[] filebuffer;

   //now iterate over file
   json array = j["repos"];
   for(json::iterator it = array.begin(); it != array.end(); ++it) {
      gitRepositories.pushBack((*it).get<std::string>().c_str());
   }
}

void saveGitRepositoriesToFile(eastl::vector<eastl::string> &gitRepositories) {
   json j;

   json listArray = json::array();
   for(int i=0; i<gitRepositories.size(); ++i) {
      listArray.push_back(gitRepositories[i].c_str());
   }

   j["repos"] = listArray;

   FILE *fp = fopen(".repotool.cache", "wb");
   if(!fp) {
      //TODO: error handling
      return;
   }
   std::string dump = j.dump();
   fwrite(dump.c_str(), sizeof(char), dump.length(), fp);
   fclose(fp);
}