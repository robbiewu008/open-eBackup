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
#include "alarm/AlarmTrapTest.h"
#include "alarm/AlarmHandle.h"
#include "alarm/alarmdb.h"
#include "alarm/Trap.h"
#include "common/Log.h"
#include "common/DB.h"
#include "common/ConfigXmlParse.h"
#include "common/AlarmInfoXmlParser.h"
#include "message/curlclient/CurlHttpClient.h"
#include <cstdlib>
#include <vector>

using namespace std;
namespace {
mp_void LogTest(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...) {}
#define DoGetJsonStringTest()                                                                                          \
    do {                                                                                                               \
        m_stub.set(ADDR(CLogger, Log), LogTest);                                                                       \
    } while (0)

mp_int32 StubSUCCESS(mp_void* pThis)
{
    return MP_SUCCESS;
}

mp_void StubVoid(mp_void* pThis)
{
    return;
}

static mp_int32 StubGetSN(mp_int32& iAlarmSn)
{
    iAlarmSn = 1001;
    return MP_SUCCESS;
}

static mp_int32 StubGetSNFailed(mp_int32& iAlarmSn)
{
    return MP_FAILED;
}

mp_int32 StubFAILED(mp_void* pThis)
{
    return MP_FAILED;
}

mp_string StuGetLocalNodeCode(mp_void* pThis)
{
    return "StuGetLocalNodeCode";
}

mp_bool StuGetPduSecurInfo(pdu_security_info &stPduSecurInfo)
{
    stPduSecurInfo.iSecurityLevel = 0;
    stPduSecurInfo.strContextName = "strContextName";
    stPduSecurInfo.strContextEngineID = "000000";
    return MP_TRUE;
}

mp_int32 StuReadFileFailed()
{
    return MP_FAILED;
}

mp_int32 StuQueryTable1(mp_string strSql, DbParamStream& dpl, DBReader& readBuff, mp_int32& iRowCount, mp_int32& iColCount)
{
    iRowCount = 1;
    return MP_SUCCESS;
}

mp_int32 StuQueryTable0(mp_string strSql, DbParamStream& dpl, DBReader& readBuff, mp_int32& iRowCount, mp_int32& iColCount)
{
    iRowCount = 0;
    return MP_SUCCESS;
}

mp_bool StuFALSE()
{
    return false;
}

mp_bool StuTRUE()
{
    return true;
}

mp_string StubGetTimeString(mp_time& pTime)
{
    return "";
}

mp_int32 StubGetCurrentAlarmInfoByAlarmID(const mp_string& strAlarmID, alarm_Info_t& stAlarmInfo)
{
    stAlarmInfo.iAlarmSN = 1001;
    return MP_SUCCESS;
}

static mp_int32 StubGetAllTrapInfo(vector<trap_server>& vecStServerInfo)
{
    trap_server serverInfo;
    serverInfo.iPort = 56320;
    serverInfo.iVersion = 3;
    serverInfo.strServerIP = "172.168.1.1";
    vecStServerInfo.push_back(serverInfo);
    return MP_SUCCESS;
}

mp_void StubGetSnmpV3Param(mp_void* pThis, snmp_v3_param& stSnmpV3Param)
{
    stSnmpV3Param.strPrivPassword = "strPrivPassword";
    stSnmpV3Param.strAuthPassword = "strAuthPassword";
    stSnmpV3Param.strSecurityName = "strSecurityName";
    stSnmpV3Param.iSecurityModel = SECURITY_MODEL_USM;
    stSnmpV3Param.iAuthProtocol = AUTH_PROTOCOL_SHA2;
    stSnmpV3Param.iPrivProtocol = PRIVATE_PROTOCOL_AES128;
}

mp_int32 StubGetValueString(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    if (strKey == CFG_ADMINNODE_IP) {
        strValue = "192.168.1.1";
    } else if (strKey == CFG_IAM_PORT) {
        strValue = "30086";
    } else if (strKey == CFG_DOMAIN_NAME_VALUE) {
        strValue = "CDMWebServer";
    } else if (strKey == CFG_CONTEXT_NAME) {
        strValue = "context name";
    } else if (strKey == CFG_ENGINE_ID) {
        strValue = "context engine id";
    }
    return MP_SUCCESS;
}

mp_int32 StubGetValueInt32(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    if (strKey == CFG_SECURE_CHANNEL) {
        iValue = 1;
    } else if (strKey == CFG_SECURITY_LEVEL) {
        iValue = CFG_DEFAULT_SECURITY_LEVEL;
    }
    return MP_SUCCESS;
}

IHttpResponse* StubSendRequest(mp_void* pthis, const HttpRequest& req, const mp_uint32 time_out)
{
    CurlHttpResponse* pRsp = new (std::nothrow) CurlHttpResponse();
    pRsp->m_ErrorCode = CURLE_OK;
    pRsp->m_StatusCode = SC_OK;
    return pRsp;
}

std::vector<mp_string> StubSendTrap(mp_void* pthis, Pdu &pdu, std::vector<trap_server> &vecServerInfo)
{
    vector<mp_string> vecIp;
    vecIp.push_back("172.168.1.1");
    return vecIp;
}

mp_string StubGetAlarmIngo(mp_void* pthis, const mp_string& strAlarmID)
{
    return "473921ac-6b2f-4e01-b01d-37ea1cadba7f";
}

mp_int32 StubReadFile(const mp_string& strFilePath, vector<mp_string>& vecOutput)
{
    vecOutput.push_back("473921ac-6b2f-4e01-b01d-37ea1cadba7f");
    return MP_SUCCESS;
}

}


