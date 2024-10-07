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
#include "mockcpp/mockcpp.hpp"
#include "log/Log.h"
#include "ClientInvoke.h"
#include "FrameworkService.h"

using namespace startup;
using namespace AppProtect;

namespace {
    const int INNER_ERROR = 200;
    const int FAILED = -1;
    const int SUCCESS = 0;
}

class OtherException {
    OtherException() {};
};

/*
 * 用例名称: 测试thrift插件客户端接口
 * 前置条件：未正常启动客户端
 * check点：发送rpc失败
 */
class ShareResourceTest : public testing::Test {
public:
    void SetUp() {
        MOCKER((std::this_thread::sleep_for<int64_t, std::ratio<1>>))
            .stubs()
            .will(ignoreReturnValue());

        MOCKER_CPP(&PluginThriftClient::GetAgentClient<ShareResourceConcurrentClient>)
            .stubs()
            .will(throws(apache::thrift::transport::TTransportException()))
            .then(throws(apache::thrift::protocol::TProtocolException()))
            .then(throws(apache::thrift::TApplicationException()))
            .then(throws(std::exception()))
            .then(throws(OtherException()))
            .then(ignoreReturnValue());
    };
    void TearDown() {
        GlobalMockObject::verify(); // 校验mock规范并清除mock规范
        // GlobalMockObject::reset();  // 清除所有桩
    };
};

TEST_F(ShareResourceTest, CreateResource)
{
    ActionResult actionResult;
    Resource resource;
    std::string jobId;

// 先进入异常分支
    ShareResource::CreateResource(actionResult, resource, jobId);
    // 最后为创建失败，正常退出
    ShareResource::CreateResource(actionResult, resource, jobId);
    EXPECT_EQ(actionResult.code, FAILED);
}

TEST_F(ShareResourceTest, QueryResource)
{
    ResourceStatus returnValue;
    Resource resource;
    std::string jobId;

    // 先进入异常分支
    ShareResource::QueryResource(returnValue, resource, jobId);
    // 最后为创建失败，正常退出
    ShareResource::QueryResource(returnValue, resource, jobId);
    EXPECT_EQ(returnValue.lockNum, 0);
}

TEST_F(ShareResourceTest, UpdateResource)
{
    ActionResult returnValue;
    Resource resource;
    std::string jobId;

    // 先进入异常分支
    ShareResource::UpdateResource(returnValue, resource, jobId);
    // 最后为创建失败，正常退出
    ShareResource::UpdateResource(returnValue, resource, jobId);
    EXPECT_EQ(returnValue.code, FAILED);
}

TEST_F(ShareResourceTest, DeleteResource)
{
    ActionResult returnValue;
    Resource resource;
    std::string jobId;

    // 先进入异常分支
    ShareResource::DeleteResource(returnValue, resource, jobId);
    // 最后为创建失败，正常退出
    ShareResource::DeleteResource(returnValue, resource, jobId);
    EXPECT_EQ(returnValue.code, FAILED);
}

TEST_F(ShareResourceTest, LockResource)
{
    ActionResult returnValue;
    Resource resource;
    std::string jobId;

    // 先进入异常分支
    ShareResource::LockResource(returnValue, resource, jobId);
    // 最后为创建失败，正常退出
    ShareResource::LockResource(returnValue, resource, jobId);
    EXPECT_EQ(returnValue.code, FAILED);
}

TEST_F(ShareResourceTest, UnLockResource)
{
    ActionResult returnValue;
    Resource resource;
    std::string jobId;

    // 先进入异常分支
    ShareResource::UnLockResource(returnValue, resource, jobId);
    // 最后为创建失败，正常退出
    ShareResource::UnLockResource(returnValue, resource, jobId);
    EXPECT_EQ(returnValue.code, FAILED);
}

