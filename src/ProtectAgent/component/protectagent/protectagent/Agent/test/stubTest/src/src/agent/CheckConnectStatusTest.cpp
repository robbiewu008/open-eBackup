#include "agent/CheckConnectStatusTest.h"
#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include "common/AlarmInfoXmlParser.h"
#include "common/Ip.h"
#include "common/Utils.h"
#include "message/curlclient/CurlHttpClient.h"
#include "host/host.h"
#include <cstdlib>
#include <vector>

using namespace std;
namespace {
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

mp_int32 StubGetValueString(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    if (strSection == CFG_SYSTEM_SECTION) {
        if (strKey == CFG_USER_NAME) {
            strValue = CFG_USER_NAME;
        } else if (strKey == CFG_HASH_VALUE) {
            strValue = CFG_HASH_VALUE;
        } else if (strKey == CFG_DOMAIN_NAME_VALUE) {
            strValue = CFG_DOMAIN_NAME_VALUE;
        }
    } else if (strSection == CFG_SECURITY_SECTION) {
        if (strKey == CFG_PM_CA_INFO) {
            strValue = CFG_PM_CA_INFO;
        } else if (strKey == CFG_SSL_CERT) {
            strValue = CFG_SSL_CERT;
        } else if (strKey == CFG_SSL_KEY) {
            strValue = CFG_SSL_KEY;
        } else if (strKey == CFG_CERT) {
            strValue = CFG_CERT;
        }
         else if (strKey == CFG_AGENT_CA_INFO) {
            strValue = CFG_AGENT_CA_INFO;
        }
    } else if (strSection == CFG_BACKUP_SECTION) {
        if (strKey == CFG_ADMINNODE_IP) {
            strValue = "192.168.1.1";
        } else if (strKey == CFG_IAM_PORT) {
            strValue = "8092";
        } else if (strKey == CFG_BACKUP_ROLE) {
            strValue = "2";
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

mp_int32 StubCheckConnectStatusSendRequestGetIPAndPortAndStatus(const HttpRequest& req, mp_string& responseBody)
{
    responseBody = "{\"pages\": \"1\", \"items\": [{\"ip\": \"10.10.10.11\", \"port\": \"8080\", \"status\": \"1\",\"agentip\": \"10.10.10.10\"}]}";
    return MP_SUCCESS;
}

mp_int32 StubConvertStringtoJson(mp_void* pthis, const mp_string& rawBuffer, Json::Value& jsValue)
{
    jsValue["error_code"] = "1000";
    jsValue["error_msg"] = "error message";
    return MP_SUCCESS;
}

IHttpResponse* StubSendRequest_Null(mp_void* pthis, const HttpRequest& req, const mp_uint32 time_out)
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

mp_int32 StubDoSendRequestClusterNodes(mp_void* pthis, const mp_string& requrl, std::vector<Json::Value>& vecJsonValue)
{
    Json::Value jValue;
    jValue["managementIPv4"] = "192.168.1.1:25080/#/login";
    jValue["managementIPv6"] = "[fe80::2a6e:d4ff:fe89:4506]:25080/#/login";
    jValue["nodeName"] = "nodeName";
    vecJsonValue.push_back(jValue);
    return MP_SUCCESS;
}

mp_void StubDoSleep(mp_uint32 ms)
{
    return;
}
}

TEST_F(CCheckConnectStatusTest, Handle)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    EXPECT_EQ(MP_FAILED, worker.Handle(""));

    stub.set(ADDR(CheckConnectStatus, GetPMIPandPort), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.Handle("registerHost"));

    stub.set(ADDR(CheckConnectStatus, GetPMIPandPort), StubSuccess);
    stub.set(ADDR(CheckConnectStatus, ReportHost), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, worker.Handle("registerHost"));

    stub.set(ADDR(CheckConnectStatus, DeleteHost), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, worker.Handle("DeleteHost"));
}

TEST_F(CCheckConnectStatusTest, ReportHost)
{
    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set(ADDR(CheckConnectStatus, InitRegisterReq), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.ReportHost());

    stub.set(ADDR(CheckConnectStatus, InitRegisterReq), StubSuccess);
    stub.set(ADDR(CheckConnectStatus, SendRequest), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.ReportHost());

    stub.set(ADDR(CheckConnectStatus, InitRegisterReq), StubSuccess);
    stub.set(ADDR(CheckConnectStatus, SendRequest), StubSuccess);
    stub.set(ADDR(CJsonUtils, ConvertStringtoJson), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.ReportHost());

    stub.set(ADDR(CheckConnectStatus, InitRegisterReq), StubSuccess);
    stub.set(ADDR(CheckConnectStatus, SendRequest), StubSuccess);
    stub.set(ADDR(CJsonUtils, ConvertStringtoJson), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, worker.ReportHost());

    stub.set(ADDR(CheckConnectStatus, InitRegisterReq), StubSuccess);
    stub.set(ADDR(CheckConnectStatus, SendRequest), StubSuccess);
    stub.set(ADDR(CJsonUtils, ConvertStringtoJson), StubConvertStringtoJson);
    EXPECT_EQ(MP_SUCCESS, worker.ReportHost());
}

TEST_F(CCheckConnectStatusTest, InitRegisterReq)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    HttpRequest req;

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.InitRegisterReq(req));

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubGetValueInt32);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubGetValueString);
    stub.set(ADDR(CheckConnectStatus, GetHostInfo), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, worker.InitRegisterReq(req));
}

