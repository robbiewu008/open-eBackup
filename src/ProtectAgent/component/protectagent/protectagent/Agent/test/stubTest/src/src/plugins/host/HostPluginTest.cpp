#include "plugins/host/HostPluginTest.h"
#include "plugins/host/HostPlugin.h"
#include "plugins/host/UpgradeHandle.h"
#include "message/tcp/TCPClientHandler.h"
#include <vector>
#include "common/Utils.h"
#include "common/Ip.h"
#include "alarm/Trap.h"
using namespace std;

static mp_int32 g_iHostPCount = 0;

namespace {
mp_void LogTest()
{}
#define DoGetJsonStringTest()                                                                                          \
    do {                                                                                                               \
        stub11.set(ADDR(CLogger, Log), LogTest);                                                                       \
    } while (0)

static mp_int32 HostPluginTest_bCounter = 0;
static mp_int32 HostPluginTest_iCounter = 0;
static mp_int32 INVALID_PARAM_iCounter = 0;
static mp_int32 HostPluginTest_iCounter2 = 0;
static mp_int32 SybaseStubJsonString(const Json::Value &jsValue, const mp_string& strKey, mp_string &strValue)
{
    return MP_SUCCESS;
}

mp_int32 GetJsonInt32Test(const Json::Value &jsValue, const mp_string& strKey, mp_int32 &iValue)
{
    return MP_SUCCESS;
}

mp_int32 StubHostPRootCallerExec(vector<mp_string> &pvecResult)
{
    if (g_iHostPCount++ == 0) {
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

static mp_int32 stub_HostPluginTest_IntWithResult(
    const mp_string& fileName, const mp_string& paramValues, vector<mp_string> &vecResult)
{
    if (HostPluginTest_iCounter2++ == 0) {
        return MP_FAILED;
    } else {
        vecResult.push_back("1");
        return MP_SUCCESS;
    }
}

mp_string stub_snmp_return_cstring(void)
{
    static mp_int32 iCounter = 0;
    if (iCounter++ == 0) {
        return "";
    } else {
        return "Agent123";
    }
}

static bool stub_HostPluginTest_bool(void)
{
    if (HostPluginTest_bCounter++ == 0) {
        return false;
    } else {
        return true;
    }
}

static mp_int32 stub_HostPluginTest_Int(mp_void)
{
    if (HostPluginTest_iCounter++ == 0) {
        return MP_FAILED;
    } else {
        return MP_SUCCESS;
    }
}

mp_int32 stubpQueryThirdPartyScripts(mp_void *ptr, vector<mp_string> &vectFileList)
{
    static mp_int32 icounter = 0;
    if (icounter++ == 0) {
        return MP_FAILED;
    } else {
        vectFileList.push_back("123");
        return MP_SUCCESS;
    }
}

static mp_void reset_HostPluginTest_Counter()
{
    HostPluginTest_bCounter = 0;
    INVALID_PARAM_iCounter = 0;
    HostPluginTest_iCounter = 0;
    HostPluginTest_iCounter2 = 0;
}

mp_int32 StubSUCCESS(mp_void* pThis)
{
    return MP_SUCCESS;
}

mp_bool StubTrue(mp_void* pThis)
{
    return MP_TRUE;
}

mp_int32 CMpThread_Create_stub(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
    return MP_SUCCESS;
}

mp_int32 StubGetUpgradeStatus(mp_void* pThis, mp_string& taskType, mp_string& strUpgradeStatus)
{
    strUpgradeStatus = "1";
    return MP_SUCCESS;
}

mp_int32 StubGetAgentInfo(mp_void* pThis, agent_info_t& agentInfo)
{
    agentInfo.curVersion = "curVersion";
    agentInfo.versionTimeStamp = "versionTimeStamp";
    return MP_SUCCESS;
}

mp_string StubGetHead(mp_void* pThis, const mp_string& name)
{
    return "";
}

mp_int32 StubGetHostSNSuccess(mp_void* pThis, mp_string &strSN)
{
    strSN = "strSN_1234";
    return MP_SUCCESS;
}

mp_int32 StubGetHostSNFailed(mp_void* pThis, mp_string &strSN)
{
    strSN = "";
    return MP_FAILED;
}

mp_int32 StubQueryWwpnsSuccess(mp_void* pThis, std::vector<mp_string> &vecWwpns)
{
    vecWwpns.push_back("aaabbb000111fff");
    vecWwpns.push_back("cccddd333444eee");
    return MP_SUCCESS;
}

mp_int32 StubQueryWwpnsFailed(mp_void* pThis, std::vector<mp_string> &vecWwpns)
{
    vecWwpns.clear();
    return MP_FAILED;
}

}  // namespace

mp_string StubGetLogId()
{
    return "uuid_timestamp";
}

mp_uint32 StubGetCollectLogStatus()
{
    static mp_uint32 status = -1;
    return status++;
}

mp_uint32 StubGetCollectLogStatusSucc()
{
    static mp_uint32 status = 1;
    return status++;
}

TEST_F(CHostPluginTest, DoAction)
{
    DoGetJsonStringTest();
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    HostPlugin plugObj;

    iRet = plugObj.DoAction(req, rsp);

    CDppMessage reqMsg;
    CDppMessage rspMsg;
    iRet = plugObj.DoAction(reqMsg, rspMsg);
    // EXPECT_EQ(ERROR_COMMON_FUNC_UNIMPLEMENT, iRet);
}

TEST_F(CHostPluginTest, QueryAgentVersion)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    HostPlugin plugObj;

    stub.set(ADDR(CHost, GetAgentVersion), stub_return_ret);

    iRet = plugObj.QueryAgentVersion(req, rsp);

    iRet = plugObj.QueryAgentVersion(req, rsp);
}

TEST_F(CHostPluginTest, UpdateLinksInfo)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    HostPlugin plugObj;

