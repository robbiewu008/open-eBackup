#include "agent/AgentTest.h"
#include <vector>
#include "pluginfx/ExternalPluginManager.h"
#include "plugins/host/HostPlugin.h"
#include "common/Utils.h"
using namespace std;

#define StubClogToVoidLogNullPointReference() do { \
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32ReturnSuccess); \
} while (0)

IPlugin* StubCPluginManagerGetPluginOK(mp_void* pthis, const mp_string& pszPlg)
{
    return (new HostPlugin);
}

static mp_void StubCLoggerLog(mp_void){
    return;
}
//Begin CAuthenticationTest
TEST_F(CAuthenticationTest, Init)
{
    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), stub_return_ret_success);
    EXPECT_EQ(MP_SUCCESS, security::Authentication::GetInstance().Init());

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), stub_return_ret_failed);
    EXPECT_EQ(MP_FAILED, security::Authentication::GetInstance().Init());

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), stub_return_ret_test);
    EXPECT_EQ(MP_FAILED, security::Authentication::GetInstance().Init());
}

TEST_F(CAuthenticationTest, Check)
{
    mp_string strUsr;
    mp_string strPwd;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), stub_return_ret_failed);
    EXPECT_EQ(MP_FALSE, security::Authentication::GetInstance().Check(strUsr, strPwd));

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), stub_return_ret_success);
    stub.set(GetSha256Hash, stub_return_ret_failed);
    EXPECT_EQ(MP_FALSE, security::Authentication::GetInstance().Check(strUsr, strPwd));

    stub.set(GetSha256Hash, stub_return_ret_success);
    stub.set(PBKDF2Hash, stub_return_ret_failed);
    EXPECT_EQ(MP_FALSE, security::Authentication::GetInstance().Check(strUsr, strPwd));
}

TEST_F(CAuthenticationTest, CheckUserPwd)
{
    mp_string strClientIP;
    mp_string strUsr;
    mp_string strPw;
    flag = 0;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), stub_return_ret_failed);
    EXPECT_EQ(ERROR_COMMON_READ_CONFIG_FAILED, security::Authentication::GetInstance().CheckUserPwd(strClientIP, strUsr, strPw));

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), stub_return_ret_test);
    EXPECT_EQ(ERROR_COMMON_READ_CONFIG_FAILED, security::Authentication::GetInstance().CheckUserPwd(strClientIP, strUsr, strPw));
}

TEST_F(CAuthenticationTest, Auth)
{
    mp_string strClientIP = "127.0.0.1";
    mp_string strUsr = "admin";
    mp_string strPw = "Admin@123";
    mp_int32 rst = 0;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&PBKDF2Hash, &stub_return_ret_success);
    //CFG_USER_NAME < 0
    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubCConfigXmlParserGetValueStringLt0);
        rst = security::Authentication::GetInstance().Auth(strClientIP, strUsr, strPw, "cert_dn");
        EXPECT_EQ(rst, ERROR_COMMON_READ_CONFIG_FAILED);
        stub.reset((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_string&))ADDR(CConfigXmlParser, GetValueString));
    }
    //ckeck != MP_FALSE
    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(
            CConfigXmlParser,GetValueString), StubCConfigXmlParserGetValueStringEq0AuthAuth);
        rst = security::Authentication::GetInstance().Auth(strClientIP, strUsr, strPw, "cert_dn");
        EXPECT_EQ(rst, MP_SUCCESS);
    }
    //ckeck = MP_FALSE and unlock
    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubCConfigXmlParserGetValueStringEq0AuthAuth);
        strPw = "error";
        rst = security::Authentication::GetInstance().Auth(strClientIP, strUsr, strPw, "cert_dn");
        EXPECT_EQ(rst, ERROR_COMMON_USER_OR_PASSWD_IS_WRONG);
        strPw = "Admin@123";
        rst = security::Authentication::GetInstance().Auth(strClientIP, strUsr, strPw, "cert_dn");
        EXPECT_EQ(rst, MP_SUCCESS);
    }

   //certificate Auth
   {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubCConfigXmlParserGetValueStringEq0CERTIFICATEAuth);
        rst = security::Authentication::GetInstance().Auth(strClientIP, strUsr, strPw, "CN=BCManager eBackup Client Cert");
        EXPECT_EQ(rst, MP_SUCCESS);
   }
   // 鉴权功能未完成，Authentication::CheckCert直接返回，后续上库需修改
   {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubCConfigXmlParserGetValueStringEq0CERTIFICATEAuth);
        rst = security::Authentication::GetInstance().Auth(strClientIP, strUsr, strPw, "CN=cert_dn");
        EXPECT_EQ(rst, MP_SUCCESS);
   }
   // 鉴权功能未完成，Authentication::CheckCert直接返回，后续上库需修改
   {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubCConfigXmlParserGetValueStringEq0CERTIFICATEAuth);
        rst = security::Authentication::GetInstance().Auth(strClientIP, strUsr, strPw, "cert_dn");
        EXPECT_EQ(rst, MP_SUCCESS);
   }
     //not certificate Auth and not password Auth
   {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubCConfigXmlParserGetValueStringEq0CERTIFICATEAuth0);
        rst = security::Authentication::GetInstance().Auth(strClientIP, strUsr, strPw, "cert_dn");
        EXPECT_EQ(rst, MP_FAILED);
   }

     //ckeck = MP_FALSE and lock
   {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubCConfigXmlParserGetValueStringEq0AuthAuth);
        strPw = "error";
        for (int i = 0; i <= security::MAX_TRY_TIME; i++)
        {
            rst = security::Authentication::GetInstance().Auth(strClientIP, strUsr, strPw, "cert_dn");
        }
        EXPECT_EQ(rst, ERROR_COMMON_CLIENT_IS_LOCKED);
   }
}
//End CAuthenticationTest

//Begin CCommunicationTest
TEST_F(CCommunicationTest, Init)
{
    mp_string strUsr = "test";
    mp_string strPwd = "test";
    mp_int32 rst = 0;
    stub.set(&Communication::CheckIPandPort, StubCheckipandPort);
    //FCGX_Init < 0
    {
        stub.set(FCGX_Init, StubFCGX_InitLt0);
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32ReturnError);

        rst = Communication::GetInstance().Init();
        EXPECT_EQ(rst, ERROR_COMMON_OPER_FAILED);
    }
    //CFG_PORT < 0
    {
        stub.set(&FCGX_Init, StubFCGX_InitEq0);
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubCConfigXmlParserGetValueStringLt0);
        rst = Communication::GetInstance().Init();
        EXPECT_EQ(rst, ERROR_COMMON_READ_CONFIG_FAILED);
    }
    //FCGX_OpenSocket < 0
    {
        stub.set(&FCGX_Init, StubFCGX_InitEq0);
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubCConfigXmlParserGetValueStringEq0);
        stub.set(&FCGX_OpenSocket, StubFCGX_OpenSocketLt0);
        rst = Communication::GetInstance().Init();
        EXPECT_EQ(rst, ERROR_COMMON_OPER_FAILED);
    }
    //fcntl < 0
    {
        stub.set(&FCGX_Init, StubFCGX_InitEq0);
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubCConfigXmlParserGetValueStringEq0);
        stub.set(&FCGX_OpenSocket, StubFCGX_OpenSocketEq0);
        stub.set(&fcntl, StubfcntlLt0);
        stub.set(&OS_Close,&StubOS_CloseEq0);
        rst = Communication::GetInstance().Init();
        EXPECT_EQ(rst, ERROR_COMMON_OPER_FAILED);
    }
    //fcntl < 0
    {   
        flag = 0;
        stub.set(&FCGX_Init, StubFCGX_InitEq0);
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubCConfigXmlParserGetValueStringEq0);
        stub.set(&FCGX_OpenSocket, StubFCGX_OpenSocketEq0);
        stub.set(&fcntl, stub_return_ret_test);
        stub.set(&OS_Close,&StubOS_CloseEq0);
        rst = Communication::GetInstance().Init();
        EXPECT_EQ(rst, ERROR_COMMON_OPER_FAILED);
    }
    //Create < 0
    {
        stub.set(&FCGX_Init, &StubFCGX_InitEq0);
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubCConfigXmlParserGetValueStringEq0);
        stub.set(&FCGX_OpenSocket, StubFCGX_OpenSocketEq0);
        stub.set(&fcntl, StubfcntlEq0);
        stub.set(&OS_Close, StubOS_CloseEq0);
        stub.set(&CMpThread::Create, StubCMpThreadCreateLt0);
        rst = Communication::GetInstance().Init();
        EXPECT_EQ(rst, -1);
    }
    //init < 0
    {
        stub.set(&FCGX_Init, StubFCGX_InitEq0);
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubCConfigXmlParserGetValueStringEq0);
        stub.set(&FCGX_OpenSocket, StubFCGX_OpenSocketEq0);
        stub.set(&fcntl, StubfcntlEq0);
        stub.set(&OS_Close, StubOS_CloseEq0);
        stub.set(&CMpThread::Create, StubCMpThreadCreateEq0);
        stub.set(&security::Authentication::Init, StubCAuthenticationInitLt0);
        rst = Communication::GetInstance().Init();
        EXPECT_EQ(rst, -1);
        Communication::GetInstance().GetFcgxReq();
    }
    
    //InitRequest < 0
    {
        stub.set(&FCGX_Init, StubFCGX_InitEq0);
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubCConfigXmlParserGetValueStringEq0);
        stub.set(&FCGX_OpenSocket, StubFCGX_OpenSocketEq0);
        stub.set(&fcntl, StubfcntlEq0);
        stub.set(&OS_Close, StubOS_CloseEq0);
        stub.set(&Communication::InitRequest, StubCCommunicationInitRequestLt0);
        rst = Communication::GetInstance().Init();
        EXPECT_EQ(rst, -1);
    }
}

