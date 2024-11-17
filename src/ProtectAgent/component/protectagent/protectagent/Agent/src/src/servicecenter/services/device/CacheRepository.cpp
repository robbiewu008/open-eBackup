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
