#define private public

#include "dataprocess/datamessage/DataMessage.h"
#include "plugins/DataPathProcessClientTest.h"
#include "plugins/DataPathProcessClient.h"
#include "common/CSystemExec.h"
#include "common/Utils.h"
#include "securecom/RootCaller.h"
#ifndef WIN32
#include <sys/time.h>
#endif
#include <vector>
namespace {
mp_void LogTest()
{}
#define DoGetJsonStringTest()                                                                                          \
    do {                                                                                                               \
        stub11.set(ADDR(CLogger, Log), LogTest);                                                                       \
    } while (0)

const mp_uint32 SYSTEM_PREALLOCATED_PORT = 1024;

mp_bool IsDataProcessStartedFailed()
{
    return MP_FALSE;
}
mp_bool IsDataProcessStartedSucc()
{
    return MP_TRUE;
}

mp_int32 StartCommonDataPathProcessFailed()
{
    return MP_FAILED;
}

mp_int32 StartCommonDataPathProcessSucc()
{
    return MP_SUCCESS;
}

mp_int32 StartLoopThreadsFailed()
{
    return MP_FAILED;
}

mp_int32 StartLoopThreadsSucc()
{
    return MP_SUCCESS;
}

mp_int32 EstablishClientTestFailed()
{
    return MP_FAILED;
}

mp_int32 EstablishClientTestSucc()
{
    return MP_SUCCESS;
}

mp_int32 InitTest1(mp_socket sock, mp_string ip, mp_uint16 port)
{
    return MP_FAILED;
}
mp_void DoSleepTest(mp_uint32 ms)
{
    return;
}
mp_void StubDoSleepTest100ms(mp_uint32 ms)
{
#ifdef WIN32
    Sleep(100);
#else
    struct timeval stTimeOut;
    const mp_int32 timeUnitChange = 1000;
    stTimeOut.tv_sec  = 100 / timeUnitChange;
    stTimeOut.tv_usec = (100 % timeUnitChange) * timeUnitChange;
    (mp_void)select(0, NULL, NULL, NULL, &stTimeOut);
#endif
}

mp_int32 InitializeClientFailed()
{
    return MP_FAILED;
}

mp_int32 InitializeClientSucc()
{
    return MP_SUCCESS;
}

LinkState GetLinkStateTest()
{
    return LINK_STATE_LINKED;
}

mp_int32 StartReceiveDppSucc()
{
    return MP_SUCCESS;
}

mp_int32 DataMessageInitFailed(mp_socket sock, mp_string ip, mp_uint16 port)
{
    return MP_FAILED;
}

mp_int32 DataMessageInitSucc(mp_socket sock, mp_string ip, mp_uint16 port)
{
    return MP_SUCCESS;
}

mp_int32 DataMessageCreateClientFailed(mp_socket &clientSock)
{
    return MP_SUCCESS;
}

mp_int32 DataMessageCreateClientSucc(mp_socket &clientSock)
{
    return MP_FAILED;
}

mp_int32 GetDataPathPortLess()
{
    return SYSTEM_PREALLOCATED_PORT-1;
}

mp_int32 GetDataPathPortMore()
{
    return SYSTEM_PREALLOCATED_PORT+1;
}

mp_int32 CRootCallerExecFailed(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_FAILED;
}

mp_int32 CRootCallerExecSucc(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_SUCCESS;
}

}

TEST_F(DataPathProcessClientTest, InitTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_int32 serviceType;
    const mp_string dpParam;
    DataPathProcessClient client(serviceType, dpParam);

    stub.set(ADDR(DataPathProcessClient, IsDataProcessStarted), IsDataProcessStartedFailed);
    stub.set(ADDR(DataPathProcessClient, StartCommonDataPathProcess), StartCommonDataPathProcessFailed);
    mp_int32 iRet = client.Init();
    EXPECT_NE(iRet, MP_SUCCESS);

    stub.set(ADDR(DataPathProcessClient, StartCommonDataPathProcess), StartCommonDataPathProcessSucc);
    stub.set(ADDR(DataPathProcessClient, EstablishClient), EstablishClientTestFailed);
    iRet = client.Init();
    EXPECT_NE(iRet, MP_SUCCESS);

    stub.set(ADDR(DataPathProcessClient, EstablishClient), EstablishClientTestSucc);
    stub.set(ADDR(DataPathProcessClient, StartLoopThreads), StartLoopThreadsFailed);
    iRet = client.Init();
    EXPECT_NE(iRet, MP_SUCCESS);

    stub.set(ADDR(DataPathProcessClient, StartLoopThreads), StartLoopThreadsSucc);
    iRet = client.Init();
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set(ADDR(DataPathProcessClient, IsDataProcessStarted), IsDataProcessStartedSucc);
    stub.set(ADDR(DataPathProcessClient, EstablishClient), EstablishClientTestFailed);
    iRet = client.Init();
    EXPECT_NE(iRet, MP_SUCCESS);
}

