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
#ifdef WIN32
#ifndef FS_SCANNER_WIN32_FOLDER_TRAVERSAL_H
#define FS_SCANNER_WIN32_FOLDER_TRAVERSAL_H

#include "FolderTraversal.h"
#include "Win32ProducerThread.h"
#include "Win32ScannerUtils.h"
#include "ScanFilter.h"
#include "ScanPathMapper.h"
#include "common/FileSystemUtil.h"
#include <string>
#include <map>

class Win32FolderTraversal : public FolderTraversal {
public:
    explicit Win32FolderTraversal(
        std::shared_ptr<StatisticsMgr> statsMgr,
        std::shared_ptr<FSScannerCheckPoint> chkPntMgr,
        std::shared_ptr<ScanFilter> scanFilter,
        std::shared_ptr<BufferQueue<DirectoryScan>> buffer,
        const ScanConfig& config);
    ~Win32FolderTraversal() override = default;

    SCANNER_STATUS Enqueue(const std::string& path, const std::string& prefix = "", uint8_t filterFlag = 0) override;
    SCANNER_STATUS EnqueueWithoutSnapshot(
        const std::string& path,
        const std::string& prefix = "",
        uint8_t filterFlag = 0) override;
    SCANNER_STATUS Start() override;
    SCANNER_STATUS Poll() override;
    SCANNER_STATUS Suspend() override;
    SCANNER_STATUS Resume() override;
    SCANNER_STATUS Abort() override;
    SCANNER_STATUS Destroy() override;
    void ProcessCheckPointContainers() override;

private:
    void EnqueueDirToScanQueue(
        const Module::FileSystemUtil::StatResult&   statResult,
        const std::string&                          dirPath,
        const std::string&                          prefix,
        char                                        originDriver,
        uint8_t                                     filterFlag);

    SCANNER_STATUS PushSingleFileToWriteQueue(
        const Module::FileSystemUtil::StatResult&   statResult,
        const std::string&                          filepath,
        const std::string&                          prefix,
        char                                        originDriver);

    SCANNER_STATUS EnqueueReparsePoint(
        const Module::FileSystemUtil::StatResult&   statResult,
        const std::string&                          fullpath,
        const std::string&                          prefix,
        char                                        originDriver,
        uint8_t filterFlag);

    bool ShouldSkipEnqueue(
        const std::string&                          path,
        const Module::FileSystemUtil::StatResult&   statResult,
        const std::string&                          prefix,
        char                                        originDriver) const;

    void PushEmptyDirToWriteQueue(
        const Module::FileSystemUtil::StatResult&   statResult,
        const std::string&                          dirPath,
        const std::string&                          prefix,
        char                                        originDriver);

    void PushUncompletedDirsToWriteQueue();

    void PushDirToScanQueue(
        const Module::FileSystemUtil::StatResult&   statResult,
        const std::string&                          path,
        std::string                                 prefix,
        char                                        originDriver,
        uint8_t                                     filterFlag);

    std::pair<std::string, char> SplitCompoundPrefix(const std::string& compoundPrefix);
    std::pair<std::string, char> SplitCompoundPrefixWithoutSnapshot(const std::string& compoundPrefix);

private:
    std::vector<std::shared_ptr<Win32ProducerThread>> m_threads;
    ScanPathMapper m_mapper {};
    ScanConfig m_config {};
    std::unordered_map<std::string,
        std::pair<std::string, char>> m_uncompletedDirMap {}; /* <path, <prefix, originDriver>> */
    bool m_completedPush = false;
};

#endif
#endif