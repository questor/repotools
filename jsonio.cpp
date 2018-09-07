
#include "jsonio.h"

#include <string>

#include "json/json.hpp"
using json = nlohmann::json;

void loadGitRepositoriesFromFile(eastl::vector<eastl::string> &gitRepositories) {
   FILE *fp = fopen(".repotool.cache", "rb");
   if(!fp) {
      //TODO: error handling?
      return;
   }
   fseek(fp, 0, SEEK_END);
   int filelen = ftell(fp);
   fseek(fp, 0, SEEK_SET);
   char *filebuffer = new char[filelen+1];
   memset(filebuffer, 0, filelen+1);
   fread(fp, 1, sizeof(char), filebuffer);
   fclose(fp);

   json j = json::parse(filebuffer);
   delete[] filebuffer;

   //now iterate over file
}

void saveGitRepositoriesToFile(eastl::vector<eastl::string> &gitRepositories) {
   json j;

}