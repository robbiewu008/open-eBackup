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
#include <common/JsonHelper.h>
#include "common/CommonMock.h"
#include "protect_engines/openstack/utils/OpenStackTokenMgr.h"
#include "protect_engines/openstack/common/OpenStackMacros.h"
#include "protect_engines/openstack/api/cinder/CinderClient.h"
#include "protect_engines/openstack/consistent_snapshot/OpenStackConsistentSnapshot.h"
#include "config_reader/ConfigIniReader.h"
#include "protect_engines/openstack/api/cinder/model/SnapshotDetails.h"

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;

// using namespace VirtPlugin;
using namespace OpenStackPlugin;
using OpenStackPlugin::ConsistentSnapshotGroupInfo;
using OpenStackPlugin::OpenStackConsistentSnapshot;

namespace HDT_TEST {

bool StubGetConsistentSnapshotToken(void *obj, ModelBase &model, std::string &tokenValue, std::string &endPoint) {
	tokenValue = "stubtokenstring";
	endPoint = "https://identity.az236.dc236.huawei.com/identity/v3";
	return true;
}

std::string OpenStack_ConfigReaderAPIVersionResult(std::string section, std::string keyName)
{
    return "volume 3.59";
}

class OpenStackConsistentSnapshotTest : public testing::Test {
protected:
    void SetUp()
    {
		m_stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CreateClient_SendRequest);
		m_stub.set(ADDR(OpenStackPlugin::OpenStackTokenMgr, GetToken), StubGetConsistentSnapshotToken);
        m_stub.set(ADDR(Module::ConfigReader, getString), OpenStack_ConfigReaderAPIVersionResult);
    };
    void TearDown()
    {
        m_stub.reset(ADDR(Module::IHttpClient, GetInstance));
		m_stub.reset(ADDR(OpenStackPlugin::OpenStackTokenMgr, GetToken));
        m_stub.reset(sleep);
        m_stub.reset(ADDR(Module::ConfigReader, getString));
    };
    Stub m_stub;
};

void StubSetConsistentSnapshotAppEnvUseCertInfo(ApplicationEnvironment& appEnv)
{
    appEnv.__set_id("136");
    appEnv.__set_name("OpenStackPlugin");
    appEnv.__set_type("Virtual");
    appEnv.__set_endpoint("https://10.9.6.2:443/v2/13f648f23eac4f3abc870eef7f41bc56");
    appEnv.auth.__set_authkey("admin");
    appEnv.auth.__set_authPwd("xxxxxxxx");
	appEnv.auth.__set_extendInfo(
		"{\"certification\":\"\",\"enableCert\":\"0\",\"revocationlist\":\"\"}");
    appEnv.__set_extendInfo(
        "{\"projectId\":\"projectId\",\"domainName\":\"domains\"}");
}

void StubSetConsistentSnapshotApplication(Application& application)
{
    application.__set_id("136");
    application.__set_name("OpenStackPlugin");
    application.__set_type("Virtual");
    application.__set_parentId("parentId");
    application.auth.__set_authkey("admin");
    application.auth.__set_authPwd("xxxxxxxx");
    application.__set_extendInfo(
        "{\"projectId\":\"projectId\",\"domainName\":\"domains\"}");
}

void SetOpenstackConsistentSnapshotParam(OpenStackConsistentSnapshot &openstackConstSnapshot)
{
    std::string m_requestId = "12345678";
    StorageRepository repo;
    repo.__set_repositoryType(RepositoryDataType::META_REPOSITORY);
    repo.path.push_back("/tmp/");
    std::vector<AppProtect::StorageRepository> backupRepos;
    backupRepos.push_back(repo);

    std::shared_ptr<VirtPlugin::CertManger> certMgr = std::make_shared<VirtPlugin::CertManger>();
    bool ret = openstackConstSnapshot.InitParam(backupRepos, certMgr, m_requestId);
    EXPECT_EQ(ret, true);
}
int32_t g_restSendCount = 0;
const std::string getGroupTypeResponseBody = {
"{"
    "\"group_type\": {"
        "\"id\": \"6685584b-1eac-4da6-b5c3-555430cf68ff\","
        "\"name\": \"grp-type-001\","
        "\"description\": \"group type 001\","
        "\"is_public\": true,"
        "\"group_specs\": {"
            "\"consistent_group_snapshot_enabled\": \"<is> False\""
        "}"
    "}"
"}"
};