TEST_F(CCommunicationTest, InitThread)
{   
    mp_int32 handler;
    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set(&Communication::InitRequest, StubSuccess);
    stub.set(&OS_Close, stub_return_ret_test);
    EXPECT_EQ(MP_FAILED, Communication::GetInstance().InitThread(handler));

    stub.set(&Communication::InitRequest, StubSuccess);
    stub.set(&OS_Close, StubSuccess);
    stub.set(ADDR(security::Authentication, Init), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, Communication::GetInstance().InitThread(handler));
}

TEST_F(CCommunicationTest, NeedExit)
{
    mp_int32 rst = Communication::GetInstance().NeedExit();
    EXPECT_EQ(rst, Communication::GetInstance().m_bNeedExit);
}
TEST_F(CCommunicationTest, SetRecvThreadStatus)
{
    Communication::GetInstance().SetRecvThreadStatus(1);
    EXPECT_TRUE(1);
}
TEST_F(CCommunicationTest, SetSendThreadStatus)
{
    Communication::GetInstance().SetSendThreadStatus(1);
    EXPECT_TRUE(1);
}
TEST_F(CCommunicationTest, ReleaseRequest)
{
    FCGX_Request req;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32ReturnError);

    Communication::GetInstance().ReleaseRequest(req);
    EXPECT_TRUE(1);
}
TEST_F(CCommunicationTest, SendFailedMsg)
{
    FCGX_Request req;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32ReturnError);
    stub.set(&CResponseMsg::Send, StubCResponseMsgSendEq0);

    Communication::GetInstance().SendFailedMsg(Communication::GetInstance(), req, 1, 1);
    EXPECT_TRUE(1);
}
TEST_F(CCommunicationTest, HandleReceiveMsg)
{
    FCGX_Request req;
    stub.set(&CLogger::Log, StubCLoggerLog);

    {
        stub.set(&FCGX_GetParam, StubFCGX_GetParamNULL);
        stub.set(&CRequestMsg::Parse, StubCRequestMsgParseLt0);
        stub.set(&CResponseMsg::Send, StubCResponseMsgSendEq0);
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32ReturnSuccess);

        Communication::GetInstance().HandleReceiveMsg(Communication::GetInstance(), req);
    }
    {
        stub.set(&FCGX_GetParam, StubFCGX_GetParamNULL);
        stub.set(&CRequestMsg::Parse, StubCRequestMsgParseEq0);
        Communication::GetInstance().HandleReceiveMsg(Communication::GetInstance(), req);
    }
    EXPECT_TRUE(1);
    vector<message_pair_t>::iterator it = Communication::GetInstance().m_reqMsgQueue.begin();
    delete it->pReqMsg;
    delete it->pRspMsg;
    Communication::GetInstance().m_reqMsgQueue.erase(it);
}
TEST_F(CCommunicationTest, ReceiveThreadFunc)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    Communication::GetInstance().m_bNeedExit = 1;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32ReturnSuccess);

    Communication::GetInstance().ReceiveThreadFunc(&(Communication::GetInstance()));
    EXPECT_TRUE(1);
}
TEST_F(CCommunicationTest, SendThreadFunc)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    Communication::GetInstance().m_bNeedExit = 1;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32ReturnSuccess);

    Communication::GetInstance().SendThreadFunc(&(Communication::GetInstance()));
    EXPECT_TRUE(1);
}
TEST_F(CCommunicationTest, PushReq_PopReq)
{
    message_pair_t pair;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32ReturnSuccess);

    // 当前有个残留，需要确认残留原因
    Communication::GetInstance().PopReqMsgQueue(pair);
    Communication::GetInstance().PushReqMsgQueue(pair);
    mp_int32 rst = Communication::GetInstance().PopReqMsgQueue(pair);
    EXPECT_EQ(rst, MP_SUCCESS);
    rst = Communication::GetInstance().PopReqMsgQueue(pair);
    EXPECT_EQ(rst, MP_FAILED);
}
TEST_F(CCommunicationTest, PushRsp_PopRsp)
{
    CResponseMsg rsp;
    CRequestMsg req;
    message_pair_t pair(req, rsp);
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32ReturnSuccess);

    Communication::GetInstance().PushRspMsgQueue(pair);
    mp_int32 rst = Communication::GetInstance().PopRspMsgQueue(pair);
    EXPECT_EQ(rst, MP_FAILED);
    rst = Communication::GetInstance().PopRspInternalMsgQueue(pair);
    EXPECT_EQ(rst, MP_SUCCESS);
    rst = Communication::GetInstance().PopRspMsgQueue(pair);
    EXPECT_EQ(rst, MP_FAILED);
    
    FCGX_Request pfcg;
    CResponseMsg rsp2(pfcg);
    message_pair_t pair2(req, rsp2);
    Communication::GetInstance().PushRspMsgQueue(pair2);
    rst = Communication::GetInstance().PopRspMsgQueue(pair2);
    EXPECT_EQ(rst, MP_SUCCESS);
    
    Communication::GetInstance().PushRspMsgQueue(pair2);
    rst = Communication::GetInstance().PopRspInternalMsgQueue(pair2);
    EXPECT_EQ(rst, MP_FAILED);
    rst = Communication::GetInstance().PopRspMsgQueue(pair2);
    EXPECT_EQ(rst, MP_SUCCESS);
    rst = Communication::GetInstance().PopRspInternalMsgQueue(pair2);
    EXPECT_EQ(rst, MP_FAILED);
}

TEST_F(CCommunicationTest, CheckHeader)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet;
    Communication instance;
    FCGX_Request fcgiReq;
    CRequestMsg reqMsg;

    stub.set(ADDR(CHttpRequest,GetAllHead),StubReturnNull);
    stub.set(ADDR(Communication,SendFailedMsg),CommunicationSendFailedMsg);
    iRet = Communication::GetInstance().CheckHeader(instance, fcgiReq, reqMsg);
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(CCommunicationTest, CheckAuth)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet;
    Communication instance;
    FCGX_Request fcgiReq;
    CRequestMsg reqMsg;

    stub.set(ADDR(CHttpRequest,GetHead),StubReturnGeneralString);
    stub.set(ADDR(CHttpRequest,GetRemoteIP),StubReturnGeneralString);
    stub.set(ADDR(CHttpRequest,GetClientCertDN),StubReturnGeneralString);
    stub.set(ADDR(security::Authentication,Auth),StubAuthenticationAuthFailed);
    stub.set(ADDR(Communication,SendFailedMsg),CommunicationSendFailedMsg);
    iRet = Communication::GetInstance().CheckAuth(instance, fcgiReq, reqMsg);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(security::Authentication,Auth), StubSuccess);
    iRet = Communication::GetInstance().CheckAuth(instance, fcgiReq, reqMsg);
    EXPECT_EQ(iRet, MP_SUCCESS);
}
//End CCommunicationTest


//Begin CFTExceptionHandleTest
TEST_F(CFTExceptionHandleTest, HandleMonitorObjsProc)
{
    FTExceptionHandle::GetInstance().m_bNeedExit = true;
    FTExceptionHandle::GetInstance().HandleMonitorObjsProc(&(FTExceptionHandle::GetInstance()));
    EXPECT_TRUE(1);
}
TEST_F(CFTExceptionHandleTest, GetThreadStatus)
{
    mp_int32 rst = FTExceptionHandle::GetInstance().GetThreadStatus();
    EXPECT_EQ(rst, FTExceptionHandle::GetInstance().m_iThreadStatus);
}