    iRet = plugObj.UpdateLinksInfo(req, rsp);

    iRet = plugObj.UpdateLinksInfo(req, rsp);
}

TEST_F(CHostPluginTest, InitTest)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    HostPlugin plugObj;
    vector<mp_uint32> cmds;
    plugObj.Init(cmds);
}

TEST_F(CHostPluginTest, QueryHostInfo)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    HostPlugin plugObj;

    stub.set(ADDR(CHost, GetInfo), stub_return_ret);

    reset_cunit_counter();
    iRet = plugObj.QueryHostInfo(req, rsp);
    iRet = plugObj.QueryHostInfo(req, rsp);
}

TEST_F(CHostPluginTest, QueryAgentInfo)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    HostPlugin plugObj;

    stub.set(ADDR(CHost, GetAgentInfo), stub_return_ret);

    iRet = plugObj.QueryAgentInfo(req, rsp);

    iRet = plugObj.QueryAgentInfo(req, rsp);
}

#ifdef FRAME_SIGN
TEST_F(CHostPluginTest, QueryHostV1Info)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    HostPlugin plugObj;

    stub.set(ADDR(CHost, GetInfo), stub_return_ret);
    stub.set(ADDR(CHost, GetHostAgentIplist), stub_return_ret);

    reset_cunit_counter();
    iRet = plugObj.QueryHostV1Info(req, rsp);
    EXPECT_EQ(iRet, MP_FAILED);

    iRet = plugObj.QueryHostV1Info(req, rsp);
    EXPECT_EQ(iRet, MP_SUCCESS);
}
#endif

#ifdef FRAME_SIGN
TEST_F(CHostPluginTest, QueryHostV1AppPlugins)
{
    DoGetJsonStringTest();
    Stub stub;
    stub.set(ADDR(CHost, GetHostSN), stub_return_ret);
    stub.set(ADDR(CIP, GetApplications), StubSUCCESS);
    reset_cunit_counter();

    CRequestMsg req;
    CResponseMsg rsp;
    HostPlugin plugObj;

    mp_int32 iRet = iRet = plugObj.QueryHostV1AppPlugins(req, rsp);
    EXPECT_EQ(iRet, MP_FAILED);

    iRet = plugObj.QueryHostV1AppPlugins(req, rsp);
    EXPECT_EQ(iRet, MP_SUCCESS);
}
#endif

static mp_int32 stubGetDiskInfo(void *ptr, vector<host_lun_info_t> &vecLunInfo)
{
    static mp_int32 icounter = 0;
    if (icounter++ == 0) {
        return MP_FAILED;
    } else {
        vecLunInfo.push_back(host_lun_info_t());
        return MP_SUCCESS;
    }
}

static mp_int32 stubGetInitiators(void *ptr, initiator_info_t &initInfo)
{
    static mp_int32 icounter = 0;
    if (icounter++ == 0) {
        return MP_FAILED;
    } else {
        initInfo.iscsis.push_back("1123");
        initInfo.fcs.push_back("1123");
        return MP_SUCCESS;
    }
}

TEST_F(CHostPluginTest, QueryDiskInfo)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    HostPlugin plugObj;

    stub.set(ADDR(CHost, GetDiskInfo), stubGetDiskInfo);

    iRet = plugObj.QueryDiskInfo(req, rsp);

    iRet = plugObj.QueryDiskInfo(req, rsp);
}

TEST_F(CHostPluginTest, QueryTimeZone)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    HostPlugin plugObj;

    stub.set(ADDR(CHost, GetTimeZone), stub_return_ret);

    iRet = plugObj.QueryTimeZone(req, rsp);

    iRet = plugObj.QueryTimeZone(req, rsp);
}

TEST_F(CHostPluginTest, QueryInitiators)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    HostPlugin plugObj;

    stub.set(ADDR(CHost, GetInitiators), stubGetInitiators);

    iRet = plugObj.QueryInitiators(req, rsp);

    iRet = plugObj.QueryInitiators(req, rsp);
}

