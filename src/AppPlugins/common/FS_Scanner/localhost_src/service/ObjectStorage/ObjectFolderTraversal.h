/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
* Description: Object storage bucket scanner.
* Author: w00444223
* Create: 2023-12-04
*/

#ifndef FS_SCANNER_OBJECT_TRAVERSAL_H
#define FS_SCANNER_OBJECT_TRAVERSAL_H

#include "FolderTraversal.h"
#include "ScanFilter.h"
#include "ObjectProducerThread.h"

enum class TraversalStatus {
    DeFault,
    Failed,
    Success
};

class ObjectFolderTraversal : public FolderTraversal {
public:
    explicit ObjectFolderTraversal(std::shared_ptr<StatisticsMgr> statsMgr,
        std::shared_ptr<FSScannerCheckPoint> chkPntMgr,
        std::shared_ptr<ScanFilter> scanFilter,
        std::shared_ptr<BufferQueue<DirectoryScan>> buffer,
        ScanConfig config);
    ~ObjectFolderTraversal() override;

    SCANNER_STATUS Enqueue(const std::string& path, const std::string& prefix = "", uint8_t filterFlag = 0) override;
    SCANNER_STATUS Start() override;
    SCANNER_STATUS Poll() override;
    SCANNER_STATUS Suspend() override;
    SCANNER_STATUS Resume() override;
    SCANNER_STATUS Abort() override;
    SCANNER_STATUS Destroy() override;
    void ProcessCheckPointContainers() override;

private:
    std::vector<std::shared_ptr<ObjectProducerThread>> m_threads;
    ScanConfig m_config {};

    SCANNER_STATUS EnqueueDirToScanQueue(std::string path, std::string prefix, uint8_t filterFlag);
    void PushDirToScanQueue(std::string path, std::string prefix, uint8_t filterFlag);
    int EnqueueDirAndCount();
    void CreateMetaProducerThreads(int threadCount);
    void LogTraversal();

    std::shared_ptr<std::thread> m_logTraversalThread = nullptr;
    TraversalStatus m_status = TraversalStatus::DeFault;
};

#endif