// TEST_F(CFTExceptionHandleTest, HandleMonitorObjsTest)
// {
//     flag = 0;
//     stub.set(&CLogger::Log, StubCLoggerLog);
//     stub.set(&FTExceptionHandle::ProcessInternalRsps, stub_return_void);
//     stub.set(&FTExceptionHandle::GetHandleMonitorObj, StubGetMonitorObjTwoIsNull);
//     stub.set(&FTExceptionHandle::HandleFreezedMonitorObj, stub_return_void);
//     FTExceptionHandle::GetInstance().HandleMonitorObjs();
//     EXPECT_EQ(1, flag);
// }

TEST_F(CFTExceptionHandleTest, ProcessInternalRsps)
{   
    flag = 0;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&Communication::PopRspInternalMsgQueue, stub_return_ret_test);
    FTExceptionHandle::GetInstance().ProcessInternalRsps();
}

/*
* 用例名称：释放请求消息体
* 前置条件：无
* check点：检查接口返回值
*/
TEST_F(CFTExceptionHandleTest, ProccessUnFreezeRsp)
{   
    CRequestMsg pReqMsg;
    CResponseMsg pRspMsg;
    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set(&FTExceptionHandle::GetRequestInstanceName, stub_return_ret_failed);
    EXPECT_EQ(FTExceptionHandle::GetInstance().ProccessUnFreezeRsp(pReqMsg, pRspMsg), MP_FAILED);

    stub.set(&FTExceptionHandle::GetRequestInstanceName, stub_return_ret_success);
    stub.set(&FTExceptionHandle::GetRequestDbName, stub_return_ret_failed);
    EXPECT_EQ(FTExceptionHandle::GetInstance().ProccessUnFreezeRsp(pReqMsg, pRspMsg), MP_FAILED);

    stub.set(&FTExceptionHandle::GetRequestDbName, stub_return_ret_success);
    stub.set(&FTExceptionHandle::GetRequestAppType, StubGetRequestAppType);
    stub.set((MONITOR_OBJ*(FTExceptionHandle::*)(mp_int32, mp_string&, mp_string&))
        ADDR(FTExceptionHandle, GetMonitorObj), StubGetMonitorObjNull);
    EXPECT_EQ(FTExceptionHandle::GetInstance().ProccessUnFreezeRsp(pReqMsg, pRspMsg), MP_SUCCESS);

    stub.set((MONITOR_OBJ*(FTExceptionHandle::*)(mp_int32, mp_string&, mp_string&))
        ADDR(FTExceptionHandle, GetMonitorObj), StubGetMonitorObj);
    stub.set(&FTExceptionHandle::HandleUnFreezingMonitorObj, stub_return_ret_failed);
    EXPECT_EQ(FTExceptionHandle::GetInstance().ProccessUnFreezeRsp(pReqMsg, pRspMsg), MP_FAILED);

    stub.set(&FTExceptionHandle::HandleUnFreezingMonitorObj, stub_return_ret_success);
    EXPECT_EQ(FTExceptionHandle::GetInstance().ProccessUnFreezeRsp(pReqMsg, pRspMsg), MP_SUCCESS);
}

/*
* 用例名称：查询对象队列中的对象状态
* 前置条件：无
* check点：检查接口返回值
*/
TEST_F(CFTExceptionHandleTest, ProcessQueryStatusRsp)
{   
    CRequestMsg pReqMsg;
    CResponseMsg pRspMsg;
    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set(&FTExceptionHandle::GetRequestInstanceName, stub_return_ret_failed);
    EXPECT_EQ(FTExceptionHandle::GetInstance().ProcessQueryStatusRsp(pReqMsg, pRspMsg), MP_FAILED);

    stub.set(&FTExceptionHandle::GetRequestInstanceName, stub_return_ret_success);
    stub.set(&FTExceptionHandle::GetRequestDbName, stub_return_ret_failed);
    EXPECT_EQ(FTExceptionHandle::GetInstance().ProcessQueryStatusRsp(pReqMsg, pRspMsg), MP_FAILED);

    stub.set(&FTExceptionHandle::GetRequestDbName, stub_return_ret_success);
    stub.set(&FTExceptionHandle::GetRequestAppType, StubGetRequestAppType);
    stub.set((MONITOR_OBJ*(FTExceptionHandle::*)(mp_int32, mp_string&, mp_string&))
        ADDR(FTExceptionHandle, GetMonitorObj), StubGetMonitorObjNull);
    EXPECT_EQ(FTExceptionHandle::GetInstance().ProcessQueryStatusRsp(pReqMsg, pRspMsg), MP_SUCCESS);

    stub.set((MONITOR_OBJ*(FTExceptionHandle::*)(mp_int32, mp_string&, mp_string&))
        ADDR(FTExceptionHandle, GetMonitorObj), StubGetMonitorObj);
    stub.set(&FTExceptionHandle::HandleQueryStatusMonitorObj, stub_return_ret_failed);
    EXPECT_EQ(FTExceptionHandle::GetInstance().ProcessQueryStatusRsp(pReqMsg, pRspMsg), MP_FAILED);

    stub.set(&FTExceptionHandle::HandleQueryStatusMonitorObj, stub_return_ret_success);
    EXPECT_EQ(FTExceptionHandle::GetInstance().ProcessQueryStatusRsp(pReqMsg, pRspMsg), MP_SUCCESS);
}

/*
* 用例名称：监控逻辑原则，始终监控最后一次冻结操作
* 前置条件：无
* check点：检查接口返回值
*/
TEST_F(CFTExceptionHandleTest, HandleUnFreezingMonitorObj)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    MONITOR_OBJ obj;
    obj.uiStatus = MONITOR_STATUS_GETSTATUSING;
    obj.uiLoopTime = 60;
    mp_int32 rst = FTExceptionHandle::GetInstance().HandleUnFreezingMonitorObj(obj, false);
    EXPECT_EQ(rst, MP_FAILED);
    rst = FTExceptionHandle::GetInstance().HandleUnFreezingMonitorObj(obj, true);
    EXPECT_EQ(rst, MP_SUCCESS);
    stub.set(&FTExceptionHandle::RemoveFromDB, stub_return_void);
    stub.set(&FTExceptionHandle::DelMonitorObj, stub_return_void);
    rst = FTExceptionHandle::GetInstance().HandleUnFreezingMonitorObj(obj, true);
    EXPECT_EQ(rst, MP_SUCCESS);
}

/*
* 用例名称：查询监控对象状态
* 前置条件：无
* check点：检查接口返回值
*/
TEST_F(CFTExceptionHandleTest, HandleQueryStatusMonitorObj)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet;
    MONITOR_OBJ obj;
    mp_int32 iQueryStatus;

    {
        obj.uiStatus = MONITOR_STATUS_FREEZED;
        iRet = FTExceptionHandle::GetInstance().HandleQueryStatusMonitorObj(obj, iQueryStatus);
        EXPECT_EQ(iRet, MP_SUCCESS);
    }

    {
        obj.uiStatus = MONITOR_STATUS_UNFREEZING;
        iQueryStatus = DB_UNFREEZE;
        stub.set(&FTExceptionHandle::RemoveFromDB, stub_return_void);
        stub.set(&FTExceptionHandle::DelMonitorObj, stub_return_void);
        iRet = FTExceptionHandle::GetInstance().HandleQueryStatusMonitorObj(obj, iQueryStatus);
        EXPECT_EQ(iRet, MP_SUCCESS);
    }

    {
        iQueryStatus = DB_FREEZE;
        obj.uiStatus == MONITOR_STATUS_UNFREEZING;
        stub.set(&FTExceptionHandle::PushUnFreezeReq, stub_return_ret_failed);
        iRet = FTExceptionHandle::GetInstance().HandleQueryStatusMonitorObj(obj, iQueryStatus);
        EXPECT_EQ(iRet, MP_FAILED);
        stub.set(&FTExceptionHandle::PushUnFreezeReq, stub_return_ret_success);
        iRet = FTExceptionHandle::GetInstance().HandleQueryStatusMonitorObj(obj, iQueryStatus);
        EXPECT_EQ(iRet, MP_SUCCESS);
    }

    {
        obj.uiStatus = MONITOR_STATUS_UNFREEZING;
        iQueryStatus = DB_UNFREEZING;
        iRet = FTExceptionHandle::GetInstance().HandleQueryStatusMonitorObj(obj, iQueryStatus);
        EXPECT_EQ(iRet, MP_SUCCESS);
    }
}

