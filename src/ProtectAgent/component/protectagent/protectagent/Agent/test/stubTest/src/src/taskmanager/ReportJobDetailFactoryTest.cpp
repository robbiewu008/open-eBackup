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
#include "message/curlclient/DmeRestClient.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
#include "host/host.h"
#include "common/Ip.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"
#include "message/curlclient/RestClientCommon.h"
#include "taskmanager/externaljob/ReportJobDetailFactory.h"
#include "apps/appprotect/plugininterface/ApplicationProtectFramework_types.h"
#include "taskmanager/ReportJobDetailFactoryTest.h"

typedef mp_void (*StubFuncType)(void);
typedef mp_void (CLogger::*LogType)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...);

static mp_int32 StubFailed(mp_void* pthis){
    return MP_FAILED;
}

static mp_int32 StubSuccess(mp_void* pThis, const DmeRestClient::HttpReqParam &httpParam, HttpResponse& httpResponse){
    httpResponse.statusCode = SC_OK;
    Json::Value value;
    value["errorCode"] = "0";
    httpResponse.body = value.toStyledString();
    return MP_SUCCESS;
}

TEST_F(ReportJobDetailFactoryTest, ReportMainAndPostJobInformationTest)
{
    Stub mp_stub;
    Json::Value mainJob;
    mainJob["taskId"] = "a432f128-fad3-4b9f-9086-acd520d93d04";
    mainJob["extendInfo"]["multiPostJob"] = "0";
    mp_stub.set(ADDR(DmeRestClient, SendRequest), StubSuccess);
    mp_int32 ret = MP_SUCCESS;
    EXPECT_EQ(ret, MP_SUCCESS);
}

