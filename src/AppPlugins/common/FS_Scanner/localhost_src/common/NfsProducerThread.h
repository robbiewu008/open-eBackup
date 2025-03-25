/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
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