TEST_F(DataPathProcessClientTest, SetUpClientTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_int32 serviceType;
    mp_string dpParam;
    DataPathProcessClient client(serviceType, dpParam);
    mp_string strIP;
    mp_uint16 iPort;

    stub.set(ADDR(DataMessage, Init), DataMessageInitFailed);
    client.SetUpClient(strIP, iPort);

    stub.set(ADDR(DataMessage, Init), DataMessageInitSucc);
    stub.set(ADDR(DataMessage, CreateClient), DataMessageCreateClientFailed);
    client.SetUpClient(strIP, iPort);

    stub.set(ADDR(DataMessage, Init), DataMessageInitSucc);
    stub.set(ADDR(DataMessage, CreateClient), DataMessageCreateClientSucc);
    client.SetUpClient(strIP, iPort);
}

TEST_F(DataPathProcessClientTest, InitializeClientTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_int32 serviceType;
    mp_string dpParam;
    DataPathProcessClient client(serviceType, dpParam);

    stub.set(ADDR(DataPathProcessClient, GetDataPathPort), GetDataPathPortLess);
    client.InitializeClient();

    stub.set(ADDR(DataPathProcessClient, GetDataPathPort), GetDataPathPortMore);
    client.InitializeClient();
}

TEST_F(DataPathProcessClientTest, SendDPMessageTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_int32 serviceType;
    mp_string dpParam;
    DataPathProcessClient client(serviceType, dpParam);
    mp_string taskId;
    mp_uint32 timeout = 1;

    CDppMessage *reqMsg = NULL;
    CDppMessage *rspMsg = NULL;
    client.SendDPMessage(taskId, reqMsg, rspMsg, timeout);

    reqMsg = new CDppMessage;
    client.SendDPMessage(taskId, reqMsg, rspMsg, timeout);
}

