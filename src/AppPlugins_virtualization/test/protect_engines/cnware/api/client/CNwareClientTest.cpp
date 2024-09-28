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
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include <protect_engines/cnware/api/client/CNwareClient.h>

using namespace VirtPlugin;
using namespace CNwarePlugin;

namespace HDT_TEST {

class CNwareClientTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void InitLogger();

public:
    Stub stub;

private:
    AuthObj m_userInfo;
    Authentication m_auth;
    ApplicationEnvironment m_appEnv;
    CNwareRequest m_req;
    std::tuple<std::string, std::string> m_tupleSession("8.40.162.56", "admin");
    std::shared_ptr<CNwareClient> m_client;
}

void CNwareClientTest::SetUp()
{
    InitLogger();
    stub.set(sleep, Stub_Sleep);
    std::shared_ptr<ResponseModel> response;
    m_client = std::make_shared<CNwareClient>(m_auth);
    m_userInfo.name = "admin";
    m_userInfo.passwd = "007";
}

void CNwareClientTest::InitLogger()
{
    std::string logFileName = "virt_plugin_cnware_client_test.log";
    std::string logFilePath = "/tmp/log/";
    int logLevel = DEBUG;
    int logFileCount = 10;
    int logFileSize = 30;
    Module::CLogger::GetInstance().Init(
        logFileName.c_str(), logFilePath, logLevel, logFileCount, logFileSize);
}

int32_t StubCallApiF(RequestInfo &requestInfo, std::shared_ptr<ResponseModel> response, ModelBase &model)
{
    return FAILED;
}

int32_t StubCallApiT(RequestInfo &requestInfo, std::shared_ptr<ResponseModel> response, ModelBase &model)
{
    return SUCCESS;
}

int32_t StubRefreshSessionF(const std::shared_ptr<ResponseModel> &response, CNwareRequest &req)
{
    return FAILED;
}

int32_t StubRefreshSessionT(const std::shared_ptr<ResponseModel> &response, CNwareRequest &req)
{
    return SUCCESS;
}

TEST_F(CNwareClientTest, CheckParams_Test)
{
    CNwareClient client(auth);
    m_req.m_userInfoIsSet = false;
    EXPECT_EQ(false, client.CheckParams(m_req));

    m_req.m_userInfoIsSet = true;
    m_req.m_endPointIsSet = false;
    EXPECT_EQ(false, client.CheckParamsm_req);

    m_req.m_endPointIsSet = true;
    EXPECT_EQ(true, client.CheckParams(m_req));
}

TEST_F(CNwareClientTest, InitCNwareClient_Test)
{
    CNwareClient client(m_auth);
    EXPECT_EQ(SUCCESS, client.InitCNwareClient(m_appEnv));
}

TEST_F(CNwareClientTest, CheckSessionValidity_Test)
{
    CNwareClient client(m_auth);
    std::shared_ptr<CNwareSession> session = std::make_shared<CNwareSession>("8.40.162.56", "443", "admin", "007");
    client.m_cnwareSessionCache->AddCNwareSession(session);
    EXPECT_EQ(true, client.CheckSessionValidity(m_tupleSession));

    client.m_cnwareSessionCache->EraseSession(session);
    EXPECT_EQ(false, client.CheckSessionValidity(m_tupleSession));
}

TEST_F(CNwareClientTest, GetSessionAndlogin_Test)
{
    m_auth.__set_authkey("agent");
    m_auth.__set_authPwd("007");
    CNwareClient client(m_auth);
    int64_t errorCode;
    m_req.SetEndpoint("8.40.162.56");
    m_userInfo.name = "admin";

    std::shared_ptr<CNwareSession> session = std::make_shared<CNwareSession>("8.40.162.56", "443", "admin", "007");
    client.m_cnwareSessionCache->AddCNwareSession(session);
    EXPECT_EQ(SUCCESS, client.GetSessionAndlogin(m_req, errorCode));

    m_userInfo.name = "root";
    stub.set(ADDR(RestClient, CallApi), StubCallApiF);
    EXPECT_EQ(FAILED, client.GetSessionAndlogin(m_req, errorCode));

    stub.set(ADDR(RestClient, CallApi), StubCallApiT);
    stub.set(ADDR(CNwareClient, RefreshSession), StubRefreshSessionF);
    EXPECT_EQ(FAILED, client.GetSessionAndlogin(m_req, errorCode));

    stub.set(ADDR(CNwareClient, RefreshSession), StubRefreshSessionT);
    EXPECT_EQ(FAILED, client.GetSessionAndlogin(m_req, errorCode));
}

