
#include "report.h"

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
   eastl::string fullReportFilename = "reporttemplates/" + reportFilename + "." + reportExtension;
   inja::Template templ = injaEnv.parse_template(fullReportFilename.c_str());
   std::string renderedReport = injaEnv.render_template(templ, reportData);

   if(options.getValue('o') == nullptr && options.getValue("output") == nullptr) {
      printf("%s", renderedReport.c_str());
   } else {
      //TODO: output to file
   }

}