const std::string getGroupResponseBody = {
"{"
    "\"group\": {"
        "\"id\": \"6f519a48-3183-46cf-a32f-41815f813986\","
        "\"status\": \"available\","
        "\"availability_zone\": \"az1\","
        "\"created_at\": \"2015-09-16T09:28:52.000000\","
        "\"name\": \"first_group\","
        "\"description\": \"my first group\","
        "\"group_type\": \"29514915-5208-46ab-9ece-1cc4688ad0c1\","
        "\"volume_types\": ["
            "\"c4daaf47-c530-4901-b28e-f5f0a359c4e6\""
        "],"
        "\"volumes\": [\"a2cdf1ad-5497-4e57-bd7d-f573768f3d03\"],"
        "\"group_snapshot_id\": null,"
        "\"source_group_id\": null,"
        "\"project_id\": \"7ccf4863071f44aeb8f141f65780c51b\""
    "}"
"}"
};

const std::string getGroupSnapshotsBody = {
"{"
    "\"group_snapshot\": {"
        "\"id\": \"6f519a48-3183-46cf-a32f-41815f816666\","
        "\"name\": \"first_group_snapshot\","
        "\"status\": \"available\","
        "\"group_type_id\": \"58737af7-786b-48b7-ab7c-2447e74b0ef4\""
    "}"
"}"
};

const std::string getSnapshotsBody = {
"{"
    "\"snapshots\":["
        "{"
            "\"created_at\": \"2019-03-11T16:29:08.973832\","
            "\"description\": \"Daily backup\","
            "\"id\": \"2c228773-50eb-422d-be7e-b5c6ced0c7a9\","
            "\"metadata\": {"
                "\"key\": \"v3\""
            "},"
            "\"name\": \"snap-001\","
            "\"size\": 10,"
            "\"status\": \"creating\","
            "\"updated_at\": null,"
            "\"volume_id\": \"428ec041-b999-40d8-8a54-9e98b19406cc\""
        "}"
    "]"
"}"
};

/*
 * 测试用例： 1. 创建GroupType
 * 前置条件:  调用rest创建CreateGroupType成功
 * CHECK点: 
 */
TEST_F(OpenStackConsistentSnapshotTest, DoCreateGroupType_SUCC)
{
    ApplicationEnvironment appEnv;
    Application application;
	StubSetConsistentSnapshotAppEnvUseCertInfo(appEnv);
    StubSetConsistentSnapshotApplication(application);
    OpenStackConsistentSnapshot openstackConstSnapshot(appEnv, application);
    SetOpenstackConsistentSnapshotParam(openstackConstSnapshot);

    g_httpStatusCode = 202;
    g_httpResponsebody = getGroupTypeResponseBody;
    bool ret = openstackConstSnapshot.DoCreateGroupType();
    EXPECT_EQ(ret, true);
}

/*
 * 测试用例： 1. 创建GroupType失败
 * 前置条件:  调用rest创建GroupType失败
 * CHECK点: 
 */
TEST_F(OpenStackConsistentSnapshotTest, DoCreateGroupType_FAILED)
{
    ApplicationEnvironment appEnv;
    Application application;
	StubSetConsistentSnapshotAppEnvUseCertInfo(appEnv);
    StubSetConsistentSnapshotApplication(application);
    OpenStackConsistentSnapshot openstackConstSnapshot(appEnv, application);
    SetOpenstackConsistentSnapshotParam(openstackConstSnapshot);

    g_httpStatusCode = 404;
    g_httpResponsebody = "";
    bool ret = openstackConstSnapshot.DoCreateGroupType();
    EXPECT_EQ(ret, false);
}

/*
 * 测试用例： 1. 创建VolumeGroup失败
 * 前置条件:  创建VolumeGroup失败
 * CHECK点: 
 */
TEST_F(OpenStackConsistentSnapshotTest, DoCreateVolumeGroup_FAILED)
{
    ApplicationEnvironment appEnv;
    Application application;
	StubSetConsistentSnapshotAppEnvUseCertInfo(appEnv);
    StubSetConsistentSnapshotApplication(application);
    OpenStackConsistentSnapshot openstackConstSnapshot(appEnv, application);
    SetOpenstackConsistentSnapshotParam(openstackConstSnapshot);

    VolInfo volume;
    volume.m_volumeType = "type";
    std::vector<VolInfo> volList;
    volList.push_back(volume);
    ConsistentSnapshotGroupInfo m_groupInfo;
    openstackConstSnapshot.m_groupInfo = m_groupInfo;
    bool ret = openstackConstSnapshot.DoCreateVolumeGroup(volList);
    EXPECT_EQ(ret, false);

    m_groupInfo.m_volumeType.push_back("type");
    openstackConstSnapshot.m_groupInfo = m_groupInfo;
    g_httpStatusCode = 400;
    g_httpResponsebody = "";
    ret = openstackConstSnapshot.DoCreateVolumeGroup(volList);
    EXPECT_EQ(ret, false);
}

