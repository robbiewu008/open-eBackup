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
#ifndef FS_SCANNER_POSIX_META_PRODUCER_H
#define FS_SCANNER_POSIX_META_PRODUCER_H

#include <dirent.h>
#include "PosixUtils.h"
#include "MetaProducer.h"
#include "FSScannerCheckPoint.h"

class PosixMetaProducer : public MetaProducer, public PosixUtils {
public:
    explicit PosixMetaProducer(std::shared_ptr<ScanQueue> scanQueue,
        std::shared_ptr<BufferQueue<DirectoryScan>> output,
        std::shared_ptr<StatisticsMgr> statsMgr,
        std::shared_ptr<ScanFilter> scanFilter,
        std::shared_ptr<FSScannerCheckPoint> chkPntMgr,
        ScanConfig config)
        : MetaProducer(scanQueue, output, statsMgr, scanFilter, chkPntMgr), m_config(config)
    {};
    PosixMetaProducer() {};

    ~PosixMetaProducer() override {};

    void Produce(int count) override;

private:
    bool TryStat(DirStat &dirStat);

    void PartiallyOrCompletelyPushDirectoryToWriteQueue(
        bool                            scanComplete,
        DirectoryScan&                  node,
        const Module::DirMetaWrapper&   dirWrapper,
        uint8_t                         baseFilterFlag,
        const std::string&              prefix);

    void ScanDirectory(
        Module::DirMetaWrapper& dirWrapper,
        uint8_t                 baseFilterFlag,
        const std::string&      prefix);

    void ReadDirectoryEntry(
        const struct dirent*    direntry,
        const std::string&      dirPath,
        const uint8_t&          baseFilterFlag,
        const std::string&      prefix,
        DirectoryScan&          node,
        Module::DirMetaWrapper& dirWrapper);

    // Search for the position of Objcet in ScanInProgressList and erase Object if found in list
    bool SkipDirEntry(
        const std::string &name,
        const std::string &fullPath,
        const std::string& prefix) const;

    // V2 methods
    void ProduceV2(int count);

    void PartiallyOrCompletelyPushDirectoryToWriteQueueV2(
        DirectoryScan&                  node,
        const Module::DirMetaWrapper&   dirWrapper,
        uint8_t                         baseFilterFlag);

    void ScanDirectoryV2(Module::DirMetaWrapper& dirWrapper, uint8_t baseFilterFlag);

    void ReadDirectoryEntryV2(
        const struct dirent*    direntry,
        const std::string&      dirPath,
        const uint8_t&          baseFilterFlag,
        DirectoryScan&          node,
        Module::DirMetaWrapper& dirWrapper);

    bool SkipDirEntryV2(const std::string &name, const std::string &mappedDirPath) const;

private:
    ScanConfig m_config {};
    int m_resumeFlag = 0;
};
#endif