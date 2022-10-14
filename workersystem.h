
#ifndef __WORKERSYSTEM_H__
#define __WORKERSYSTEM_H__

#include <functional>

class WorkerParams {
public:
   virtual ~WorkerParams() {}
};


void initJobSystem();
void shutdownJobSystem();

void addJob(std::function<void(WorkerParams*)> job, WorkerParams *params);
void waitForAllJobsFinished();

#endif   //#ifndef __WORKERSYSTEM_H__