/*
 * 测试用例： 1. 创建VolumeGroup成功
 * 前置条件:  创建VolumeGroup成功
 * CHECK点: 
 */
TEST_F(OpenStackConsistentSnapshotTest, DoCreateVolumeGroup_SUCCESS)
{
    ApplicationEnvironment appEnv;
    Application application;
	StubSetConsistentSnapshotAppEnvUseCertInfo(appEnv);
    StubSetConsistentSnapshotApplication(application);
    OpenStackConsistentSnapshot openstackConstSnapshot(appEnv, application);
    SetOpenstackConsistentSnapshotParam(openstackConstSnapshot);

    VolInfo volume;
    volume.m_volumeType = "type";
    std::vector<VolInfo> volList;
    volList.push_back(volume);
    ConsistentSnapshotGroupInfo m_groupInfo;
    m_groupInfo.m_volumeType.push_back("type");
    openstackConstSnapshot.m_groupInfo = m_groupInfo;
    g_httpStatusCode = 202;
    g_httpResponsebody = getGroupResponseBody;
    bool ret = openstackConstSnapshot.DoCreateVolumeGroup(volList);
    EXPECT_EQ(ret, true);
}

/*
 * 测试用例： 1. 创建VolumeGroup成功
 * 前置条件:  创建VolumeGroup成功
 * CHECK点: 
 */
TEST_F(OpenStackConsistentSnapshotTest, DoAddVolumeListToGroup_SUCCESS)
{
    ApplicationEnvironment appEnv;
    Application application;
	StubSetConsistentSnapshotAppEnvUseCertInfo(appEnv);
    StubSetConsistentSnapshotApplication(application);
    OpenStackConsistentSnapshot openstackConstSnapshot(appEnv, application);
    SetOpenstackConsistentSnapshotParam(openstackConstSnapshot);

    VolInfo volume;
    volume.m_volumeType = "type";
    std::vector<VolInfo> volList;
    volList.push_back(volume);
    ConsistentSnapshotGroupInfo m_groupInfo;
    m_groupInfo.m_volumeType.push_back("type");
    openstackConstSnapshot.m_groupInfo = m_groupInfo;
    g_httpStatusCode = 202;
    g_httpResponsebody = getGroupResponseBody;
    bool ret = openstackConstSnapshot.DoAddVolumeListToGroup(volList);
    EXPECT_EQ(ret, true);
}

/*
 * 测试用例： 更新卷组信息到文件成功
 * 前置条件:  更新卷信息成功
 * CHECK点: 返回SUCCESS
 */
TEST_F(OpenStackConsistentSnapshotTest, UpdateConsistentSnapshotInfoToFileSucc)
{
    ApplicationEnvironment appEnv;
    Application application;
	StubSetConsistentSnapshotAppEnvUseCertInfo(appEnv);
    StubSetConsistentSnapshotApplication(application);
    OpenStackConsistentSnapshot openstackConstSnapshot(appEnv, application);
    SetOpenstackConsistentSnapshotParam(openstackConstSnapshot);

    ConsistentSnapshotGroupInfo newGroupInfo;
    newGroupInfo.m_uuid = "12345678";
    bool ret = openstackConstSnapshot.UpdateConsistentSnapshotInfoToFile(newGroupInfo);
    EXPECT_EQ(ret, true);

    ConsistentSnapshotGroupInfo loadGroupInfo;
    ret = openstackConstSnapshot.LoadConsistentSnapshotInfoFromFile(loadGroupInfo);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(loadGroupInfo.m_uuid, "12345678");

    newGroupInfo.m_groupSnapshotId = "test_snapshot_id";
    ret = openstackConstSnapshot.UpdateConsistentSnapshotInfoToFile(newGroupInfo);
    EXPECT_EQ(ret, true);

    ret = openstackConstSnapshot.LoadConsistentSnapshotInfoFromFile(loadGroupInfo);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(loadGroupInfo.m_groupSnapshotId, "test_snapshot_id");

    ret = openstackConstSnapshot.UpdateConsistentSnapshotInfoToFile(newGroupInfo, true);
    EXPECT_EQ(ret, true);

    ret = openstackConstSnapshot.LoadConsistentSnapshotInfoFromFile(loadGroupInfo);
    EXPECT_EQ(ret, false);
}

