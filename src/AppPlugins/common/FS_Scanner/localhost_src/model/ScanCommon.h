/*
* Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
* Description: common header
* Author: z30020916
* Create: 2022-06-05
*/
#ifndef COMMON_HEADER_H
#define COMMON_HEADER_H

#include <vector>
#include "ScanQueue.h"
#include "BufferQueue.h"
#include "ScanFilter.h"
#include "StatisticsMgr.h"
#include "FSScannerCheckPoint.h"

enum THREADPOOLTYPE {
    SCAN_THREAD_POOL = 1,
    BAKCUP_THREAD_POOL = 2,
};

struct ProduceParams {
    std::shared_ptr<ScanQueue> scanQueue {nullptr};
    std::shared_ptr<BufferQueue<DirectoryScan>> output {nullptr};
    std::shared_ptr<StatisticsMgr> statsMgr {nullptr};
    std::shared_ptr<ScanFilter> scanFilter {nullptr};
    std::shared_ptr<FSScannerCheckPoint> chkPntMgr {nullptr};
};

#endif