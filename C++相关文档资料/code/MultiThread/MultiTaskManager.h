#ifndef _MULTI_TASK_MANAGER_
#define _MULTI_TASK_MANAGER_

#include <iostream>
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include "InterFace.h"

class MultiThreadTask;

using namespace std;

using MultiThreadTaskQueue = std::vector<shared_ptr<MultiThreadTask>>;
using MultiThreadPool = std::vector<shared_ptr<MultiThreadRunner>> ;


class MultiTaskManager : public IMultiTaskManager
{
    IMULTITASKMANAGER_INTERFACE
public:
    explicit MultiTaskManager();
    ~MultiTaskManager();

public:
    MultiThreadTask *FetchTask();
    void ReleaseTask();

private:
    void AddTask(MultiThreadTask *task);
    void WaitForAddTask();
    void FillEmptyTask();
    void WaitAllThreadEnd();

private:
    int                       _maxThreadSum;
    // 可用线程数量
    int                       _canRunnerSum;
    // 任务锁和条件变量
    mutex                     _taskLock;
    condition_variable        _taskCond; 
    // 多线程锁和条件变量
    mutex                     _multiLock;
    condition_variable        _multiCond;

    // 停止状态
    atomic_bool               _hasStop;
    atomic_bool               _hasWait;
    atomic_bool               _hasStart;

    MultiThreadPool           _threadPool;
    MultiThreadTaskQueue      _multiThreadTask;
};

#endif