/*
 * 测试用例： 1. 创建VolumeGroup成功
 * 前置条件:  创建VolumeGroup成功
 * CHECK点: 
 */
TEST_F(OpenStackConsistentSnapshotTest, DoCreateVolumeGroupSnapshot_SUCCESS)
{
    ApplicationEnvironment appEnv;
    Application application;
	StubSetConsistentSnapshotAppEnvUseCertInfo(appEnv);
    StubSetConsistentSnapshotApplication(application);
    OpenStackConsistentSnapshot openstackConstSnapshot(appEnv, application);
    SetOpenstackConsistentSnapshotParam(openstackConstSnapshot);

    ConsistentSnapshotGroupInfo m_groupInfo;
    m_groupInfo.m_isGroupSnapshotCreated = true;
    openstackConstSnapshot.m_groupInfo = m_groupInfo;
    bool ret = openstackConstSnapshot.DoCreateVolumeGroupSnapShot();
    EXPECT_EQ(ret, true);

    m_groupInfo.m_isGroupSnapshotCreated = false;
    openstackConstSnapshot.m_groupInfo = m_groupInfo;
    g_httpStatusCode = 202;
    g_httpResponsebody = getGroupSnapshotsBody;
    ret = openstackConstSnapshot.DoCreateVolumeGroupSnapShot();
    EXPECT_EQ(ret, true);

    g_httpStatusCode = 404;
    g_httpResponsebody = "";
    m_groupInfo.m_isGroupSnapshotCreated = false;
    openstackConstSnapshot.m_groupInfo = m_groupInfo;
    ret = openstackConstSnapshot.DoCreateVolumeGroupSnapShot();
    EXPECT_EQ(ret, false);
}

/*
 * 测试用例： 1. 获取快照列表
 * 前置条件:  获取快照列表成功
 * CHECK点: 
 */
TEST_F(OpenStackConsistentSnapshotTest, DoGetSnapShotList_SUCCESS)
{
    ApplicationEnvironment appEnv;
    Application application;
	StubSetConsistentSnapshotAppEnvUseCertInfo(appEnv);
    StubSetConsistentSnapshotApplication(application);
    OpenStackConsistentSnapshot openstackConstSnapshot(appEnv, application);
    SetOpenstackConsistentSnapshotParam(openstackConstSnapshot);

    VolInfo volume;
    volume.m_volumeType = "type";
    std::vector<VolInfo> volList;
    volList.push_back(volume);
    ConsistentSnapshotGroupInfo m_groupInfo;
    m_groupInfo.m_groupSnapshotName = "snap-001";
    openstackConstSnapshot.m_groupInfo = m_groupInfo;
    g_httpStatusCode = 200;
    g_httpResponsebody = getSnapshotsBody;
    VirtPlugin::SnapshotInfo snapshot;
    int32_t retval = openstackConstSnapshot.DoGetSnapShotList(volList, snapshot, m_groupInfo);
    EXPECT_EQ(retval, SUCCESS);

    g_httpStatusCode = 404;
    g_httpResponsebody = getSnapshotsBody;
    retval = openstackConstSnapshot.DoGetSnapShotList(volList, snapshot, m_groupInfo);
    EXPECT_EQ(retval, FAILED);
}

static int32_t Stub_SendRequestTo_DeleteGroupSnapshot(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    if (g_restSendCount == 0) { // 第一次删除快照，接口返回202
        response->SetSuccess(true);
        response->SetStatusCode(202);
        response->SetGetBody(getSnapshotsBody);
        g_restSendCount++;
        return SUCCESS;
    }

    if (g_restSendCount >=1) { // 查询状态不在
        response->SetSuccess(true);
        response->SetGetBody(getSnapshotsBody);
        response->SetStatusCode(404);
        g_restSendCount++;
        return SUCCESS;
    }
}

/*
 * 测试用例： 1. 删除卷组快照
 * 前置条件:  删除卷组快照成功
 * CHECK点: 
 */
