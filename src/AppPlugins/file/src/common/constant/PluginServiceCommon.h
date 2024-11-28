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
#ifndef PLUGIN_SERVICE_COMMON_H
#define PLUGIN_SERVICE_COMMON_H
#include "PluginNasTypes.h"
#include <common/JsonHelper.hpp>
enum NAS_APPTYPE_E {
    APPTYPE_ERR         = -1,
    APPTYPE_NAS_REPLICATION   = 0,
    APPTYPE_NAS_SHARE         = 1,
    APPTYPE_NAS_SNAPSHOT      = 2,
    APPTYPE_NAS_LIVEMOUNT     = 3,
    APPTYPE_BUILDINDEX        = 4
};

// RPC API sync type
enum SyncType {
    SYNCTYPE_SYNC,
    SYNCTYPE_ASYNC
};

// according to different operations from agent RPC API's name
enum OperationType {
    PREREQUISITE_JOB,
    GENERATESUB_JOB,
    EXECUTE_SUBJOB,
    POST_JOB,
    CHECK_BACKUP_JOBTYPE,
    ALLOW_BACKUP_IN_LOCAL_NODE
};

enum ReplicationPairStatus {
    NORMAL = 1,
    SYNCHRONIZING = 23,
    TO_BE_RECOVERED = 33,
    INTERRUPTED = 34,
    SPLIT = 26,
    INVALIID = 35,
    STANDBY = 110
};

struct CopyExtendInfo {
    bool isAggregation {false};

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(isAggregation, isAggregation)
    END_SERIAL_MEMEBER
};

struct FlrTargetObjectAuthExtend {
    std::string protocol {};
    std::string sharePath {};

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(protocol, protocol)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(sharePath, sharePath)
    END_SERIAL_MEMEBER
};

struct FlrRestoreExtend {
    std::string fileReplaceStrategy;
    std::string shareIp;
    std::string targetLocation;
    std::string authType;
    std::string protocol;
    std::string sharePath;
    std::string targetPath;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(fileReplaceStrategy, fileReplaceStrategy)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(shareIp, shareIp)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(targetLocation, targetLocation)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(authType, authType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(protocol, protocol)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(sharePath, sharePath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(targetPath, targetPath)
    END_SERIAL_MEMEBER
};

enum FLR_RESTORE_TYPE {
    FLR_RESTORE_TYPE_ORIGIN         = 0, // 原位置恢复
    FLR_RESTORE_TYPE_NEW            = 1, // 新位置恢复
    FLR_RESTORE_TYPE_NATIVE         = 2, // 本机恢复
};

#endif  // _MSDEF_H_
