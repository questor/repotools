
#include "callexecutable.h"

#include "reproc/include/c/reproc/reproc.h"

#include "loguru/loguru.hpp"

#define BUFFER_SIZE 1024

bool callExecutable(CallParams *params) {
   reproc_type callStatus;

   LOG_F(5, "executing %s in %s", params->process.c_str(), params->workingDir.c_str());
   int argc = params->arguments.size()+1;  //+1 for the process itself
   const char **argv = new const char*[argc+1];        //+1 for endmarker
   argv[0] = params->process.c_str();
   for(int i=0; i<argc-1; ++i) {
      LOG_F(5, "with argument %s", params->arguments[i].c_str());
      argv[1+i] = params->arguments[i].c_str();
   }
   argv[argc] = NULL;                      //endmarker

   REPROC_ERROR error = REPROC_SUCCESS;
   error = reproc_start(&callStatus, argc, argv, params->workingDir.c_str());
   if(error == REPROC_FILE_NOT_FOUND) {
      //TODO: error handling
      LOG_F(5, "REPROC_FILE_NOT_FOUND");
      return false;
   } else if(error) {
      LOG_F(5, "reproc_start error!");
      //TODO: error handling
      return false;
   }

   size_t outputLength = 0;
   char *output = (char*)malloc(1);

   char buffer[BUFFER_SIZE];
   while(true) {
      unsigned int bytesRead = 0;
      error = reproc_read(&callStatus, REPROC_OUT, buffer, BUFFER_SIZE, &bytesRead);
      if(error)
         break;
      char *reallocResult = (char*)realloc(output, outputLength+bytesRead+1);
      if(!reallocResult) {
         //TODO!
      }
      output = reallocResult;
      memcpy(output+outputLength, buffer, bytesRead);
      outputLength += bytesRead;
   }
   if(error != REPROC_STREAM_CLOSED) {
      free(output);
      //TODO error handling
   }

   output[outputLength] = '\0';
   params->output = output;

   LOG_F(5, "result: <%s>", output);

   free(output);

cleanup:;
   unsigned int exitStatus = 0;
   error = reproc_stop(&callStatus, REPROC_WAIT, REPROC_INFINITE, &exitStatus);
   if(error) {
      //TODO
   }
   return exitStatus;
}