TEST_F(CCheckConnectStatusTest, SendRequest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    HttpRequest req;
    mp_string responseBody;

    typedef IHttpResponse* (*fptr)(CurlHttpClient*, const HttpRequest& req, const mp_uint32 time_out);
    stub.set((fptr)ADDR(CurlHttpClient, SendRequest), StubSendRequest_Null);
    EXPECT_EQ(MP_FAILED, worker.SendRequest(req, responseBody));

    stub.set((fptr)ADDR(CurlHttpClient, SendRequest), StubSendRequest);
    EXPECT_EQ(MP_SUCCESS, worker.SendRequest(req, responseBody));
}

TEST_F(CCheckConnectStatusTest, DeleteHost)
{
    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set(ADDR(CHost, GetHostSN), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.DeleteHost());

    stub.set(ADDR(CHost, GetHostSN), StubSuccess);
    stub.set(ADDR(CheckConnectStatus, InitDeleteHostReq), StubSuccess);
    stub.set(ADDR(CheckConnectStatus, SendRequest), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.DeleteHost());

    stub.set(ADDR(CHost, GetHostSN), StubSuccess);
    stub.set(ADDR(CheckConnectStatus, InitDeleteHostReq), StubSuccess);
    stub.set(ADDR(CheckConnectStatus, SendRequest), StubSuccess);
    stub.set(ADDR(CJsonUtils, ConvertStringtoJson), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.DeleteHost());

    stub.set(ADDR(CHost, GetHostSN), StubSuccess);
    stub.set(ADDR(CheckConnectStatus, InitDeleteHostReq), StubSuccess);
    stub.set(ADDR(CheckConnectStatus, SendRequest), StubSuccess);
    stub.set(ADDR(CJsonUtils, ConvertStringtoJson), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, worker.DeleteHost());

    stub.set(ADDR(CHost, GetHostSN), StubSuccess);
    stub.set(ADDR(CheckConnectStatus, InitDeleteHostReq), StubSuccess);
    stub.set(ADDR(CheckConnectStatus, SendRequest), StubSuccess);
    stub.set(ADDR(CJsonUtils, ConvertStringtoJson), StubConvertStringtoJson);
    EXPECT_EQ(MP_SUCCESS, worker.DeleteHost());
}

TEST_F(CCheckConnectStatusTest, InitDeleteHostReq)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string hostid;
    HttpRequest req;

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.InitDeleteHostReq(hostid, req));

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubGetValueInt32);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubGetValueString);
    stub.set(ADDR(CheckConnectStatus, GetHostInfo), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.InitDeleteHostReq(hostid, req));

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubGetValueInt32);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubGetValueString);
    stub.set(ADDR(CheckConnectStatus, GetHostInfo), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, worker.InitDeleteHostReq(hostid, req));
}