TEST_F(DataPathProcessClientTest, EndDataProcessOnTimeoutTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_int32 serviceType;
    mp_string dpParam;
    mp_string strCmd;
    DataPathProcessClient client(serviceType, dpParam);

    stub.set(ADDR(DataPathProcessClient, IsDataProcessStarted), IsDataProcessStartedFailed);
    mp_int32 iRet = client.EndDataProcessOnTimeout();
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.set(ADDR(DataPathProcessClient, IsDataProcessStarted), IsDataProcessStartedSucc);
    iRet = client.EndDataProcessOnTimeout();
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(DataPathProcessClientTest, EndDataPathClientest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_int32 serviceType;
    mp_string dpParam;
    DataPathProcessClient client(serviceType, dpParam);
    client.EndDataPathClient();
}

TEST_F(DataPathProcessClientTest, GetDataPathPortTest)
{
    DoGetJsonStringTest();
    Stub stub;
    stub.set(ADDR(CMpTime, DoSleep), StubDoSleepTest100ms);
    mp_int32 serviceType;
    mp_string dpParam;
    DataPathProcessClient client(serviceType, dpParam);
    client.GetDataPathPort();
}

TEST_F(DataPathProcessClientTest, IsDataProcessStartedTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_int32 serviceType;
    mp_string dpParam;
    DataPathProcessClient client(serviceType, dpParam);
    client.IsDataProcessStarted();
}

TEST_F(DataPathProcessClientTest, EstablishClientTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_int32 serviceType;
    mp_string dpParam;
    DataPathProcessClient client(serviceType, dpParam);

    stub.set(ADDR(DataPathProcessClient, InitializeClient), InitializeClientFailed);
    client.EstablishClient();

    stub.set(ADDR(DataPathProcessClient, InitializeClient), InitializeClientSucc);
    client.EstablishClient();
}

TEST_F(DataPathProcessClientTest, DoWatchDogProcessTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_int32 serviceType;
    mp_string dpParam;
    DataPathProcessClient client(serviceType, dpParam);

    stub.set(ADDR(CMpTime, DoSleep), DoSleepTest);
    client.EndDataPathClient();
    client.DoWatchDogProcess();
}

TEST_F(DataPathProcessClientTest, SendMsgToDPTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_int32 serviceType;
    mp_string dpParam;
    DataPathProcessClient client(serviceType, dpParam);
    
    client.m_bReqExitFlag = true;
    //stub.set(ADDR(CMpTime, DoSleep), DoSleepTest);
    client.SendMsgToDP();

    //client.m_bReqExitFlag = false;
   // stub.set(ADDR(CConnection, GetLinkState), GetLinkStateTest);
    //client.SendMsgToDP();
}

TEST_F(DataPathProcessClientTest, RecvMsgFromDPTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_int32 serviceType;
    mp_string dpParam;
    DataPathProcessClient client(serviceType, dpParam);

    client.m_bRspExitFlag = true;
    //stub.set(ADDR(CMpTime, DoSleep), DoSleepTest);
    client.RecvMsgFromDP(); 
/*
    client.m_bRspExitFlag = false;
    client.m_dataMsg.recvdMsg = new CDppMessage;
    stub.set(ADDR(CConnection, GetLinkState), GetLinkStateTest);
    stub.set(ADDR(DataMessage, StartReceiveDpp), StartReceiveDppSucc);
    stub.set(ADDR(CDppMessage, GetOrgSeqNo), DoSleepTest);
    client.DoWatchDogProcess();
*/
}

TEST_F(DataPathProcessClientTest, StartLoopThreadsTest)
{
    DoGetJsonStringTest();
    Stub stub;
    stub.set(ADDR(CMpTime, DoSleep), StubDoSleepTest100ms);
    mp_int32 serviceType;
    mp_string dpParam;
    DataPathProcessClient client(serviceType, dpParam);
    
    client.StartLoopThreads();
}

TEST_F(DataPathProcessClientTest, GetProcessIDbyProcessNameTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_int32 serviceType;
    mp_string dpParam;
    DataPathProcessClient client(serviceType, dpParam);
    mp_string strCurrentDataProcessName;

    stub.set(ADDR(CRootCaller, Exec), CRootCallerExecFailed);
    client.GetProcessIDbyProcessName(strCurrentDataProcessName);

    stub.set(ADDR(CRootCaller, Exec), CRootCallerExecSucc);
    client.GetProcessIDbyProcessName(strCurrentDataProcessName);
}

TEST_F(DataPathProcessClientTest, StartOMDataPathProcessTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_int32 serviceType;
    mp_string dpParam;
    DataPathProcessClient client(serviceType, dpParam);
    client.StartOMDataPathProcess();
}

TEST_F(DataPathProcessClientTest, StartVMwareNativeDataPathProcessTest)
{
    DoGetJsonStringTest();
    Stub stub;
    stub.set(ADDR(CMpTime, DoSleep), StubDoSleepTest100ms);
    mp_int32 serviceType;
    mp_string dpParam;
    DataPathProcessClient client(serviceType, dpParam);

    stub.set(ADDR(CRootCaller, Exec), CRootCallerExecFailed);
    client.StartVMwareNativeDataPathProcess();

    stub.set(ADDR(CRootCaller, Exec), CRootCallerExecSucc);
    client.StartVMwareNativeDataPathProcess();
}

TEST_F(DataPathProcessClientTest, StartCommonDataPathProcessTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_string dpParam;
    DataPathProcessClient client(OCEAN_MOBILITY_SERVICE, dpParam);
    stub.set(ADDR(DataPathProcessClient, StartOMDataPathProcess), StartCommonDataPathProcessSucc);
    EXPECT_EQ(MP_SUCCESS, client.StartCommonDataPathProcess());

    client.m_iServiceType = OCEAN_VMWARENATIVE_SERVICE;
    stub.set(ADDR(DataPathProcessClient, StartVMwareNativeDataPathProcess), StartCommonDataPathProcessSucc);
    EXPECT_EQ(MP_SUCCESS, client.StartCommonDataPathProcess());

    client.m_iServiceType = INVALID_SERVICE;
    EXPECT_EQ(MP_FAILED, client.StartCommonDataPathProcess());
}

TEST_F(DataPathProcessClientTest, ResetConnectionTest)
{
    DoGetJsonStringTest();
    Stub stub;
    stub.set(DoSleep, DoSleepTest);
    mp_int32 serviceType;
    mp_string dpParam;
    DataPathProcessClient client(serviceType, dpParam);
    client.ResetConnection();
}

TEST_F(DataPathProcessClientTest, PushMsg2QueueTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_int32 serviceType;
    mp_string dpParam;
    DataPathProcessClient client(serviceType, dpParam);
    CDppMessage *reqMsg = nullptr;
    client.PushMsg2Queue(reqMsg);
}

TEST_F(DataPathProcessClientTest, GetSeqNoTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_int32 serviceType;
    mp_string dpParam;
    DataPathProcessClient client(serviceType, dpParam);
    client.GetSeqNo();
}