/*
* 用例名称：从主机查询wwpn号成功
* 前置条件：1、获取主机SN号成功，2、查询wwpn号成功
* check点：返回值为MP_SUCCESS
*/
TEST_F(CHostPluginTest, QueryWwpnsSuccess)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_FAILED;
    HostPlugin plugObj;

    stub.set(ADDR(CHost, GetHostSN), StubGetHostSNSuccess);
    stub.set(ADDR(CHost, QueryWwpns), StubQueryWwpnsSuccess);
    iRet = plugObj.QueryWwpns(req, rsp);
    EXPECT_EQ(iRet, MP_SUCCESS);
    EXPECT_EQ(rsp.m_msgJsonData["uuid"], "strSN_1234");
    EXPECT_EQ(rsp.m_msgJsonData["wwpns"][0], "aaabbb000111fff");
    EXPECT_EQ(rsp.m_msgJsonData["wwpns"][1], "cccddd333444eee");
}

/*
* 用例名称：从主机查询wwpn号失败
* 前置条件：1、获取主机SN号失败或查询wwpn号失败
* check点：返回值为MP_FAILED
*/
TEST_F(CHostPluginTest, QueryWwpnsFailed)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    HostPlugin plugObj;

    stub.set(ADDR(CHost, GetHostSN), StubGetHostSNFailed);
    iRet = plugObj.QueryWwpns(req, rsp);
    EXPECT_EQ(iRet, MP_FAILED);
    stub.set(ADDR(CHost, GetHostSN), StubGetHostSNSuccess);
    stub.set(ADDR(CHost, QueryWwpns), StubQueryWwpnsFailed);
    iRet = MP_SUCCESS;
    iRet = plugObj.QueryWwpns(req, rsp);
    EXPECT_EQ(iRet, MP_FAILED);
}

mp_int32 StubQueryWwpnsV2Success(mp_void* pThis, std::map<mp_string, mp_string> &mapWwpns)
{
    mapWwpns["aaabbb000111fff"] = "27";
    mapWwpns["cccddd333444eee"] = "28";
    return MP_SUCCESS;
}

mp_int32 StubQueryWwpnsV2Failed(mp_void* pThis, std::map<mp_string, mp_string> &mapWwpns)
{
    mapWwpns["aaabbb000111fff"] = "27";
    mapWwpns["cccddd333444eee"] = "28";
    return MP_FAILED;
}

mp_int32 StubGetHostInfoSuccess(mp_void* pThis, Json::Value& jValue)
{
    jValue["uuid"] = "strSN_1234";
    jValue["type"] = "sanclient";
    return MP_SUCCESS;
}

mp_int32 StubGetHostInfoFailed(mp_void* pThis, Json::Value& jValue)
{
    jValue["uuid"] = "strSN_1234";
    jValue["type"] = "sanclient";
    return MP_FAILED;
}

/*
* 用例名称：v2接口从主机查询wwpn号成功
* 前置条件：1、获取主机SN号成功，2、查询wwpn号成功
* check点：返回值为MP_SUCCESS
*/
TEST_F(CHostPluginTest, QueryWwpnsV2Success)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_FAILED;
    HostPlugin plugObj;

    stub.set(ADDR(CHost, GetHostInfo), StubGetHostInfoSuccess);
    stub.set(ADDR(CHost, QueryWwpnsV2), StubQueryWwpnsV2Success);
    iRet = plugObj.QueryWwpnsV2(req, rsp);
    EXPECT_EQ(iRet, MP_SUCCESS);
    EXPECT_EQ(rsp.m_msgJsonData["uuid"], "strSN_1234");
    EXPECT_EQ(rsp.m_msgJsonData["type"], "sanclient");
    EXPECT_EQ(rsp.m_msgJsonData["wwpnInfoList"][0]["configKey"], "aaabbb000111fff");
    EXPECT_EQ(rsp.m_msgJsonData["wwpnInfoList"][0]["configValue"], "27");
    EXPECT_EQ(rsp.m_msgJsonData["wwpnInfoList"][1]["configKey"], "cccddd333444eee");
    EXPECT_EQ(rsp.m_msgJsonData["wwpnInfoList"][1]["configValue"], "28");
}

/*
* 用例名称：v2接口从主机查询wwpn号失败
* 前置条件：1、获取主机SN号失败
* check点：返回值为MP_FAILED
*/
TEST_F(CHostPluginTest, QueryWwpnsV2FAILED)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_FAILED;
    HostPlugin plugObj;

    stub.set(ADDR(CHost, GetHostInfo), StubGetHostInfoFailed);
    iRet = plugObj.QueryWwpnsV2(req, rsp);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CHost, GetHostInfo), StubGetHostInfoSuccess);
    stub.set(ADDR(CHost, QueryWwpnsV2), StubQueryWwpnsV2Failed);
    iRet = plugObj.QueryWwpnsV2(req, rsp);
    EXPECT_EQ(iRet, MP_FAILED);
}

