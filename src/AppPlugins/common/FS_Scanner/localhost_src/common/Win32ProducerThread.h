/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Author: w30029850
 * Create: 2023/01/16
 */

#ifndef FS_SCANNER_Win32_PRODUCER_THREAD_H
#define FS_SCANNER_Win32_PRODUCER_THREAD_H

#ifdef WIN32
#include "Win32MetaProducer.h"
#include "ProducerThread.h"

class Win32ProducerThread : public ProducerThread {
public:
    explicit Win32ProducerThread(
        std::shared_ptr<ScanQueue> scanQueue,
        std::shared_ptr<BufferQueue<DirectoryScan>> output,
        std::shared_ptr<StatisticsMgr> statsMgr,
        std::shared_ptr<ScanFilter> scanFilter,
        std::shared_ptr<FSScannerCheckPoint> chkPntMgr,
        ScanConfig config)
        : ProducerThread(scanQueue, output, statsMgr, scanFilter, chkPntMgr), m_config(config)
    {};
    Win32ProducerThread() {};
    ~Win32ProducerThread() override {};

    bool Start() override;
    void ProducerMain() override;
    void Exit() override;

private:
    ScanConfig m_config {};
};

#endif
#endif