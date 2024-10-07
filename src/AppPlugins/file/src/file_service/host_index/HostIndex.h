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
#ifndef HOSTINDEX_H
#define HOSTINDEX_H

#include <memory>
#include <future>
#include "BasicJob.h"
#include "Module/src/common/JsonHelper.h"
#include "ScanMgr.h"
#include "log/Log.h"
#ifdef WIN32
#include "ApplicationProtectFramework_types.h"
#include "Defines.h"
#endif

namespace FilePlugin {
enum class HostIndexType {
    HOST_INDEX_TYPE_FULL,
    HOST_INDEX_TYPE_INC
};

class HostIndex : public BasicJob, public std::enable_shared_from_this<HostIndex> {
public:
    HostIndex() {}
    ~HostIndex()
    {
        INFOLOG("Host index deconstruct!");
    }

    EXTER_ATTACK int PrerequisiteJob() override;
    EXTER_ATTACK int GenerateSubJob() override;
    EXTER_ATTACK int ExecuteSubJob() override;
    EXTER_ATTACK int PostJob() override;
    uint32_t m_rfiIndex {0};
    bool m_isLastScan {false};
    struct StorageRepositoryExtendInfo {
        bool isCurrentCopyRepo{};
        int timestamp{};
        std::string copyId;
        std::string fsId;

        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(isCurrentCopyRepo, isCurrentCopyRepo);
        SERIAL_MEMBER_TO_SPECIFIED_NAME(timestamp, timestamp);
        SERIAL_MEMBER_TO_SPECIFIED_NAME(copyId, copyId);
        SERIAL_MEMBER_TO_SPECIFIED_NAME(fsId, fsId);
        END_SERIAL_MEMEBER
    };

private:
    // 索引任务StorageRepositoryExtendInfo结构体

    struct RfiGeneratationParam {
        std::string copyId;
        std::vector<std::string> rfiFiles;

        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(copyId, copyId)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(rfiFiles, rfiFiles)
        END_SERIAL_MEMEBER
    };

    int PrerequisiteJobInner();
    int GenerateSubJobInner();
    int ExecuteSubJobInner();
    int PostJobInner();

    bool StartScanner();

    void ReportJob(AppProtect::SubJobStatus::type status);
    std::shared_ptr<AppProtect::BuildIndexJob> GetJobInfoBody();

    int IdentifyRepos();
    int IdentifyRepo(AppProtect::StorageRepository& repo);
    int ProcessFullHostIndex();
    int ProcessIncHostIndex();
    int ProcessIncHostIndex2(const std::vector<std::string>& curDirList, const std::vector<std::string>& prevDirList);
    int ProcessIncHostIndex3(std::queue<std::string>& curQueue, std::queue<std::string>& prevQueue);
    void PrepareForGenrateRfi(std::string preMetaFilePath, std::string curMetafilepath);
    void FillScanConfigForGenerateRfi(const std::string& prevDcachePath,
        const std::string& curDcachePath, bool isLastScan, bool isPre);
    bool CheckDcacheExist(std::string metaFilePath) const;
    int GenerateIncRfiInPath(const std::string& prevPath, const std::string& curPath);
    SCANNER_STATUS MonitorScanner();
    int GenerateFullRfiInPath(const std::string& path, bool isPre);
    void UnzipMetafileToCurPathAndRemoveAsync(const std::string& path, std::promise<int>& promiseObj);
    int UnzipMetafileToCurPathAndRemove(const std::string& path) const;
    void CheckIsLastScan(const std::queue<std::string>& curQueue, const std::queue<std::string>& preQueue);
    static int GetRfiIndex(const std::string& rfiPath);
    void PrepareForGenerateIncRfiInPath(const std::string& metaFilePath, const std::string& prevMetaFilePath,
        std::promise<int>& promiseObj);

    static void GeneratedCopyCtrlFileCb(void *usrData, std::string ctrlFile);
    static void GeneratedHardLinkCtrlFileCb(void *usrData, std::string ctrlFile);
    static void GenerateRfiCtrlFileCb(void *usrData, RfiCbStruct cbParam);

    std::shared_ptr<AppProtect::BuildIndexJob> m_indexPara = nullptr;
    std::shared_ptr<AppProtect::StorageRepository> m_cacheRepo = nullptr;
    std::shared_ptr<AppProtect::StorageRepository> m_preRepo = nullptr;
    std::shared_ptr<AppProtect::StorageRepository> m_curRepo = nullptr;
    std::shared_ptr<AppProtect::StorageRepository> m_metaRepo = nullptr;
    std::shared_ptr<AppProtect::StorageRepository> m_preMetaRepo = nullptr;
    std::shared_ptr<AppProtect::StorageRepository> m_indexRepo = nullptr;
    std::shared_ptr<StorageRepositoryExtendInfo> m_preRepoExtendInfo = nullptr;
    std::shared_ptr<StorageRepositoryExtendInfo> m_curRepoExtendInfo = nullptr;
    ScanConfig m_scanConfig {};
    HostIndexType m_indexType { HostIndexType::HOST_INDEX_TYPE_FULL };
    std::shared_ptr<Scanner> m_scanner = nullptr;

    std::string m_workDir;
    std::atomic<bool> m_isPreparing {false};
};
}
#endif