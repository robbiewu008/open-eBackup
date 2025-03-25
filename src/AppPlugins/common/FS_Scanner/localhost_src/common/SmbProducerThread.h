/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Author: z30016470
 * Create: 2022/07/29
 */

#ifndef FS_SCANNER_SMB_PRODUCER_THREAD_H
#define FS_SCANNER_SMB_PRODUCER_THREAD_H
#include "SmbMetaProducer.h"
#include "ProducerThread.h"

class SmbProducerThread : public ProducerThread {
public:
    explicit SmbProducerThread(std::shared_ptr<ScanQueue> scanQueue,
        std::shared_ptr<BufferQueue<DirectoryScan>> output,
        std::shared_ptr<StatisticsMgr> statsMgr,
        std::shared_ptr<ScanFilter> scanFilter,
        std::shared_ptr<FSScannerCheckPoint> chkPntMgr,
        Module::SmbContextArgs args,
        ScanConfig &config)
        : ProducerThread(scanQueue, output, statsMgr, scanFilter, chkPntMgr),
          m_args(args),
          m_config(config)
    {};
    SmbProducerThread() = delete;
    ~SmbProducerThread() override {};

    bool Start() override;
    void ProducerMain() override;
    void Exit() override;
    SCANNER_STATUS GetStatus();

private:
    Module::SmbContextArgs m_args {};
    ScanConfig &m_config;
};

#endif