TEST_F(CNwareClientTest, RefreshSession_Test)
{
    CNwareClient client(m_auth);
    std::shared_ptr<CNwareSession> session = std::make_shared<CNwareSession>("8.40.162.56", "443", "admin", "007");
    client.m_cnwareSessionCache->AddCNwareSession(session);
    EXPECT_EQ(true, client.RefreshSession(m_tupleSession));

    client.m_cnwareSessionCache->EraseSession(session);
    EXPECT_EQ(false, client.RefreshSession(m_tupleSession));
}

bool CheckParamsSuccess(ModelBase &model)
{
    return true;
}
bool CheckParamsFail(ModelBase &model)
{
    return false;
}

int32_t SendRequestSuccess(std::shared_ptr<ResponseModel> response, CNwareRequest &req,
    RequestInfo &requestInfo, std::string &errorDes, int64_t &errorCode)
{
    return SUCCESS;
}
int32_t SendRequestFail(std::shared_ptr<ResponseModel> response, CNwareRequest &req,
    RequestInfo &requestInfo, std::string &errorDes, int64_t &errorCode)
    return FAILED;
}

TEST_F(CNwareClientTest, AttachVolumeSnapshot_Test)
{
    CNwareClient client(m_auth);
    CNwareRequest &req;
    std::shared_ptr<ResponseModel> response;
    std::string domainId = "8.8.8.8";
 
    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsFail);
    EXPECT_EQ(SUCCESS, client.DeleteDisk(req, response, domainId));
 
    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsSuccess);
    stub.set(ADDR(CNwareClient, SendRequest), SendRequestFail);
    EXPECT_EQ(SUCCESS, client.DeleteDisk(req, response, domainId));
 
    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsSuccess);
    stub.set(ADDR(CNwareClient, SendRequest), SendRequestSuccess);
    EXPECT_EQ(SUCCESS, client.DeleteDisk(req, response, domainId));
 
    response = nullptr;
}

TEST_F(CNwareClientTest, AddDisk_Test)
{
    CNwareClient client(m_auth);
    CNwareRequest &req;
    std::shared_ptr<ResponseModel> response;
    std::string domainId = "8.8.8.8";
 
    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsFail);
    EXPECT_EQ(SUCCESS, client.DeleteDisk(req, response, domainId));
 
    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsSuccess);
    stub.set(ADDR(CNwareClient, SendRequest), SendRequestFail);
    EXPECT_EQ(SUCCESS, client.DeleteDisk(req, response, domainId));
 
    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsSuccess);
    stub.set(ADDR(CNwareClient, SendRequest), SendRequestSuccess);
    EXPECT_EQ(SUCCESS, client.DeleteDisk(req, response, domainId));
 
    response = nullptr;
}

TEST_F(CNwareClientTest, CheckNameUnique_Test)
{
    CNwareClient client(m_auth);
    CNwareRequest &req;
    std::shared_ptr<ResponseModel> response;
    std::string name = "test";
    std::string domainName = "testDomin";

    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsFail);
    EXPECT_EQ(SUCCESS, client.CheckNameUnique(req, response, name, domainName));

    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsSuccess);
    stub.set(ADDR(CNwareClient, SendRequest), SendRequestFail);
    EXPECT_EQ(SUCCESS, client.CheckNameUnique(req, response, name, domainName));

    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsSuccess);
    stub.set(ADDR(CNwareClient, SendRequest), SendRequestSuccess);
    EXPECT_EQ(SUCCESS, client.CheckNameUnique(req, response, name, domainName));

    response = nullptr;
}