/*
* 用例名称：处理待处理的监控对象
* 前置条件：无
* check点：检查接口返回值
*/
TEST_F(CFTExceptionHandleTest, HandleFreezedMonitorObj)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet;
    MONITOR_OBJ obj;
    stub.set(&CMpTime::GetTimeSec, StubGetTimeSec);
    {
        obj.ulBeginTime = 30001;
        iRet = FTExceptionHandle::GetInstance().HandleFreezedMonitorObj(obj);
        EXPECT_EQ(iRet, MP_SUCCESS);
    }

    {
        obj.ulBeginTime = 0;
        iRet = FTExceptionHandle::GetInstance().HandleFreezedMonitorObj(obj);
        EXPECT_EQ(iRet, MP_SUCCESS);
    }

    {
        obj.ulBeginTime = 29999;
        obj.uiLoopTime = 10;
        iRet = FTExceptionHandle::GetInstance().HandleFreezedMonitorObj(obj);
        EXPECT_EQ(iRet, MP_SUCCESS);
    }

    {
        obj.ulBeginTime = 20000;
        obj.uiLoopTime = 10;
        stub.set(ADDR(FTExceptionHandle, GetQueryStatusUrl), StubGetQueryStatusUrlEmpty);
        iRet = FTExceptionHandle::GetInstance().HandleFreezedMonitorObj(obj);
        EXPECT_EQ(iRet, MP_FAILED);
    }

    {
        obj.ulBeginTime = 20000;
        obj.uiLoopTime = 10;
        stub.set(ADDR(FTExceptionHandle, GetQueryStatusUrl), StubGetQueryStatusUrl);
        stub.set(ADDR(FTExceptionHandle, PushQueryStatusReq), stub_return_ret_failed);
        iRet = FTExceptionHandle::GetInstance().HandleFreezedMonitorObj(obj);
        EXPECT_EQ(iRet, MP_FAILED);
        stub.set(ADDR(FTExceptionHandle, PushQueryStatusReq), stub_return_ret_success);
        iRet = FTExceptionHandle::GetInstance().HandleFreezedMonitorObj(obj);
        EXPECT_EQ(iRet, MP_SUCCESS);
    }
}

TEST_F(CFTExceptionHandleTest, WaitForExit)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    FTExceptionHandle::GetInstance().m_iThreadStatus = THREAD_STATUS_EXITED;
    FTExceptionHandle::GetInstance().WaitForExit();
    EXPECT_TRUE(1);

    FTExceptionHandle::GetInstance().m_iThreadStatus = THREAD_STATUS_RUNNING;
    stub.set(DoSleep, StubWaitForExitDoSleep);
    FTExceptionHandle::GetInstance().WaitForExit();
    EXPECT_TRUE(1);
}
TEST_F(CFTExceptionHandleTest, MonitorFreezeOper)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CRequestMsg req;
    stub.set(&CDB::QueryTable, StubCDBQueryTableEq0);
    req.m_url.m_procURL = "/device/filesystems/freezestate";
    FTExceptionHandle::GetInstance().MonitorFreezeOper(req);
    req.m_msgBody.m_msgJsonData[DISKNAMES].append("test");
    req.m_msgBody.m_msgJsonData[DISKNAMES].append("test");
    FTExceptionHandle::GetInstance().MonitorFreezeOper(req);
    EXPECT_TRUE(1);
}

/*
* 用例名称：处理单个冻结请求
* 前置条件：无
* check点：检查接口返回值
*/
TEST_F(CFTExceptionHandleTest, MonitorSingleFreezeOper)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet;
    CRequestMsg obj;
    {
        stub.set(&FTExceptionHandle::IsFreezeRequest, stub_return_false);
        stub.set(&FTExceptionHandle::IsUnFreezeRequest, stub_return_false);
        FTExceptionHandle::GetInstance().MonitorSingleFreezeOper(obj);
        EXPECT_TRUE(1);
    }

    {
        stub.set(&FTExceptionHandle::IsFreezeRequest, stub_return_true);
        stub.set(&FTExceptionHandle::IsUnFreezeRequest, stub_return_true);
        stub.set(&FTExceptionHandle::CreateMonitorObj, stub_return_ret_failed);
        FTExceptionHandle::GetInstance().MonitorSingleFreezeOper(obj);
        EXPECT_TRUE(1);
    }

    {
        stub.set(&FTExceptionHandle::IsFreezeRequest, stub_return_true);
        stub.set(&FTExceptionHandle::IsUnFreezeRequest, stub_return_true);
        stub.set(&FTExceptionHandle::CreateMonitorObj, stub_return_ret_success);
        stub.set(&FTExceptionHandle::SaveToDB, stub_return_ret_failed);
        stub.set(&FTExceptionHandle::AddMonitorObj, stub_return_ret_failed);
        stub.set(&FTExceptionHandle::FreeMonitorObj, stub_return_void);
        FTExceptionHandle::GetInstance().MonitorSingleFreezeOper(obj);
        EXPECT_TRUE(1);
    }
}

/*
* 用例名称：根据监控对象类型创建对应的监控对象
* 前置条件：无
* check点：检查接口返回值
*/
TEST_F(CFTExceptionHandleTest, CreateMonitorObj)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet;
    CRequestMsg pReqMsg;
    MONITOR_OBJ monitorObj;
    {
        stub.set(&FTExceptionHandle::IsFSRequest, stub_return_true);
        stub.set(&FTExceptionHandle::CreateFSMonitorObj, stub_return_ret_failed);
        iRet = FTExceptionHandle::GetInstance().CreateMonitorObj(pReqMsg, monitorObj);
        EXPECT_EQ(iRet, MP_FAILED);
    }

    {
        stub.set(&FTExceptionHandle::IsFSRequest, stub_return_false);
        stub.set(&FTExceptionHandle::IsThirdPartyRequest, stub_return_true);
        stub.set(&FTExceptionHandle::CreateThirdPartyMonitorObj, stub_return_ret_success);
        iRet = FTExceptionHandle::GetInstance().CreateMonitorObj(pReqMsg, monitorObj);
        EXPECT_EQ(iRet, MP_SUCCESS);
    }

    {
        stub.set(&FTExceptionHandle::IsFSRequest, stub_return_false);
        stub.set(&FTExceptionHandle::IsThirdPartyRequest, stub_return_false);
        stub.set(&FTExceptionHandle::IsAppRequest, stub_return_true);
        stub.set(&FTExceptionHandle::CreateAppMonitorObj, stub_return_ret_success);
        iRet = FTExceptionHandle::GetInstance().CreateMonitorObj(pReqMsg, monitorObj);
        EXPECT_EQ(iRet, MP_SUCCESS);
    }

    {
        stub.set(&FTExceptionHandle::IsFSRequest, stub_return_false);
        stub.set(&FTExceptionHandle::IsThirdPartyRequest, stub_return_false);
        stub.set(&FTExceptionHandle::IsAppRequest, stub_return_false);
        iRet = FTExceptionHandle::GetInstance().CreateMonitorObj(pReqMsg, monitorObj);
        EXPECT_EQ(iRet, MP_FAILED);
    }
}

TEST_F(CFTExceptionHandleTest, CreateVSSMonitorObj)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    FCGX_Request fcg;
    stub.set(&FCGX_GetParam, StubFCGX_GetParamOk);
    CRequestMsg req(fcg);
    MONITOR_OBJ obj;
    stub.set(&FTExceptionHandle::InitMonitorObj, stub_return_ret_failed);
    mp_int32 rst = FTExceptionHandle::GetInstance().CreateVSSMonitorObj(req, obj);
    EXPECT_EQ(MP_FAILED, rst);
    stub.set(&FTExceptionHandle::InitMonitorObj, stub_return_ret_success);
    rst = FTExceptionHandle::GetInstance().CreateVSSMonitorObj(req, obj);
    EXPECT_EQ(MP_SUCCESS, rst);
}

