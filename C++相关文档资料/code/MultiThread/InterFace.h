#ifndef _INTERFACE_
#define _INTERFACE_

class IMultiTaskManager
{
public:
    virtual void Start(int threadSum) = 0;
    virtual void Stop() = 0;
    virtual void End() = 0;
    virtual void CreateTask() = 0;  // 任务类型暂定
};

#define IMULTITASKMANAGER_INTERFACE       \ 
public:                                   \
    virtual void Start(int threadSum);    \
    virtual void Stop();                  \
    virtual void End();                   \
    virtual void CreateTask();            \


#endif
