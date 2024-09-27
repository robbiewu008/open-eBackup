/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskProtectWorker.h
 * @brief  Contains function declarations TaskProtectWorker
 * @version 1.0.0
 * @date 2020-06-27
 * @author wangguitao 00510599
 */
#ifndef AGENT_TASK_PORTECT_WORKER_H
#define AGENT_TASK_PORTECT_WORKER_H

#include "pluginfx/iplugin.h"
#include "pluginfx/PluginCfgParse.h"
#include "pluginfx/PluginManager.h"
#include "plugins/ServicePlugin.h"
#include "common/Types.h"
#include "common/CMpThread.h"

class TaskProtectWorker {
public:
    TaskProtectWorker();
    ~TaskProtectWorker();

private:
};

#endif  // _AGENT_TASK_PORTECT_WORKER_H_
