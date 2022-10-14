
#ifndef __JSONIO_H__
#define __JSONIO_H__

#include "eastl/vector.h"
#include "eastl/string.h"

void loadGitRepositoriesFromFile(const char *filename, eastl::vector<eastl::string> &gitRepositories);
void saveGitRepositoriesToFile(eastl::vector<eastl::string> &gitRepositories);

#endif