/*
* 用例名称：创建数据库监控对象
* 前置条件：无
* check点：检查接口返回值
*/
TEST_F(CFTExceptionHandleTest, CreateDBMonitorObj)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet;
    CRequestMsg pReqMsg;
    MONITOR_OBJ monitorObj;
    {
        stub.set(&CJsonUtils::GetJsonString, stub_return_ret_failed);
        iRet = FTExceptionHandle::GetInstance().CreateDBMonitorObj(pReqMsg, monitorObj);
        EXPECT_EQ(iRet, MP_FAILED);
    }

    {
        flag = 0;
        stub.set(&CJsonUtils::GetJsonString, StubGetJsonString);
        iRet = FTExceptionHandle::GetInstance().CreateDBMonitorObj(pReqMsg, monitorObj);
        EXPECT_EQ(iRet, MP_FAILED);
    }

    {
        stub.set(&CJsonUtils::GetJsonString, stub_return_ret_success);
        stub.set(&FTExceptionHandle::InitMonitorObj, stub_return_ret_failed);
        iRet = FTExceptionHandle::GetInstance().CreateDBMonitorObj(pReqMsg, monitorObj);
        EXPECT_EQ(iRet, MP_FAILED);
    }

    {
        stub.set(&CJsonUtils::GetJsonString, stub_return_ret_success);
        stub.set(&FTExceptionHandle::InitMonitorObj, stub_return_ret_success);
        iRet = FTExceptionHandle::GetInstance().CreateDBMonitorObj(pReqMsg, monitorObj);
        EXPECT_EQ(iRet, MP_SUCCESS);
    }
}

/*
* 用例名称：创建文件系统监控对象
* 前置条件：无
* check点：检查接口返回值
*/
TEST_F(CFTExceptionHandleTest, CreateFSMonitorObj)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    FCGX_Request fcg;
    stub.set(&FCGX_GetParam, StubFCGX_GetParamOk);
    CRequestMsg req(fcg);
    MONITOR_OBJ obj;
    mp_int32 rst = FTExceptionHandle::GetInstance().CreateFSMonitorObj(req, obj);
    EXPECT_EQ(rst, ERROR_COMMON_INVALID_PARAM);

    stub.set(ADDR(CJsonUtils, GetJsonArrayString), stub_return_ret_success);
    stub.set(&FTExceptionHandle::InitMonitorObj, stub_return_ret_failed);
    rst = FTExceptionHandle::GetInstance().CreateFSMonitorObj(req, obj);
    EXPECT_EQ(rst, MP_FAILED);

    stub.set(&FTExceptionHandle::InitMonitorObj, stub_return_ret_success);
    rst = FTExceptionHandle::GetInstance().CreateFSMonitorObj(req, obj);
    EXPECT_EQ(rst, MP_SUCCESS);
}

/*
* 用例名称：创建第三方脚本监控对象
* 前置条件：无
* check点：检查接口返回值
*/
TEST_F(CFTExceptionHandleTest, CreateThirdPartyMonitorObj)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CRequestMsg reqs;
    MONITOR_OBJ obj;
    reqs.m_msgBody.m_msgJsonData["freezeFile"] = "test";

    mp_int32 rst = FTExceptionHandle::GetInstance().CreateThirdPartyMonitorObj(reqs, obj);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, rst);

    stub.set(&CJsonUtils::GetJsonString, stub_return_ret_failed);
    rst = FTExceptionHandle::GetInstance().CreateThirdPartyMonitorObj(reqs, obj);
    EXPECT_EQ(MP_FAILED, rst);

    stub.set(&CJsonUtils::GetJsonString, stub_return_ret_success);
    stub.set(&CJsonUtils::GetJsonKeyString, stub_return_ret_success);
    stub.set(&FTExceptionHandle::InitMonitorObj, stub_return_ret_failed);
    rst = FTExceptionHandle::GetInstance().CreateThirdPartyMonitorObj(reqs, obj);
    EXPECT_EQ(MP_FAILED, rst);

    stub.set(&FTExceptionHandle::InitMonitorObj, stub_return_ret_success);
    rst = FTExceptionHandle::GetInstance().CreateThirdPartyMonitorObj(reqs, obj);
    EXPECT_EQ(MP_SUCCESS, rst);
}


/*
* 用例名称：创建备份不区分应用接口监控对象
* 前置条件：无
* check点：检查接口返回值
*/
TEST_F(CFTExceptionHandleTest, CreateAppMonitorObj)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CRequestMsg reqs;
    MONITOR_OBJ obj;
    mp_int32 iRet;

    stub.set(&FTExceptionHandle::InitMonitorObj, stub_return_ret_failed);
    iRet = FTExceptionHandle::GetInstance().CreateAppMonitorObj(reqs, obj);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(&FTExceptionHandle::InitMonitorObj, stub_return_ret_success);
    iRet = FTExceptionHandle::GetInstance().CreateAppMonitorObj(reqs, obj);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

/*
* 用例名称：更新监控对象状态
* 前置条件：无
* check点：检查接口返回值
*/
TEST_F(CFTExceptionHandleTest, UpdateFreezeOper)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CRequestMsg pReqMsg;
    CResponseMsg pRspMsg;

    stub.set(&FTExceptionHandle::IsFSRequest, stub_return_true);
    stub.set(&CJsonUtils::GetJsonArrayString, stub_return_ret_failed);
    FTExceptionHandle::GetInstance().UpdateFreezeOper(pReqMsg, pRspMsg);
    EXPECT_TRUE(1);
}

/*
* 用例名称：更新监控对象状态
* 前置条件：无
* check点：更新单个监控对象状态
*/
TEST_F(CFTExceptionHandleTest, UpdateSingleFreezeOper)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    FCGX_Request fcg;
    stub.set(&FCGX_GetParam, StubFCGX_GetParamOk);
    CRequestMsg reqMsg(fcg);
    CResponseMsg rspMsg;

    stub.set(&FTExceptionHandle::IsFreezeRequest, stub_return_true);
    stub.set(&FTExceptionHandle::CreateMonitorObj, stub_return_ret_failed);
    FTExceptionHandle::GetInstance().UpdateSingleFreezeOper(reqMsg, rspMsg);
    EXPECT_TRUE(1);

    stub.set(&FTExceptionHandle::CreateMonitorObj, stub_return_ret_success);
    stub.set(&FTExceptionHandle::FreeMonitorObj, stub_return_void);
    stub.set((MONITOR_OBJ*(FTExceptionHandle::*)(MONITOR_OBJ&))ADDR(FTExceptionHandle, GetMonitorObj), StubGetMonitorObjNull);
    FTExceptionHandle::GetInstance().UpdateSingleFreezeOper(reqMsg, rspMsg);
    EXPECT_TRUE(1);

    rspMsg.SetRetCode(ERROR_COMMON_DB_USERPWD_WRONG);
    stub.set((MONITOR_OBJ*(FTExceptionHandle::*)(MONITOR_OBJ&))ADDR(FTExceptionHandle, GetMonitorObj), StubGetMonitorObj);
    stub.set(&FTExceptionHandle::RemoveFromDB, stub_return_ret_success);
    stub.set(&FTExceptionHandle::DelMonitorObj, stub_return_void);
    stub.set(&FTExceptionHandle::FreeMonitorObj, stub_return_void);
    FTExceptionHandle::GetInstance().UpdateSingleFreezeOper(reqMsg, rspMsg);
    EXPECT_TRUE(1);

    rspMsg.SetRetCode(ERROR_COMMON_RECOVER_INSTANCE_NOSTART);
    FTExceptionHandle::GetInstance().UpdateSingleFreezeOper(reqMsg, rspMsg);
    EXPECT_TRUE(1);

    rspMsg.SetRetCode(MP_SUCCESS);
    stub.set(&FTExceptionHandle::IsFreezeRequest, stub_return_false);
    stub.set(&FTExceptionHandle::IsUnFreezeRequest, stub_return_true);
    FTExceptionHandle::GetInstance().UpdateSingleFreezeOper(reqMsg, rspMsg);
    EXPECT_TRUE(1);
}
TEST_F(CFTExceptionHandleTest, InitPushParam)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    MONITOR_OBJ pMonitorObj;
    mp_string strQueryParam;

    pMonitorObj.iAppType == TYPE_APP_FILESYSTEM;
    mp_int32 iRet = FTExceptionHandle::GetInstance().InitPushParam(pMonitorObj, strQueryParam);
    EXPECT_EQ(MP_SUCCESS, iRet);

    pMonitorObj.iAppType == TYPE_APP_THIRDPARTY;
    stub.set((mp_string(CPath::*)(const mp_string&, const mp_string&))ADDR(CPath, GetThirdPartyFilePath), StubGetThirdPartyFilePath);
    stub.set(&CMpFile::FileExist, stub_return_false);
    iRet = FTExceptionHandle::GetInstance().InitPushParam(pMonitorObj, strQueryParam);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(ADDR(CMpFile, FileExist), stub_return_true);
    iRet = FTExceptionHandle::GetInstance().InitPushParam(pMonitorObj, strQueryParam);
    EXPECT_EQ(MP_SUCCESS, iRet);

    pMonitorObj.iAppType == TYPE_APP_HANA;
    iRet = FTExceptionHandle::GetInstance().InitPushParam(pMonitorObj, strQueryParam);
    EXPECT_EQ(MP_SUCCESS, iRet);

    pMonitorObj.iAppType == TYPE_APP_EXCHANGE;
    iRet = FTExceptionHandle::GetInstance().InitPushParam(pMonitorObj, strQueryParam);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CFTExceptionHandleTest, PushQueryStatusReq)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    MONITOR_OBJ pMonitorObj;
    mp_string strQueryParam;

    stub.set(ADDR(FTExceptionHandle, GetQueryStatusUrl), StubReturnEmptyString);
    mp_int32 iRet = FTExceptionHandle::GetInstance().PushQueryStatusReq(pMonitorObj);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(ADDR(FTExceptionHandle, GetQueryStatusUrl), StubReturnGeneralString);
    stub.set(ADDR(FTExceptionHandle, InitPushParam), stub_return_ret_failed);
    iRet = FTExceptionHandle::GetInstance().PushQueryStatusReq(pMonitorObj);
    EXPECT_EQ(MP_FAILED, iRet);
}