mp_int32 StubQueryIqnsSanClientSuccess(mp_void* pThis, std::map<mp_string, mp_string>& mapIqns)
{
    mapIqns["iqn.2023-07.com.example:01:b2da64a85d58"] = "27";
    mapIqns["iqn.2023-07.com.example:01:b2da64a85d60"] = "28";
    return MP_SUCCESS;
}

mp_int32 StubQueryIqnsSanClientFailed(mp_void* pThis, std::map<mp_string, mp_string>& mapIqns)
{
    mapIqns["iqn.2023-07.com.example:01:b2da64a85d58"] = "27";
    mapIqns["iqn.2023-07.com.example:01:b2da64a85d60"] = "28";
    return MP_FAILED;
}

/*
* 用例名称：从主机查询iqn号成功
* 前置条件：1、获取主机SN号成功，2、查询iqn号成功
* check点：返回值为MP_SUCCESS
*/
TEST_F(CHostPluginTest, ScanIqnSuccess)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_FAILED;
    HostPlugin plugObj;

    stub.set(ADDR(CHost, GetHostInfo), StubGetHostInfoSuccess);
    stub.set(ADDR(CHost, QueryIqns), StubQueryIqnsSanClientSuccess);
    iRet = plugObj.ScanIqns(req, rsp);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
* 用例名称：从主机查询iqn号失败
* 前置条件：1、获取主机SN号失败，2、查询iqn号失败
* check点：返回值为MP_FAILED
*/
TEST_F(CHostPluginTest, ScanIqnFailed)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_FAILED;
    HostPlugin plugObj;

    stub.set(ADDR(CHost, GetHostInfo), StubGetHostInfoFailed);
    iRet = plugObj.ScanIqns(req, rsp);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CHost, GetHostInfo), StubGetHostInfoSuccess);
    stub.set(ADDR(CHost, QueryIqns), StubQueryIqnsSanClientFailed);
    iRet = plugObj.ScanIqns(req, rsp);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
* 用例名称：从主机校验iqn号成功
* 前置条件：1、获取主机SN号成功，2、查询iqn号成功
* check点：返回值为MP_SUCCESS
*/
TEST_F(CHostPluginTest, QueryIqnsSuccess)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    Json::Value jsonReq;
    jsonReq["configValues"].append("iqn.2023-07.com.example:01:b2da64a85d58");
    jsonReq["configValues"].append("iqn.2023-07.com.example:01:b2da64a85d60");
    req.GetMsgBody().SetJsonValue(jsonReq);
    mp_int32 iRet = MP_FAILED;
    HostPlugin plugObj;

    stub.set(ADDR(CHost, GetHostInfo), StubGetHostInfoSuccess);
    stub.set(ADDR(CHost, QueryIqns), StubQueryIqnsSanClientSuccess);
    iRet = plugObj.QueryIqns(req, rsp);
    EXPECT_EQ(iRet, MP_SUCCESS);
    EXPECT_EQ(rsp.m_msgJsonData["uuid"], "strSN_1234");
    EXPECT_EQ(rsp.m_msgJsonData["type"], "sanclient");
    EXPECT_EQ(rsp.m_msgJsonData["wwpnInfoList"][0]["configKey"], "iqn.2023-07.com.example:01:b2da64a85d58");
    EXPECT_EQ(rsp.m_msgJsonData["wwpnInfoList"][0]["configValue"], "27");
    EXPECT_EQ(rsp.m_msgJsonData["wwpnInfoList"][1]["configKey"], "iqn.2023-07.com.example:01:b2da64a85d60");
    EXPECT_EQ(rsp.m_msgJsonData["wwpnInfoList"][1]["configValue"], "27");
}

/*
* 用例名称：从主机校验iqn号失败
* 前置条件：1、获取主机SN号失败，2、查询iqn号失败
* check点：返回值为MP_FAILED
*/
TEST_F(CHostPluginTest, QueryIqnsFailed)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_FAILED;
    HostPlugin plugObj;

    stub.set(ADDR(CHost, GetHostInfo), StubGetHostInfoFailed);
    iRet = plugObj.QueryIqns(req, rsp);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CHost, GetHostInfo), StubGetHostInfoSuccess);
    stub.set(ADDR(CHost, QueryIqns), StubQueryIqnsSanClientFailed);
    iRet = plugObj.QueryIqns(req, rsp);
    EXPECT_EQ(iRet, MP_FAILED);
}

/*
* 用例名称：在主机上扫盘成功或失败
* 前置条件：1、执行CHost的扫盘接口成功或失败
* check点：返回值为MP_SUCCESS或MP_FAILED
*/
TEST_F(CHostPluginTest, ScanDiskSuccess)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    HostPlugin plugObj;

    reset_cunit_counter();
    stub.set(ADDR(CHost, ScanDisk), stub_return_ret);
    iRet = plugObj.ScanDisk(req, rsp);
    EXPECT_EQ(iRet, MP_FAILED);
    iRet = MP_FAILED;
    iRet = plugObj.ScanDisk(req, rsp);
    EXPECT_EQ(iRet,MP_SUCCESS);
}

