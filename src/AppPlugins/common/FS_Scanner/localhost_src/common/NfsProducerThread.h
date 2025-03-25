/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Author: g00554214
 * Create: 2022/07/05
 */

#ifndef FS_SCANNER_NFS_PRODUCER_THREAD_H
#define FS_SCANNER_NFS_PRODUCER_THREAD_H
#include "NfsMetaProducer.h"
#include "ProducerThread.h"

class NfsProducerThread : public ProducerThread {
public:
    explicit NfsProducerThread(std::shared_ptr<ScanQueue> scanQueue,
        std::shared_ptr<BufferQueue<DirectoryScan>> output,
        std::shared_ptr<StatisticsMgr> statsMgr,
        std::shared_ptr<ScanFilter> scanFilter, ScanConfig &config,
        std::shared_ptr<FSScannerCheckPoint> chkPntMgr)
        : ProducerThread(scanQueue, output, statsMgr, scanFilter, chkPntMgr),
        m_config(config)
    {};
    ~NfsProducerThread() override {};

    bool Start() override;
    void ProducerMain() override;
    void Exit() override;
    SCANNER_STATUS HandleProtectedServerIssue() override;

private:
    bool m_exit = false;
    std::shared_ptr<std::thread> m_mainThread {};
    ScanConfig &m_config;
};

#endif