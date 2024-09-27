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
#include "apps/dws/XBSAServer/BSAServiceHandler.h"
#include "apps/dws/XBSAServer/BsaSessionManager.h"
#include "apps/dws/XBSAServer/BsaIntfAdaptor.h"
#include "gtest/gtest.h"
#include "stub.h"
#include "common/Log.h"

extern std::shared_ptr<apache::thrift::server::TNonblockingServer> server_ptr;
extern std::shared_ptr<apache::thrift::concurrency::Thread> thread_ptr;

class BSAServiceHandlerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
private:
    CallResult m_callRet;
};

void BSAServiceHandlerTest::SetUp() {}
void BSAServiceHandlerTest::TearDown() {}
void BSAServiceHandlerTest::SetUpTestCase() {}
void BSAServiceHandlerTest::TearDownTestCase() {}

namespace {
mp_void CLogger_Log_Stub(mp_void* pthis)
{
    return;
}

mp_int32 StubSetLastErr(mp_int32 errNo)
{
    return errNo;
}

}

TEST_F(BSAServiceHandlerTest, HeartBeat) {
    BSAServiceHandler om;
    HeartBeatResult _return;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    om.HeartBeat(_return);
}

mp_bool BsaIntfAdaptor_HandleValid_Stub(const int64_t handle)
{
    return true;
}

mp_bool BsaIntfAdaptor_HandleValidFalse_Stub(const int64_t handle)
{
    return false;
}

mp_int32 BsaSessionManager_BeginTxn_Stub(mp_long sessionId)
{
    return BSA_RC_SUCCESS;
}

TEST_F(BSAServiceHandlerTest, BSABeginTxn) {
    BSAServiceHandler om;
    int64_t handle;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);

    stub.set(ADDR(BsaIntfAdaptor, HandleValid), BsaIntfAdaptor_HandleValidFalse_Stub);
    om.BSABeginTxn(m_callRet, handle);
    EXPECT_EQ(m_callRet.response, BSA_RC_INVALID_HANDLE);

    stub.set(ADDR(BsaIntfAdaptor, HandleValid), BsaIntfAdaptor_HandleValid_Stub);
    stub.set(ADDR(BsaSessionManager, BeginTxn), BsaSessionManager_BeginTxn_Stub);
    om.BSABeginTxn(m_callRet, handle);
    EXPECT_EQ(m_callRet.response, BSA_RC_SUCCESS);
}

mp_void BsaSessionManager_CreateObject_Stub(mp_long sessionId, const BsaObjectDescriptor &objDesc, CreateObjectResult &rsp)
{}

TEST_F(BSAServiceHandlerTest, BSACreateObject) {
    BSAServiceHandler om;
    CreateObjectResult _return;
    int64_t handle;
    BsaObjectDescriptor objectDescriptor;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(BsaIntfAdaptor, HandleValid), BsaIntfAdaptor_HandleValid_Stub);
    stub.set(ADDR(BsaSessionManager, CreateObject), BsaSessionManager_CreateObject_Stub);
    om.BSACreateObject(_return, handle, objectDescriptor);

    stub.set(ADDR(BsaIntfAdaptor, HandleValid), BsaIntfAdaptor_HandleValidFalse_Stub);
    om.BSACreateObject(_return, handle, objectDescriptor);
    EXPECT_EQ(_return.response, BSA_RC_INVALID_HANDLE);
}

mp_int32 BsaSessionManager_DeleteObject_Stub(mp_long sessionId)
{
    return BSA_RC_SUCCESS;
}

TEST_F(BSAServiceHandlerTest, BSADeleteObject) {
    BSAServiceHandler om;
    int64_t handle;
    BsaUInt64 copyId;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);

    stub.set(ADDR(BsaIntfAdaptor, HandleValid), BsaIntfAdaptor_HandleValidFalse_Stub);
    om.BSADeleteObject(m_callRet, handle, copyId);
    EXPECT_EQ(m_callRet.response, BSA_RC_INVALID_HANDLE);

    stub.set(ADDR(BsaIntfAdaptor, HandleValid), BsaIntfAdaptor_HandleValid_Stub);
    stub.set(ADDR(BsaSessionManager, DeleteObject), BsaSessionManager_DeleteObject_Stub);
    om.BSADeleteObject(m_callRet, handle, copyId);
    EXPECT_EQ(m_callRet.response, BSA_RC_SUCCESS);
}

mp_int32 BsaSessionManager_EndData_Stub(mp_long bsaHandle, const BsaUInt64 &estimatedSize, int64_t size)
{
    return BSA_RC_SUCCESS;
}

