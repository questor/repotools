
#include "repostateio.h"

void saveState(char *filename, nlohmann::json &jsondata) {
   FILE *fp = fopen(filename, "wb");
   if(!fp) {
      //TODO: error handling!
      printf("could not open output file to save the state!\n");
      return;
   }
   std::string dump = jsondata.dump();
   fwrite(dump.c_str(), sizeof(char), dump.length(), fp);
   fclose(fp);
}

nlohmann::json loadState(char *filename) {
   FILE *fp = fopen(filename, "rb");
   if(!fp) {
      //TODO error handling!
      printf("could not open input file to load state\n");
      return nlohmann::json();
   }
   
   fseek(fp, 0, SEEK_END);
   int filelen = ftell(fp);
   fseek(fp, 0, SEEK_SET);
   char *filebuffer = new char[filelen+1];
   memset(filebuffer, 0, filelen+1);
   fread(filebuffer, sizeof(char), filelen, fp);
   fclose(fp);

   nlohmann::json j = nlohmann::json::parse(filebuffer);
   delete[] filebuffer;

   //TODO: error handling or checking the structure?

   return j;
}

