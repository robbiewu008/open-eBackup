/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
 * @file DefaultFolderTraversal.h
 * @date 8/27/2022
 * @author w30029850
 * @brief
 */
#ifdef NAS_SNAPDIFF

#ifndef FS_SCANNER_NAS_SNAPDIFF_TRAVERSAL_H
#define FS_SCANNER_NAS_SNAPDIFF_TRAVERSAL_H

#include "FolderTraversal.h"
#include <memory>
#include <map>
#include <list>
#include <thread>
#include "NfsContextWrapper.h"
#include "SmbContextWrapper.h"
#include "SnapdiffBackupEntry.h"
#include "ParserUtils.h"
#include "ControlDevice.h"
#include "CommonService.h"

class NasSnapdiffTraversal : public FolderTraversal {
public:
    explicit NasSnapdiffTraversal(std::shared_ptr<StatisticsMgr> statsMgr,
        std::shared_ptr<ScanFilter> scanFilter,
        std::shared_ptr<BufferQueue<SnapdiffResultMap>> m_snapdiffBuffer,
        ScanConfig &config);

    ~NasSnapdiffTraversal();

    SCANNER_STATUS Enqueue(const std::string& directory,
        const std::string& prefix = "", uint8_t filterFlag = 0) override;
    SCANNER_STATUS Start() override;
    SCANNER_STATUS Poll() override;
    SCANNER_STATUS Suspend() override;
    SCANNER_STATUS Resume() override;
    SCANNER_STATUS Abort() override;
    SCANNER_STATUS Destroy() override;
    void ProcessCheckPointContainers() override;

private:

    SCANNER_STATUS SnapdiffScan();
    void PerformSnapdiffRequest(std::unique_ptr<Module::ControlDevice> &oceanstorDevice, const std::string &sessionID);
    int FilterSnapdiffResultMap(SnapdiffResultMap& snapdiffResultMap);
    void ReadDiffInfoIntoMap(Module::SnapdiffMetadataInfo diffInfo[],
        SnapdiffResultMap& snapdiffResultMap, int numChanges);
    void CheckDeviceResourceName();
    std::unique_ptr<Module::ControlDevice> InitOceanStorDevice();
    void CloseSnapdiffSession(const std::string &sessionID, std::unique_ptr<Module::ControlDevice> &oceanstorDevice);

private:
    ScanConfig &m_config;
    std::shared_ptr<BufferQueue<SnapdiffResultMap>> m_buffer;

    bool m_snapdiffRequestDone {false};
};

#endif
#endif