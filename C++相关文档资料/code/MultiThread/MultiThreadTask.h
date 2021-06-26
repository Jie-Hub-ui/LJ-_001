#ifndef _MULTI_THREAD_TASK_
#define _MULTI_THREAD_TASK_

class MultiThreadTask
{
public:
    MultiThreadTask(MultiTaskManager *taskManager, MultiThreadExecTask *task);
    virtual ~MultiThreadTask();

public:
    void Exec();

private:
    shared_ptr<MultiThreadExecTask>              _task; 
    shared_ptr<MultiTaskManager>                 _taskManager;
};





#endif