class JobServiceTest : public testing::Test {
public:
    void SetUp() {
        MOCKER((std::this_thread::sleep_for<int64_t, std::ratio<1>>))
            .stubs()
            .will(ignoreReturnValue());

        MOCKER_CPP(&PluginThriftClient::GetAgentClient<JobServiceConcurrentClient>)
            .stubs()
            .will(throws(apache::thrift::transport::TTransportException()))
            .then(throws(apache::thrift::protocol::TProtocolException()))
            .then(throws(apache::thrift::TApplicationException()))
            .then(throws(std::exception()))
            .then(throws(OtherException()))
            .then(ignoreReturnValue());
    }
    void TearDown() {
        GlobalMockObject::verify(); // 校验mock规范并清除mock规范
    };
};

TEST_F(JobServiceTest, AddNewJob)
{
    ActionResult returnValue;
    std::vector<SubJob> job;

    // 先进入异常分支
    JobService::AddNewJob(returnValue, job);
    // 最后为创建失败，正常退出
    JobService::AddNewJob(returnValue, job);
    EXPECT_EQ(returnValue.code, FAILED);
}

TEST_F(JobServiceTest, ReportJobDetails)
{
    ActionResult returnValue;
    SubJobDetails jobInfo;

    // 先进入异常分支
    JobService::ReportJobDetails(returnValue, jobInfo);
    // 最后为创建失败，正常退出
    JobService::ReportJobDetails(returnValue, jobInfo);
    EXPECT_EQ(returnValue.code, FAILED);
}

TEST_F(JobServiceTest, ReportCopyAdditionalInfo)
{
    ActionResult returnValue;
    std::string jobId;
    Copy copy;

    // 先进入异常分支
    JobService::ReportCopyAdditionalInfo(returnValue, jobId, copy);
    // 最后为创建失败，正常退出
    JobService::ReportCopyAdditionalInfo(returnValue, jobId, copy);
    EXPECT_EQ(returnValue.code, FAILED);
}

TEST_F(JobServiceTest, ComputerFileLocationInMultiFileSystem)
{
    std::map<std::string, std::string> returnValue;
    std::vector<std::string> files;
    std::vector<std::string> fileSystems;

    // 先进入异常分支
    JobService::ComputerFileLocationInMultiFileSystem(returnValue, files, fileSystems);
    // 最后为创建失败，正常退出
    JobService::ComputerFileLocationInMultiFileSystem(returnValue, files, fileSystems);
    EXPECT_EQ(returnValue.size(), 0);
}

TEST_F(JobServiceTest, QueryPreviousCopy)
{
    Copy returnValue;
    Application application;
    std::set<AppProtect::CopyDataType> types;
    std::string copyId;
    std::string jobId;

// 先进入异常分支
    JobService::QueryPreviousCopy(returnValue, application, types, copyId, jobId);
    // 最后为创建失败，正常退出
    JobService::QueryPreviousCopy(returnValue, application, types, copyId, jobId);
    EXPECT_EQ(returnValue.id, "");
}

TEST_F(JobServiceTest, MountRepositoryByPlugin)
{
    ActionResult returnValue;
    PrepareRepositoryByPlugin mountinfo;

    // 先进入异常分支
    JobService::MountRepositoryByPlugin(returnValue, mountinfo);
    // 最后为创建失败，正常退出
    JobService::MountRepositoryByPlugin(returnValue, mountinfo);
    EXPECT_EQ(returnValue.code, FAILED);
}

TEST_F(JobServiceTest, UnMountRepositoryByPlugin)
{
    ActionResult returnValue;
    PrepareRepositoryByPlugin mountinfo;

    // 先进入异常分支
    JobService::UnMountRepositoryByPlugin(returnValue, mountinfo);
    // 最后为创建失败，正常退出
    JobService::UnMountRepositoryByPlugin(returnValue, mountinfo);
    EXPECT_EQ(returnValue.code, FAILED);
}