TEST_F(BSAServiceHandlerTest, BSAEndData) {
    BSAServiceHandler om;
    int64_t handle;
    BsaUInt64 estimatedSize;
    int64_t size;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);

    stub.set(ADDR(BsaIntfAdaptor, HandleValid), BsaIntfAdaptor_HandleValidFalse_Stub);
    om.BSAEndData(m_callRet, handle, estimatedSize, size);
    EXPECT_EQ(m_callRet.response, BSA_RC_INVALID_HANDLE);

    stub.set(ADDR(BsaIntfAdaptor, HandleValid), BsaIntfAdaptor_HandleValid_Stub);
    stub.set(ADDR(BsaSessionManager, EndData), BsaSessionManager_EndData_Stub);
    om.BSAEndData(m_callRet, handle, estimatedSize, size);
    EXPECT_EQ(m_callRet.response, BSA_RC_SUCCESS);
}

mp_int32 BsaSessionManager_EndTxn_Stub(mp_long sessionId, mp_int32 vote)
{
    return BSA_RC_SUCCESS;
}

mp_int32 BsaSessionManager_EndTxnInvalid_Stub(mp_long sessionId, mp_int32 vote)
{
    return BSA_RC_INVALID_HANDLE;
}

TEST_F(BSAServiceHandlerTest, BSAEndTxn) {
    BSAServiceHandler om;
    int64_t handle;
    int32_t vote;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);

    stub.set(ADDR(BsaIntfAdaptor, HandleValid), BsaIntfAdaptor_HandleValidFalse_Stub);
    stub.set(ADDR(BsaSessionManager, EndTxn), BsaSessionManager_EndTxnInvalid_Stub);
    om.BSAEndTxn(m_callRet, handle, vote);
    EXPECT_EQ(m_callRet.response, BSA_RC_INVALID_HANDLE);

    stub.set(ADDR(BsaIntfAdaptor, HandleValid), BsaIntfAdaptor_HandleValid_Stub);
    stub.set(ADDR(BsaSessionManager, EndTxn), BsaSessionManager_EndTxn_Stub);
    om.BSAEndTxn(m_callRet, handle, vote);
    EXPECT_EQ(m_callRet.response, BSA_RC_SUCCESS);
}

mp_int32 BsaSessionManager_GetData_Stub(mp_long sessionId, BsaDataBlock32& dataBlock)
{
    return BSA_RC_SUCCESS;
}

TEST_F(BSAServiceHandlerTest, BSAGetData) {
    BSAServiceHandler om;
    GetDataResult _return;
    int64_t handle;
    BsaDataBlock32 dataBlock;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);

    stub.set(ADDR(BsaIntfAdaptor, HandleValid), BsaIntfAdaptor_HandleValidFalse_Stub);
    om.BSAGetData(_return, handle, dataBlock);
    EXPECT_EQ(_return.response, BSA_RC_INVALID_HANDLE);

    stub.set(ADDR(BsaIntfAdaptor, HandleValid), BsaIntfAdaptor_HandleValid_Stub);
    stub.set(ADDR(BsaSessionManager, GetData), BsaSessionManager_GetData_Stub);
    om.BSAGetData(_return, handle, dataBlock);
}

mp_string BsaSessionManager_GetProvider_Stub()
{
    mp_string provider = "";
    return provider;
}

TEST_F(BSAServiceHandlerTest, BSAGetEnvironment) {
    BSAServiceHandler om;
    GetEnvironmentResult _return;
    int64_t handle;
    BsaObjectOwner objectOwner;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(BsaSessionManager, GetProvider), BsaSessionManager_GetProvider_Stub);
    om.BSAGetEnvironment(_return, handle, objectOwner);
}

TEST_F(BSAServiceHandlerTest, BSAGetLastError) {
    BSAServiceHandler om;
    GetLastErrorResult _return;
    int64_t bufferSize;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    om.BSAGetLastError(_return, bufferSize);
}

mp_void BsaSessionManager_GetNextObj_Stub(mp_long sessionId, GetNextQueryObjectResult &rsp)
{}

TEST_F(BSAServiceHandlerTest, BSAGetNextQueryObject) {
    BSAServiceHandler om;
    GetNextQueryObjectResult _return;
    int64_t handle;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);

    stub.set(ADDR(BsaIntfAdaptor, HandleValid), BsaIntfAdaptor_HandleValidFalse_Stub);
    om.BSAGetNextQueryObject(_return, handle);
    EXPECT_EQ(_return.response, BSA_RC_INVALID_HANDLE);

    stub.set(ADDR(BsaIntfAdaptor, HandleValid), BsaIntfAdaptor_HandleValid_Stub);
    stub.set(ADDR(BsaSessionManager, GetNextObj), BsaSessionManager_GetNextObj_Stub);
    om.BSAGetNextQueryObject(_return, handle);
}

mp_void BsaSessionManager_GetObject_Stub(mp_long sessionId, const BsaObjectDescriptor &objDesc, GetObjectResult &rsp)
{}