TEST_F(CAlarmTrapTest, AlarmHandle_Alarm)
{
    DoGetJsonStringTest();
    m_stub.set(ADDR(AlarmDB, GetAlarmInfoByParam), StubSUCCESS);
    m_stub.set(ADDR(AlarmDB, GetSN), StubGetSN);
    m_stub.set(ADDR(AlarmDB, InsertAlarmInfo), StubSUCCESS);
    m_stub.set(ADDR(AlarmDB, SetSN), StubSUCCESS);
    m_stub.set(ADDR(AlarmHandle, SendAlarm_Http), StubSUCCESS);
    alarm_param_t alarmParam;
    alarmParam.iAlarmID = "0x64032C0003";
    alarmParam.strAlarmParam = "Alarm Generation Test";
    AlarmHandle handle;
    EXPECT_EQ(MP_SUCCESS, handle.Alarm(alarmParam));
}

TEST_F(CAlarmTrapTest, AlarmHandle_Event)
{
    DoGetJsonStringTest();
    m_stub.set(ADDR(AlarmHandle, SendAlarm_Http), StubSUCCESS);
    alarm_param_t alarmParam;
    alarmParam.iAlarmID = "0x64032C0003";
    alarmParam.strAlarmParam = "Alarm Generation Test";
    AlarmHandle handle;
    EXPECT_EQ(MP_SUCCESS, handle.Event(alarmParam));
}

TEST_F(CAlarmTrapTest, AlarmHandle_ClearAlarm)
{
    DoGetJsonStringTest();
    m_stub.set(ADDR(AlarmDB, GetCurrentAlarmInfoByAlarmID), StubGetCurrentAlarmInfoByAlarmID);
    m_stub.set(ADDR(AlarmDB, DeleteAlarmInfo), StubSUCCESS);
    m_stub.set(ADDR(AlarmHandle, SendAlarm_Http), StubSUCCESS);
    alarm_param_t alarmParam;
    alarmParam.iAlarmID = "0x64032C0003";
    alarmParam.strAlarmParam = "Alarm Generation Test";
    AlarmHandle handle;
    EXPECT_EQ(MP_SUCCESS, handle.ClearAlarm(alarmParam));
}

TEST_F(CAlarmTrapTest, AlarmHandle_SendAlarm_Http)
{
    DoGetJsonStringTest();
    m_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubGetValueString);
    m_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubGetValueInt32);
    typedef IHttpResponse* (*fptr)(CurlHttpClient*, const HttpRequest&, const mp_uint32);
    m_stub.set((fptr)(&CurlHttpClient::SendRequest), StubSendRequest);
    alarm_Info_t alarmInfo;
    alarmInfo.iAlarmSN = 1001;
    alarmInfo.iAlarmID = "0x64032C0003";
    alarmInfo.strAlarmParam = "Alarm Generation Test";
    AlarmHandle handle;
    EXPECT_EQ(MP_SUCCESS, handle.SendAlarm_Http(alarmInfo));
}