TEST_F(OpenStackConsistentSnapshotTest, DoDeleteVolumeGroup_SUCCESS)
{
    ApplicationEnvironment appEnv;
    Application application;
	StubSetConsistentSnapshotAppEnvUseCertInfo(appEnv);
    StubSetConsistentSnapshotApplication(application);
    OpenStackConsistentSnapshot openstackConstSnapshot(appEnv, application);
    SetOpenstackConsistentSnapshotParam(openstackConstSnapshot);

    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_DeleteGroupSnapshot);

    g_restSendCount == 0;
    ConsistentSnapshotGroupInfo m_groupInfo;
    m_groupInfo.m_groupId = "group_id";
    openstackConstSnapshot.m_groupInfo = m_groupInfo;
    int32_t retval = openstackConstSnapshot.DoDeleteVolumeGroup(m_groupInfo);
    EXPECT_EQ(retval, SUCCESS);

    g_restSendCount == 0;
    retval = openstackConstSnapshot.DoDeleteVolumeGroup(m_groupInfo);
    EXPECT_EQ(retval, FAILED);
    g_restSendCount == 0;
}

/*
 * 测试用例： 1. 删除GroupType
 * 前置条件:  删除GroupType成功
 * CHECK点: 
 */
TEST_F(OpenStackConsistentSnapshotTest, DoDeleteGroupType_SUCCESS)
{
    ApplicationEnvironment appEnv;
    Application application;
	StubSetConsistentSnapshotAppEnvUseCertInfo(appEnv);
    StubSetConsistentSnapshotApplication(application);
    OpenStackConsistentSnapshot openstackConstSnapshot(appEnv, application);
    SetOpenstackConsistentSnapshotParam(openstackConstSnapshot);

    ConsistentSnapshotGroupInfo m_groupInfo;
    m_groupInfo.m_groupId = "group_id";
    openstackConstSnapshot.m_groupInfo = m_groupInfo;
    g_httpStatusCode = 500;
    int32_t retval = openstackConstSnapshot.DoDeleteGroupType(m_groupInfo);
    EXPECT_EQ(retval, FAILED);

    g_httpStatusCode = 404;
    retval = openstackConstSnapshot.DoDeleteGroupType(m_groupInfo);
    EXPECT_EQ(retval, SUCCESS);
}

static int32_t Stub_SendRequestTo_RemoveVolume(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    if (g_restSendCount == 0) { // 第一次删除快照，接口返回202
        response->SetSuccess(true);
        response->SetStatusCode(202);
        g_restSendCount++;
        return SUCCESS;
    }

    if (g_restSendCount >=1) { // 查询状态不在
        response->SetSuccess(true);
        response->SetGetBody(getSnapshotsBody);
        response->SetStatusCode(200);
        g_restSendCount++;
        return SUCCESS;
    }
}

/*
 * 测试用例： 1. 从卷组中移除卷
 * 前置条件:  从卷组中移除卷成功
 * CHECK点: 
 */
TEST_F(OpenStackConsistentSnapshotTest, RemoveVolumeFromGroup_SUCCESS)
{
    ApplicationEnvironment appEnv;
    Application application;
	StubSetConsistentSnapshotAppEnvUseCertInfo(appEnv);
    StubSetConsistentSnapshotApplication(application);
    OpenStackConsistentSnapshot openstackConstSnapshot(appEnv, application);
    SetOpenstackConsistentSnapshotParam(openstackConstSnapshot);
    Stub stub;
    stub.set(ADDR(HttpClient, Send), Stub_SendRequestTo_RemoveVolume);

    ConsistentSnapshotGroupInfo m_groupInfo;
    m_groupInfo.m_volumeType.push_back("type");
    openstackConstSnapshot.m_groupInfo = m_groupInfo;
    int32_t ret = openstackConstSnapshot.RemoveVolumeFromGroup(m_groupInfo);
    EXPECT_EQ(ret, FAILED);
}

static int32_t Stub_CreateConsistencySnapshot_HttpSend(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    static int consist_count = 0;
    if (consist_count == 0) { 
        response->SetSuccess(true);
        response->SetStatusCode(202);
        response->SetGetBody(getGroupTypeResponseBody);
        consist_count++;
    }
    else if (consist_count <= 5){
        response->SetSuccess(true);
        response->SetStatusCode(202);
        response->SetGetBody(getGroupResponseBody);
        consist_count++;
    }
    else {
        response->SetSuccess(true);
        response->SetStatusCode(200);
        response->SetGetBody(getGroupSnapshotsBody);
        consist_count++;
    }
    return SUCCESS;
}

/*
 * 测试用例： 创建一致性快照
 * 前置条件:  
 * CHECK点: 
 */
