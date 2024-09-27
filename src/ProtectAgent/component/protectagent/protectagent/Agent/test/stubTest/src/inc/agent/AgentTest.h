#ifndef __AGENTTEST_H__
#define __AGENTTEST_H__

#define private public

#include <vector>

#include "agent/AgentExitFlag.h"
#include "agent/TaskDispatchWorker.h"
#include "agent/TaskProtectWorker.h"
#include "agent/TaskVssWorker.h"
#include "agent/TaskWorker.h"
#include "agent/TaskPool.h"
#include "agent/Communication.h"
#include "agent/Authentication.h"
#include "agent/FTExceptionHandle.h"
#include "host/CheckConnectStatus.h"
#include "common/AppVersion.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/Path.h"
#include "securecom/UniqueId.h"
#include "common/MpString.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "common/CSystemExec.h"
#include "securec.h"
#include "common/Types.h"
#include "common/ErrorCode.h"
#include "common/DB.h"
#include "securecom/CryptAlg.h"
#include "message/rest/interfaces.h"
#include "fcgi/include/fcgios.h"
#include "cunitpub/publicInc.h"
#include "gtest/gtest.h"
#include "stub.h"
#include "host/host.h"
#include "common/Ip.h"

class PluginTest : public IPlugin {
public:
    mp_int32 Initialize(std::vector<mp_uint32> &cmds) {}

    // 卸载并销毁
    mp_int32 Destroy() {}

    // 设置插件运行环境
    // [in]  pszName  参数名称
    // [in]  pvVal    参数指针
    // 插件运行环境设置方法，插件运行所需要的资源、配置参数
    // 通过这个方法“Push”到插件，这个方法先于initialize()方法
    mp_void SetOption(mp_string pszName, mp_string pvVal) {}

    // 获取插件配置的方法，可以不实现，仅用于后续扩展
    // [in]  pszName  参数名称
    // [out] pvVal    参数指针
    // [in]  sz       参数Buffer长度
    // retval true  参数有效
    // retval false 不支持该参数
    mp_bool GetOption(mp_string pszName, mp_string& pvVal) {}

    // 根据输入类名创建一个对象（组件 or 具体实现）
    // [in]  pszName  对象类名
    // retval  !=NULL 成功创建的对象的指针
    // retval  NULL   创建对象失败
    mp_void* CreateObject(mp_string pszName) {}

    // 获取插件中“发布”的类信息。
    // [out]  pClasses  存放类定义的Buffer
    // [in]   sz        Buffer长度
    // return  插件内支持动态创建的类的个数。
    // 返回值为类的个数，如果入参内存不足，则返回-1，同时填充pClassses
    mp_int32 GetClasses(IPlugin::DYN_CLASS& pClasses, mp_int32 sz) {}

    // 获取加载插件的名称
    mp_string GetName() {}

    // 获取加载插件的版本
    mp_string GetVersion() {}

    // 获取加载插件的SCN
    std::size_t GetSCN() {}

    // 获取插件类型
    mp_int32 GetTypeId() const
    {
        return 0;
    }
};

class CAuthenticationTest : public testing::Test
{
public:
    Stub stub;
};


class CCommunicationTest : public testing::Test
{
public:
    Stub stub;
};

class CFTExceptionHandleTest : public testing::Test
{
public:
    Stub stub;
};

class CTaskDispatchWorkerTest : public testing::Test
{
public:
    Stub stub;
};

class CTaskPoolTest : public testing::Test
{
public:
    Stub stub;
};

class CTaskProtectWorkerTest : public testing::Test
{
};

class CTaskWorkerTest : public testing::Test
{
public:
    Stub stub;
};

mp_int32 flag = 0;