TEST_F(CHostPluginTest, QueryThirdPartyScripts)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    HostPlugin plugObj;

    stub.set(ADDR(CHost, QueryThirdPartyScripts), stubpQueryThirdPartyScripts);

    iRet = plugObj.QueryThirdPartyScripts(req, rsp);

    iRet = plugObj.QueryThirdPartyScripts(req, rsp);
}

TEST_F(CHostPluginTest, ExecThirdPartyScript)
{
    DoGetJsonStringTest();
    Stub stub;
    stub.set(ADDR(CHost, ExecThirdPartyScript), StubSUCCESS);
    CRequestMsg req;
    CResponseMsg rsp;
    Json::Value jsonReq;
    jsonReq[REST_ISUSERDEFINED_SCRIPT] = AGENT_USER_DEFINED_SCRIPT;
    jsonReq[REST_PARAM_HOST_FILENAME] = "test.sh";
    jsonReq[REST_PARAM_HOST_PARAMS] = Json::Value();
    req.GetMsgBody().SetJsonValue(jsonReq);
    HostPlugin plugObj;
    EXPECT_EQ(MP_SUCCESS, plugObj.ExecThirdPartyScript(req, rsp));
}

TEST_F(CHostPluginTest, UnRegTrapServer)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    CRequestMsgBody reqBody;
    mp_int32 iRet = MP_SUCCESS;
    HostPlugin plugObj;

    stub.set(ADDR(CJsonUtils, GetJsonInt32), GetJsonInt32Test);
    stub.set(ADDR(CHost, UnRegTrapServer), stub_return_ret);

    stub.set((bool (Json::Value::*)(const char *) const)ADDR(Json::Value, isMember), stub_return_bool_true);

    stub.set(CheckParamStringIsIP, stub_HostPluginTest_Int);

    stub.set(CheckParamInteger32, stub_HostPluginTest_Int);

    reset_cunit_counter();
    reset_HostPluginTest_Counter();

    iRet = plugObj.UnRegTrapServer(req, rsp);

    iRet = plugObj.UnRegTrapServer(req, rsp);

    iRet = plugObj.UnRegTrapServer(req, rsp);
}

TEST_F(CHostPluginTest, VerifySnmp)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    HostPlugin plugObj;

    stub.set(ADDR(CJsonUtils, GetJsonInt32), GetJsonInt32Test);
    stub.set(ADDR(CHttpRequest, GetHeadNoCheck), stub_snmp_return_cstring);

    stub.set((bool (Json::Value::*)(const char *) const)ADDR(Json::Value, isMember), stub_HostPluginTest_bool);

    stub.set(CheckParamInteger32, stub_HostPluginTest_Int);

    stub.set(ADDR(CHost, VerifySnmp), stub_return_ret);

    reset_cunit_counter();
    reset_HostPluginTest_Counter();

    iRet = plugObj.VerifySnmp(req, rsp);

    iRet = plugObj.VerifySnmp(req, rsp);

    iRet = plugObj.VerifySnmp(req, rsp);
}

TEST_F(CHostPluginTest, ExecFreezeScript)
{
    DoGetJsonStringTest();
    Stub stub;
    stub.set(ADDR(CHost, ExecThirdPartyScript), StubSUCCESS);
    stub.set(ADDR(CMpFile, FileExist), StubTrue);
    CRequestMsg req;
    CResponseMsg rsp;
    Json::Value jsonReq;
    jsonReq[REST_ISUSERDEFINED_SCRIPT] = AGENT_USER_DEFINED_SCRIPT;
    jsonReq[REST_PARAM_HOST_FREEZE_SCRIPT_FILENAME] = "freeze.sh";
    jsonReq[REST_PARAM_HOST_FREEZE_SCRIPT_PARAM] = Json::Value();
    jsonReq[REST_PARAM_HOST_UNFREEZE_SCRIPT_FILENAME] = "unfreeze.sh";
    jsonReq[REST_PARAM_HOST_QUERY_SCRIPT_FILENAME] = "query.sh";
    req.GetMsgBody().SetJsonValue(jsonReq);
    HostPlugin plugObj;
    EXPECT_EQ(MP_SUCCESS, plugObj.ExecFreezeScript(req, rsp));
}

