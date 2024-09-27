#include "apps/dws/XBSAServer/BsaTransStateTest.h"
namespace {
mp_void LogTest(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...) {}
#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)

mp_uint64 GetTimeSecTest()
{
    return MP_SUCCESS;
}
mp_int32 StateErrTest()
{
    return MP_SUCCESS;
}
}

TEST_F(BsaTransStateBaseTest, CreateObjTest)
{
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateBase base(&action);
    BsaObjInfo obj;
    stub.set(ADDR(BsaTransStateBase, StateErr), StateErrTest);
    base.CreateObj(obj);
}

TEST_F(BsaTransStateBaseTest, SendDataTest)
{
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateBase base(&action);
    stub.set(ADDR(BsaTransStateBase, StateErr), StateErrTest);
    base.SendData();
}

TEST_F(BsaTransStateBaseTest, EndDataTest33)
{
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateBase base(&action);
    stub.set(ADDR(BsaTransStateBase, StateErr), StateErrTest);
    base.EndData();
}

TEST_F(BsaTransStateBaseTest, DelObjTest)
{
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateBase base(&action);
    stub.set(ADDR(BsaTransStateBase, StateErr), StateErrTest);
    mp_uint64 copyId;
    base.DelObj(copyId);
}

TEST_F(BsaTransStateBaseTest, QueryObjTest)
{
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateBase base(&action);
    stub.set(ADDR(BsaTransStateBase, StateErr), StateErrTest);
    BsaObjInfo queryCond;
    BsaObjInfo queryResult;
    base.QueryObj(queryCond, queryResult);
}

TEST_F(BsaTransStateBaseTest, GetObjTest)
{
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateBase base(&action);
    stub.set(ADDR(BsaTransStateBase, StateErr), StateErrTest);
    base.GetObj();
}

TEST_F(BsaTransStateBaseTest, GetNextObjTest)
{
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateBase base(&action);
    stub.set(ADDR(BsaTransStateBase, StateErr), StateErrTest);
    BsaObjInfo queryResult;
    base.GetNextObj(queryResult);
}

TEST_F(BsaTransStateBaseTest, GetDataTest)
{
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateBase base(&action);
    stub.set(ADDR(BsaTransStateBase, StateErr), StateErrTest);
    base.GetData();
}

TEST_F(BsaTransStateBaseTest, DoCreateObjTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateBase base(&action);
    stub.set(ADDR(BsaTransStateBase, StateErr), StateErrTest);
    BsaObjInfo obj;
    base.m_trans->m_lastOpRet = MP_SUCCESS;
    base.DoCreateObj(obj);
}

TEST_F(BsaTransStateBaseTest, DoSendDataTest)
{
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateBase base(&action);
    stub.set(ADDR(BsaTransStateBase, StateErr), StateErrTest);
    base.DoSendData();
}

TEST_F(BsaTransStateBaseTest, DoEndDataTest)
{
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateBase base(&action);
    stub.set(ADDR(BsaTransStateBase, StateErr), StateErrTest);
    base.DoEndData();
}

TEST_F(BsaTransStateBaseTest, DoDelObjTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    mp_uint64 copyId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateBase base(&action);
    base.DoDelObj(copyId);
}

TEST_F(BsaTransStateBaseTest, DoQueryObjTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateBase base(&action);
    BsaObjInfo queryCond;
    BsaObjInfo queryResult;
    base.DoQueryObj(queryCond, queryResult);
}

TEST_F(BsaTransStateBaseTest, DoGetObjTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateBase base(&action);
    base.DoGetObj();
}

TEST_F(BsaTransStateBaseTest, DoGetNextObjTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateBase base(&action);
    BsaObjInfo queryResult;
    base.DoGetNextObj(queryResult);
}

TEST_F(BsaTransStateBaseTest, DoGetDataTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateBase base(&action);
    BsaObjInfo queryResult;
    base.DoGetData();
}

TEST_F(BsaTransStateBaseTest, ChangeStateTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateBase base(&action);
    BsaObjInfo queryResult;
    BsaTransStatus status;
    base.ChangeState(status);
}

TEST_F(BsaTransStateBaseTest, StateErrTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateBase base(&action);
    BsaObjInfo queryResult;
    base.StateErr();
}

TEST_F(BsaTransStateBaseTest, GetNextQueryObjTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateBase base(&action);
    BsaObjInfo queryResult;
    // base.GetNextQueryObj(queryResult);
}

TEST_F(BsaTransStateBaseTest, CreateObjTest1)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateIdle base(&action);
    BsaObjInfo obj;
    base.CreateObj(obj);
}

TEST_F(BsaTransStateBaseTest, QueryObjTest1)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateIdle base(&action);
    BsaObjInfo queryCond;
    BsaObjInfo queryResult;

    base.QueryObj(queryCond, queryResult);
}

TEST_F(BsaTransStateBaseTest, GetObjTest2)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateIdle base(&action);
    BsaObjInfo queryCond;
    BsaObjInfo queryResult;
    base.GetObj();
}

TEST_F(BsaTransStateBaseTest, SendDataTest1)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateCreateObj base(&action);
    base.SendData();
}

TEST_F(BsaTransStateBaseTest, EndDataTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateCreateObj base(&action);
    base.EndData();
}


TEST_F(BsaTransStateBaseTest, EndDataTest2)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateCreateObj base(&action);
    base.EndData();
}

TEST_F(BsaTransStateBaseTest, SendDataTest3)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateSendData base(&action);
    base.SendData();
}

TEST_F(BsaTransStateBaseTest, EndDataTest3)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateDelObj base(&action);
    base.EndData();
}

TEST_F(BsaTransStateBaseTest, GetObjTest3)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateQueryObj base(&action);
    base.GetObj();
}

TEST_F(BsaTransStateBaseTest, GetNextObjTest3)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateQueryObj base(&action);
    BsaObjInfo queryResult;
    base.GetNextObj(queryResult);
}

TEST_F(BsaTransStateBaseTest, EndDataTest4)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateQueryObj base(&action);
    base.EndData();
}

TEST_F(BsaTransStateBaseTest, GetDataTest4)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateGetObj base(&action);
    base.GetData();
}

TEST_F(BsaTransStateBaseTest, GetNextObjTest4)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateGetObj base(&action);
    BsaObjInfo queryResult;
    base.GetNextObj(queryResult);
}

TEST_F(BsaTransStateBaseTest, EndDataTest5)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateGetObj base(&action);
    base.EndData();
}

TEST_F(BsaTransStateBaseTest, GetDataTest6)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateGetNextObj base(&action);
    base.GetData();
}


TEST_F(BsaTransStateBaseTest, GetNextObjTest6)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateGetNextObj base(&action);
    BsaObjInfo queryResult;
    base.GetNextObj(queryResult);
}

TEST_F(BsaTransStateBaseTest, EndDataTest6)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateGetNextObj base(&action);
    base.EndData();
}


TEST_F(BsaTransStateBaseTest, GetDataTest7)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateGetData base(&action);
    base.GetData();
}

TEST_F(BsaTransStateBaseTest, EndDataTest7)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStateGetData base(&action);
    base.EndData();
}