TEST_F(BSAServiceHandlerTest, BSAGetObject) {
    BSAServiceHandler om;
    GetObjectResult _return;
    int64_t handle;
    BsaObjectDescriptor objectDesc;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(BsaIntfAdaptor, HandleValid), BsaIntfAdaptor_HandleValid_Stub);
    stub.set(ADDR(BsaSessionManager, GetObject), BsaSessionManager_GetObject_Stub);
    om.BSAGetObject(_return, handle, objectDesc);

    stub.set(ADDR(BsaIntfAdaptor, HandleValid), BsaIntfAdaptor_HandleValidFalse_Stub);
    om.BSAGetObject(_return, handle, objectDesc);
    EXPECT_EQ(_return.response, BSA_RC_INVALID_HANDLE);
}

mp_void BsaSessionManager_NewSession_Stub(BSAInitResult& rsp, const BsaObjectOwner& objectOwner, const std::string& env)
{}

TEST_F(BSAServiceHandlerTest, BSAInit) {
    BSAServiceHandler om;
    BSAInitResult _return;
    BsaObjectOwner objectOwner;
    std::string envPtr;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(BsaSessionManager, NewSession), BsaSessionManager_NewSession_Stub);
    om.BSAInit(_return, objectOwner, envPtr, BSA_AppType::BSA_DWS);
}

TEST_F(BSAServiceHandlerTest, BSAQueryApiVersion) {
    BSAServiceHandler om;
    QueryApiVersionResult _return;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    om.BSAQueryApiVersion(_return);
}

mp_void BsaSessionManager_QueryObject_Stub(mp_long sessionId, const BsaQueryDescriptor &queryDesc, QueryObjectResult &rsp)
{}

TEST_F(BSAServiceHandlerTest, BSAQueryObject) {
    BSAServiceHandler om;
    QueryObjectResult _return;
    int64_t handle;
    BsaQueryDescriptor queryDesc;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);

    stub.set(ADDR(BsaIntfAdaptor, HandleValid), BsaIntfAdaptor_HandleValidFalse_Stub);
    om.BSAQueryObject(_return, handle, queryDesc);
    EXPECT_EQ(_return.response, BSA_RC_INVALID_HANDLE);

    stub.set(ADDR(BsaIntfAdaptor, HandleValid), BsaIntfAdaptor_HandleValid_Stub);
    stub.set(ADDR(BsaSessionManager, QueryObject), BsaSessionManager_QueryObject_Stub);
    om.BSAQueryObject(_return, handle, queryDesc);
}

TEST_F(BSAServiceHandlerTest, BSAQueryServiceProvider) {
    BSAServiceHandler om;
    QueryServiceProviderResult _return;
    int64_t retSize;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(BsaSessionManager, GetProvider), BsaSessionManager_GetProvider_Stub);
    om.BSAQueryServiceProvider(_return, retSize);
}

mp_int32 BsaSessionManager_SendData_Stub(mp_long sessionId, const BsaDataBlock32& dataBlock)
{
    return BSA_RC_SUCCESS;
}

TEST_F(BSAServiceHandlerTest, BSASendData) {
    BSAServiceHandler om;
    int64_t handle;
    BsaDataBlock32 dataBlock;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);

    stub.set(ADDR(BsaIntfAdaptor, HandleValid), BsaIntfAdaptor_HandleValidFalse_Stub);
    om.BSASendData(m_callRet, handle, dataBlock);
    EXPECT_EQ(m_callRet.response, BSA_RC_INVALID_HANDLE);

    stub.set(ADDR(BsaIntfAdaptor, HandleValid), BsaIntfAdaptor_HandleValid_Stub);
    stub.set(ADDR(BsaSessionManager, SendData), BsaSessionManager_SendData_Stub);
    om.BSASendData(m_callRet, handle, dataBlock);
    EXPECT_EQ(m_callRet.response, BSA_RC_SUCCESS);
}

mp_int32 BsaSessionManager_CloseSession_Stub(mp_long sessionId)
{
    return BSA_RC_SUCCESS;
}

mp_int32 BsaSessionManager_CloseSessionInvalid_Stub(mp_long sessionId)
{
    return BSA_RC_INVALID_HANDLE;
}

TEST_F(BSAServiceHandlerTest, BSATerminate) {
    BSAServiceHandler om;
    int64_t handle;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    
    stub.set(ADDR(BsaIntfAdaptor, HandleValid), BsaIntfAdaptor_HandleValidFalse_Stub);
    stub.set(ADDR(BsaSessionManager, CloseSession), BsaSessionManager_CloseSessionInvalid_Stub);
    om.BSATerminate(m_callRet, handle);
    EXPECT_EQ(m_callRet.response, BSA_RC_INVALID_HANDLE);

    stub.set(ADDR(BsaIntfAdaptor, HandleValid), BsaIntfAdaptor_HandleValid_Stub);
    stub.set(ADDR(BsaSessionManager, CloseSession), BsaSessionManager_CloseSession_Stub);
    om.BSATerminate(m_callRet, handle);
    EXPECT_EQ(m_callRet.response, BSA_RC_SUCCESS);
}