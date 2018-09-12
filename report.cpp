
#include "report.h"

eastl::string removeNewlines(eastl::string &source) {
   eastl::string result = source;
   result.erase(0, result.findFirstNotOf(" \n\r\t"));
   result.erase(result.findLastNotOf(" \n\r\t") + 1);
   return result;
}


void generateAndOutputReport(AnyOption &options, eastl::string reportFilename, json reportData) {
   eastl::string reportExtension = "txt";
   char *extOption = options.getValue('t');
   if(extOption == nullptr) {
      extOption = options.getValue("type");
   }
   if(extOption != nullptr) {
      reportExtension = extOption;
   }

   inja::Environment injaEnv = inja::Environment();
   injaEnv.set_element_notation(inja::ElementNotation::Dot);
   eastl::string fullReportFilename = "reporttemplates/" + reportFilename + "." + reportExtension;
   inja::Template templ = injaEnv.parse_template(fullReportFilename.c_str());
   std::string renderedReport = injaEnv.render_template(templ, reportData);

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

