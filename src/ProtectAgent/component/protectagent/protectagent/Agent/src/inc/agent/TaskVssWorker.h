/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskVssWorker.h
 * @brief  Contains function declarations TaskVssWorker
 * @version 1.0.0
 * @date 2020-06-27
 * @author wangguitao 00510599
 */
#ifndef _AGENT_TASK_VSS_WORKER_H_
#define _AGENT_TASK_VSS_WORKER_H_

#ifdef WIN32
#include "pluginfx/iplugin.h"
#include "pluginfx/PluginCfgParse.h"
#include "pluginfx/PluginManager.h"
#include "plugins/ServicePlugin.h"
#include "common/Types.h"
#include "common/CMpThread.h"
#include "agent/TaskWorker.h"

// 为后续VSS调用扩展创建TaskVssWorker类
class TaskVssWorker : public TaskWorker {
public:
    TaskVssWorker();
    ~TaskVssWorker();

private:
};

#endif  // WIN32
#endif  // _AGENT_TASK_VSS_WORKER_H_
