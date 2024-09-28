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
#include "log/Log.h"
#include "stub.h"
#include "ArchiveDownloadFile.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using namespace FilePlugin;

namespace {
const int SUCCESS = 0;
const int FAILED = -1;
}

int ret_tag = SUCCESS;

class ArchiveClientTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    std::unique_ptr<ArchiveClient> m_client = nullptr;
    Stub stub;
};

void ArchiveClientTest::SetUp()
{
    std::string jobId = "job123";
    std::string copyId = "copy123";
    m_client = std::make_unique<ArchiveClient>(jobId, copyId);
    m_client->m_clientHandler = std::make_shared<ArchiveStreamService>();
}

void ArchiveClientTest::TearDown()
{}

void ArchiveClientTest::SetUpTestCase()
{}

void ArchiveClientTest::TearDownTestCase()
{}

int InitClientstub(void* obj){
    return ret_tag;
}

int Connectstub(void* obj){
    return ret_tag;
}

int GetFileDataStub(void* obj, const ArchiveStreamGetFileReq &getFileReq, ArchiveStreamGetFileRsq &getFileRsp)
{
    getFileRsp.data = new char [10];
    return ret_tag;
}

int DisconnectStub(void* obj)
{
    return ret_tag;
}

int EndRecoverStub(void* obj)
{
    return ret_tag;
}

/*
 * 用例名称: 测试归档客户端赋值正常
 * 前置条件：无
 * check点：fsid赋值正常
 */
TEST_F(ArchiveClientTest, SetFsIdTest)
{
    std::string fsId = "fsid123";
    m_client->SetFsId(fsId);
    EXPECT_EQ(m_client->m_fsId, fsId);
}

/*
 * 用例名称: 测试归档客户端启动
 * 前置条件：sdk连接失败
 * check点：初始化客户端返回失败
 */
// TEST_F(ArchiveClientTest, InitClientTest)
// {
//     INFOLOG("enter InitClient test");
//     std::vector<std::string> ipList = {"0.0.0.0"};
//     int port = 100;
//     bool ssl = true;
//     int ret = m_client->InitClient(ipList, port, ssl);
//     EXPECT_EQ(ret, SUCCESS);

//     ret_tag = FAILED;
//     stub.set(ADDR(ArchiveStreamService, Init), InitClientstub);
//     ret = m_client->InitClient(ipList, port, ssl);
//     EXPECT_EQ(ret, FAILED);

//     ret_tag = SUCCESS;
//     stub.set(ADDR(ArchiveStreamService, Init), InitClientstub);
//     stub.set(ADDR(ArchiveStreamService, Connect), Connectstub);
//     ret = m_client->InitClient(ipList, port, ssl);
//     EXPECT_EQ(ret, SUCCESS);
// }

/*
 * 用例名称: 测试获取文件数据
 * 前置条件：无
 * check点：获取数据返回成功/失败
 */
TEST_F(ArchiveClientTest, GetFileDataTest)
{
    INFOLOG("enter GetFileData test");
    ArchiveRequest req;
    ArchiveResponse rsp;
    req.m_fileName = "/home/1.txt";
    req.m_offset = 0;

    ret_tag = SUCCESS;
    stub.set(ADDR(ArchiveStreamService, GetFileData), GetFileDataStub);
    int ret = m_client->GetFileData(req, rsp);
    EXPECT_EQ(ret, FAILED);
    stub.reset(ADDR(ArchiveStreamService, GetFileData));

    ret_tag = FAILED;
    stub.set(ADDR(ArchiveStreamService, GetFileData), GetFileDataStub);
    ret = m_client->GetFileData(req, rsp);
    EXPECT_EQ(ret, FAILED);
    stub.reset(ADDR(ArchiveStreamService, GetFileData));

    req.m_size = 1;
    ret_tag = SUCCESS;
    stub.set(ADDR(ArchiveStreamService, GetFileData), GetFileDataStub);
    ret = m_client->GetFileData(req, rsp);
    EXPECT_EQ(ret, FAILED);
    stub.reset(ADDR(ArchiveStreamService, GetFileData));

    req.m_size = 1;
    req.m_buffer = new uint8_t[2048];
    ret_tag = SUCCESS;
    stub.set(ADDR(ArchiveStreamService, GetFileData), GetFileDataStub);
    ret = m_client->GetFileData(req, rsp);
    EXPECT_EQ(ret, SUCCESS);
    delete[] req.m_buffer;
    req.m_buffer = nullptr;
    stub.reset(ADDR(ArchiveStreamService, GetFileData));
}

 /*
 * 用例名称: 测试DisConnect
 * 前置条件：无
 * check点：返回成功/失败
 */
TEST_F(ArchiveClientTest, DisConnectTest)
{
    // 分支1
    INFOLOG("enter DisConnect test");
    int ret = m_client->Disconnect();
    EXPECT_EQ(ret, SUCCESS);

    //分支2
    ret_tag = SUCCESS;
    m_client->m_isInit = true;
    INFOLOG("m_isInit=%d", m_client->m_isInit);
    stub.set(ADDR(ArchiveStreamService, Disconnect), DisconnectStub);
    ret = m_client->Disconnect();
    EXPECT_EQ(ret, SUCCESS);

    //分支3
    ret_tag = FAILED;
    stub.set(ADDR(ArchiveStreamService, Disconnect), DisconnectStub);
    ret = m_client->Disconnect();
    EXPECT_EQ(ret, FAILED);
}

 /*
 * 用例名称: 测试EndRecover
 * 前置条件：无
 * check点：返回成功/失败
 */
TEST_F(ArchiveClientTest, EndRecoverTest)
{
    INFOLOG("enter EndRecoverTest test");
    ret_tag = SUCCESS;
    stub.set(ADDR(ArchiveStreamService, EndRecover), EndRecoverStub);
    int ret = m_client->EndRecover();
    EXPECT_EQ(ret, SUCCESS);

    ret_tag = FAILED;
    stub.set(ADDR(ArchiveStreamService, EndRecover), EndRecoverStub);
    ret = m_client->EndRecover();
    EXPECT_EQ(ret, FAILED);
}
