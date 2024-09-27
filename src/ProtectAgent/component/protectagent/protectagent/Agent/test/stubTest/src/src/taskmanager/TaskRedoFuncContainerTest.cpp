#include "taskmanager/TaskRedoFuncContainerTest.h"
#include "taskmanager/TaskRedoFuncContainer.h"

TEST_F(TaskRedoFuncContainerTest, GetFuncTest)
{
    TaskRedoFuncContainer taskRedo = TaskRedoFuncContainer::GetInstance();
    std::string taskType;
    taskRedo.GetFunc(taskType);
}

TEST_F(TaskRedoFuncContainerTest, RegisterNewFuncTest)
{
    auto taskRedo = TaskRedoFuncContainer::GetInstance();
    std::string taskType;
    RedoTaskNewFuncPtr newfunc;
    taskRedo.RegisterNewFunc(taskType, newfunc);
}
