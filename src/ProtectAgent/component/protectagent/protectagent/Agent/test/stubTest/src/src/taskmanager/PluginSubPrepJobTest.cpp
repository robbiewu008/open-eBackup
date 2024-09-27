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
#include <chrono>
#include "common/Log.h"
#include "common/JsonUtils.h"
#include "message/tcp/CSocket.h"
#include "taskmanager/externaljob/PluginSubPrepJob.h"
#include "taskmanager/PluginSubPrepJobTest.h"
#include "taskmanager/externaljob/Job.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"


static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_int32 StubCheckHostLinkStatus(
    const mp_string& strSrcIp, const mp_string& strHostIp, mp_uint16 uiPort, mp_int32 timeout)
{
    return MP_SUCCESS;
}


TEST_F(PluginSubPrepJobTest, CheckArchiveConnectIpTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    AppProtect::PluginJobData data;
    AppProtect::PluginSubPrepJob subPreJob(data);
    stub.set(&CSocket::CheckHostLinkStatus, StubCheckHostLinkStatus);
    std::vector<Json::Value> vecJsonArchiveIp;
    Json::Value archiveIp;
    archiveIp["ip"]="192.168.88.98";
    archiveIp["port"]=30066;
    vecJsonArchiveIp.push_back(archiveIp);
    Json::Value connectIp;
    subPreJob.CheckArchiveConnectIp(vecJsonArchiveIp,connectIp);
    EXPECT_EQ(30066, connectIp[0]["port"].asInt());
}
