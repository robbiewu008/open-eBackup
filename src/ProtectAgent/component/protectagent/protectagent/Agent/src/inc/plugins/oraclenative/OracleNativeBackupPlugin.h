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
#ifndef _ORACLE_NATIVE_BACKUP_PLUGIN_H_
#define _ORACLE_NATIVE_BACKUP_PLUGIN_H_

#include <vector>

#include "securecom/RootCaller.h"
#include "message/tcp/CDppMessage.h"
#include "plugins/ServicePlugin.h"
#include "apps/oraclenative/OracleNativeBackup.h"
#include "apps/oracle/Oracle.h"

#include "taskmanager/TaskRedoFuncContainer.h"
#include "apps/oraclenative/OracleNativeBackupTask.h"
#include "apps/oraclenative/OracleNativeExpireCopyTask.h"

class OracleNativeRedoTaskRegister {
public:
    OracleNativeRedoTaskRegister()
    {
        TaskRedoFuncContainer::GetInstance().RegisterNewFunc(
            "OracleNativeBackupTask", OracleNativeBackupTask::CreateRedoTask);
        TaskRedoFuncContainer::GetInstance().RegisterNewFunc(
            "OracleNativeExpireCopyTask", OracleNativeExpireCopyTask::CreateRedoTask);
    }
    ~OracleNativeRedoTaskRegister()
    {}
};

class OracleNativeBackupPlugin : public CServicePlugin {
public:
    OracleNativeBackupPlugin();
    virtual ~OracleNativeBackupPlugin();

    mp_int32 Init(std::vector<mp_uint32>& cmds);
    mp_int32 DoAction(CDppMessage& reqMsg, CDppMessage& rspMsg);

private:
    OracleNativeBackup oracleNativeBackup;
    Oracle oracleCommon;

    EXTER_ATTACK mp_int32 GetDBStorInfo(CDppMessage& reqMsg, CDppMessage& rspMsg);
    EXTER_ATTACK mp_int32 PrepareMedia(CDppMessage& reqMsg, CDppMessage& rspMsg);
    EXTER_ATTACK mp_int32 QueryBackupLevel(CDppMessage& reqMsg, CDppMessage& rspMsg);
    EXTER_ATTACK mp_int32 BackupDataFile(CDppMessage& reqMsg, CDppMessage& rspMsg);
    EXTER_ATTACK mp_int32 BackupLogFile(CDppMessage& reqMsg, CDppMessage& rspMsg);
    EXTER_ATTACK mp_int32 RestoreOracleDB(CDppMessage& reqMsg, CDppMessage& rspMsg);
    EXTER_ATTACK mp_int32 LiveMountOracleDB(CDppMessage& reqMsg, CDppMessage& rspMsg);
    EXTER_ATTACK mp_int32 CancelLiveMountOracleDB(CDppMessage& reqMsg, CDppMessage& rspMsg);
    EXTER_ATTACK mp_int32 InstanceRestore(CDppMessage& reqMsg, CDppMessage& rspMsg);
    EXTER_ATTACK mp_int32 DisMountMedium(CDppMessage& reqMsg, CDppMessage& rspMsg);
    EXTER_ATTACK mp_int32 ExpireCopy(CDppMessage& reqMsg, CDppMessage& rspMsg);
    EXTER_ATTACK mp_int32 StopTask(CDppMessage& reqMsg, CDppMessage& rspMsg);

    // Query host's role in Oracle RAC
    EXTER_ATTACK mp_int32 QueryHostRoleInCluster(CDppMessage& reqMsg, CDppMessage& rspMsg);
    mp_int32 CreateOracleNativeRedoTask(std::string taskType);
};

#endif
