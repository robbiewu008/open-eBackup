#include "agent/CheckCertValidityTest.h"
#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include "common/Ip.h"
#include "common/Utils.h"
#include "alarm/alarmdb.h"
#include "message/curlclient/CurlHttpClient.h"
#include <vector>

using namespace std;
namespace {
mp_int32 flag = 0;
static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_int32 StubSuccess(mp_void* pthis)
{
    return MP_SUCCESS;
}

mp_int32 StubFailed(mp_void* pthis)
{
    return MP_FAILED;
}

mp_void StubDoSleep(mp_uint32 ms)
{
    return;
}

mp_int32 StubFailedOnTwo(mp_void* pthis)
{
    if (flag == 0) {
        flag ++;
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

mp_int32 StubFailedOnThree(mp_void* pthis)
{
    if (flag == 0 || flag == 1) {
        flag ++;
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

mp_int32 StubFailedOnFour(mp_void* pthis)
{
    if (flag == 0 || flag == 1 || flag == 2) {
        flag ++;
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

mp_int32 StubGetValueString(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    if (strSection == CFG_SYSTEM_SECTION) {
        if (strKey == CFG_DOMAIN_NAME_VALUE) {
            strValue = "domain_name";
        }
    } else if (strSection == CFG_BACKUP_SECTION) {
        if (strKey == CFG_ADMINNODE_IP) {
            strValue = "192.168.1.1";
        } else if (strKey == CFG_IAM_PORT) {
            strValue = "8091";
        } 
    }
    return MP_SUCCESS;
}

mp_int32 StubGetValueInt32(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    if (strSection == CFG_SYSTEM_SECTION) {
        if (strKey == CFG_SECURE_CHANNEL) {
            iValue = 1;
        }
    } else if (strSection == CFG_BACKUP_SECTION) {
        if (strKey == CFG_BACKUP_ROLE) {
            iValue = 1;
        } else if (strKey == CFG_CHECK_VSPHERE_CONN_TIME) {
            iValue = 1;
        }
    }
    return MP_SUCCESS;
}

IHttpResponse* StubSendRequest_Null(mp_void* pthis, const HttpRequest& req, const mp_uint32 time_out)
{
    return nullptr;
}

IHttpResponse* StubGetInstance_Null()
{
    return nullptr;
}

IHttpResponse* StubSendRequest(mp_void* pthis, const HttpRequest& req, const mp_uint32 time_out)
{
    CurlHttpResponse* pRsp = new (std::nothrow) CurlHttpResponse();
    pRsp->m_ErrorCode = CURLE_OK;
    pRsp->m_StatusCode = SC_OK;
    return pRsp;
}

IHttpResponse* StubSendRequest_Code100(mp_void* pthis, const HttpRequest& req, const mp_uint32 time_out)
{
    CurlHttpResponse* pRsp = new (std::nothrow) CurlHttpResponse();
    pRsp->m_ErrorCode = CURLE_OK;
    pRsp->m_StatusCode = SC_CONTINUE;
    return pRsp;
}
}

TEST_F(CheckCertValidityTest, Init)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    worker.m_bNeedExit = MP_TRUE;

    stub.set(ADDR(CMpThread, Create), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.Init());

    stub.reset(ADDR(CMpThread, Create));
    stub.set(ADDR(CMpThread, WaitForEnd), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, worker.Init());
}

TEST_F(CheckCertValidityTest, GetParameters)
{
    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.GetParameters());

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubSuccess);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.GetParameters());

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubSuccess);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubSuccess);
    stub.set(ADDR(CheckCertValidity, GetCertEndTime), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.GetParameters());

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubSuccess);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubSuccess);
    stub.set(ADDR(CheckCertValidity, GetCertEndTime), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, worker.GetParameters());

    flag = 0;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubFailedOnTwo);
    EXPECT_EQ(MP_FAILED, worker.GetParameters());

    flag = 0;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubSuccess);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubFailedOnTwo);
    EXPECT_EQ(MP_FAILED, worker.GetParameters());

    flag = 0;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubSuccess);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubFailedOnThree);
    EXPECT_EQ(MP_FAILED, worker.GetParameters());
    
    flag = 0;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubSuccess);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubFailedOnFour);
    EXPECT_EQ(MP_FAILED, worker.GetParameters());
}

TEST_F(CheckCertValidityTest, SendAlarm)
{
    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set(ADDR(AlarmDB, GetCurrentAlarmInfoByAlarmID), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.SendAlarm());

    worker.m_AlarmID = "1001";
    stub.set(ADDR(AlarmDB, GetCurrentAlarmInfoByAlarmID), StubSuccess);
    stub.set(ADDR(CheckCertValidity, NewAlarmRecord), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.SendAlarm());

    stub.set(ADDR(AlarmDB, GetCurrentAlarmInfoByAlarmID), StubSuccess);
    stub.set(ADDR(CheckCertValidity, NewAlarmRecord), StubSuccess);
    stub.set(ADDR(CheckCertValidity, SendMessage), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.SendAlarm());

    stub.set(ADDR(AlarmDB, GetCurrentAlarmInfoByAlarmID), StubSuccess);
    stub.set(ADDR(CheckCertValidity, NewAlarmRecord), StubSuccess);
    stub.set(ADDR(CheckCertValidity, SendMessage), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, worker.SendAlarm());
}

