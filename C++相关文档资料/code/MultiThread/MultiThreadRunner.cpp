#include "MultiThreadRunner.h"
#include "MultiThreadTask.h"

MultiThreadRunner::MultiThreadRunner(MultiTaskManager *taskManager)
    : _taskManager(taskManager),
      _thread(Null)
{

}

MultiThreadRunner::~MultiThreadRunner()
{
    if (_thread) {
        _thread = NULL;
    }
}

void MultiThreadRunner::Start()
{
    _thread = new std::thread([this] {
        while (1) {
            shared_ptr<MultiThreadTask> task = _taskManager->FetchTask();
            if (task == 0) {
                break;
            }
            task->Exec();
        }
    });
}

void MultiThreadRunner::Join()
{
    if (_thread) {
        _thread->join();
    }
}