TEST_F(CAlarmTrapTest, CTrapSender_SendAlarm)
{
    DoGetJsonStringTest();
    m_stub.set(ADDR(AlarmDB, GetAlarmInfoByParam), StubSUCCESS);
    m_stub.set(ADDR(CTrapSender, NewAlarmRecord), StubGetSNFailed);
    alarm_param_t alarmParam;
    alarmParam.iAlarmID = "0x64032C0003";
    alarmParam.strAlarmParam = "Alarm Generation Test";
    A8000Sender sender;

    m_stub.set(ADDR(CTrapSender, NewAlarmRecord), StubGetSN);
    m_stub.set(ADDR(AlarmDB, GetAllTrapInfo), StubGetAllTrapInfo);
    m_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubGetValueString);
    m_stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubGetValueInt32);
    m_stub.set(ADDR(AlarmInfoXmlParser, GetRectification), StubGetAlarmIngo);
    m_stub.set(ADDR(AlarmInfoXmlParser, GetFaultTitle), StubGetAlarmIngo);
    m_stub.set(ADDR(AlarmInfoXmlParser, GetFaultType), StubGetAlarmIngo);
    m_stub.set(ADDR(AlarmInfoXmlParser, GetFaultLevel), StubGetAlarmIngo);
    m_stub.set(ADDR(AlarmInfoXmlParser, GetAdditionInfo), StubGetAlarmIngo);
    m_stub.set(ADDR(AlarmInfoXmlParser, GetFaultCategory), StubGetAlarmIngo);
    m_stub.set(ADDR(CTrapSender, SendTrap), StubSendTrap);
    m_stub.set(ADDR(AlarmDB, UpdateAlarmInfo), StubSUCCESS);
}

TEST_F(CAlarmTrapTest, CTrapSender_ResumeAlarm)
{
    DoGetJsonStringTest();
    m_stub.set(ADDR(AlarmDB, GetCurrentAlarmInfoByAlarmID), StubGetCurrentAlarmInfoByAlarmID);
    m_stub.set(ADDR(AlarmDB, DeleteAlarmInfo), StubSUCCESS);

    alarm_param_t alarmParam;
    alarmParam.iAlarmID = "0x64032C0003";
    alarmParam.strAlarmParam = "Alarm Generation Test";
    A8000Sender sender;
    EXPECT_EQ(MP_SUCCESS, sender.ResumeAlarm(alarmParam));
}


TEST_F(CAlarmTrapTest, CTrapSender_GetLocalNodeCode)
{
    DoGetJsonStringTest();
    m_stub.set(ADDR(CMpFile, ReadFile), StubReadFile);

    A8000Sender sender;
    EXPECT_EQ("473921ac6b2f4e01b01d37ea1cadba7f", sender.GetLocalNodeCode());

    m_stub.set(ADDR(CMpFile, ReadFile), StuReadFileFailed);
    mp_string sRet = sender.GetLocalNodeCode();
    EXPECT_EQ("", sRet);
}

TEST_F(CAlarmTrapTest, RDSender_ConstructPDUCommon)
{
    DoGetJsonStringTest();
    mp_int32 count = 0;
    RDSender sender;
    Pdu pdu;
    m_stub.set(ADDR(CTrapSender, GetLocalNodeCode), StuGetLocalNodeCode);
    m_stub.set(ADDR(CMpTime, Now), StubVoid);
    m_stub.set(ADDR(CMpTime, GetTimeString), StubGetTimeString);
    sender.ConstructPDUCommon(pdu);
    EXPECT_EQ(0, count);
    m_stub.reset(ADDR(CMpTime, Now));
    m_stub.reset(ADDR(CMpTime, GetTimeString));
    m_stub.reset(ADDR(CTrapSender, GetLocalNodeCode));
}

TEST_F(CAlarmTrapTest, RDSender_ConstructPDU)
{
    DoGetJsonStringTest();
    RDSender sender;
    alarm_Info_t stAlarm;
    stAlarm.iAlarmID = "0";
    stAlarm.iAlarmSN = 0;
    stAlarm.strAlarmParam = "0";
    stAlarm.iAlarmCategoryType = 0;
    Pdu pdu;

    EXPECT_EQ(MP_TRUE, sender.ConstructPDU(stAlarm, pdu));
}

TEST_F(CAlarmTrapTest, CTrapSender_ConstructPDUCommon)
{
    DoGetJsonStringTest();
    mp_int32 count = 0;
    Pdu pdu;
    CTrapSender sender;
    sender.ConstructPDUCommon(pdu);
    EXPECT_EQ(0, count);
}

