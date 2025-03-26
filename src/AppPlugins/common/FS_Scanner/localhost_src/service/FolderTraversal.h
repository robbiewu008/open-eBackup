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
#ifndef FS_SCANNER_FOLDER_TRAVERSAL_H
#define FS_SCANNER_FOLDER_TRAVERSAL_H

#include <memory>
#include <functional>
#include "ScanConsts.h"
#include "BufferQueue.h"
#include "ScanStructs.h"
#include "StatisticsMgr.h"
#include "ScanFilter.h"
#include "FSScannerCheckPoint.h"
#include "ScanQueue.h"

class FolderTraversal {
public:
    explicit FolderTraversal(std::shared_ptr<StatisticsMgr> statsMgr,
        std::shared_ptr<FSScannerCheckPoint> chkPntMgr = nullptr,
        std::shared_ptr<ScanFilter> scanFilter = nullptr,
        std::shared_ptr<BufferQueue<DirectoryScan>> buffer = nullptr,
        std::shared_ptr<BufferQueue<DirStat>> inputQueue = nullptr,
        std::shared_ptr<ScanQueue> scanQueue = nullptr)
        : m_statsMgr(statsMgr),
          m_chkPntMgr(chkPntMgr),
          m_scanFilter(scanFilter),
          m_bufferQueue(buffer),
          m_input(inputQueue),
          m_scanQueue(scanQueue)
        {};
    
    explicit FolderTraversal(std::shared_ptr<StatisticsMgr> statsMgr,
        std::shared_ptr<ScanFilter> scanFilter)
        : m_statsMgr(statsMgr),
          m_chkPntMgr(nullptr),
          m_scanFilter(scanFilter),
          m_bufferQueue(nullptr),
          m_input(nullptr),
          m_scanQueue(nullptr)
        {};
    virtual ~FolderTraversal() {};

    virtual SCANNER_STATUS Enqueue(const std::string& path, const std::string& prefix = "", uint8_t filterFlag = 0) = 0;
#ifdef WIN32
    virtual SCANNER_STATUS EnqueueWithoutSnapshot(
        const std::string& path,
        const std::string& prefix = "",
        uint8_t filterFlag = 0)
    {
        return SCANNER_STATUS::SUCCESS;
    }
#endif
    virtual SCANNER_STATUS EnqueueV2(const std::string& path)
    {
        (void)path;
        ERRLOG("EnqueueV2 need to be override!");
        return SCANNER_STATUS::FAILED;
    }
    virtual SCANNER_STATUS Start() = 0;
    virtual SCANNER_STATUS Poll() = 0;
    virtual SCANNER_STATUS Suspend() = 0;
    virtual SCANNER_STATUS Resume() = 0;
    virtual SCANNER_STATUS Abort() = 0;
    virtual SCANNER_STATUS Destroy() = 0;
    virtual void ProcessCheckPointContainers() = 0;

protected:
    std::shared_ptr<StatisticsMgr> m_statsMgr {};
    std::shared_ptr<FSScannerCheckPoint> m_chkPntMgr {};
    std::shared_ptr<ScanFilter> m_scanFilter {};
    std::shared_ptr<BufferQueue<DirectoryScan>> m_bufferQueue {};
    std::shared_ptr<BufferQueue<DirStat>> m_input {};
    std::shared_ptr<ScanQueue> m_scanQueue {};
};

#endif