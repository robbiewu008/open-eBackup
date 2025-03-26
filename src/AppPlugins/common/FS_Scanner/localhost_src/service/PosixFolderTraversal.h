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
#ifndef FS_SCANNER_POSIX_TRAVERSAL_H
#define FS_SCANNER_POSIX_TRAVERSAL_H
#include "FolderTraversal.h"
#include "PosixProducerThread.h"
#include "PosixUtils.h"
#include "ScanFilter.h"
#include "ScanPathMapper.h"

class PosixFolderTraversal : public FolderTraversal, public PosixUtils {
public:
    explicit PosixFolderTraversal(std::shared_ptr<StatisticsMgr> statsMgr,
        std::shared_ptr<FSScannerCheckPoint> chkPntMgr,
        std::shared_ptr<ScanFilter> scanFilter,
        std::shared_ptr<BufferQueue<DirectoryScan>> buffer,
        ScanConfig config);
    ~PosixFolderTraversal() override;

    SCANNER_STATUS Enqueue(const std::string& path,
        const std::string& prefix = "", uint8_t filterFlag = 0) override;
    SCANNER_STATUS Start() override;
    SCANNER_STATUS Poll() override;
    SCANNER_STATUS Suspend() override;
    SCANNER_STATUS Resume() override;
    SCANNER_STATUS Abort() override;
    SCANNER_STATUS Destroy() override;
    void ProcessCheckPointContainers() override;

    // V2 methods
    SCANNER_STATUS StartV2();
    SCANNER_STATUS EnqueueV2(const std::string& path) override;
    void PushDirToWriteQueueV2(struct stat st, const std::string& originDirPath);
    void PushDirToScanQueueV2(struct stat st, const std::string& mappedDirPath, uint8_t filterFlag);
    SCANNER_STATUS EnqueueFileToWriteQueueV2(struct stat st, const std::string& path);
    void EnqueueDirToScanQueueV2(struct stat st, const std::string& path);

private:
    std::vector<std::shared_ptr<PosixProducerThread>> m_threads;
    ScanPathMapper m_mapper {};
    ScanConfig m_config {};
    std::set<std::string> m_uncompletedDirSet;  // store mapped dir path in v2 case, origin dir path otherwise
    bool m_completedPush = false;

    bool ShouldSkipEnqueue(const std::string& path, const struct stat& st) const;
    SCANNER_STATUS EnqueueDirToScanQueue(struct stat st, std::string path, std::string prefix, uint8_t filterFlag);
    SCANNER_STATUS EnqueueFileToWriteQueue(struct stat st, std::string path, std::string prefix);
    void PushDirToWriteQueue(struct stat st, std::string path);
    bool PushUncompletedDirsToWriteQueue();
    void PushDirToScanQueue(struct stat st, std::string path, std::string prefix, uint8_t filterFlag);
};

#endif