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
#ifndef COMMON_BACKUP_SERVICE_H
#define COMMON_BACKUP_SERVICE_H

#include "trjsontostruct.h"
#include "trstructtojson.h"

namespace GeneralDB {
class CommonBackupService {
public:
    CommonBackupService() = default;
    ~CommonBackupService() = default;
public:
    static void CheckBackupJobType(ActionResult& returnValue, const AppProtect::BackupJob& job);
    static void AllowBackupInLocalNode(ActionResult& returnValue, const AppProtect::BackupJob& job,
        const AppProtect::BackupLimit::type limit);
    static void AllowBackupSubJobInLocalNode(ActionResult& returnValue, const AppProtect::BackupJob& job,
        const AppProtect::SubJob& subJob);
    static void AllowRestoreInLocalNode(ActionResult& returnValue, const AppProtect::RestoreJob& job);
    static bool IsScriptExist(const mp_string &appType, const mp_string &cmdType);
    static void AllowRestoreSubJobInLocalNode(ActionResult& returnValue, const AppProtect::RestoreJob& job,
        const AppProtect::SubJob& subJob);
    static void QueryJobPermission(AppProtect::JobPermission& returnJobPermission,
        const ApplicationEnvironment& appEnv, const Application& application);
    static void DeliverTaskStatus(ActionResult& returnValue, const std::string& status,
        const std::string& taskId, const std::string& script);
    static void AllowCheckCopyInLocalNode(ActionResult& returnValue, const AppProtect::CheckCopyJob& job);
    static void AllowCheckCopySubJobInLocalNode(ActionResult& returnValue, const AppProtect::CheckCopyJob& job,
        const AppProtect::SubJob& subJob);
};
}
#endif // COMMON_BACKUP_SERVICE_H