TEST_F(CHostPluginTest, ExecThawScript)
{
    DoGetJsonStringTest();
    Stub stub;
    stub.set(ADDR(CHost, ExecThirdPartyScript), StubSUCCESS);
    CRequestMsg req;
    CResponseMsg rsp;
    Json::Value jsonReq;
    jsonReq[REST_ISUSERDEFINED_SCRIPT] = AGENT_USER_DEFINED_SCRIPT;
    jsonReq[REST_PARAM_HOST_UNFREEZE_SCRIPT_FILENAME] = "unfreeze.sh";
    jsonReq[REST_PARAM_HOST_UNFREEZE_SCRIPT_PARAM] = Json::Value();
    req.GetMsgBody().SetJsonValue(jsonReq);
    HostPlugin plugObj;
    EXPECT_EQ(MP_SUCCESS, plugObj.ExecThawScript(req, rsp));
}

TEST_F(CHostPluginTest, QueryFreezeStatusScript)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    HostPlugin plugObj;

    stub.set((mp_int32(*)(const mp_string&, mp_int32, mp_int32, const mp_string&, const mp_string&))CheckParamString, stub_return_ret);

    stub.set(CheckCmdDelimiter, stub_return_bool);
    plugObj.QueryFreezeStatusScript(req, rsp);

    stub.set((mp_int32(*)(mp_string &, const mp_string&))CheckPathString, stub_return_ret);
    plugObj.QueryFreezeStatusScript(req, rsp);

    stub.set(ADDR(CHost, ExecThirdPartyScript), stub_HostPluginTest_IntWithResult);

    reset_cunit_counter();
    iRet = plugObj.QueryFreezeStatusScript(req, rsp);

    iRet = plugObj.QueryFreezeStatusScript(req, rsp);
}

TEST_F(CHostPluginTest, UpgradeAgent)
{
    DoGetJsonStringTest();
    Stub stub;
    stub.set(ADDR(CIPCFile, WriteFile), StubSUCCESS);
    stub.set(ADDR(UpgradeHandle, UpdateUpgradeStatus), StubSUCCESS);
    stub.set(ADDR(CMpThread, Create), CMpThread_Create_stub);
    CRequestMsg req;
    CResponseMsg rsp;
    Json::Value jsonReq;
    jsonReq[REST_PARAM_AGENT_UPGRADE_DOWNLOADLINK] = "downloadLink";
    jsonReq[REST_PARAM_AGENT_UPGRADE_AGENTID] = "agentId";
    jsonReq[REST_PARAM_AGENT_UPGRADE_AGENTNAME] = "agentName";
    jsonReq[REST_PARAM_AGENT_UPGRADE_JOBID] = "jobId";
    req.GetMsgBody().SetJsonValue(jsonReq);
    HostPlugin plugObj;
    EXPECT_EQ(MP_SUCCESS, plugObj.UpgradeAgent(req, rsp));
}

TEST_F(CHostPluginTest, QueryUpgradeStatus)
{
    DoGetJsonStringTest();
    Stub stub;
    stub.set(ADDR(CHost, GetTaskStatus), StubGetUpgradeStatus);
    stub.set(ADDR(CHost, GetAgentInfo), StubGetAgentInfo);
    CRequestMsg req;
    CResponseMsg rsp;
    HostPlugin plugObj;
    EXPECT_EQ(MP_SUCCESS, plugObj.QueryUpgradeStatus(req, rsp));
}

TEST_F(CHostPluginTest, CollectAgentLog)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    HostPlugin plugObj;

    stub.set(ADDR(CHost, CollectLog), stub_return_ret);
    stub.set(ADDR(CHost, GetLogId), StubGetLogId);

    iRet = plugObj.CollectAgentLog(req, rsp);

    iRet = plugObj.CollectAgentLog(req, rsp);
}

TEST_F(CHostPluginTest, CollectAgentLogStauts)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    HostPlugin plugObj;

    stub.set(ADDR(CHost, GetCollectLogStatus), StubGetCollectLogStatus);
    EXPECT_EQ(plugObj.CollectAgentLogStauts(req, rsp), MP_FAILED);
    EXPECT_EQ(plugObj.CollectAgentLogStauts(req, rsp), MP_SUCCESS);
    EXPECT_EQ(plugObj.CollectAgentLogStauts(req, rsp), MP_SUCCESS);
    EXPECT_EQ(plugObj.CollectAgentLogStauts(req, rsp), MP_SUCCESS);
    EXPECT_EQ(plugObj.CollectAgentLogStauts(req, rsp), MP_SUCCESS);
    EXPECT_EQ(plugObj.CollectAgentLogStauts(req, rsp), MP_FAILED);
}

TEST_F(CHostPluginTest, ExportAgentLog)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    HostPlugin plugObj;

    reset_cunit_counter();
    reset_HostPluginTest_Counter();

    stub.set(ADDR(CJsonUtils, GetJsonString), SybaseStubJsonString);
    stub.set(ADDR(CHost, GetCollectLogStatus), StubGetCollectLogStatusSucc);

    stub.set((bool (Json::Value::*)(const char *) const)ADDR(Json::Value, isMember), stub_return_bool);

    stub.set((mp_int32(*)(const mp_string&, mp_int32, mp_int32, const mp_string&, const mp_string&))CheckParamString, stub_return_ret);

    stub.set(CheckParamStringEnd, stub_return_ret);

    stub.set((mp_int32(*)(mp_string &, const mp_string&))CheckPathString, stub_return_ret);
    stub.set(ADDR(HostPlugin, CheckExportLogParams), stub_return_success);

    EXPECT_EQ(plugObj.ExportAgentLog(req, rsp), MP_SUCCESS);
    EXPECT_EQ(plugObj.ExportAgentLog(req, rsp), MP_SUCCESS);
    EXPECT_EQ(plugObj.ExportAgentLog(req, rsp), MP_SUCCESS);
}