TEST_F(OpenStackConsistentSnapshotTest, DoCreateConsistencySnapshot_SUCCESS)
{
    ApplicationEnvironment appEnv;
    Application application;
	StubSetConsistentSnapshotAppEnvUseCertInfo(appEnv);
    StubSetConsistentSnapshotApplication(application);
    OpenStackConsistentSnapshot openstackConstSnapshot(appEnv, application);
    SetOpenstackConsistentSnapshotParam(openstackConstSnapshot);
    VolInfo volume;
    volume.m_volumeType = "type";
    std::vector<VolInfo> volList;
    volList.push_back(volume);
    ConsistentSnapshotGroupInfo m_groupInfo;
    m_groupInfo.m_volumeType.push_back("type");
    m_groupInfo.m_groupsnapshotStatus = "available";
    openstackConstSnapshot.m_groupInfo = m_groupInfo;
    VirtPlugin::SnapshotInfo snapshot;
    std::string errcode = "false";
    m_stub.set(ADDR(HttpClient, Send), Stub_CreateConsistencySnapshot_HttpSend);
    bool ret = openstackConstSnapshot.DoCreateConsistencySnapshot(volList, snapshot, errcode);
    EXPECT_EQ(ret, true);
    m_stub.reset(ADDR(HttpClient, Send));
}

static int32_t Stub_DeleteConsistencySnapshot_HttpSend(
    void *obj, Module::HttpRequest &httpParam, std::shared_ptr<ResponseModel> response)
{
    static int consist_count = 0;
    if (consist_count == 0) { 
        response->SetSuccess(true);
        response->SetStatusCode(202);
        response->SetGetBody(getGroupSnapshotsBody);
        consist_count++;
    }
    else if (consist_count <= 3){
        response->SetSuccess(true);
        response->SetStatusCode(404);
        response->SetGetBody(getGroupResponseBody);
        consist_count++;
    }
    else if (consist_count == 4){
        response->SetSuccess(true);
        response->SetStatusCode(202);
        response->SetGetBody(getGroupSnapshotsBody);
        consist_count++;
    }
    else if (consist_count <= 6){
        response->SetSuccess(true);
        response->SetStatusCode(404);
        response->SetGetBody(getGroupSnapshotsBody);
        consist_count++;
    }
    else {
        response->SetSuccess(true);
        response->SetStatusCode(200);
        response->SetGetBody(getGroupSnapshotsBody);
        consist_count++;
    }
    return SUCCESS;
}

static bool Stub_LoadConsistentSnapshotInfoFromFile_SUCCESS(ConsistentSnapshotGroupInfo &groupInfo)
{
    return true;
}

/*
 * 测试用例： 删除一致性快照
 * 前置条件:  
 * CHECK点: 
 */
TEST_F(OpenStackConsistentSnapshotTest, DeleteConsistencySnapshot_SUCCESS)
{
    ApplicationEnvironment appEnv;
    Application application;
	StubSetConsistentSnapshotAppEnvUseCertInfo(appEnv);
    StubSetConsistentSnapshotApplication(application);
    OpenStackConsistentSnapshot openstackConstSnapshot(appEnv, application);
    SetOpenstackConsistentSnapshotParam(openstackConstSnapshot);
    ConsistentSnapshotGroupInfo m_groupInfo;
    m_groupInfo.m_volumeType.push_back("type");
    m_groupInfo.m_groupsnapshotStatus = "available";
    m_groupInfo.m_uuid = "12345678";
    m_groupInfo.m_isGroupSnapshotCreated = true;
    m_groupInfo.m_groupSnapshotId = "6f519a48-3183-46cf-a32f-41815f813986";
    m_groupInfo.m_isVolumeAddGroup = true;
    m_groupInfo.m_isGroupCreated = true;
    m_groupInfo.m_isGroupTypeCreated = true;
    openstackConstSnapshot.m_groupInfo = m_groupInfo;
    VirtPlugin::SnapshotInfo snapshot;
    m_stub.set(ADDR(HttpClient, Send), Stub_DeleteConsistencySnapshot_HttpSend);
    m_stub.set(ADDR(OpenStackConsistentSnapshot, LoadConsistentSnapshotInfoFromFile), Stub_LoadConsistentSnapshotInfoFromFile_SUCCESS);
    int ret = openstackConstSnapshot.DeleteConsistencySnapshot(snapshot);
    EXPECT_EQ(ret, SUCCESS);
    m_stub.reset(ADDR(HttpClient, Send));
}

}


