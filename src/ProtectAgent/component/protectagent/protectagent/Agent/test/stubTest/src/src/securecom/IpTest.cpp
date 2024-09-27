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
#include "common/ErrorCode.h"
#include "securecom/IpTest.h"
#include "message/tcp/CSocket.h"

using namespace std;

namespace {
int flag = 0;

mp_int32 StubCheckHostLinkStatusFailed(
    const mp_string& strSrcIp, const mp_string& strHostIp, mp_uint16 uiPort, mp_int32 timeout)
{
    return MP_FAILED;
}

mp_int32 StubCheckHostLinkStatusTrueOnTwo(
    const mp_string& strSrcIp, const mp_string& strHostIp, mp_uint16 uiPort, mp_int32 timeout)
{
    if (flag ++ < 1) {
        return MP_FALSE;
    }
    return MP_TRUE;
}
}

static mp_void StubCLoggerLog(mp_void){
    return;
}

TEST_F(IPTest, CheckHostLinkStatus){

    mp_string strSrcIP = "192.168";
    vector<mp_string> hostIpList;
    hostIpList.push_back("192.169");
    hostIpList.push_back("192.170");
    SecureCom::CIP om;
    mp_bool bRet;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(CSocket, CheckHostLinkStatus), StubCheckHostLinkStatus);
    bRet = om.CheckHostLinkStatus(strSrcIP, hostIpList);
    EXPECT_EQ(bRet, MP_TRUE);

    vector<mp_string> hostIpList1;
    hostIpList1.push_back("");
    bRet = om.CheckHostLinkStatus(strSrcIP, hostIpList1);
    EXPECT_EQ(bRet, MP_FALSE);

    stub.set(ADDR(CSocket, CheckHostLinkStatus), StubCheckHostLinkStatusFailed);
    bRet = om.CheckHostLinkStatus(strSrcIP, hostIpList);
    EXPECT_EQ(bRet, MP_FALSE);
}

TEST_F(IPTest, CheckHostLinkStatus_override){

    mp_string strSrcIP = "192.168";
    vector<mp_string> hostIpv4List;
    vector<mp_string> hostIpv6List;
    vector<mp_string> srcIpv4List;
    vector<mp_string> srcIpv6List;
    mp_bool bRet;
    
    SecureCom::CIP om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.CheckHostLinkStatus(hostIpv4List, hostIpv6List, srcIpv4List, srcIpv6List);

    hostIpv4List.push_back("192.169");
    hostIpv4List.push_back("192.170");

    hostIpv6List.push_back("192.180");
    hostIpv6List.push_back("192.181");

    om.CheckHostLinkStatus(hostIpv4List, hostIpv6List, srcIpv4List, srcIpv6List);

    srcIpv4List.push_back("192.169");
    srcIpv4List.push_back("192.170");
    flag = 0;
    stub.set(ADDR(CSocket, CheckHostLinkStatus), StubCheckHostLinkStatusTrueOnTwo);
    bRet = om.CheckHostLinkStatus(hostIpv4List, hostIpv6List, srcIpv4List, srcIpv6List);
    EXPECT_EQ(bRet, MP_SUCCESS);

    srcIpv4List.clear();
    srcIpv6List.push_back("192.180");
    srcIpv6List.push_back("192.181");
    flag = 0;
    bRet = om.CheckHostLinkStatus(hostIpv4List, hostIpv6List, srcIpv4List, srcIpv6List);
    EXPECT_EQ(bRet, MP_SUCCESS);
}

