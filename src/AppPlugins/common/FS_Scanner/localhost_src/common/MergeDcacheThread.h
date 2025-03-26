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
#ifndef FS_SCANNER_MERGE_DCACHE_THREAD_H
#define FS_SCANNER_MERGE_DCACHE_THREAD_H
 
#include <thread>
#include "StatisticsMgr.h"
#include "ScanFilter.h"
#include "BufferQueue.h"
#include "MergeDcache.h"
 
class MergeDcacheThread {
public:
    bool m_isMetaProducerExecuting = false;
 
    MergeDcacheThread(int threadId, ScanConfig &config, ScanInfo &info)
        : m_threadId(threadId),
        m_config(config),
        m_info(info)
    {};
    ~MergeDcacheThread() {};
 
    bool Start();
    void Stop();
    void Exit();
    void ProducerMain();
    bool IsMergeCompleted();
    bool MergeDcacheFiles(std::string fileName1, std::string fileName2, int index);
    SCANNER_STATUS GetMergeStatus();
 
protected:
    int32_t m_threadId = 0;
    std::shared_ptr<MergeDcache> m_mergeDcacheObj;
    bool m_exit = false;
    ScanConfig &m_config;
    ScanInfo &m_info;
    std::shared_ptr<std::thread> m_mainThread {};
    bool m_mergeCompletedFlag = true;
    SCANNER_STATUS m_mergeStatus = SCANNER_STATUS::SUCCESS;
};
 
#endif // FS_SCANNER_MERGE_DCACHE_THREAD_H