TEST_F(CCheckConnectStatusTest, GetHostInfo)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    Json::Value hostMsg;

    stub.set(ADDR(CHost, GetInfo), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.GetHostInfo(hostMsg));

    stub.set(ADDR(CHost, GetInfo), StubSuccess);
    stub.set(ADDR(CIP, GetListenIPAndPort), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.GetHostInfo(hostMsg));

    stub.set(ADDR(CHost, GetInfo), StubSuccess);
    stub.set(ADDR(CIP, GetListenIPAndPort), StubSuccess);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubFailed);
    EXPECT_EQ(MP_SUCCESS, worker.GetHostInfo(hostMsg));

    stub.set(ADDR(CHost, GetInfo), StubSuccess);
    stub.set(ADDR(CIP, GetListenIPAndPort), StubSuccess);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubGetValueInt32);
    EXPECT_EQ(MP_SUCCESS, worker.GetHostInfo(hostMsg));
}

TEST_F(CCheckConnectStatusTest, Init)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    HttpRequest req;
    mp_int32 iRet;
    CheckConnectStatus work;

    /* GetListenIPAndPort Fail */
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubSuccess);
    iRet = work.Init();
    EXPECT_EQ(MP_FAILED, iRet);

    /* Get AgentRole Fail */
    stub.set(ADDR(CIP,GetListenIPAndPort), StubSuccess);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubFailed);
    iRet = work.Init();
    EXPECT_EQ(MP_FAILED, iRet);

    /* Get AgentRole Success, but AgentRole is null */
    stub.reset((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString));
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubSuccess);
    iRet = work.Init();
    EXPECT_EQ(MP_FAILED, iRet);

    /* GetListenIPAndPortFailed */
    stub.reset((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString));
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubGetValueString);
    stub.set(ADDR(CheckConnectStatus,GetPMIPandPort), StubFailed);
    iRet = work.Init();
    EXPECT_EQ(MP_FAILED, iRet);

    /* GetListenIPAndPortSuccess,Stub Create Thread Failed */
    stub.reset(ADDR(CheckConnectStatus,GetPMIPandPort));
    stub.set(ADDR(CMpThread,Create), StubFailed);
    iRet = work.Init();
    EXPECT_EQ(MP_FAILED, iRet);
}

TEST_F(CCheckConnectStatusTest, CheckConnectivity)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CheckConnectStatus work;
    mp_int32 iRet;
    mp_string param;

    stub.set(DoSleep, StubDoSleep); // 取消发送之后的定时器时长
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubGetValueString);
    iRet = work.GetPMIPandPort();
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(ADDR(CheckConnectStatus,CheckConnect),StubFailed);
    /* 打桩构造连通性消息ACK Rsp */
    stub.set(ADDR(CheckConnectStatus,SendRequest), StubCheckConnectStatusSendRequestGetIPAndPortAndStatus);

    stub.reset(ADDR(CheckConnectStatus,CheckConnect));
    iRet = work.CheckConnect();
    EXPECT_EQ(MP_SUCCESS, iRet);

    /* 打桩构造secure_channel生效 */
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubGetValueInt32);
    iRet = work.UpdatePMInfo();
    EXPECT_EQ(MP_SUCCESS, iRet);

    // ipv6
    work.m_ComList.clear();
    CheckConnectStatus::Componet comp;
    comp.destAddr = "";
    comp.destAddr6 = "";
    work.m_ComList.clear();
    work.m_ComList.push_back(comp);
    iRet = work.CheckConnect();
    EXPECT_EQ(MP_SUCCESS, iRet);
    comp.destAddr = "";
    comp.destAddr6 = "28:6e:d4:89:40:fb";
    work.m_ComList.clear();
    work.m_ComList.push_back(comp);
    iRet = work.CheckConnect();
    comp.destAddr = "";
    comp.destAddr6 = "[28:6e:d4:89:40:fb]";
    work.m_ComList.clear();
    work.m_ComList.push_back(comp);
    iRet = work.CheckConnect();
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CCheckConnectStatusTest, GetUpdateInterval)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CheckConnectStatus work;
    mp_int32 iRet;
    mp_uint32 intervaltime;

    /* 打桩构造获取intervaltime失败 */
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubFailed);
    iRet = work.GetUpdateInterval(intervaltime);
    EXPECT_EQ(MP_FAILED, iRet);

    /* 打桩构造获取intervaltime失败 */
    stub.reset((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString));
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubSuccess);
    iRet = work.GetUpdateInterval(intervaltime);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CCheckConnectStatusTest, CheckVsphereConnectivity)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_uint64 lastTime = 0;

    stub.set(ADDR(CheckConnectStatus, UpdateVsphereConnectivity), StubSuccess);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubFailed);
    worker.CheckVsphereConnectivity(lastTime);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubGetValueInt32);
    worker.CheckVsphereConnectivity(lastTime);
}

