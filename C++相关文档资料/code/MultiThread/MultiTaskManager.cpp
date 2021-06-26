#include "MultiTaskManager.h"
#include "MultiThreadRunner.h"
#include "MultiThreadTask.h"

MultiTaskManager::MultiTaskManager()
    : _maxThreadSum(0),
      _canRunnerSum(0),
      _hasStop(false),
      _hasStart(false),
      _hasWait(false)
{
    std::cout << "MultiTaskManager()" << std::endl;
}

MultiTaskManager::~MultiTaskManager()
{
    std::cout << "~MultiTaskManager()" << std::endl;
}

/*
创建对应数量的线程，并启动
*/
void MultiTaskManager::Start(int threadSum)
{
    if (_hasStart == true) {
        return;
    }
    _hasStart = true;
    _maxThreadSum = threadSum;

    for (int i = 0; i < threadSum; i++) {
        shared_ptr<MultiThreadRunner> multiThreadRunner = new MultiThreadRunner(this);
        _threadPool.push_backup(multiThreadRunner);
        multiThreadRunner->Start();
    }

    _canRunnerSum = threadSum;
    _taskCond.notify_one();
}

// 创建线程执行任务，MultiThreadExecTask为任务类
void MultiTaskManager::CreateTask(MultiThreadExecTask *task)
{
    if (_hasStop || task == NULL) {
        return;
    }
    WaitForAddTask();

    if (_hasStop == false) {
        shared_ptr<MultiThreadTask> threadTask = new MultiThreadTask(task, this);
        AddTask(threadTask);
        std::unique_lock<std::mutex> lock(_taskLock);
        while (_canRunnerSum <= 0) {
            _taskCond.wait(_taskLock);
        }
    }
}

void MultiTaskManager::WaitForAddTask()
{
    std::unique_lock<std::mutex> lock(_taskLock);
    if (_canRunnerSum > 0) {
        --_canRunnerSum;
        return;
    }

    while (_canRunnerSum <= 0) {
        _taskCond.wait(_taskLock);
    }
    --_canRunnerSum;
}


void MultiTaskManager::AddTask(MultiThreadTask *task)
{
    {
        std::unique_lock<std::mutex> lock(_multiLock);
        _multiThreadTask.push_back(task);
    }
    _multiCond.notify_one();
}


MultiThreadTask *MultiTaskManager::FetchTask()
{
    std::unique_lock<std::mutex> lock(_multiLock);
    if (!_multiThreadTask.empty()) {
        MultiThreadTask *task = _multiThreadTask.front();
        _multiThreadTask.pop();
        return task;
    }    

    while (_multiThreadTask.empty()) {
        if (_hasStop) {
            return NULL;
        }
        // 此处wait在等待被唤醒时会释放当前锁资源，直到通过notify_one被唤醒才会重新加锁，然后wait返回
        // wait返回后可安全使用共享数据，此时的锁为加锁状态
        _multiCond.wait(_multiLock);
    }
    MultiThreadTask *task = _multiThreadTask.front();
    _multiThreadTask.pop();
    return task;
}

void MultiTaskManager::ReleaseTask() 
{
    {
        std::unique_lock<std::mutex> lock(_taskLock);
        _canRunnerSum++;
    }
    _taskCond.notify_one();
}

void MultiTaskManager::FillEmptyTask()
{
    for (int i = 0; i < _maxThreadSum; i++) {
        _multiThreadTask.push_backup(0);
    }
}

void MultiTaskManager::WaitAllThreadEnd()
{
    auto iter = _threadPool.begin();
    for (; iter != _threadPool.end(); ++iter) {
        (*iter)->Join();
    }
}

void MultiTaskManager::End()
{
    if (_hasWait == true || _hasStop == true || _hasStart == false) {
        return;
    }
    _hasWait = true;

    {
        std::unique_lock<std::mutex> lock(_multiLock);
        FillEmptyTask();
    }

    _multiCond.notify_all();
    WaitAllThreadEnd();
}