mp_int32 StubCConfigXmlParserGetValueStringLt0(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return -1;
}
mp_int32 StubCConfigXmlParserGetValueStringLt0AuthInit(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    if (CFG_USER_NAME == strKey)
    {
        return 0;
    }
    return -1;
}
mp_int32 StubCConfigXmlParserGetValueStringLt0AuthAuth(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    if (CFG_USER_NAME == strKey)
    {
        return 0;
    }
    return -1;
}
mp_int32 StubCConfigXmlParserGetValueStringEq0AuthAuth(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    if (CFG_USER_NAME == strKey)
    {
        strValue = "admin";
    }
    else if (CFG_HASH_VALUE == strKey)
    {
        strValue = "e86f78a8a3caf0b60d8e74e5942aa6d86dc150cd3c03338aef25b7d2d7e3acc7";//Admin@123
    }
    else if (CFG_PORT == strKey)
    {
        strValue = "8091";
    }
    else if (CFG_AUTH_TYPE == strKey)
    {
        strValue = "1";
    }
    else
    {}
    return 0;
}
mp_int32 StubCConfigXmlParserGetValueStringEq0CERTIFICATEAuth(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    if(CFG_AUTH_TYPE == strKey)
       {
           strValue = "2";
       }
     return 0;
}
mp_int32 StubCConfigXmlParserGetValueStringEq0CERTIFICATEAuth0(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    if(CFG_AUTH_TYPE == strKey)
       {
           strValue = "3";
       }
     return 0;
}
mp_int32 StubCConfigXmlParserGetValueStringEq0(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return 0;
}
mp_int32 StubFCGX_InitLt0(mp_void)
{
    return -1;
}
mp_int32 StubFCGX_InitEq0(mp_void)
{
    return 0;
}

mp_int32 StubCheckipandPort(mp_string& strport)
{
    return 0;
}