TEST_F(CCheckConnectStatusTest, GetPMIPandPort)
{
    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.GetPMIPandPort());

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubSuccess);
    EXPECT_EQ(MP_FAILED, worker.GetPMIPandPort());

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubGetValueString);
    EXPECT_EQ(MP_SUCCESS, worker.GetPMIPandPort());
}

TEST_F(CCheckConnectStatusTest, InitRequest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string reqmethod;
    mp_string requrl;
    mp_string ip;
    HttpRequest req;

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.InitRequest(reqmethod, requrl, ip, req));

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), StubGetValueInt32);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubGetValueString);
    stub.set(ADDR(CheckConnectStatus, GetHostInfo), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, worker.InitRequest(reqmethod, requrl, ip, req));
}

TEST_F(CCheckConnectStatusTest, UpdatePMInfo)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    worker.m_PMIpVec.push_back("192.168.1.1");

    stub.set(ADDR(CheckConnectStatus, BuildPMBody), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.UpdatePMInfo());

    stub.set(ADDR(CheckConnectStatus, BuildPMBody), StubSuccess);
    stub.set(ADDR(CheckConnectStatus, InitRequest), StubFailed);
    EXPECT_EQ(MP_SUCCESS, worker.UpdatePMInfo());

    stub.set(ADDR(CheckConnectStatus, BuildPMBody), StubSuccess);
    stub.set(ADDR(CheckConnectStatus, InitRequest), StubSuccess);
    stub.set(ADDR(CheckConnectStatus, SendRequest), StubFailed);
    EXPECT_EQ(MP_SUCCESS, worker.UpdatePMInfo());
    
    stub.set(ADDR(CheckConnectStatus, BuildPMBody), StubSuccess);
    stub.set(ADDR(CheckConnectStatus, InitRequest), StubSuccess);
    stub.set(ADDR(CheckConnectStatus, SendRequest), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, worker.UpdatePMInfo());
}

TEST_F(CCheckConnectStatusTest, BuildPMBody)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    Json::Value registerReq;

    CheckConnectStatus::Componet stComp1;
    stComp1.destAddr = "127.0.0.1";
    stComp1.destName = "destName";
    worker.m_ComList.push_back(stComp1);
    CheckConnectStatus::Componet stComp2;
    stComp2.destAddr6 = "[ff:ff:ff:ff]";
    worker.m_ComList.push_back(stComp2);
    EXPECT_EQ(MP_SUCCESS, worker.BuildPMBody(registerReq));
}

TEST_F(CCheckConnectStatusTest, GetvSphereJsonValue)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 page_no = 10;
    mp_string requrl;
    vector<Json::Value> vecJsonValue;

    worker.m_PMIpVec.clear();
    stub.set(ADDR(CJsonUtils, ConvertStringtoJson), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.GetvSphereJsonValue(page_no, requrl, vecJsonValue));

    stub.set(ADDR(CJsonUtils, ConvertStringtoJson), StubSuccess);
    stub.set(ADDR(CJsonUtils, GetJsonArrayJson), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.GetvSphereJsonValue(page_no, requrl, vecJsonValue));

    stub.set(ADDR(CJsonUtils, ConvertStringtoJson), StubSuccess);
    stub.set(ADDR(CJsonUtils, GetJsonArrayJson), StubSuccess);
    stub.set(ADDR(CJsonUtils, GetJsonInt32), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.GetvSphereJsonValue(page_no, requrl, vecJsonValue));

    worker.m_PMIpVec.push_back("192.168.1.1");
    stub.set(ADDR(CheckConnectStatus, InitRequest), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.GetvSphereJsonValue(page_no, requrl, vecJsonValue));

    stub.set(ADDR(CheckConnectStatus, InitRequest), StubSuccess);
    stub.set(ADDR(CheckConnectStatus, SendRequest), StubSuccess);
    stub.set(ADDR(CJsonUtils, ConvertStringtoJson), StubSuccess);
    stub.set(ADDR(CJsonUtils, GetJsonArrayJson), StubSuccess);
    stub.set(ADDR(CJsonUtils, GetJsonInt32), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, worker.GetvSphereJsonValue(page_no, requrl, vecJsonValue));
}