TEST_F(CHostPluginTest, UpdateAgentLogLevel)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    mp_int32 iRet = MP_SUCCESS;
    HostPlugin plugObj;

    Json::Value jv;
    jv["level"] = 0;
    req.SetJsonData(jv);
    stub.set(ADDR(CHost, SetLogLevel), stub_return_ret);

    EXPECT_EQ(plugObj.UpdateAgentLogLevel(req, rsp), MP_FAILED);
    EXPECT_EQ(plugObj.UpdateAgentLogLevel(req, rsp), MP_SUCCESS);
}

TEST_F(CHostPluginTest, GetLogName)
{
    DoGetJsonStringTest();
    Stub stub;
    CRequestMsg req;
    CResponseMsg rsp;
    HostPlugin plugObj;

    plugObj.GetLogName();
}

TEST_F(CHostPluginTest, QueryFusionStorageIP)
{
    DoGetJsonStringTest();
    Stub stub;
    HostPlugin plugObj;
    CRequestMsg req;
    CResponseMsg rsp;

    stub.set(ADDR(CHost, QueryFusionStorageIP), StubHostPRootCallerExec);
    mp_int32 rst = plugObj.QueryFusionStorageIP(req, rsp);
    rst = plugObj.QueryFusionStorageIP(req, rsp);
}

TEST_F(CHostPluginTest, ScanDiskByDppTest)
{
    DoGetJsonStringTest();
    Stub stub;
    HostPlugin plugObj;
    CDppMessage reqMsg;
    CDppMessage rspMsg;

    stub.set(ADDR(CJsonUtils, GetJsonString), SybaseStubJsonString);
    stub.set(ADDR(CHost, ScanDisk), stub_return_ret);
    mp_int32 rst = plugObj.ScanDiskByDpp(reqMsg, rspMsg);
}

TEST_F(CHostPluginTest, GetHostIpsTest)
{
    DoGetJsonStringTest();
    Stub stub;
    HostPlugin plugObj;
    CDppMessage reqMsg;
    CDppMessage rspMsg;

    stub.set(ADDR(CJsonUtils, GetJsonString), SybaseStubJsonString);
    mp_int32 rst = plugObj.GetHostIps(reqMsg, rspMsg);
}

TEST_F(CHostPluginTest, UpdateTrapServer)
{
    DoGetJsonStringTest();
    Stub stub;
    HostPlugin plugObj;
    CRequestMsg req;
    CResponseMsg rsp;
    FCGX_Request pFcgxReq;
    req.GetHttpReq().m_pFcgRequest = &pFcgxReq;
    stub.set(ADDR(CJsonUtils, GetJsonString), SybaseStubJsonString);
    stub.set(ADDR(HostPlugin, GenerateTrapInfo), StubSUCCESS);
    stub.set(ADDR(CTrapSender, SendAlarm), StubSUCCESS);
    stub.set(ADDR(CTrapSender, ResumeAlarm), StubSUCCESS);
    EXPECT_EQ(MP_SUCCESS, plugObj.UpdateTrapServer(req, rsp));
}

TEST_F(CHostPluginTest, GenerateTrapInfo)
{
    DoGetJsonStringTest();
    Stub stub;
    stub.set(ADDR(CHttpRequest, GetHead), StubGetHead);
    HostPlugin plugObj;
    CRequestMsg req;
    Json::Value jsonReq;
    Json::Value confJson;
    confJson["version"] = "V3";
    confJson["contextEngineId"] = "contextEngineId";
    confJson["contextName"] = "contextName";
    confJson["securityName"] = "securityName";
    confJson["authPwd"] = "authPwd";
    confJson["encryptPwd"] = "encryptPwd";
    confJson["authProtocol"] = "HMAC_SHA2";
    confJson["encryptProtocol"] = "AES";
    jsonReq["trap_config"] = confJson;
    Json::Value addrJson;
    addrJson["trapIp"] = "192.168.1.1";
    addrJson["port"] = 56630;
    jsonReq["trap_addresses"].append(addrJson);
    req.GetMsgBody().SetJsonValue(jsonReq);

    std::vector<trap_server> vecTrapServer;
    snmp_v3_param stParam;
    EXPECT_EQ(MP_SUCCESS, plugObj.GenerateTrapInfo(req, vecTrapServer, stParam));
    EXPECT_EQ("192.168.1.1", vecTrapServer.front().strServerIP);
    EXPECT_EQ("contextEngineId", stParam.strContextEngineId);
}