TEST_F(CNwareClientTest, GetClientInfo_Test)
{
    CNwareClient client(m_auth);
    CNwareRequest &req;
    std::shared_ptr<ResponseModel> response;
    std::string domainId="8.8.8.8";

    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsFail);
    EXPECT_EQ(FAILED, client.GetClientInfo(req, response, domainId));

    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsSuccess);
    stub.set(ADDR(CNwareClient, SendRequest), SendRequestFail);
    EXPECT_EQ(FAILED, client.GetClientInfo(req, response, domainId));

    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsSuccess);
    stub.set(ADDR(CNwareClient, SendRequest), SendRequestSuccess);
    EXPECT_EQ(SUCCESS, client.GetClientInfo(req, response, domainId));

    response = nullptr;
}

TEST_F(CNwareClientTest, DeleteDisk_Test)
{
    CNwareClient client(m_auth);
    CNwareRequest &req;
    std::shared_ptr<ResponseModel> response;
    std::string domainId="8.8.8.8";
    std::string busDev = "virtio-vdb";

    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsFail);
    EXPECT_EQ(SUCCESS, client.DeleteDisk(req, response, domainId, busDev));

    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsSuccess);
    stub.set(ADDR(CNwareClient, SendRequest), SendRequestFail);
    EXPECT_EQ(SUCCESS, client.DeleteDisk(req, response, domainId, busDev));

    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsSuccess);
    stub.set(ADDR(CNwareClient, SendRequest), SendRequestSuccess);
    EXPECT_EQ(SUCCESS, client.DeleteDisk(req, response, domainId, busDev));

    response = nullptr;
}
TEST_F(CNwareClientTest, UninstallOrDeleteDiskVolId_Test)
{
    CNwareClient client(m_auth);
    CNwareRequest &req;
    std::shared_ptr<ResponseModel> response;
    std::string domainId="8.8.8.8";
    std::string volId = "test-123-456-789";
    int32_t deleteType = 3;
    EXPECT_EQ(FAILED, client.GetClientInfo(req, response, domainId, volId, deleteType));

    deleteType = 1;
    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsFail);
    EXPECT_EQ(FAILED, client.UninstallOrDeleteDiskVolId(req, response, domainId, volId, deleteType));

    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsSuccess);
    stub.set(ADDR(CNwareClient, SendRequest), SendRequestFail);
    EXPECT_EQ(FAILED, client.UninstallOrDeleteDiskVolId(req, response, domainId, volId, deleteType));

    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsSuccess);
    stub.set(ADDR(CNwareClient, SendRequest), SendRequestSuccess);
    EXPECT_EQ(SUCCESS, client.UninstallOrDeleteDiskVolId(req, response, domainId, volId, deleteType));

    response = nullptr;
}
TEST_F(CNwareClientTest, DeleteSnapshotVolume_Test)
{
    CNwareClient client(m_auth);
    client.m_taskId = "test-test-test-test";
    CNwareRequest &req;
    std::shared_ptr<ResponseModel> response;

    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsFail);
    EXPECT_EQ(FAILED, client.DeleteSnapshotVolume(req, response));

    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsSuccess);
    stub.set(ADDR(CNwareClient, SendRequest), SendRequestFail);
    EXPECT_EQ(FAILED, client.DeleteSnapshotVolume(req, response));

    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsSuccess);
    stub.set(ADDR(CNwareClient, SendRequest), SendRequestSuccess);
    EXPECT_EQ(SUCCESS, client.DeleteSnapshotVolume(req, response));

    response = nullptr;
}

TEST_F(CNwareClientTest, BuildNewClient_Test)
{
    CNwareClient client(m_auth);
    CNwareRequest &req;
    std::shared_ptr<ResponseModel> response;

    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsFail);
    EXPECT_EQ(FAILED, client.BuildNewClient(req, response));

    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsSuccess);
    stub.set(ADDR(CNwareClient, SendRequest), SendRequestFail);
    EXPECT_EQ(FAILED, client.BuildNewClient(req, response));

    stub.set(ADDR(CNwareClient, CheckParams), CheckParamsSuccess);
    stub.set(ADDR(CNwareClient, SendRequest), SendRequestSuccess);
    EXPECT_EQ(SUCCESS, client.BuildNewClient(req, response));

    response = nullptr;
}

}
