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
#ifndef APP_TASK_MANAGE_H_
#define APP_TASK_MANAGE_H_

#include "apps/dws/XBSAServer/xbsa_types.h"
#include "apps/dws/XBSAServer/DwsTaskCommonDef.h"
class AppTaskManage {
public:
    AppTaskManage() {}
    virtual mp_int32 UpdateTaskWhenCreateObject(const BsaObjectDescriptor &objDesc) = 0;
    virtual mp_int32 UpdateTaskWhenQueryObject(const BsaQueryDescriptor &objDesc) = 0;
    virtual mp_void AllocFilesystem(BsaObjInfo &objInfo) = 0;
    mp_string GetBsaDbFilePath();
    const DwsCacheInfo &GetCacheInfo();
    mp_string GetTaskId();
    void GenStorePath(BsaObjInfo &objInfo);
    virtual mp_bool FillQuryRsp(mp_long bsaHandle, const BsaObjInfo &queryReslt, QueryObjectResult &rsp);
    virtual mp_bool FillQuryRsp(mp_long bsaHandle, const BsaObjInfo &queryReslt, GetNextQueryObjectResult &rsp);

protected:
    mp_bool IsBackupTask();
    mp_bool IsRestoreTask();
    bool IsBackupCopy(mp_uint32 copyType);
    bool IsReplicationCopy(mp_uint32 copyType);
    bool IsCloudArchiveCopy(mp_uint32 copyType);
    bool IsTapeArchiveCopy(mp_uint32 copyType);
    mp_int32 CreateBsaDb();
    mp_int32 GetFsRelation();
    mp_int32 ParseFsRelation(DwsFsRelation &relation);
    mp_bool TransFilesystem(const BsaObjInfo &queryReslt, FsKeyInfo &fsInfo);
    mp_bool GetNewFsInRelationMap(const BsaObjInfo &queryReslt, FsKeyInfo &newFsInfo);
    mp_string GetCloudCopyFsId(const BsaObjInfo &queryReslt);
    mp_string GetArchiveServerIp();
    template<typename T>
    mp_bool FillQuryRspCommon(mp_long bsaHandle, const BsaObjInfo &queryReslt, T &rsp);

protected:
    mp_string m_taskId; // 当前正在执行的任务
    DwsTaskInfo m_taskInfo;
    DwsCacheInfo m_cacheInfo;
    std::map<FsKeyInfo, FsKeyInfo> m_fsRelationMap;
};

#endif