TEST_F(CFTExceptionHandleTest, GetHandleMonitorObj)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    MONITOR_OBJ pMonitorObj;
    FTExceptionHandle::GetInstance().m_vecMonitors.push_back(pMonitorObj);
    FTExceptionHandle::GetInstance().m_iCurrIndex = 0;

    FTExceptionHandle::GetInstance().GetHandleMonitorObj();
    EXPECT_TRUE(1);
    FTExceptionHandle::GetInstance().m_vecMonitors.pop_back();
}

TEST_F(CFTExceptionHandleTest, CreateReqMsg1)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    MONITOR_OBJ monitorObj;
    mp_string strDbUser;
    mp_string strDbPp;
    Json::Value jvJsonData;
    monitorObj.pReqMsg = nullptr;

    mp_int32 iRet = FTExceptionHandle::GetInstance().CreateReqMsg(monitorObj, strDbUser, strDbPp, jvJsonData);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CFTExceptionHandleTest, SaveToDB)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    MONITOR_OBJ monitorObj;
    monitorObj.strInstanceName = "test";
    monitorObj.strDBName = "test";

    stub.set(ADDR(FTExceptionHandle, IsExistInDB), stub_return_true);
    mp_bool iRet = FTExceptionHandle::GetInstance().SaveToDB(monitorObj);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CFTExceptionHandleTest, PushUnFreezeReq)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    MONITOR_OBJ obj;
    mp_int32 rst = FTExceptionHandle::GetInstance().PushUnFreezeReq(obj);
    EXPECT_EQ(rst, MP_FAILED);
}
TEST_F(CFTExceptionHandleTest, Add_DelMonitorObjs)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    MONITOR_OBJ obj;
    obj.iAppType = TYPE_APP_DB2;
    obj.strInstanceName = "db2inst1";
    obj.strDBName = "db_sss";
    CRequestMsg* reqMsg = new CRequestMsg;
    reqMsg->m_httpReq.m_pFcgRequest = new FCGX_Request;
    reqMsg->m_httpReq.m_pFcgRequest->envp = new char*[2];
    reqMsg->m_httpReq.m_pFcgRequest->envp[0] = new char[3];
    reqMsg->m_httpReq.m_pFcgRequest->envp[1] = new char[3];
    obj.pReqMsg = reqMsg;
    vector<MONITOR_OBJ> vec;
    vec.push_back(obj);
    MONITOR_OBJ obj2;
    obj2.iAppType = TYPE_APP_ORACLE;
    obj2.strInstanceName = "db2inst1";
    obj2.strDBName = "db_sss";
    CRequestMsg* reqMsg1 = new CRequestMsg;
    reqMsg1->m_httpReq.m_pFcgRequest = new FCGX_Request;
    reqMsg1->m_httpReq.m_pFcgRequest->envp = new char*[2];
    reqMsg1->m_httpReq.m_pFcgRequest->envp[0] = new char[3];
    reqMsg1->m_httpReq.m_pFcgRequest->envp[1] = new char[3];
    obj2.pReqMsg = reqMsg1;
    vec.push_back(obj2);
    FTExceptionHandle::GetInstance().AddMonitorObjs(vec);
    FTExceptionHandle::GetInstance().DelMonitorObj(obj2);
    FTExceptionHandle::GetInstance().DelMonitorObj(obj);
    EXPECT_TRUE(1);
}
TEST_F(CFTExceptionHandleTest, LoadFromDB)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    vector<MONITOR_OBJ> vecObj;
    stub.set(&CDB::QueryTable, StubCDBQueryTableOk);
    mp_int32 rst = FTExceptionHandle::GetInstance().LoadFromDB(vecObj);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(CFTExceptionHandleTest, GetDBNameFromObj)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    MONITOR_OBJ obj;
    CRequestMsg req;
    obj.pReqMsg = &req;
    obj.iAppType = TYPE_APP_FILESYSTEM;
    obj.strInstanceName = "test";
    obj.strDBName = "test";
    mp_string rst = FTExceptionHandle::GetInstance().GetDBNameFromObj(obj);
    EXPECT_EQ(rst, "test");

    obj.iAppType = TYPE_APP_EXCHANGE;
    stub.set(&CJsonUtils::GetArrayJson, stub_return_ret_failed);
    rst = FTExceptionHandle::GetInstance().GetDBNameFromObj(obj);
    EXPECT_EQ(rst, "test");
}

TEST_F(CFTExceptionHandleTest, IsThirdPartyRequest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CRequestMsg req;
    req.SetProcURL(REST_HOST_FREEZE_SCRIPT);
   
    mp_bool rst = FTExceptionHandle::GetInstance().IsThirdPartyRequest(req);
    EXPECT_EQ(rst, MP_TRUE);
}

TEST_F(CFTExceptionHandleTest, IsAppRequest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CRequestMsg req;
    req.SetProcURL(REST_APP_QUERY_DB_FREEZESTATE);
   
    mp_bool rst = FTExceptionHandle::GetInstance().IsAppRequest(req);
    EXPECT_EQ(rst, MP_TRUE);
}

TEST_F(CFTExceptionHandleTest, GetRequestAppType)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CRequestMsg req;

    req.SetProcURL(ORACLE);
    mp_int32 rst = FTExceptionHandle::GetInstance().GetRequestAppType(req);
    EXPECT_EQ(rst, TYPE_APP_ORACLE);

    req.SetProcURL(EXCHANGE);
    rst = FTExceptionHandle::GetInstance().GetRequestAppType(req);
    EXPECT_EQ(rst, TYPE_APP_EXCHANGE);

    req.SetProcURL(THIRDPARTY);
    rst = FTExceptionHandle::GetInstance().GetRequestAppType(req);
    EXPECT_EQ(rst, TYPE_APP_THIRDPARTY);

    req.SetProcURL(APP);
    rst = FTExceptionHandle::GetInstance().GetRequestAppType(req);
    EXPECT_EQ(rst, TYPE_APP_APP);

    req.SetProcURL(SYBASE);
    rst = FTExceptionHandle::GetInstance().GetRequestAppType(req);
    EXPECT_EQ(rst, TYPE_APP_SYBASE);

    req.SetProcURL(HANA);
    rst = FTExceptionHandle::GetInstance().GetRequestAppType(req);
    EXPECT_EQ(rst, TYPE_APP_HANA);
}

TEST_F(CFTExceptionHandleTest, GetRequestInstanceNameFS)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CRequestMsg req;
    mp_string strInstanceName;

    stub.set(&CJsonUtils::GetJsonArrayString, stub_return_ret_failed);
    mp_bool rst = FTExceptionHandle::GetInstance().GetRequestInstanceNameFS(req, strInstanceName);
    EXPECT_EQ(rst, MP_FAILED);
}

TEST_F(CFTExceptionHandleTest, GetRequestInstanceName2)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CRequestMsg req;
    mp_string strInstanceName;
    req.m_msgBody.m_msgJsonData[DISKNAMES].append("disk0");

    mp_bool rst = FTExceptionHandle::GetInstance().GetRequestInstanceName(req, strInstanceName);
    EXPECT_EQ(rst, MP_FAILED);

    req.SetProcURL(REST_APP_FREEZE);
    rst = FTExceptionHandle::GetInstance().GetRequestInstanceName(req, strInstanceName);
    EXPECT_EQ(rst, MP_SUCCESS);
}

