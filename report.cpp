
#include "report.h"
#include "loguru/loguru.hpp"

eastl::string removeNewlines(eastl::string &source) {
   eastl::string result = source;
   result.erase(0, result.findFirstNotOf(" \n\r\t"));
   result.erase(result.findLastNotOf(" \n\r\t") + 1);
   return result;
}

inja::Environment injaEnv = inja::Environment();
inja::Template templ;

void prepareOutputReport(AnyOption &options, eastl::string reportFilename) {
   eastl::string reportExtension = "txt";
   char *extOption = options.getValue('t');
   if(extOption == nullptr) {
      extOption = options.getValue("type");
   }
   if(extOption != nullptr) {
      reportExtension = extOption;
   }

   eastl::string pathToTemplFolder(".");
   if(options.getValue('p')) {
      pathToTemplFolder = options.getValue('p');
   }
   if(options.getValue("path")) {
      pathToTemplFolder = options.getValue("path");
   }
   pathToTemplFolder += "/";

   injaEnv.set_element_notation(inja::ElementNotation::Dot);
   eastl::string fullReportFilename = pathToTemplFolder + "reporttemplates/" + reportFilename + "." + reportExtension;
   templ = injaEnv.parse_template(fullReportFilename.c_str());
   if(templ.content.size() == 0) {
      printf("WARNING: your template seems to be empty!");
   }
}

void generateAndOutputReport(AnyOption &options, json reportData) {

   std::string renderedReport = injaEnv.render(templ, reportData);

   if(options.getValue('o') == nullptr && options.getValue("output") == nullptr) {
      printf("%s", renderedReport.c_str());
   } else {
      eastl::string filename;
      if(options.getValue('o')) {
         filename = options.getValue('o');
      } else {
         filename = options.getValue("output");
      }
      FILE *fp = fopen(filename.c_str(), "wb");
      fprintf(fp, "%s", renderedReport.c_str());
      fclose(fp);
   }
}

