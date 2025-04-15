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
#ifndef ARCHIVE_RESTORE_JOB_H
#define ARCHIVE_RESTORE_JOB_H
#include <unordered_map>
#include <vector>
#include <list>
#include "RestoreJob.h"

namespace VirtPlugin {
struct FilePathPair {
    std::string srcName;  // 源卷信息
    std::string cloudFilePath;  // 云端路径
    std::string fsId;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(srcName, srcName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(cloudFilePath, cloudFilePath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(fsId, fsId)
    END_SERIAL_MEMEBER
};

struct FilePathPairInfo {
    std::vector<FilePathPair> filePathPairList;
    void AddFilePathPair(const std::string &srcName, const std::string &cloudFilePath,
        const std::string &fsId)
    {
        FilePathPair filePathPair;
        filePathPair.srcName = srcName;
        filePathPair.cloudFilePath = cloudFilePath;
        filePathPair.fsId = fsId;
        filePathPairList.push_back(filePathPair);
    }
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(filePathPairList, filePathPairList)
    END_SERIAL_MEMEBER
};
class ArchiveRestoreJob : public RestoreJob {
public:
    ArchiveRestoreJob() {};
    ~ArchiveRestoreJob() {};
    EXTER_ATTACK virtual int PrerequisiteJob() override;
    EXTER_ATTACK virtual int ExecuteSubJob() override;
    EXTER_ATTACK virtual int GenerateSubJob() override;
    EXTER_ATTACK virtual int PostJob() override;
private:
    int PrerequisiteArchiveRestoreJobInner();
    int InitArchiveAndGetMeta();
    int InitJobInfo();
    bool InitRepoPaths();
    bool InitS3Info();
    bool InitRepoInfo();
    bool InitArchiveClient();
    bool CloseArchiveClient() const;
    bool PrepareS3Client();
    bool CopyFileToCacheRepo(const std::string& file) const;
    bool GetFileListFromS3(int subJobNum, std::string& checkpoint);
    bool EndArchiveTask();
    int InitArchiveReqPara();
    int ExcuteSubTaskInitialize();
    int ArchiveExecuteSubJobInner();
    bool LoadArchiveMetaData();
    bool LoadVmMetaData() override;
    int ArchivePostJobInner();
    int GetCloudPathList();
    bool GetFileListFromS3();
#ifndef WIN32
    bool AddTypeIpRulePolicy(const std::string &targetIP, const std::string &targetType);
    bool AddArchiveIpRoutePolicy(const std::string &ip);
#endif
    bool ProcessControlFile(const std::string& control);
    bool HandleRestoreFileName(const std::string& inFile, std::string& decName, std::string& srcName) const;
    int RestoreBackupMetaData(const std::string& cloudFilePath, const std::string& fileName, const std::string& fsId);
    int RestoreBackupMetaFile(const std::string& cloudFilePath, const std::string& fileName, const std::string& fsId);
    std::string m_jobId {};
    std::string m_copyId {};
    std::string m_protectId {};
    bool isInitArchiveClient {false};
    int m_maxWriteSizeSMB {0};

    ArchiveS3Info m_s3Info {};                       // 归档服务信息
    std::vector<std::string> m_serviceIpList {};     // 恢复目标端的ip列表

    std::string m_metaFileMountPath {};
    FilePathPairInfo m_filePathPairInfo;
    std::string m_cloudFileMatchPath;
};
}
#endif