mp_int32 StubFCGX_OpenSocketLt0(const mp_char *path, mp_int32 backlog)
{
    return -1;
}
mp_int32 StubFCGX_OpenSocketEq0(const mp_char *path, mp_int32 backlog)
{
    return 0;
}
mp_char* StubFCGX_GetParamNULL(const mp_char *name, FCGX_ParamArray envp)
{
    return NULL;
}
mp_char* StubFCGX_GetParamOk(const mp_char *name, FCGX_ParamArray envp)
{
    if (strcmp(name, HTTPPARAM_DBUSERNAME.c_str()) == 0)
    {
        static mp_char name[10] = "";
        return name;
    }
    if (strcmp(name, HTTPPARAM_DBPASSWORD.c_str()) == 0)
    {
        static mp_char pdword[10] = "huawei";
        return pdword;
    }
    return NULL;
}
mp_int32 StubOS_CloseEq0(mp_int32 fd, mp_int32 shutdown)
{
    return 0;
}
mp_int32 StubfcntlLt0(mp_int32 fd, mp_int32 cmd)
{
    return -1;
}
mp_int32 StubfcntlEq0(mp_int32 fd, mp_int32 cmd)
{
    return 0;
}
mp_int32 StubCCommunicationInitRequestLt0(mp_void* pthis, mp_int32 handler)
{
    return -1;
}
mp_int32 StubCMpThreadCreateLt0(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
    return -1;
}
mp_int32 StubCMpThreadCreateEq0(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
    return 0;
}
mp_int32 StubCAuthenticationInitLt0(mp_void* pthis)
{
    return -1;
}
mp_int32 StubCRequestMsgParseEq0(mp_void* pthis)
{
    return 0;
}
mp_int32 StubCRequestMsgParseLt0(mp_void* pthis)
{
    return -1;
}
mp_int32 StubCResponseMsgSendEq0(mp_void* pthis)
{
    return 0;
}
mp_int32 StubCDBQueryTableLt0(mp_void* pthis, const mp_string& strSql, DbParamStream &dps, DBReader& readBuff, mp_int32& iRowCount,mp_int32& iColCount)
{
    return -1;
}
mp_int32 StubCDBQueryTableEq0(mp_void* pthis, const mp_string& strSql, DbParamStream &dps, DBReader& readBuff, mp_int32& iRowCount,mp_int32& iColCount)
{
    return 0;
}
mp_int32 StubCDBQueryTableOk(mp_void* pthis, const mp_string& strSql, DbParamStream &dps, DBReader& readBuff, mp_int32& iRowCount,mp_int32& iColCount)
{
    iRowCount = 1;
    mp_string str = "1";
    readBuff << str;
    readBuff << str;
    readBuff << str;
    readBuff << str;
    readBuff << str;
    readBuff << str;
    readBuff << str;
    readBuff << str;
    readBuff << str;
    return 0;
}
mp_int32 StubCDBExecSqlLt0(mp_void* pthis, mp_string strSql, DbParamStream &dpl)
{
    return -1;
}
mp_int32 StubCDBExecSqlEq0(mp_void* pthis, const mp_string& strSql, DbParamStream &dpl)
{
    return 0;
}
mp_uint64 StubCMpTimeGetTimeSec6H()
{
    return 3600*6;
}
mp_uint64 StubCMpTimeGetTimeSec50s()
{
    return 50;
}
mp_uint64 StubCMpTimeGetTimeSec100s()
{
    return 100;
}
mp_int32 StubCMpThreadWaitForEndEq0(thread_id_t* id, mp_void** retValue)
{
    return 0;
}
mp_int32 StubCPluginCfgParseInitLt0(mp_void* pthis, mp_string pszFileName)
{
    return -1;
}
mp_int32 StubCPluginCfgParseInitEq0(mp_void* pthis, mp_string pszFileName)
{
    return 0;
}
mp_int32 StubCPluginManagerInitializeLt0(mp_void* pthis, IPluginCallback& pCallback)
{
    return -1;
}
mp_int32 StubCTaskWorkerInitLt0(mp_void* pthis, PluginCfgParse& pPlgCfgParse, CPluginManager& pPlgMgr)
{
    return -1;
}
mp_int32 StubCTaskWorkerInitEq0(mp_void* pthis, PluginCfgParse& pPlgCfgParse, CPluginManager& pPlgMgr)
{
    return 0;
}
mp_int32 StubCTaskDispatchWorkerInitLt0(mp_void* pthis, TaskWorker*& pTaskWorkers, mp_int32 iCount)
{
    return -1;
}
mp_void StubCTaskWorkerExitNull(mp_void* pthis)
{
    return;
}
mp_bool StubCTaskWorkerNeedExitEq1(mp_void* pthis)
{
    return 1;
}
mp_int32 StubCPluginCfgParseGetPluginByServiceEq0(mp_void* pthis, const mp_string& pszServiceName, plugin_def_t& plgDef)
{
    return 0;
}

