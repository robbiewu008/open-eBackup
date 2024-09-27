#include "apps/dws/XBSAServer/BsaSessionTest.h"
#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include "apps/dws/XBSAServer/CTimer.h"
#include "apps/dws/XBSAServer/BsaSessionManager.h"

namespace {
mp_int32 flag = 0;
mp_void LogTest(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...) {}
#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)

mp_int32 StubFailed()
{
    return MP_FAILED;
}

std::shared_ptr<BsaTransaction> StubMakeShared()
{
    std::shared_ptr<BsaTransaction> trans;
    return trans;
}

mp_int32 StubSuccess()
{
    return MP_SUCCESS;
}

mp_long StubGetTransId()
{
    return 0;
}


mp_bool StubTrue()
{
    return MP_TRUE;
}

mp_bool StubFalse()
{
    return MP_FALSE;
}

mp_int32 StubFailedTwo()
{
    if (flag ++ < 1) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

mp_int32 StubGetSessionTimeoutInterval(mp_int32 &timeoutMs)
{
    timeoutMs = 1;
    return MP_SUCCESS;
}

mp_void StubVoid(mp_void *pthis)
{
    return;
}
}

mp_int32 Stub_CConfigXmlParser_GetValueInt32_Succ(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    iValue = 2;
    return MP_SUCCESS;
}

mp_int32 Stub_CConfigXmlParser_GetValueInt32_Succ_0(mp_void* pthis, mp_string strSection, mp_string strKey, mp_int32& iValue)
{
    iValue = 0;
    return MP_SUCCESS;
}

mp_int32 Stub_GetSessionTimeoutIntervalSucc(mp_int32 &timeoutMs)
{
    return MP_SUCCESS;
}

TEST_F(BsaSessionTest, InitTest)
{
    DoGetJsonStringTest();
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32),
        Stub_CConfigXmlParser_GetValueInt32_Succ);

    stub.set(ADDR(BsaSession, GetSessionTimeoutInterval), Stub_GetSessionTimeoutIntervalSucc);
    stub.set(ADDR(BsaSession, GetSessionTimeoutInterval), Stub_GetSessionTimeoutIntervalSucc);
    BsaObjectOwner objectOwner;
    mp_string env[BSA_ENV_BUTT];
    mp_long sessinId;
    mp_uint32 envSize;
    BsaSession session(sessinId, BSA_AppType::BSA_DWS);

    stub.set(strcpy_s, StubFailed);
    mp_int32 iRet = session.Init(objectOwner, env, envSize);
    EXPECT_EQ(iRet, MP_FAILED);

    flag = 0;
    stub.set(strcpy_s, StubFailedTwo);
    iRet = session.Init(objectOwner, env, envSize);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(strcpy_s, StubSuccess);
    stub.set(&BsaSession::GetSessionTimeoutInterval, StubFailed);
    iRet = session.Init(objectOwner, env, envSize);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(strcpy_s, StubSuccess);
    stub.set(&BsaSession::GetSessionTimeoutInterval, StubGetSessionTimeoutInterval);
    stub.set(&BsaSession::SetEnv, StubVoid);
    stub.set(&BsaSessionManager::CloseSession, StubSuccess);
    stub.set(&CTimer::StartTimer, StubFalse);
    iRet = session.Init(objectOwner, env, envSize);
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(BsaSessionTest, FinitTest)
{
    DoGetJsonStringTest();
    mp_long sessinId;
    BsaSession session(sessinId, BSA_AppType::BSA_DWS);
    session.Finit();
}

TEST_F(BsaSessionTest, SetEnvTest)
{
    DoGetJsonStringTest();
    mp_long sessinId;
    BsaSession session(sessinId, BSA_AppType::BSA_DWS);
    BsaEnv idx;
    mp_string val;
    session.SetEnv(idx, val);

    val = "test";
    session.SetEnv(idx, val);
}