TEST_F(CheckCertValidityTest, SendClearAlarm)
{
    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set(ADDR(AlarmDB, GetCurrentAlarmInfoByAlarmID), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.SendClearAlarm());

    stub.set(ADDR(AlarmDB, GetCurrentAlarmInfoByAlarmID), StubSuccess);
    stub.set(ADDR(AlarmDB, DeleteAlarmInfo), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.SendClearAlarm());

    stub.set(ADDR(AlarmDB, GetCurrentAlarmInfoByAlarmID), StubSuccess);
    stub.set(ADDR(AlarmDB, DeleteAlarmInfo), StubSuccess);
    stub.set(ADDR(CheckCertValidity, SendMessage), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.SendClearAlarm());

    stub.set(ADDR(AlarmDB, GetCurrentAlarmInfoByAlarmID), StubSuccess);
    stub.set(ADDR(AlarmDB, DeleteAlarmInfo), StubSuccess);
    stub.set(ADDR(CheckCertValidity, SendMessage), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, worker.SendClearAlarm());
}

TEST_F(CheckCertValidityTest, NewAlarmRecord)
{
    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set(ADDR(AlarmDB, InsertAlarmInfo), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.NewAlarmRecord());

    stub.set(ADDR(AlarmDB, InsertAlarmInfo), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, worker.NewAlarmRecord());
}

TEST_F(CheckCertValidityTest, SendMessage)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    worker.m_PmIpVec.push_back("192.168.1.1");

    stub.set(ADDR(CheckCertValidity, GetRequestPara), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.SendMessage(MP_TRUE));

    stub.set(ADDR(CheckCertValidity, GetRequestPara), StubSuccess);
    stub.set(ADDR(CheckCertValidity, BuildMessageBody), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.SendMessage(MP_TRUE));

    stub.set(ADDR(CheckCertValidity, GetRequestPara), StubSuccess);
    stub.set(ADDR(CheckCertValidity, BuildMessageBody), StubSuccess);
    stub.set(ADDR(CheckCertValidity, InitMessage), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.SendMessage(MP_TRUE));

    stub.set(ADDR(CheckCertValidity, GetRequestPara), StubSuccess);
    stub.set(ADDR(CheckCertValidity, BuildMessageBody), StubSuccess);
    stub.set(ADDR(CheckCertValidity, InitMessage), StubSuccess);
    stub.set(ADDR(CheckCertValidity, SendRequest), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.SendMessage(MP_TRUE));

    stub.set(ADDR(CheckCertValidity, GetRequestPara), StubSuccess);
    stub.set(ADDR(CheckCertValidity, BuildMessageBody), StubSuccess);
    stub.set(ADDR(CheckCertValidity, InitMessage), StubSuccess);
    stub.set(ADDR(CheckCertValidity, SendRequest), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, worker.SendMessage(MP_TRUE));
}

TEST_F(CheckCertValidityTest, InitMessage)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_bool isAlarmMsg = MP_TRUE;
    mp_string PmIp = "192.168.1.1";
    HttpRequest req;

    worker.m_SecureChannel = 1;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, worker.InitMessage(isAlarmMsg, PmIp, req));

    worker.m_SecureChannel = 0;
    EXPECT_EQ(MP_SUCCESS, worker.InitMessage(isAlarmMsg, PmIp, req));

    worker.m_SecureChannel = 1;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.InitMessage(isAlarmMsg, PmIp, req));
}

TEST_F(CheckCertValidityTest, BuildMessageBody)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    Json::Value reqBody;
    EXPECT_EQ(MP_SUCCESS, worker.BuildMessageBody(reqBody));

    stub.set(sprintf_s, StubFailed);
    EXPECT_EQ(MP_FAILED, worker.BuildMessageBody(reqBody));

    stub.set(sprintf_s, StubFailedOnTwo);
    EXPECT_EQ(MP_FAILED, worker.BuildMessageBody(reqBody));
}

TEST_F(CheckCertValidityTest, GetRequestPara)
{
    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.GetRequestPara());

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubSuccess);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.GetRequestPara());

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubSuccess);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubSuccess);
    EXPECT_EQ(MP_FAILED, worker.GetRequestPara());

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubSuccess);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubGetValueString);
    stub.set(ADDR(CIP, GetListenIPAndPort), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.GetRequestPara());

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubSuccess);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubGetValueString);
    stub.set(ADDR(CIP, GetListenIPAndPort), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, worker.GetRequestPara());

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubSuccess);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubGetValueString);
    stub.set(ADDR(CIP, GetListenIPAndPort), StubFailedOnTwo);
    EXPECT_EQ(MP_FAILED, worker.GetRequestPara());
}

TEST_F(CheckCertValidityTest, SendRequest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(DoSleep, StubDoSleep);
    HttpRequest req;

    typedef IHttpResponse* (*fptr)(CurlHttpClient*, const HttpRequest& req, const mp_uint32 time_out);
    stub.set((fptr)ADDR(CurlHttpClient, SendRequest), StubSendRequest_Null);
    EXPECT_EQ(MP_FAILED, worker.SendRequest(req));

    stub.set((fptr)ADDR(CurlHttpClient, SendRequest), StubSendRequest);
    EXPECT_EQ(MP_SUCCESS, worker.SendRequest(req));

    stub.set((fptr)ADDR(CurlHttpClient, SendRequest), StubSendRequest_Code100);
    EXPECT_EQ(MP_FAILED, worker.SendRequest(req)); 

    stub.set((fptr)ADDR(CurlHttpClient, GetInstance), StubGetInstance_Null);
    EXPECT_EQ(MP_FAILED, worker.SendRequest(req));
}