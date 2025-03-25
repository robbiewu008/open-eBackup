/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Author: z30016470
 * Create: 2022/06/30
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