TEST_F(BsaSessionTest, GetEnvTest)
{
    DoGetJsonStringTest();
    mp_long sessinId;
    BsaSession session(sessinId, BSA_AppType::BSA_DWS);
    BsaEnv idx;
    session.GetEnv(idx);

    idx = BSA_ENV_BUTT;
    session.GetEnv(idx);
}

TEST_F(BsaSessionTest, GetAllEnvTest)
{
    DoGetJsonStringTest();
    mp_long sessinId;
    BsaSession session(sessinId, BSA_AppType::BSA_DWS);
    session.GetAllEnv();

    session.m_envs->push_back('t');
}

TEST_F(BsaSessionTest, BeginTxnTest)
{
    DoGetJsonStringTest();
    mp_long sessinId;
    BsaSession session(sessinId, BSA_AppType::BSA_DWS);
    session.BeginTxn();
    mp_long sessionId;
    mp_long transId;
    session.m_trans = std::make_shared<BsaTransaction>(sessionId, transId);
    session.BeginTxn();

    session.m_bsaHandle = 1;
    stub.set(&BsaSessionManager::NewTransId, StubTrue);
    stub.set(&BsaSession::TransExist, StubFalse);
    stub.set(&BsaTransaction::BeginTxn, StubSuccess);
    mp_int32 iRet = session.BeginTxn();
    EXPECT_EQ(iRet, BSA_RC_SUCCESS);

    stub.set(&BsaTransaction::BeginTxn, StubFailed);
    iRet = session.BeginTxn();
    EXPECT_EQ(iRet, BSA_RC_ABORT_SYSTEM_ERROR);
}

TEST_F(BsaSessionTest, EndTxnTest)
{
    DoGetJsonStringTest();
    mp_long sessinId;
    BsaSession session(sessinId, BSA_AppType::BSA_DWS);
    BSA_Vote vote;
    session.EndTxn(vote);

    stub.set(&BsaSession::TransExist, StubTrue);
    stub.set(&BsaTransaction::EndTxn, StubFailed);
    mp_int32 iRet = session.EndTxn(vote);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(&BsaTransaction::EndTxn, StubSuccess);
    stub.set(&BsaTransaction::GetTransId, StubGetTransId);
    iRet = session.EndTxn(vote);
    EXPECT_EQ(iRet, BSA_RC_SUCCESS);
}

TEST_F(BsaSessionTest, GetTransTest)
{
    DoGetJsonStringTest();
    mp_long sessinId;
    BsaSession session(sessinId, BSA_AppType::BSA_DWS);
    session.GetTrans();
}

TEST_F(BsaSessionTest, GetOwnerTest)
{
    DoGetJsonStringTest();
    mp_long sessinId;
    BsaSession session(sessinId, BSA_AppType::BSA_DWS);
    session.GetOwner();
}
/*
TEST_F(BsaSessionTest, TransExistTest)
{
    DoGetJsonStringTest();
    mp_long sessinId;
    BsaSession session(sessinId);
    session.TransExist();
}
*/
TEST_F(BsaSessionTest, ResetTimerTest)
{
    DoGetJsonStringTest();
    mp_long sessinId;
    BsaSession session(sessinId, BSA_AppType::BSA_DWS);
    session.ResetTimer();
}

TEST_F(BsaSessionTest, GetSessionTimeoutInterval)
{
    DoGetJsonStringTest();
    mp_int32 timeoutMs;
    mp_long sessinId;
    BsaSession session(sessinId, BSA_AppType::BSA_DWS);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser,GetValueInt32),
    StubFailed);
    mp_int32 iRet = session.GetSessionTimeoutInterval(timeoutMs);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser,GetValueInt32),
    Stub_CConfigXmlParser_GetValueInt32_Succ_0);
    iRet = session.GetSessionTimeoutInterval(timeoutMs);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser,GetValueInt32),
    Stub_CConfigXmlParser_GetValueInt32_Succ);
    iRet = session.GetSessionTimeoutInterval(timeoutMs);
    EXPECT_EQ(iRet, MP_SUCCESS);
}