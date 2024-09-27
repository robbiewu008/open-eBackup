/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file CacheRepository.cpp
 * @brief Mount And Umount File System
 * @version 1.1.0
 * @date 2022-02-22
 * @author lixilong wx1101878
 */
#include "servicecenter/services/device/CacheRepository.h"
#include "common/File.h"
#include "securecom/RootCaller.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
namespace AppProtect {
mp_int32 CacheRepository::Umount(
    const std::vector<mp_string> &mountPoints, const mp_string &jobID, const bool &isFileClientMount)
{
    PrepareFileSystem mountHandler;
#ifdef WIN32
    mp_bool deleteFlag = MP_FALSE;
    for (auto mountPoint : mountPoints) {
        std::vector<mp_string> mountInfoVec;
        CMpString::StrSplit(mountInfoVec, mountPoint, '&');
        if (CMpFile::DelFile(mountInfoVec.front() + PATH_SEPARATOR + jobID) == MP_SUCCESS) {
            deleteFlag = MP_TRUE;
            break;
        }
    }
    if (!deleteFlag) {
        WARNLOG("Delete cache repository jobID dir failed, jobId=%s.", jobID.c_str());
    }
#else
    mountHandler.DeleteRepoTempDir("cache", jobID);
#endif
    mp_int32 ret = mountHandler.UmountNasFileSystem(mountPoints, jobID, isFileClientMount);
    if (ret != MP_SUCCESS) {
        WARNLOG("Umount cache repository failed, jobId=%s.", jobID.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_void CacheRepository::QueryRepoSubPath(const PluginJobData &data, std::vector<mp_string> &path)
{
    path.push_back(data.mainID);
    return;
}

mp_int32 CacheRepository::AssembleRepository(const PluginJobData &data, StorageRepository &stRep,
    Json::Value &jsonRep_new)
{
    Json::Value afterHandleJson;
    StructToJson(stRep, afterHandleJson);
    afterHandleJson["path"] = Json::arrayValue;
    for (auto mountPt : m_mountPoint) {
#ifdef WIN32
        std::vector<mp_string> mountInfoVec;
        CMpString::StrSplit(mountInfoVec, mountPt, '&');
        afterHandleJson["path"].append(mountInfoVec.front() + PATH_SEPARATOR + data.mainID);
#else
        mp_string cacheMopuntPoint = mountPt + PATH_SEPARATOR + data.mainID;
        afterHandleJson["path"].append(cacheMopuntPoint);
#endif
    }
    jsonRep_new.append(std::move(afterHandleJson));
    return MP_SUCCESS;
}
}
