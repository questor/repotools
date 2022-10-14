
#include "workersystem.h"

#include "eastl/vector.h"
#include <thread>

#include "concurrentqueue/blockingconcurrentqueue.h"

eastl::vector<std::thread> threadPool;

typedef struct {
   std::function<void(WorkerParams*)> func;
   WorkerParams *params;
} JobDescr;

moodycamel::BlockingConcurrentQueue<JobDescr> workerJobQueue;
std::atomic<int> pendingJobs(0);

std::atomic<bool> quit(false);

void workerThreadFunc() {
   while(!quit) {
      JobDescr descr;
      bool successfull = workerJobQueue.wait_dequeue_timed(descr, 100);
      if(successfull) {
         descr.func(descr.params);
         pendingJobs.fetch_add(-1, std::memory_order_release);         
      }
   }
}

void initJobSystem() {
   int numberThreads = std::thread::hardware_concurrency();
   for(int i=0; i<numberThreads; ++i) {
      threadPool.pushBack(std::thread(workerThreadFunc));
   }   
}

void shutdownJobSystem() {
   quit = true;
   for(int i=0; i<threadPool.size(); ++i) {
      threadPool[i].join();
   }   
}

// parameters need to be deleted by the caller!
void addJob(std::function<void(WorkerParams*)> job, WorkerParams *params) {
   JobDescr descr;
   descr.func = job;
   descr.params = params;
   pendingJobs.fetch_add(1, std::memory_order_release);
   workerJobQueue.enqueue(descr);
}

void waitForAllJobsFinished() {
   while(pendingJobs.load(std::memory_order_acquire) != 0) {
      JobDescr descr;
      if(!workerJobQueue.try_dequeue(descr)) {
         continue;
      }
      descr.func(descr.params);
      pendingJobs.fetch_add(-1, std::memory_order_release);
   }
}
