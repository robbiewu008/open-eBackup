/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Author: g00554214
 * Create: 2022/09/30
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