TEST_F(CFTExceptionHandleTest, GetRequestDbName)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CRequestMsg req;
    mp_string db;
    //fs
    req.m_url.m_procURL = REST_DEVICE_FILESYS_FREEZE;
    mp_bool rst = FTExceptionHandle::GetInstance().GetRequestDbName(req, db);
    EXPECT_EQ(rst, MP_SUCCESS);
    //other
    req.m_url.m_procURL = "no support";
    rst = FTExceptionHandle::GetInstance().GetRequestDbName(req, db);
    EXPECT_EQ(rst, MP_FAILED);

    req.m_url.m_procURL = REST_HOST_FREEZE_SCRIPT;
    rst = FTExceptionHandle::GetInstance().GetRequestDbName(req, db);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(CFTExceptionHandleTest, IsQueryStatusRequest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CRequestMsg req;
    
    req.m_url.m_procURL = REST_DEVICE_FILESYS_FREEZESTATUS;
    mp_bool rst = FTExceptionHandle::GetInstance().IsQueryStatusRequest(req);
    EXPECT_TRUE(rst);

    req.m_url.m_procURL = REST_HOST_QUERY_STATUS_SCRIPT;
    rst = FTExceptionHandle::GetInstance().IsQueryStatusRequest(req);
    EXPECT_TRUE(rst);

    req.m_url.m_procURL = REST_APP_QUERY_DB_FREEZESTATE;
    rst = FTExceptionHandle::GetInstance().IsQueryStatusRequest(req);
    EXPECT_TRUE(rst);

    req.m_url.m_procURL = "test";
    rst = FTExceptionHandle::GetInstance().IsQueryStatusRequest(req);
    EXPECT_FALSE(rst);
}
TEST_F(CFTExceptionHandleTest, GetQueryStatusUrl)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string rst = FTExceptionHandle::GetInstance().GetQueryStatusUrl(TYPE_APP_FILESYSTEM);
    EXPECT_EQ(rst, REST_DEVICE_FILESYS_FREEZESTATUS);
}
TEST_F(CFTExceptionHandleTest, GetUnFreezeUrl)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string rst = FTExceptionHandle::GetInstance().GetUnFreezeUrl(TYPE_APP_FILESYSTEM);
    EXPECT_EQ(rst, REST_DEVICE_FILESYS_UNFREEZE);
}
TEST_F(CFTExceptionHandleTest, Init)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 rst = 0;
    //LoadFromDB < 0
    {
        stub.set(&CDB::QueryTable, StubCDBQueryTableLt0);
        rst = FTExceptionHandle::GetInstance().Init();
        EXPECT_EQ(rst, -1);
    }
    //Create < 0
    {
        stub.set(&CDB::QueryTable, StubCDBQueryTableEq0);
        stub.set(&CMpThread::Create, StubCMpThreadCreateLt0);
        rst = FTExceptionHandle::GetInstance().Init();
        EXPECT_EQ(rst, -1);
    }
    // LoadFromDB == 0 && Create == 0
    {
        stub.set(&FTExceptionHandle::LoadFromDB, stub_return_ret_success);
        stub.set(&CMpThread::Create, stub_return_ret_success);
        rst = FTExceptionHandle::GetInstance().Init();
        EXPECT_EQ(MP_SUCCESS, rst);
    }
}
//End CFTExceptionHandleTest

//Begin CTaskDispatchWorkerTest
TEST_F(CTaskDispatchWorkerTest, Init)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskDispatchWorker dspwrk;
    stub.set(&CMpThread::Create, StubCMpThreadCreateLt0);
    TaskWorker *wrk = new TaskWorker;
    mp_int32 rst = dspwrk.Init(wrk, 0);
    EXPECT_EQ(rst, -1);
}
TEST_F(CTaskDispatchWorkerTest, NeedExit)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskDispatchWorker dspwrk;
    mp_bool rst = dspwrk.NeedExit();
    EXPECT_EQ(rst, dspwrk.m_bNeedExit);
}
TEST_F(CTaskDispatchWorkerTest, PushMsgToWorker)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskDispatchWorker dspwrk;
    message_pair_t stPair;
    //m_iWorkerCount <= 0
    dspwrk.PushMsgToWorker(stPair);
    //
    dspwrk.m_iWorkerCount = 1;
    TaskWorker *wrk = new TaskWorker;
    dspwrk.m_pWorkers = &wrk;
    dspwrk.PushMsgToWorker(stPair);
    EXPECT_TRUE(1);
    delete wrk;
}
bool Ret_True(){
    return true;
}
TEST_F(CTaskDispatchWorkerTest, DispacthProc)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskDispatchWorker dspwrk;
    dspwrk.m_bNeedExit = 1;
    mp_void* rst = dspwrk.DispacthProc(&dspwrk);
    mp_void* pnull = NULL;
    EXPECT_EQ(rst, pnull);
    {
        stub.set(&TaskDispatchWorker::NeedExit, Ret_True);
        rst = dspwrk.DispacthProc(&dspwrk);
        EXPECT_EQ(rst, pnull);
    }
}
TEST_F(CTaskDispatchWorkerTest, Exit)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskDispatchWorker dspwrk;
    stub.set(&CMpThread::WaitForEnd, StubCMpThreadWaitForEndEq0);
    dspwrk.Exit();
    EXPECT_TRUE(1);
}

static TaskWorker *wrk = NULL;
void StubDoSleepWaitOk(mp_uint32 ms)
{
    wrk->m_bProcReq = MP_FALSE;
}

TEST_F(CTaskDispatchWorkerTest, PushMsgToWorkerWaitTaskOk)
{
    StubClogToVoidLogNullPointReference();
    stub.set(DoSleep, StubDoSleepWaitOk);
    TaskDispatchWorker dspwrk;
    message_pair_t stPair;
    // 只有一个线程，两次推送消息，通过stub函数构造线程释放场景
    dspwrk.m_iWorkerCount = 1;
    wrk = new TaskWorker;
    dspwrk.m_pWorkers = &wrk;
    dspwrk.PushMsgToWorker(stPair);
    wrk->m_bProcReq = MP_TRUE;
    // 通过Sleep函数触发线程可用
    dspwrk.PushMsgToWorker(stPair);
    EXPECT_TRUE(1);
    delete wrk;
}
//End CTaskDispatchWorkerTest

//Begin CTaskPoolTest
TEST_F(CTaskPoolTest, Init)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskPool tskpool;
    tskpool.m_workerThreadCount = 1;

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))
        ADDR(CConfigXmlParser,GetValueInt32), stub_return_ret_failed);

    stub.set(ADDR(TaskPool, CreatePlgConfParse), stub_return_ret_failed);
    EXPECT_EQ(MP_FAILED, tskpool.Init());

    stub.set(ADDR(TaskPool, CreatePlgConfParse), stub_return_ret_success);
    stub.set(ADDR(TaskPool, CreatePlugMgr), stub_return_ret_failed);
    EXPECT_EQ(MP_FAILED, tskpool.Init());

    stub.set(ADDR(TaskPool, CreatePlgConfParse), stub_return_ret_success);
    stub.set(ADDR(TaskPool, CreatePlugMgr), stub_return_ret_success);
    stub.set(ADDR(TaskPool, CreateWorkers), stub_return_ret_failed);
    EXPECT_EQ(MP_FAILED, tskpool.Init());

    stub.set(ADDR(TaskPool, CreatePlgConfParse), stub_return_ret_success);
    stub.set(ADDR(TaskPool, CreatePlugMgr), stub_return_ret_success);
    stub.set(ADDR(TaskPool, CreateWorkers), stub_return_ret_success);
    stub.set(ADDR(TaskPool, CreateDispatchWorker), stub_return_ret_failed);
    EXPECT_EQ(MP_FAILED, tskpool.Init());

    stub.set(ADDR(TaskPool, CreatePlgConfParse), stub_return_ret_success);
    stub.set(ADDR(TaskPool, CreatePlugMgr), stub_return_ret_success);
    stub.set(ADDR(TaskPool, CreateWorkers), stub_return_ret_success);
    stub.set(ADDR(TaskPool, CreateDispatchWorker), stub_return_ret_success);
    EXPECT_EQ(MP_SUCCESS, tskpool.Init());
}

TEST_F(CTaskPoolTest, CreatePlgConfParse)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskPool tskpool;

    stub.set(ADDR(PluginCfgParse, Init), stub_return_ret_failed);
    EXPECT_EQ(MP_FAILED, tskpool.CreatePlgConfParse());

    stub.set(ADDR(PluginCfgParse, Init), stub_return_ret_success);
    EXPECT_EQ(MP_SUCCESS, tskpool.CreatePlgConfParse());
}

