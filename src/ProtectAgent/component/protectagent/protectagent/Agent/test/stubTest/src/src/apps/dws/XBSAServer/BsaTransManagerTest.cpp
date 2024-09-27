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
#include "apps/dws/XBSAServer/BsaTransManagerTest.h"
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
}

mp_int32 CreateObjTestS(BsaObjInfo &obj)
{
    return MP_SUCCESS;
}

TEST_F(BsaTransactionTest, CreateObjTest)
{
    DoGetJsonStringTest();
    mp_long sessionId = 1;
    mp_long transId = 1;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    
    BsaObjInfo obj;
    // action.CreateObj(obj);
}

TEST_F(BsaTransactionTest, SendDataTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaObjInfo obj;
    // action.SendData();
}

TEST_F(BsaTransactionTest, EndDataTestA)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaObjInfo obj;
    // action.EndData();
}

TEST_F(BsaTransactionTest, DelObjTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    mp_uint64 copyId;
    // action.DelObj(copyId);
}

TEST_F(BsaTransactionTest, QueryObjTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaObjInfo queryCond;
    BsaObjInfo queryResult;
    action.QueryObj(queryCond, queryResult);
}

TEST_F(BsaTransactionTest, GetObjTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    // action.GetObj();
}

TEST_F(BsaTransactionTest, GetNextObjTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaObjInfo queryResult;
    action.GetNextObj(queryResult);
}

TEST_F(BsaTransactionTest, GetDataTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    // action.GetData();
}

TEST_F(BsaTransactionTest, BeginTxnTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    action.BeginTxn();
}

TEST_F(BsaTransactionTest, EndTxnTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BSA_Vote vote;
    action.EndTxn(vote);
    vote = BSA_Vote_COMMIT;
    action.EndTxn(vote);
}

TEST_F(BsaTransactionTest, ChangeStateTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaTransStatus status;
    action.ChangeState(status);
    status = BSA_TRANS_CREATE_OBJ;
    action.ChangeState(status);
    status = BSA_TRANS_QUERY_OBJ;
    action.ChangeState(status);
    status = BSA_TRANS_GET_NEXT_OBJ;
    action.ChangeState(status);
    status = BSA_TRANS_GET_NEXT_OBJ;
    action.ChangeState(status);
    // action.ChangeState(-1);
}

TEST_F(BsaTransactionTest, GetCurStatusTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    action.GetCurStatus();
}

TEST_F(BsaTransactionTest, GetSessionIdTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    action.GetSessionId();
}

TEST_F(BsaTransactionTest, GetTransIdTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    action.GetTransId();
}

TEST_F(BsaTransactionTest, GetStartTimeTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    action.GetStartTime();
}

TEST_F(BsaTransactionTest, GetTaskIdTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    action.GetTaskId();
}

TEST_F(BsaTransactionTest, GetQueryListTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    action.GetQueryList();
}

TEST_F(BsaTransactionTest, IsLastOpSuccTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    action.IsLastOpSucc();
}

TEST_F(BsaTransactionTest, SetLastOpRetTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    mp_int32 ret;
    action.SetLastOpRet(ret);
}

TEST_F(BsaTransactionTest, AddCreatListTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaObjInfo obj;
    action.AddCreatList(obj);
}

TEST_F(BsaTransactionTest, AddDelListTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaObjInfo obj;
    action.AddDelList(obj);
}

TEST_F(BsaTransactionTest, InitStateMachineTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    action.InitStateMachine();
}

TEST_F(BsaTransactionTest, TxnEndNormalTest)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    // action.TxnEndNormal();
}

TEST_F(BsaTransactionTest, CreateObjTestA)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    BsaObjInfo obj;
    action.CreateObj(obj);
}

TEST_F(BsaTransactionTest, SendDataTestA)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    action.SendData();
}

TEST_F(BsaTransactionTest, EndDataTestAA)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    action.EndData();
}


TEST_F(BsaTransactionTest, DelObjTestA)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    mp_uint64 copyId;
    action.DelObj(copyId);
}

TEST_F(BsaTransactionTest, GetObjTestA)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    action.GetObj();
}


TEST_F(BsaTransactionTest, GetDataTestA)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    action.GetData();
}

TEST_F(BsaTransactionTest, TxnCanCommitTestA)
{
    DoGetJsonStringTest();
    mp_long sessionId;
    mp_long transId;
    stub.set(ADDR(CMpTime, GetTimeSec), GetTimeSecTest);
    BsaTransaction action(sessionId, transId);
    action.TxnCanCommit();
}