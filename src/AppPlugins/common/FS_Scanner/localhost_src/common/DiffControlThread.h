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
#ifndef FS_SCANNER_DIFF_CONTROL_THREAD_H
#define FS_SCANNER_DIFF_CONTROL_THREAD_H

#include <thread>
#include "StatisticsMgr.h"
#include "ScanFilter.h"
#include "BufferQueue.h"
#include "DiffControlService.h"
#include "HardlinkManager.h"

class DiffControlThread {
public:
    bool m_isMetaProducerExecuting = false;

    DiffControlThread(int threadId, ScanConfig &config, HardlinkManager &hardLinkManager, const DiffControlInfo& info)
        : m_threadId(threadId),
        m_config(config),
        m_hardLinkManager(hardLinkManager),
        m_info(info) {}
    ~DiffControlThread() {}

    bool Start(bool isFull);
    void DiffServiceMain();
    void Exit();
    void Push(CompareDirectory &dirObj);
    void ProducerMain();
    bool IsDiffCompleted() const;
    int FlushHardlinkMap() const;

protected:
    int32_t m_threadId = 0;
    std::shared_ptr<DiffControlService> m_diffControlService;
    bool m_exit = false;
    std::shared_ptr<StatisticsMgr> m_statsMgr {};
    ScanConfig &m_config;
    HardlinkManager &m_hardLinkManager;
    std::shared_ptr<BufferQueue<CompareDirectory>> m_dirQueue = nullptr;
    std::shared_ptr<std::thread> m_mainThread {};
    DiffControlInfo m_info;
};

#endif