TEST_F(CTaskPoolTest, CreatePlugMgr)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskPool tskpool;
    tskpool.m_plugCfgParse = new PluginCfgParse;

    stub.set(ADDR(CPluginManager, SetPluginPath), stub_return_void);

    stub.set(ADDR(CPluginManager, Initialize), stub_return_ret_failed);
    EXPECT_EQ(MP_FAILED, tskpool.CreatePlugMgr());

    stub.set(ADDR(CPluginManager, Initialize), stub_return_ret_success);
    stub.set(ADDR(CPluginManager, LoadPreLoadPlugins), stub_return_void);
    EXPECT_EQ(MP_SUCCESS, tskpool.CreatePlugMgr());
}

TEST_F(CTaskPoolTest, CreateWorkers)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskPool tskpool;
    tskpool.m_workerThreadCount = 1;

    stub.set(ADDR(TaskWorker, Init), stub_return_ret_failed);
    EXPECT_EQ(MP_FAILED, tskpool.CreateWorkers());

    stub.set(ADDR(TaskWorker, Init), stub_return_ret_success);
    EXPECT_EQ(MP_SUCCESS, tskpool.CreateWorkers());

    stub.set(ADDR(TaskDispatchWorker, Init), stub_return_ret_failed);
    EXPECT_EQ(MP_FAILED, tskpool.CreateDispatchWorker());

    stub.set(ADDR(TaskDispatchWorker, Init), stub_return_ret_success);
    EXPECT_EQ(MP_SUCCESS, tskpool.CreateDispatchWorker());
}

TEST_F(CTaskPoolTest, CanUnload)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskPool tskpool;
    PluginTest plugin;
    EXPECT_EQ(MP_FALSE, tskpool.CanUnload(plugin));
}

TEST_F(CTaskPoolTest, OnUpgraded)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskPool tskpool;
    PluginTest plugin1;
    PluginTest plugin2;
    tskpool.OnUpgraded(plugin1, plugin2);
    EXPECT_TRUE(1);
}
TEST_F(CTaskPoolTest, SetOptions)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskPool tskpool;
    PluginTest plugin;
    tskpool.SetOptions(plugin);
    EXPECT_TRUE(1);
}
TEST_F(CTaskPoolTest, GetReleaseVersion)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskPool tskpool;
    mp_string str1, str2;
    mp_string pc = tskpool.GetReleaseVersion(str1, str2);
    EXPECT_EQ(pc, "");
}


//End CTaskPoolTest
//Begin CTaskProtectWorkerTest
TEST_F(CTaskProtectWorkerTest, Con_Des)
{
    TaskProtectWorker pwrk;
}
//End CTaskProtectWorkerTest
//Begin CTaskWorkerTest
TEST_F(CTaskWorkerTest, Init)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskWorker work;
    PluginCfgParse pPlgCfgParse;
    CPluginManager pPlgMgr;
    stub.set(&CMpThread::Create, StubCMpThreadCreateLt0);
    mp_int32 rst  = work.Init(pPlgCfgParse, pPlgMgr);
    EXPECT_EQ(rst, -1);
}
TEST_F(CTaskWorkerTest, Exit)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskWorker work;
    stub.set(&CMpThread::WaitForEnd, StubCMpThreadWaitForEndEq0);
    work.Exit();
    EXPECT_TRUE(1);
}
TEST_F(CTaskWorkerTest, WorkProc)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskWorker work;
    stub.set(&TaskWorker::NeedExit, StubCTaskWorkerNeedExitEq1);
    mp_void* rst = work.WorkProc(&work);
    mp_void* pnull = NULL;
    EXPECT_EQ(rst, pnull);
}
TEST_F(CTaskWorkerTest, CanUnloadPlugin)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskWorker work;
    mp_int32 rst = work.CanUnloadPlugin(0, mp_string(""));
    EXPECT_EQ(rst, 1);
}
TEST_F(CTaskWorkerTest, push_popreq)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskWorker work;
    message_pair_t stPair;
    mp_int32 rst = work.PopRequest(stPair);
    EXPECT_EQ(rst, MP_FAILED);
    work.PushRequest(stPair);
    rst = work.PopRequest(stPair);
    EXPECT_EQ(rst, MP_SUCCESS);
}
TEST_F(CTaskWorkerTest, NeedExit)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskWorker work;
    mp_bool rst = work.NeedExit();
    EXPECT_EQ(rst, work.m_bNeedExit);
}
TEST_F(CTaskWorkerTest, GetPlugin)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskWorker work;
    PluginCfgParse cfgprs;
    CPluginManager mng;
    work.m_plgCfgParse = &cfgprs;
    work.m_plgMgr = &mng;
    //NULL
    mp_void* pnull = NULL;
    CServicePlugin* rst = work.GetPlugin("");
    EXPECT_EQ(rst, pnull);
    //
    stub.set(&PluginCfgParse::GetPluginByService, StubCPluginCfgParseGetPluginByServiceEq0);
    stub.set((IPlugin* (CPluginManager::*)(const mp_string&))ADDR(CPluginManager,GetPlugin), StubCPluginManagerGetPluginOK);
    rst = work.GetPlugin("host");
    if (rst)
    {
        delete rst;
    }
}

TEST_F(CTaskWorkerTest, SetPlugin)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskWorker work;
    HostPlugin plg;
    work.SetPlugin(plg);
    EXPECT_TRUE(1);
}

TEST_F(CTaskWorkerTest, ExitError)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskWorker work;
    CBasicReqMsg pReqMsg;
    CBasicRspMsg pRspMsg;
    message_pair_t stPair(pReqMsg,pRspMsg);

    stub.set(DoSleep,StubDoSleep); // 取消发送之后的定时器时长
    stub.set(ADDR(MessageHandler,PushRspMsg),StubPushRspMsgSuccess);
    work.ExitError(stPair, 5);
    EXPECT_EQ(stPair.pRspMsg->GetRetCode(), 5);
}

TEST_F(CTaskWorkerTest, DispatchMsg_InvalidType)
{
    stub.set(&CLogger::Log, StubCLoggerLog);

    TaskWorker work;
    {
        CBasicReqMsg req;
        CBasicRspMsg rsp;
        message_pair_t stBasicPair(req, rsp);
        mp_int32 iRet = work.DispatchMsg(stBasicPair);
        EXPECT_EQ(MP_FAILED, iRet);
    }
}

TEST_F(CTaskWorkerTest, DispatchMsg_Rest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(TaskWorker, DispatchRestMsg), StubDispatchRestMsg);
    stub.set(ADDR(Communication, PushRspMsgQueue), StubPushRspMsgSuccess);

    TaskWorker work;
    {
        CRequestMsg req;
        CResponseMsg rsp;
        message_pair_t pair(req, rsp);
        mp_int32 iRet = work.DispatchMsg(pair);
        EXPECT_EQ(MP_SUCCESS, iRet);
    }
    {
        CRequestMsg req;
        CDppMessage rsp;
        message_pair_t pair(req, rsp);
        mp_int32 iRet = work.DispatchMsg(pair);
        EXPECT_EQ(MP_FAILED, iRet);
    }
}

TEST_F(CTaskWorkerTest, DispatchMsg_Tcp)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(TaskWorker, DispatchTcpMsg), StubDispatchTcpMsg);
    stub.set(ADDR(MessageHandler, PushRspMsg), StubPushRspMsgSuccess);

    TaskWorker work;
    {
        CDppMessage req;
        CDppMessage rsp;
        message_pair_t pair(req, rsp);
        mp_int32 iRet = work.DispatchMsg(pair);
        EXPECT_EQ(MP_SUCCESS, iRet);
    }
    {
        CDppMessage req;
        CResponseMsg rsp;
        message_pair_t pair(req, rsp);
        mp_int32 iRet = work.DispatchMsg(pair);
        EXPECT_EQ(MP_FAILED, iRet);
    }
}

TEST_F(CTaskWorkerTest, DispatchTcpMsg_Failed)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    TaskWorker work;
    CDppMessage reqMsg;
    CDppMessage rspMsg;
    mp_int32 iRet;

    stub.set(DoSleep,StubDoSleep); // 取消发送之后的定时器时长
    stub.set(ADDR(MessageHandler,PushRspMsg),StubPushRspMsgSuccess);
    iRet = work.DispatchTcpMsg(reqMsg, rspMsg);
    EXPECT_EQ(iRet, MP_FAILED);
}
//End CTaskWorkerTest