/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023. All rights reserved.
 * Author: w30029850
 * Create: 2023/01/16
 */
#ifdef WIN32

#ifndef FS_SCANNER_WIN32_META_PRODUCER_H
#define FS_SCANNER_WIN32_META_PRODUCER_H

#include "Win32ScannerUtils.h"
#include "MetaProducer.h"
#include "FSScannerCheckPoint.h"
#include "common/FileSystemUtil.h"

class Win32MetaProducer : public MetaProducer {
public:
    explicit Win32MetaProducer(
        std::shared_ptr<ScanQueue> scanQueue,
        std::shared_ptr<BufferQueue<DirectoryScan>> output,
        std::shared_ptr<StatisticsMgr> statsMgr,
        std::shared_ptr<ScanFilter> scanFilter,
        std::shared_ptr<FSScannerCheckPoint> chkPntMgr,
        ScanConfig config)
        : MetaProducer(scanQueue, output, statsMgr, scanFilter, chkPntMgr), m_config(config)
    {};
    Win32MetaProducer() {};
    ~Win32MetaProducer() override {};
    void Produce(int count) override;

private:
    ScanConfig m_config {};
    int m_resumeFlag = 0;

    bool StatDir(DirStat &dirStat) const;

    void PushDirectoryToWriteQueue(
        bool                                        scanComplete,
        DirectoryScan&                              node,
        const Module::DirMetaWrapper&               dirWrapper,
        uint8_t                                     baseFilterFlag,
        const std::string&                          prefix,
        char                                        originDriver);

    void ScanDirectory(
        const std::string&                          dirPath,
        Module::DirMetaWrapper&                     dirWrapper,
        uint8_t                                     baseFilterFlag,
        const std::string&                          prefix,
        char                                        originDriver);

    void ReadDirectoryEntry(
        const Module::FileSystemUtil::OpenDirEntry& opendirEntry,
        uint8_t                                     baseFilterFlag,
        DirectoryScan&                              node,
        Module::DirMetaWrapper&                     dirWrapper,
        const std::string&                          prefix,
        char                                        originDriver);
    
    void ProcessDirEntry(
        DirStat& dirStat,
        Module::DirMetaWrapper& dirWrapper,
        const std::string& rawFilePath,
        uint8_t baseFilterFlag);
    
    void ProcessFileEntry(
        DirectoryScan& node,
        Module::FileMetaWrapper& fileWrapper,
        const std::string& rawFilePath);

    void ReadReparseSymlinkEntry(
        const Module::FileSystemUtil::StatResult&   statResult,
        const Module::FileMetaWrapper&              fileWrapper,
        const std::string&                          rawFilePath,
        uint8_t                                     baseFilterFlag,
        DirectoryScan&                              node,
        Module::DirMetaWrapper&                     dirWrapper) const;

    // Search for the position of Objcet in ScanInProgressList and erase Object if found in list
    bool SkipDirEntry(
        const Module::FileSystemUtil::OpenDirEntry& opendirEntry,
        const std::string&                          rawFilePath) const;

    void BlockingTryPush(
        DirStat&                                    dirStat,
        uint32_t                                    timeout,
        uint32_t                                    sleepMilliseconds,
        uint32_t                                    maxRetry);

    void AmendMissingParentDirectories(const std::string& dirPath);

    void QueryStreams(const std::string& fullPath, std::vector<std::string>& streams);

    std::optional<std::wstring> GetStreamNameW(const std::wstring& wStreamName);
};

#endif
#endif