TEST_F(CCheckConnectStatusTest, GetvSphereIp)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CMpThread::InitLock(&worker.m_Mutex);

    EXPECT_EQ(MP_FAILED, worker.GetvSphereIp());

    Json::Value jValue;
    jValue["type"] = "vSphere";
    jValue["sub_type"] = "sub_type";
    jValue["endpoint"] = "[fe80::2a6e:d4ff:fe89:4506]:25080/#/login";
    jValue["link_status"] = 1;
    worker.m_vecValue.push_back(jValue);
    EXPECT_EQ(MP_SUCCESS, worker.GetvSphereIp());
}

TEST_F(CCheckConnectStatusTest, UpdateVsphereConnectivity)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CMpThread::InitLock(&worker.m_Mutex);
    worker.m_AgentRole = "VMBackupAgent";
    stub.set(ADDR(CheckConnectStatus, CheckVDDK), StubSuccess);

    stub.set(ADDR(CheckConnectStatus, GetvSphereJsonValue), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, worker.UpdateVsphereConnectivity());

    stub.set(ADDR(CheckConnectStatus, GetvSphereJsonValue), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.UpdateVsphereConnectivity());
}

TEST_F(CCheckConnectStatusTest, DoSendRequestClusterNodes)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string requrl;
    vector<Json::Value> vecJsonValue;

    worker.m_PMIpVec.clear();
    stub.set(ADDR(CJsonUtils, ConvertStringtoJson), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.DoSendRequestClusterNodes(requrl, vecJsonValue));

    stub.set(ADDR(CJsonUtils, ConvertStringtoJson), StubSuccess);
    stub.set(ADDR(CJsonUtils, GetArrayJson), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.DoSendRequestClusterNodes(requrl, vecJsonValue));

    worker.m_PMIpVec.push_back("192.168.1.1");
    stub.set(ADDR(CheckConnectStatus, InitRequest), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.DoSendRequestClusterNodes(requrl, vecJsonValue));

    stub.set(ADDR(CheckConnectStatus, InitRequest), StubSuccess);
    stub.set(ADDR(CheckConnectStatus, SendRequest), StubSuccess);
    stub.set(ADDR(CJsonUtils, ConvertStringtoJson), StubSuccess);
    stub.set(ADDR(CJsonUtils, GetArrayJson), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, worker.DoSendRequestClusterNodes(requrl, vecJsonValue));
}

TEST_F(CCheckConnectStatusTest, GetPmControllerIp)
{
    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set(ADDR(CheckConnectStatus, DoSendRequestClusterNodes), StubFailed);
    EXPECT_EQ(MP_FAILED, worker.GetPmControllerIp());

    stub.set(ADDR(CheckConnectStatus, DoSendRequestClusterNodes), StubDoSendRequestClusterNodes);
    EXPECT_EQ(MP_SUCCESS, worker.GetPmControllerIp());
}


TEST_F(CCheckConnectStatusTest, GetPmControllerIpFromConfig)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubSuccess);
    EXPECT_EQ(MP_FAILED, worker.GetPmControllerIpFromConfig());

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubGetValueString);
    EXPECT_EQ(MP_SUCCESS, worker.GetPmControllerIpFromConfig());
}

TEST_F(CCheckConnectStatusTest, SecurityConfiguration)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))
        ADDR(CConfigXmlParser,GetValueString), StubFailed);
    HttpRequest req;

    worker.SecurityConfiguration(req, "");
    worker.SecurityConfiguration(req, "caInfo");
    worker.SecurityConfiguration(req, "sslCert");
    worker.SecurityConfiguration(req, "sslKey");
    worker.SecurityConfiguration(req, "cert");
}

TEST_F(CCheckConnectStatusTest, SendHeatBeatRequestTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(CheckConnectStatus, InitRequest), StubSuccess);
    stub.set(ADDR(CheckConnectStatus, SendRequest), StubSuccess);
    std::vector<mp_string> connectedIps;
    connectedIps.push_back("8.40.122.2");
    Json::Value jsonBody;
    jsonBody["uuid"] = "34242333432322";
    EXPECT_EQ(MP_SUCCESS, worker.SendHeatBeatRequest(connectedIps, jsonBody));
}