TEST_F(CAlarmTrapTest, CTrapSender_ConstructPDU2)
{
    DoGetJsonStringTest();
    Pdu pdu;
    alarm_Info_t stAlarm;
    CTrapSender sender;
    bool bRet = sender.ConstructPDU(stAlarm, pdu);
    EXPECT_EQ(MP_FALSE, bRet);
}



TEST_F(CAlarmTrapTest, CTrapSender_NewAlarmRecord)
{
    DoGetJsonStringTest();
    alarm_param_t alarmParam;
    alarm_Info_t alarmInfo;
    alarmParam.iAlarmID = "0x64032C0003";
    alarmParam.strAlarmParam = "Alarm NewAlarmRecord Test";
    CTrapSender sender;

    m_stub.set(ADDR(AlarmDB, GetSN), StubGetSNFailed);
    mp_int32 iRet = sender.NewAlarmRecord(alarmParam, alarmInfo);
    EXPECT_EQ(MP_FAILED, iRet);

    m_stub.set(ADDR(AlarmDB, GetSN), StubGetSN);
    m_stub.set(ADDR(AlarmDB, InsertAlarmInfo), StubFAILED);
    iRet = sender.NewAlarmRecord(alarmParam, alarmInfo);
    EXPECT_EQ(MP_FAILED, iRet);

    m_stub.set(ADDR(AlarmDB, GetSN), StubGetSN);
    m_stub.set(ADDR(AlarmDB, InsertAlarmInfo), StubSUCCESS);
    m_stub.set(ADDR(AlarmDB, SetSN), StubFAILED);
    iRet = sender.NewAlarmRecord(alarmParam, alarmInfo);
    EXPECT_EQ(MP_FAILED, iRet);

    m_stub.set(ADDR(AlarmDB, GetSN), StubGetSN);
    m_stub.set(ADDR(AlarmDB, InsertAlarmInfo), StubSUCCESS);
    m_stub.set(ADDR(AlarmDB, SetSN), StubFAILED);
    iRet = sender.NewAlarmRecord(alarmParam, alarmInfo);
    EXPECT_EQ(MP_FAILED, iRet);
}

TEST_F(CAlarmTrapTest, AlarmDB_GetSN)
{
    DoGetJsonStringTest();
    AlarmDB alarmDB;
    mp_int32 iAlarmSn;
    m_stub.set(ADDR(CDB, QueryTable), StubFAILED);
    mp_int32 iRet = alarmDB.GetSN(iAlarmSn);
    EXPECT_EQ(MP_FAILED, iRet);

    m_stub.set(ADDR(CDB, QueryTable), StubSUCCESS);
    iRet = alarmDB.GetSN(iAlarmSn);
    m_stub.set(ADDR(CDB, ExecSql), StubFAILED);
    iRet = alarmDB.GetSN(iAlarmSn);
    EXPECT_EQ(MP_FAILED, iRet);
}

TEST_F(CAlarmTrapTest, AlarmDB_SetSN)
{
    DoGetJsonStringTest();
    AlarmDB alarmDB;
    mp_int32 iAlarmSn;
    m_stub.set(ADDR(CDB, QueryTable), StubFAILED);
    mp_int32 iRet = alarmDB.SetSN(iAlarmSn);
    EXPECT_EQ(MP_FAILED, iRet);

    m_stub.set(ADDR(CDB, QueryTable), StubSUCCESS);
    m_stub.set(ADDR(CDB, ExecSql), StubFAILED);
    iRet = alarmDB.SetSN(iAlarmSn);
    EXPECT_EQ(MP_FAILED, iRet);

}

TEST_F(CAlarmTrapTest, AlarmDB_InsertTrapServer)
{
    DoGetJsonStringTest();
    AlarmDB alarmDB;
    trap_server trap_server;
    m_stub.set(ADDR(AlarmDB, CheckTrapInfoTable), StubFAILED);
    mp_int32 iRet = alarmDB.InsertTrapServer(trap_server);
    EXPECT_EQ(MP_FAILED, iRet);

    m_stub.set(ADDR(AlarmDB, CheckTrapInfoTable), StubSUCCESS);
    m_stub.set(ADDR(AlarmDB, BeExistInTrapInfo), StuTRUE);
    iRet = alarmDB.InsertTrapServer(trap_server);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

