/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskRedoFuncContainer.h
 * @brief  Contains function declarations for TaskRedoFuncContainer
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef _AGENT_TASKREDO_FUNCCONTAINER_H_
#define _AGENT_TASKREDO_FUNCCONTAINER_H_

#include <map>
#include "taskmanager/Task.h"

typedef Task* (*RedoTaskNewFuncPtr)(const std::string&);
class TaskRedoFuncContainer {
public:
    static TaskRedoFuncContainer &GetInstance()
    {
        static TaskRedoFuncContainer TaskRedoFuncContainer;
        return TaskRedoFuncContainer;
    }

    RedoTaskNewFuncPtr GetFunc(const std::string& taskType)
    {
        if (taskMap.find(taskType) != taskMap.end()) {
            return taskMap[taskType];
        }

        return NULL;
    }

    void RegisterNewFunc(const std::string& taskType, RedoTaskNewFuncPtr newFunc)
    {
        taskMap.insert(std::pair<std::string, RedoTaskNewFuncPtr>(taskType, newFunc));
    }

private:
    std::map<std::string, RedoTaskNewFuncPtr> taskMap;
};

#endif