class RegisterPluginServiceTest : public testing::Test {
public:
    void SetUp() {
        MOCKER((std::this_thread::sleep_for<int64_t, std::ratio<1>>))
            .stubs()
            .will(ignoreReturnValue());

        MOCKER_CPP(&PluginThriftClient::GetAgentClient<RegisterPluginServiceConcurrentClient>)
            .stubs()
            .will(throws(apache::thrift::transport::TTransportException()))
            .then(throws(apache::thrift::protocol::TProtocolException()))
            .then(throws(apache::thrift::TApplicationException()))
            .then(throws(std::exception()))
            .then(throws(OtherException()))
            .then(ignoreReturnValue());
    }
    void TearDown() {
        GlobalMockObject::verify(); // 校验mock规范并清除mock规范
    };
};

TEST_F(RegisterPluginServiceTest, RegisterPlugin)
{
    ActionResult returnValue;
    ApplicationPlugin plugin;

    // 先进入异常分支
    RegisterPluginService::RegisterPlugin(returnValue, plugin);
    // 最后为创建失败，正常退出
    RegisterPluginService::RegisterPlugin(returnValue, plugin);
    EXPECT_EQ(returnValue.code, FAILED);
}

TEST_F(RegisterPluginServiceTest, UnRegisterPlugin)
{
    ActionResult returnValue;
    ApplicationPlugin plugin;

    // 先进入异常分支
    RegisterPluginService::UnRegisterPlugin(returnValue, plugin);
    // 最后为创建失败，正常退出
    RegisterPluginService::UnRegisterPlugin(returnValue, plugin);
    EXPECT_EQ(returnValue.code, FAILED);
}

class SecurityServiceTest : public testing::Test {
public:
    void SetUp() {
        MOCKER((std::this_thread::sleep_for<int64_t, std::ratio<1>>))
            .stubs()
            .will(ignoreReturnValue());

        MOCKER_CPP(&PluginThriftClient::GetAgentClient<SecurityServiceConcurrentClient>)
            .stubs()
            .will(throws(apache::thrift::transport::TTransportException()))
            .then(throws(apache::thrift::protocol::TProtocolException()))
            .then(throws(apache::thrift::TApplicationException()))
            .then(throws(std::exception()))
            .then(throws(OtherException()))
            .then(ignoreReturnValue());
    }
    void TearDown() {
        GlobalMockObject::verify(); // 校验mock规范并清除mock规范
    };
};

TEST_F(SecurityServiceTest, CheckCertThumbPrint)
{
    ActionResult returnValue;
    std::string ip;
    int32_t port;
    std::string thumbPrint;

    // 先进入异常分支
    SecurityService::CheckCertThumbPrint(returnValue, ip, port, thumbPrint);
    // 最后为创建失败，正常退出
    SecurityService::CheckCertThumbPrint(returnValue, ip, port, thumbPrint);
    EXPECT_EQ(returnValue.code, FAILED);
}

class HeartBeatTest : public testing::Test {
public:
    void SetUp() {
        MOCKER((std::this_thread::sleep_for<int64_t, std::ratio<1>>))
            .stubs()
            .will(ignoreReturnValue());

        MOCKER_CPP(&PluginThriftClient::GetAgentClient<AppProtect::FrameworkServiceConcurrentClient>)
            .stubs()
            .will(throws(apache::thrift::transport::TTransportException()))
            .then(throws(apache::thrift::protocol::TProtocolException()))
            .then(throws(apache::thrift::TApplicationException()))
            .then(throws(std::exception()))
            .then(throws(OtherException()))
            .then(ignoreReturnValue());
    }
    void TearDown() {
        GlobalMockObject::verify(); // 校验mock规范并清除mock规范
    };
};

TEST_F(HeartBeatTest, HeartBeat)
{
    ActionResult returnValue;

    // 先进入异常分支
    FrameworkService::HeartBeat(returnValue);
    FrameworkService::HeartBeat(returnValue);
    FrameworkService::HeartBeat(returnValue);
    FrameworkService::HeartBeat(returnValue);
    FrameworkService::HeartBeat(returnValue);
    // 最后为创建失败，正常退出
    FrameworkService::HeartBeat(returnValue);
    EXPECT_EQ(returnValue.code, FAILED);
}
