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
#ifndef FS_SCANNER_PRODUCER_THREAD_H
#define FS_SCANNER_PRODUCER_THREAD_H

#include <thread>
#include "StatisticsMgr.h"
#include "ScanFilter.h"
#include "FSScannerCheckPoint.h"
#include "MetaProducer.h"
#include "ScanQueue.h"

class ProducerThread {
public:
    bool m_isMetaProducerExecuting = false;

    explicit ProducerThread(std::shared_ptr<ScanQueue> scanQueue,
        std::shared_ptr<BufferQueue<DirectoryScan>> output,
        std::shared_ptr<StatisticsMgr> statsMgr,
        std::shared_ptr<ScanFilter> scanFilter,
        std::shared_ptr<FSScannerCheckPoint> chkPntMgr)
        : m_scanQueue(scanQueue), m_output(output), m_statsMgr(statsMgr),
          m_scanFilter(scanFilter), m_chkPntMgr(chkPntMgr)
    {};
    ProducerThread() {};
    virtual ~ProducerThread() {};

    virtual bool Start() = 0;
    virtual void ProducerMain() = 0;
    virtual void Exit() = 0;
    virtual SCANNER_STATUS HandleProtectedServerIssue();

    void Suspend()
    {
        m_suspend = true;
    }
    void Resume()
    {
        m_suspend = false;
    }

protected:
    bool m_exit = false;
    bool m_suspend = false;
    std::shared_ptr<std::thread> m_mainThread {};
    std::shared_ptr<MetaProducer> m_metaProducer;
    std::shared_ptr<ScanQueue> m_scanQueue;
    std::shared_ptr<BufferQueue<DirectoryScan>> m_output;
    std::shared_ptr<StatisticsMgr> m_statsMgr {};
    std::shared_ptr<ScanFilter> m_scanFilter;
    std::shared_ptr<FSScannerCheckPoint> m_chkPntMgr {};
};

#endif