TEST_F(CHostPluginTest, RegTrapServerTest)
{
    DoGetJsonStringTest();
    Stub stub;
    stub.set(ADDR(CHttpRequest, GetHead), StubGetHead);
    stub.set(ADDR(CHost, RegTrapServer), StubSUCCESS);
    CRequestMsg req;
    CResponseMsg rsp;
    Json::Value jsonReq;
    jsonReq[REST_PARAM_HOST_IP] = "192.168.1.1";
    jsonReq[REST_PARAM_HOST_PORT] = 56630;
    jsonReq[REST_PARAM_HOST_SNMPTYPE] = 3;
    req.GetMsgBody().SetJsonValue(jsonReq);

    HostPlugin plugObj;
    EXPECT_EQ(MP_SUCCESS, plugObj.RegTrapServer(req, rsp));
}

TEST_F(CHostPluginTest, UnRegTrapServerTest)
{
    DoGetJsonStringTest();
    Stub stub;
    stub.set(ADDR(CHost, UnRegTrapServer), StubSUCCESS);
    CRequestMsg req;
    CResponseMsg rsp;
    Json::Value jsonReq;
    jsonReq[REST_PARAM_HOST_IP] = "192.168.1.1";
    jsonReq[REST_PARAM_HOST_PORT] = 56630;
    jsonReq[REST_PARAM_HOST_SNMPTYPE] = 3;
    req.GetMsgBody().SetJsonValue(jsonReq);

    HostPlugin plugObj;
    EXPECT_EQ(MP_SUCCESS, plugObj.UnRegTrapServer(req, rsp));
}

TEST_F(CHostPluginTest, ConnectDME)
{
    DoGetJsonStringTest();
    Stub stub;
    stub.set(ADDR(TCPClientHandler, Connect), StubSUCCESS);
    CRequestMsg req;
    CResponseMsg rsp;
    Json::Value jsonReq;
    jsonReq[SNMP_CONNECT_DME_TYPE] = MESSAGE_ROLE::ROLE_HOST_AGENT;
    jsonReq[SNMP_CONNECT_DME_IP] = "192.168.1.1";
    jsonReq[SNMP_CONNECT_DME_PORT] = 56630;
    req.GetMsgBody().SetJsonValue(jsonReq);

    HostPlugin plugObj;
    EXPECT_EQ(MP_SUCCESS, plugObj.ConnectDME(req, rsp));
}
/*
TEST_F(CHostPluginTest, AddControllerConfig)
{
    DoGetJsonStringTest();
    Stub stub;
    stub.set(ADDR(CSystemExec, ExecSystemWithoutEchoNoWin), StubSUCCESS);
    CRequestMsg req;
    CResponseMsg rsp;
    Json::Value jsonReq;
    jsonReq[CONTROLLER_LISTS] = "192.168.1.1,192.168.1.2";
    req.GetMsgBody().SetJsonValue(jsonReq);

    HostPlugin plugObj;
    EXPECT_EQ(MP_SUCCESS, plugObj.AddControllerInfo(req, rsp));
}
*/

TEST_F(CHostPluginTest, GetInitiatorsTest)
{
    DoGetJsonStringTest();
    Stub stub;
    HostPlugin plugObj;
    CDppMessage reqMsg;
    CDppMessage rspMsg;
    stub.set(ADDR(CHost, GetInitiators), StubSUCCESS);
    EXPECT_EQ(MP_SUCCESS, plugObj.GetInitiators(reqMsg, rspMsg));

    Json::Value jReqValue;
    rspMsg.GetManageBody(jReqValue);
    EXPECT_EQ(0, jReqValue[MANAGECMD_KEY_ERRORCODE].asInt());
}

mp_bool GetHostFlag = MP_FALSE;
mp_int32 QueryHostInfoStub(CRequestMsg &req, CResponseMsg &rsp)
{
    GetHostFlag = MP_TRUE;
    return MP_SUCCESS;
}

/*
* 用例名称：根据不带中间参数的URL可以匹配到函数并执行
* 前置条件：无
* check点：配置URL的函数可以正常被调用
*/
TEST_F(CHostPluginTest, DoActionSuccess)
{
    GetHostFlag = MP_FALSE;
    DoGetJsonStringTest();
    stub11.set(&HostPlugin::QueryHostInfo, QueryHostInfoStub);

    CRequestMsg req;
    CResponseMsg rsp;
    HostPlugin plugObj;

    req.m_url.m_procURL = REST_HOST_QUERY_INFO;
    req.m_httpReq.m_strMethod = REST_URL_METHOD_GET;

    mp_int32 iRet = plugObj.DoAction(req, rsp);
    EXPECT_EQ(MP_SUCCESS, iRet);
    EXPECT_EQ(GetHostFlag, MP_TRUE);
}