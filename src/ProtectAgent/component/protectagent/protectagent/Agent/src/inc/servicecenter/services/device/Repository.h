/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file Repository.h
 * @brief Mount And Umount File System
 * @version 1.1.0
 * @date 2022-02-22
 * @author lixilong wx1101878
 */
#ifndef _REPOSITORY_H
#define _REPOSITORY_H

#include <vector>
#include "common/Types.h"
#include "common/JsonHelper.h"
#include "taskmanager/externaljob/Job.h"

namespace AppProtect {

struct RepositoryExtendInfo {
    int copyFormat = 0;
    std::string esn;
    std::string fsId;
    bool isCloneFileSystem = true;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(copyFormat, copy_format)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(esn, esn)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(fsId, fsId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(isCloneFileSystem, isCloneFileSystem)
    END_SERIAL_MEMEBER
};

class Repository {
public:
    Repository();
    virtual ~Repository();

    virtual mp_int32 Mount(PluginJobData &data, StorageRepository &stRep,
        Json::Value &jsonRep_new, const MountPermission &permit);
    virtual mp_int32 Umount(
        const std::vector<mp_string> &mountPoints, const mp_string &jobID, const bool &isFileClientMount = false);

protected:
    virtual mp_int32 AssembleRepository(const PluginJobData &data, StorageRepository &stRep, Json::Value &jsonRep_new);
    virtual mp_void QueryRepoSubPath(const PluginJobData &data, std::vector<mp_string> &path);
    std::vector<mp_string> m_mountPoint;
private:
    mp_int32 MountFileIoSystem(
        MountNasParam &param, PluginJobData &data, StorageRepository &stRep, Json::Value &jsonRep_new);
    mp_int32 MountFileClient(
        MountNasParam &param, PluginJobData &data, StorageRepository &stRep, Json::Value &jsonRep_new);
    mp_int32 GetMountIP(const StorageRepository& stRep, MountNasParam& param);
    mp_int32 GetMountNasParam(MountNasParam& param, const PluginJobData &data,
        const StorageRepository &stRep, const MountPermission &permit);
    mp_int32 GetParamFromRepoExtenInfo(MountNasParam& param, const StorageRepository &stRep);
    mp_void GetParamFromExtenInfo(MountNasParam &param, const PluginJobData &data);
    mp_int32 GetSanclientParam(MountNasParam &param, const PluginJobData &data);
    mp_int32 AssembleRemoteHost(const std::set<mp_string>& availStorageIp, StorageRepository &stRep);
    mp_bool IsDataturboMount(PluginJobData &data);
    mp_bool IsLinkEncryption(const PluginJobData &data);
    mp_void GetCopyFormat(MountNasParam& param, const PluginJobData &data,
        const StorageRepository &stRep);
    mp_void ReportMainJobDetail(const mp_string &label, const mp_int32 &errorCode,
        std::vector<std::string> &errorParams, AppProtect::JobLogLevel::type level, PluginJobData &data);
    mp_int32 GetHostAndStorageMap(std::map<mp_string, mp_string> &ipAndEsnMap);
    mp_bool ClusterSelectStorage(mp_string &esn, mp_string &subType);
    mp_int32 GetRepositoryExtenInfo(const StorageRepository &stRep, mp_string key, mp_string &value);
    mp_bool CheckLun(const Json::Value &sanClient);
    mp_string GetMountLunInfo(const Json::Value &lunInfo, MountNasParam &param, const mp_string &resourceId);
    mp_void InsertMountPoint(PluginJobData &data, StorageRepository &stRep, const std::vector<mp_string> &mountPoints);
    mp_bool CheckSanclientMounted(PluginJobData &data, const mp_string &repositoryType);
    mp_int32 RecordSanclientMounted(PluginJobData &data, const mp_string &repositoryType, mp_string &isMountedFile);
    mp_void ClearPwd(MountNasParam &param);
    mp_void ModifyParam(MountNasParam &param);
};
}
#endif