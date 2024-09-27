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
#ifndef __BSACLIENTTEST_H__
#define __BSACLIENTTEST_H__

#define private public
#include <sstream>
#include "common/Log.h"
#include "common/Ip.h"
#include "common/Utils.h"
#include "common/DB.h"
#include "message/tcp/CConnection.h"
#include "message/tcp/CDppMessage.h"
#include "message/archivestream/ArchiveStreamClient.h"
#include "gtest/gtest.h"
#include "stub.h"
#ifdef SUPPORT_SSL
#include "openssl/err.h"
#endif

using std::ostringstream;
namespace {
const int SLEEP_1000_MS = 1000;
const int RETRY_CONNECT_NUM = 5;
const int SLEEP_200_MS = 200;
}

class TestArchiveStreamClient : public testing::Test {
public:
    void SetUp()
    {
    }

    void TearDown()
    {
    }

    static void SetUpTestCase()
    {
    }

    static void TearDownTestCase()
    {
    }
};

mp_bool IsIPV4_true(const mp_string& strIpAddr)
{
    return MP_TRUE;
}
mp_bool IsIPV4_false(const mp_string& strIpAddr)
{
    return MP_FALSE;
}

mp_int32 IPV4StrToUInt_SUCCESS(mp_string& strIpAddr, mp_uint32& uiIpAddr)
{
    return MP_SUCCESS;
}

mp_int32 IPV4StrToUInt_FAILED(mp_string& strIpAddr, mp_uint32& uiIpAddr)
{
    return MP_FAILED;
}

mp_bool IsIPV6_true(const mp_string& strIpAddr)
{
    return MP_TRUE;
}
mp_bool IsIPV6_false(const mp_string& strIpAddr)
{
    return MP_FALSE;
}

mp_int32 IPV6StrToUInt_SUCCESS(mp_string& strIpAddr, mp_uint32& uiIpAddr)
{
    return MP_SUCCESS;
}

mp_int32 IPV6StrToUInt_FAILED(mp_string& strIpAddr, mp_uint32& uiIpAddr)
{
    return MP_FAILED;
}

mp_void SetClientIpAddr_stub(mp_uint32 uiClientIpAddr){}

mp_bool SetClientIpv6Addr_true(mp_uint32 *uiClientIpv6Addr, uint32_t uLen)
{
    return MP_TRUE;
}

mp_int32 CreateTcpSocket_SUCCESS(mp_socket& sock, mp_bool keepSocketInherit, mp_bool isIPV4)
{
    return MP_SUCCESS;
}
mp_int32 CreateTcpSocket_FAILED(mp_socket& sock, mp_bool keepSocketInherit, mp_bool isIPV4)
{
    return MP_FAILED;
}

mp_int32 Connect_SUCCESS(mp_socket clientSock, mp_uint32 uiServerAddr, mp_uint16 uiPort)
{
    return MP_SUCCESS;
}
mp_int32 Connect_FAILED(mp_socket clientSock, mp_uint32 uiServerAddr, mp_uint16 uiPort)
{
    return MP_FAILED;
}

mp_string GetClientIpAddrStr_stub()
{
    return std::string("11.11.11.11");
}

mp_void Close_stub(mp_socket sock){}

mp_int32 SslConnect_FAILED(const mp_socket &sock)
{
    return MP_FALSE;
}
mp_int32 SslConnect_SUCCESS(const mp_socket &sock)
{
    return MP_SUCCESS;
}
mp_void DisConnect_stub(){}
mp_int32 ReinitMsgBody_SUCCESS()
{
    return MP_SUCCESS;
}
mp_int32 ReinitMsgBody_FAILED()
{
    return MP_FAILED;
}
mp_int32 SendMsg_SUCCESS(CDppMessage &message, CConnection &conn)
{
    return MP_SUCCESS;
}
mp_uint32 GetManageCmd_stub2()
{
    return 1;
}
mp_int32 RecvMsg_SUCCESS(CConnection &connection)
{
    return MP_SUCCESS;
}
mp_int32 RecvMsg_FAILED(CConnection &connection)
{
    return MP_FAILED;
}

mp_int32 RecvMsg_EAGAIN(CConnection &connection)
{
    return MP_EAGAIN;
}

mp_uint16 StubGetCmd()
{
    return 1;
}

mp_uint32 StubGetSize2()
{
    return 1;
}

mp_uint64 StubGetOrgSeqNo()
{
    return 1;
}

mp_int32 Connect_SUCCESS2()
{
    return MP_SUCCESS;
}

mp_int32 StartRecvMsg_FAILED()
{
    return MP_FAILED;
}

mp_int32 StartRecvMsg_SUCCESS()
{
    return MP_SUCCESS;
}

mp_int32 WaitEvents_Fail1(fd_set& fdRead, fd_set& fdWrite, mp_int32 iMaxFd)
{
    return -1;
}

mp_int32 WaitEvents_Fail2(fd_set& fdRead, fd_set& fdWrite, mp_int32 iMaxFd)
{
    return 0;
}

mp_int32 WaitEvents_Sucess(fd_set& fdRead, fd_set& fdWrite, mp_int32 iMaxFd)
{
    return 1;
}


#endif