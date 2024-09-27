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
#ifndef OBJECT_SERVICE_TASK_H
#define OBJECT_SERVICE_TASK_H

#include <memory>
#include "ThreadPool.h"
#include "Backup.h"
#include "BackupStructs.h"
#include "BlockBufferMap.h"
#include "interface/CloudServiceInterface.h"
#include "manager/CloudServiceManager.h"

enum class ObjectEvent {
    OPEN_SRC,
    OPEN_DST,
    READ_DATA,
    READ_META,
    WRITE_DATA,
    WRITE_META,
    CLOSE_SRC,
    CLOSE_DST,
    DELETE_ITEM,
    CREATE_DIR,
    INVALID
};

struct UploadObjectCbData {
    std::shared_ptr<BackupControlInfo> controlInfo { nullptr };
    std::string fileName {};
    uint32_t maxBlockNum {0};
};

struct UploadInfoMap {
    std::map<std::string, std::vector<Module::UploadInfo>> updInfo {};
    std::mutex uploadMapMtx;
};

void ObjectWriteDataCb(uint64_t partSize, int partNum, void *priData);

struct ObjectServiceParams {
    bool saveMeta { true };
    bool writeMeta { true };
    bool writeAcl { false };
    std::string dataPath;
    std::string cachePath;
    uint64_t blockSize { 0 };
    uint32_t maxBlockNum { 0 };
    Module::StorageConfig authArgs;
    std::vector<std::string> bucketNames;
    ObsBucket dstBucket; // 只能有一个
    bool isfineGrainedRestore {false};

    UploadObjectCbData writeCbData {};
    std::shared_ptr<UploadInfoMap> m_uploadInfoMap;
    BackupType           backupType { BackupType::UNKNOWN_TYPE };
    BackupDataFormat     backupDataFormat { BackupDataFormat::UNKNOWN_FORMAT };
    RestoreReplacePolicy restoreReplacePolicy { RestoreReplacePolicy::NONE };
};

class ObjectServiceTask : public Module::ExecutableItem {
public:
    ObjectServiceTask(ObjectEvent event, std::shared_ptr<BlockBufferMap> bufferMapPtr, FileHandle& fileHandle,
        const ObjectServiceParams& params);
    virtual ~ObjectServiceTask();
    void Exec();
    bool IsCriticalError() const;

public:
    ObjectEvent m_event { ObjectEvent::INVALID };
    std::shared_ptr<BlockBufferMap> m_bufferMapPtr { nullptr };
    FileHandle m_fileHandle;
    ObjectServiceParams m_params;
    BackupPhaseStatus m_backupFailReason {BackupPhaseStatus::FAILED};
    std::pair<std::string, uint64_t> m_errDetails;

private:
    std::unique_ptr<Module::CloudServiceInterface> m_obsCtx {nullptr};
    int GetObjectMtime(time_t &mtime);
    bool IsRestore();
    bool IsBucketName(std::string& path);
    std::string GetBucketName();
    std::string GetObjectKey();
    void SetCriticalErrorInfo(int64_t err);
    bool IsNeedRestore();
    void SaveUplodInfo(uint32_t partNumber, std::string& etag);
    int HandleWriteHugeData();
    int HandleWriteSmallData();
    void HandleReadData();
    void HandleReadMeta();
    void HandleCreateBucket();
    void HandleOpenDst();
    void HandleWriteData();
    void HandleWriteMeta();
    void HandleCloseDst();
    void HandleDelete();

    void FillAclGrant(std::vector<Module::ACLGrant>& aclGrants);
    int GetBucketAcl(std::unique_ptr<Module::GetBucketACLResponse>& newAcl);
    int GetObjectAcl(std::unique_ptr<Module::GetObjectACLResponse>& newAcl);
    void GetObjectMetaFromXattr(std::unordered_map<std::string, std::string> &sysDefMetaData,
        std::unordered_map<std::string, std::string> &userDefMetaData);
    void SetObjectMetaIntoXattr(std::unordered_map<std::string, std::string> &sysDefMetaData,
        std::unordered_map<std::string, std::string> &userDefMetaData);
    bool SetBucketAcl();
    bool SetObjectAcl();
    bool SetObjectMeta();
};

#endif // OBJECT_SERVICE_TASK_H