#include "MultiThreadTask.h"

MultiThreadTask::MultiThreadTask(MultiTaskManager *taskManager, MultiThreadExecTask *task)
    : _taskManager(taskManager),
      _task(task)
{
    std::cout << "MultiThreadTask()" << std::endl;
}

MultiThreadTask::~MultiThreadTask()
{

}

void MultiThreadTask::Exec()
{
    _task->OnExec();
    _taskManager->ReleaseTask();
}