mp_int32 StubCConfigXmlParserGetValueInt32ReturnError(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_FAILED;
}
mp_int32 StubCConfigXmlParserGetValueInt32ReturnSuccess(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

mp_int32 StubReturnResultSetAgentRoleVmware(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    strValue = "2"; // Get current role: 0:Database 1:AnyBackup 2: VMware
    return MP_SUCCESS;
}

mp_int32 StubGetListenIPAndPortSuccess(mp_string& strIP, mp_string& strPort)
{
    return MP_SUCCESS;
}

mp_int32 StubGetListenIPAndPortFailed(mp_string& strIP, mp_string& strPort)
{
    return MP_FAILED;
}

mp_void StubDoSleep(mp_uint32 ms)
{
    return;
}

mp_int32 StubPushRspMsgSuccess(const message_pair_t& msgPair)
{
    return MP_SUCCESS;
}

mp_int32 StubDispatchRestMsg(CRequestMsg &requestMsg, CResponseMsg &responseMsg)
{
    return MP_SUCCESS;
}

mp_int32 StubDispatchTcpMsg(CDppMessage &requestmsg, CDppMessage &responseMsg)
{
    return MP_SUCCESS;
}

mp_int32 StubReturnSuccess(mp_void* pthis)
{
    return MP_SUCCESS;
}

mp_char** StubReturnNull()
{
    return nullptr;
}

mp_void CommunicationSendFailedMsg(Communication& pInstance, FCGX_Request& pFcgiReq, mp_int32 iHttpStatus, mp_int32 iRetCode)
{

}

mp_string StubReturnGeneralString()
{
    mp_string str = "5";
    return str;
}
mp_int32 StubAuthenticationAuthFailed(mp_string& strClientIP, mp_string& strUsr, mp_string& strPw, const mp_string& strClientCertDN)
{
    return MP_FAILED;
}

mp_int32 StubStartPluginFailed(mp_void* pthis, const mp_string& strService)
{
    return MP_FAILED;
}

mp_int32 StubStartPluginSUCCESS(mp_void* pthis, const mp_string& strService)
{
    return MP_SUCCESS;
}

mp_int32 stub_return_ret_success(mp_void* pthis)
{
    return MP_SUCCESS;
}

mp_int32 stub_return_ret_failed(mp_void* pthis)
{
    return MP_FAILED;
}

mp_int32 stub_return_ret_test(mp_void* pthis)
{
    if (flag == 0) {
        flag ++;
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

mp_bool stub_return_false(mp_void* pthis)
{
    return MP_FALSE;
}

mp_bool stub_return_true(mp_void* pthis)
{
    return MP_TRUE;
}

mp_int32 StubSuccess(mp_void* pthis)
{
    return MP_SUCCESS;
}

MONITOR_OBJ* StubGetMonitorObj()
{
    MONITOR_OBJ* pMonitorObj;
    return pMonitorObj;
}

MONITOR_OBJ* StubGetMonitorObjNull()
{
    return nullptr;
}

mp_string StubGetThirdPartyFilePath()
{
    mp_string str = "test_path";
    return str;
}

mp_string StubReturnEmptyString()
{
    return "";
}
mp_uint64 StubGetTimeSec()
{
    return 30000;
}

mp_string StubGetQueryStatusUrlEmpty()
{
    return "";
}

mp_string StubGetQueryStatusUrl()
{
    return "test";
}

mp_int32 StubGetJsonString(mp_void* pthis)
{   
    if (flag ++ < 2) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

mp_int32 StubQueryTable(mp_string strSql, DbParamStream& dpl, DBReader& readBuff, mp_int32& iRowCount, mp_int32& iColCount)
{
    iRowCount = 1;
    return MP_SUCCESS;
}
mp_int32 StubQueryTable0(mp_string strSql, DbParamStream& dpl, DBReader& readBuff, mp_int32& iRowCount, mp_int32& iColCount)
{
    iRowCount = 0;
    return MP_SUCCESS;
}

mp_int32 GetJsonStringTest(const Json::Value& jsValue, const mp_string& strKey, mp_string& strValue)
{
    printf("===================\n");
    return MP_SUCCESS;
}

MONITOR_OBJ* StubGetMonitorObjTwoIsNull()
{
    MONITOR_OBJ* pMonitorObj = NULL;
    if (flag == 0) {
        flag ++;
        pMonitorObj->uiStatus = MONITOR_STATUS_UNFREEZING;
        return pMonitorObj;
    }
    return pMonitorObj;
}
mp_void StubWaitForExitDoSleep(mp_void* pthis)
{
    FTExceptionHandle::GetInstance().m_iThreadStatus = THREAD_STATUS_EXITED;
}
mp_bool StubIsFreezeRequest(mp_void* pthis)
{
    return true;
}
mp_void stub_return_void(mp_void* pthis)
{

}

mp_int32 StubGetRequestAppType(CRequestMsg& pReqMsg)
{
    return TYPE_APP_ORACLE;
}

#endif