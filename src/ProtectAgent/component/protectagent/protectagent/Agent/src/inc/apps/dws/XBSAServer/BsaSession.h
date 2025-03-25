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
#ifndef _BSA_SESSION_H_
#define _BSA_SESSION_H_

#include "common/Types.h"
#include "common/MpString.h"
#include "xbsa/xbsa.h"
#include "apps/dws/XBSAServer/xbsa_types.h"
#include "apps/dws/XBSAServer/BsaTransManager.h"
#include "apps/dws/XBSAServer/AppTaskManage.h"

typedef enum tagBsaEnv {
    BSA_API_VERSION = 0,
    BSA_SERVICE_HOST,
    BSA_TPOPS_JOB_TYPE,
    BSA_ENV_BUTT
} BsaEnv;

const mp_string BsaEnvKeys[BSA_ENV_BUTT] = {
    "BSA_API_VERSION",
    "BSA_SERVICE_HOST",
    "BSA_TPOPS_JOB_TYPE"
};

// BsaSession类
class BsaSession {
public:
    BsaSession(mp_long bsaHandle, BSA_AppType appType) : m_bsaHandle(bsaHandle), m_timerHandle(0), m_trans(NULL),
                                                         m_appType(appType) {}
    ~BsaSession() { };

    EXTER_ATTACK mp_int32 Init(const BsaObjectOwner& objectOwner, const mp_string env[BSA_ENV_BUTT], mp_uint32 envSize);
    mp_int32 CreateTaskManage();
    mp_void Finit();
    mp_void SetEnv(BsaEnv idx, const mp_string &val);
    mp_string GetEnv(BsaEnv idx);
    mp_string GetAllEnv();

    mp_int32 BeginTxn();
    mp_int32 EndTxn(BSA_Vote vote);
    BsaTransaction *GetTrans();
    BSA_ObjectOwner &GetOwner();
    mp_int32 UpdateTaskWhenCreateObject(const BsaObjectDescriptor &objDesc);
    mp_int32 UpdateTaskWhenQueryObject(const BsaQueryDescriptor &objDesc);
    mp_void AllocFilesystem(BsaObjInfo &objInfo);
    mp_string GetTaskId();
    mp_bool FillQuryRsp(mp_long bsaHandle, const BsaObjInfo &queryReslt, QueryObjectResult &rsp);
    mp_bool FillQuryRsp(mp_long bsaHandle, const BsaObjInfo &queryReslt, GetNextQueryObjectResult &rsp);
    const DwsCacheInfo &GetCacheInfo();
    int32_t GetAppType();
    void GenStorePath(BsaObjInfo &objInfo);

private:
    mp_bool inline TransExist();
    mp_void ResetTimer();
    mp_int32 GetSessionTimeoutInterval(mp_int32 &timeoutMs);

    mp_long m_bsaHandle;
    mp_uint32 m_timerHandle;
    mp_string m_envs[BSA_ENV_BUTT]; // 会话的环境变量
    BSA_ObjectOwner m_owner;        // 会话所属的owner
    std::shared_ptr<BsaTransaction> m_trans; // 会话正在进行中的事务
    BSA_AppType m_appType;
    std::shared_ptr<AppTaskManage> m_appTaskManager;
};

#endif // _BSA_SESSION_H_