#ifndef _MULTI_THREAD_RUNNER_
#define _MULTI_THREAD_RUNNER_

#include <thread>

class MultiThreadTask;

class MultiThreadRunner
{
public:
    MultiThreadRunner(MultiTaskManager *taskManager);
    ~MultiThreadRunner();

public:
    void Start();
    void Join();

private:
    MultiTaskManager         *_taskManager;
    std::thread              *_thread;

};




#endif