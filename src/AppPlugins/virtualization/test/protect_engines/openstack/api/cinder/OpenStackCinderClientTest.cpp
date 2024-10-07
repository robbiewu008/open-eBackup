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

using namespace VirtPlugin;
using namespace OpenStackPlugin;

namespace HDT_TEST {
class OpenStackCinderClientTest : public testing::Test {
protected:
    void SetUp()
    {}
    void TearDown()
    {}
};

const std::string g_projectId = "e38d227edcce4631be20bfa5aad7130b";

const std::string g_tokenStr = "MIIEhwYJKoZIhvcNAQcCoIIEeDCCBHQCAQExDTALBglghkgBZQMEAgEwggLoBgkqhkiG9w0BBwGgggLZBIIC1XsidG9rZW4iOnsiZXhwaXJlc19hdCI6IjIwMjItMDctMjJUMDc6NTk6NTkuODY5MDAwWiIsIm1ldGhvZHMiOlsicGFzc3dvcmQiXSwiY2F0YWxvZyI6W10sInJvbGVzIjpbeyJuYW1lIjoidmRjX2FkbSIsImlkIjoiY2E3MWU3NzFiYWZjNDI5OTkwOThmMDg4YTc4NGM3NTEifSx7Im5hbWUiOiJ0YWdfYWRtIiwiaWQiOiJkNmJiNWRiZjc0YjQ0Yjk1YWFmMmU3MGJmYzcyNTE0ZiJ9LHsibmFtZSI6ImFwcHJvdl9hZG0iLCJpZCI6IjBiZDBmZjlhMWZkNDRiNzc5MzZlNWUzNzExODZhMzI1In0seyJuYW1lIjoidmRjX293bmVyIiwiaWQiOiI0Nzg4ZjYyMzhmZDM0MWNjYmZkOGQwYzQzMzg4YjdlZSJ9LHsibmFtZSI6InRlX2FkbWluIiwiaWQiOiJhYzgyMWRkN2EwZDI0ZDI2OGI4ZGE2MDg0ZmRlNmQ3OCJ9XSwicHJvamVjdCI6eyJkb21haW4iOnsibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiOTkwNzYzNjFiOTVmNDIyNmIxOGRiMDAwMTU1NWJkMDAifSwibmFtZSI6InNjLWNkLTFfdGVzdCIsImlkIjoiZTM4ZDIyN2VkY2NlNDYzMWJlMjBiZmE1YWFkNzEzMGIifSwiaXNzdWVkX2F0IjoiMjAyMi0wNy0yMVQwNzo1OTo1OS44NjkwMDBaIiwidXNlciI6eyJkb21haW4iOnsibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiOTkwNzYzNjFiOTVmNDIyNmIxOGRiMDAwMTU1NWJkMDAifSwibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiZDQyMTZiN2QzYmE2NGE0ZWI2M2RiMzdjMmI5MTIyMmMifX19MYIBcjCCAW4CAQEwSTA9MQswCQYDVQQGEwJDTjEPMA0GA1UEChMGSHVhd2VpMR0wGwYDVQQDExRIdWF3ZWkgSVQgUHJvZHVjdCBDQQIIFamRIbpBmrcwCwYJYIZIAWUDBAIBMA0GCSqGSIb3DQEBAQUABIIBAGkKLMyXHOFwT4nqe4Iue5g59bBMsIAhW-bhq0MIiJklULEo8RDH+hX5e8AQ44K1Dv2KKXSctXqZoIjW+SeRFxSQm8Ifp-mw18gDn6F+DZRE1ZS+CeecSG8BmXutAfhd9YJQ2xRcw4tbOy21OY-WrXXqIkyyAW1kZpv1yejMm6d6QHDanObsrH9aMJkv79l9tpu0lk4kXM4ohAaUSbVJm47iOiRN2BNxnsHa4bymXFOCIkUYLtA+z0-BXjJIiZjem6Uhtqt6P97Z7MzyuTSFMw0fl6BGswajprEqrVvJg7tB2WCstsff2SPedA86-ufA39TrGuu1kWhLJeUWGQTf2PI=";

const std::string g_endpoint = "https://volume.az0.dc0.demo.com:443/v2/e38d227edcce4631be20bfa5aad7130b";

const std::string g_volumeInfo = {
"{"
    "\"volume\":{"
    /*
        "\"attachments\":[],"
        "\"availability_zone\":\"nova\","
        "\"bootable\":\"false\","
        "\"consistencygroup_id\":null,"
        "\"created_at\":\"2018-11-28T06:21:12.715987\","
        "\"description\":null,"
        "\"encrypted\":false," */
        "\"id\":\"2b955850-f177-45f7-9f49-ecb2c256d161\","
        "\"links\":["
            "{"
                "\"href\":\"http://127.0.0.1:33951/v3/89afd400-b646-4bbc-b12b-c0a4d63e5bd3/volumes/2b955850-f177-45f7-9f49-ecb2c256d161\","
                "\"rel\":\"self\"},"
            "{"
                "\"href\":\"http://127.0.0.1:33951/89afd400-b646-4bbc-b12b-c0a4d63e5bd3/volumes/2b955850-f177-45f7-9f49-ecb2c256d161\","
                "\"rel\":\"bookmark\"}"
        "],"
        "\"metadata\":{},"
        "\"migration_status\":null,"
        "\"multiattach\":false,"
        "\"name\":\"volume1\","
        "\"replication_status\":null,"
        "\"size\":10,"
        "\"snapshot_id\":null,"
        "\"source_volid\":null,"
        "\"status\":\"creating\","
        "\"updated_at\":null,"
        "\"user_id\":\"c853ca26-e8ea-4797-8a52-ee124a013d0e\","
        "\"volume_type\":\"__DEFAULT__\","
        "\"group_id\":null,"
        "\"provider_id\":null,"
        "\"service_uuid\":null,"
        "\"shared_targets\":true,"
        "\"cluster_name\":null,"
        "\"volume_type_id\":\"5fed9d7c-401d-46e2-8e80-f30c70cb7e1d\","
        "\"consumes_quota\":true"
    "}"
"}"
};

const std::string g_snapshots = {
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

const std::string g_groupType = {
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

const std::string g_groupSnapshots = {
"{"
    "\"group_snapshot\": {"
        "\"id\": \"6f519a48-3183-46cf-a32f-41815f816666\","
        "\"name\": \"first_group_snapshot\","
        "\"group_type_id\": \"58737af7-786b-48b7-ab7c-2447e74b0ef4\""
    "}"
"}"
};

const std::string g_group = {
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

static bool StubGetTokenSuccess(void *obj, ModelBase &model, std::string &tokenStr, std::string &endPoint)
{
    tokenStr = g_tokenStr;
    endPoint = g_endpoint;
    return true;
}

static bool StubGetTokenFailed(void *obj, ModelBase &model, std::string &tokenStr, std::string &endPoint)
{
    return false;
}

static int32_t StubCallApiFailed(void *obj, RequestInfo &requestInfo, std::shared_ptr<ResponseModel> response, ModelBase &model)
{
    return FAILED;
}

static Module::IHttpClient *StubCreateVolume()
{
    std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
    EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(g_httpStatusCode));
    EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(200));
    EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));
    std::set<std::string> GetHeadByNameReturn = {};
    EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
    std::string bodyString = g_volumeInfo;
    EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(bodyString));
    std::set<std::string> headerValue;
    headerValue.insert(g_tokenStr);
    std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

    IHttpClientMock *httpClient = new (std::nothrow) IHttpClientMock();
    EXPECT_CALL(*httpClient, SendMemCertRequest(_, _)).WillRepeatedly(Return(httpRespone));
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(httpRespone));
    return httpClient;
}

std::string getHttpBodyString = "";
static Module::IHttpClient *StubCommonClient()
{
    std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
    EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(g_httpStatusCode));
    EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(200));
    EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));
    std::set<std::string> GetHeadByNameReturn = {};
    EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
    std::string bodyString = getHttpBodyString;
    EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(bodyString));
    std::set<std::string> headerValue;
    headerValue.insert(g_tokenStr);
    std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

    IHttpClientMock *httpClient = new (std::nothrow) IHttpClientMock();
    EXPECT_CALL(*httpClient, SendMemCertRequest(_, _)).WillRepeatedly(Return(httpRespone));
    EXPECT_CALL(*httpClient, SendRequest(_,_)).WillRepeatedly(Return(httpRespone));
    return httpClient;
}

/*
 * 测试用例：创建卷成功
 * 前置条件：获取token成功、调用快照接口成功、查询快照状态为available
 * CHECK点：获取的快照id符合预期
 */
TEST_F(OpenStackCinderClientTest, CreateVolume_Success)
{
    Stub stub;
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenSuccess);
    stub.set(ADDR(Module::IHttpClient, GetInstance), StubCreateVolume);
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";

    VolumeRequest request;
    int tmpRetCode = g_httpStatusCode;
    g_httpStatusCode = 202;
    CinderClient cinder;
    std::shared_ptr<VolumeResponse> response = cinder.CreateVolume(request);
    EXPECT_TRUE((response != nullptr));
    if (response) {
        EXPECT_EQ(response->GetStatusCode(), 202);
        std::cout << "body: " << response->GetBody() << std::endl;
        Volume newVolume = response->GetVolume();
        std::string strTmp1;
        if (!Module::JsonHelper::StructToJsonString(newVolume, strTmp1)) {
            std::cout << "struct to string failed" << std::endl;
        }
        EXPECT_EQ(newVolume.m_id, "2b955850-f177-45f7-9f49-ecb2c256d161");
    }
    g_httpStatusCode = tmpRetCode;
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例：创建卷失败
 * 前置条件：发送删除卷消息失败
 * CHECK点：Response为空
 */
TEST_F(OpenStackCinderClientTest, CreateVolume_FAIL)
{
    Stub stub;
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenSuccess);
    stub.set(ADDR(Module::IHttpClient, GetInstance), StubCreateVolume);
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    // 非成功的返回码
    VolumeRequest request;
    int tmpRetCode = g_httpStatusCode;
    g_httpStatusCode = 400;
    CinderClient cinder;
    std::shared_ptr<VolumeResponse> response = cinder.CreateVolume(request);
    EXPECT_TRUE((response == nullptr));
    g_httpStatusCode = tmpRetCode;

    // 获取token失败
    stub.reset(ADDR(OpenStackTokenMgr, GetToken));
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenFailed);
    response = cinder.CreateVolume(request);
    EXPECT_TRUE((response == nullptr));

    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例：删除卷成功
 * 前置条件：获取token成功、调用快照接口成功、查询快照状态为available
 * CHECK点：获取的快照id符合预期
 */
TEST_F(OpenStackCinderClientTest, DeleteVolume_Success)
{
    Stub stub;
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenSuccess);
    stub.set(ADDR(Module::IHttpClient, GetInstance), StubCommonClient);
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";

    VolumeRequest request;
    int tmpRetCode = g_httpStatusCode;
    g_httpStatusCode = 202;
    CinderClient cinder;
    std::shared_ptr<VolumeResponse> response = cinder.DeleteVolume(request);
    EXPECT_TRUE((response != nullptr));
    if (response) {
        EXPECT_EQ(response->GetStatusCode(), 202);
    }
    g_httpStatusCode = tmpRetCode;
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例：删除卷失败
 * 前置条件：发送删除卷消息失败
 * CHECK点：Response为空
 */
TEST_F(OpenStackCinderClientTest, DeleteVolume_FAIL)
{
    Stub stub;
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenSuccess);
    stub.set(ADDR(Module::IHttpClient, GetInstance), StubCommonClient);
    // 非成功的返回码
    VolumeRequest request;
    int tmpRetCode = g_httpStatusCode;
    g_httpStatusCode = 400;
    CinderClient cinder;
    std::shared_ptr<VolumeResponse> response = cinder.DeleteVolume(request);
    EXPECT_TRUE((response->GetStatusCode() == 400));
    g_httpStatusCode = tmpRetCode;

    // 获取token失败
    stub.reset(ADDR(OpenStackTokenMgr, GetToken));
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenFailed);
    response = cinder.DeleteVolume(request);
    EXPECT_TRUE((response == nullptr));

    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例：删除卷组成功
 * 前置条件：链路正常
 * CHECK点：删除卷成功
 */
TEST_F(OpenStackCinderClientTest, DeleteGroup_Success)
{
    Stub stub;
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenSuccess);
    stub.set(ADDR(Module::IHttpClient, GetInstance), StubCommonClient);
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";

    DeleteVolumeGroupRequest request;

    DeleteVolumeGroupRequestBodyMsg body;
    body.m_delete.m_deleteVolumes = false;
    request.SetBody(body);
    request.SetApiVersion("3.14");
    request.SetGroupId("test_group_id");

    int tmpRetCode = g_httpStatusCode;
    g_httpStatusCode = 202;
    CinderClient cinder;
    std::shared_ptr<DeleteVolumeGroupResponse> response = cinder.DeleteGroup(request);
    EXPECT_TRUE((response != nullptr));
    if (response) {
        EXPECT_EQ(response->GetStatusCode(), 202);
    }
    g_httpStatusCode = tmpRetCode;
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}


/*
 * 测试用例：删除卷组
 * 前置条件：发送删除卷消息失败
 * CHECK点：Response为空
 */
TEST_F(OpenStackCinderClientTest, DeleteGroup_FAIL)
{
    Stub stub;
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenSuccess);
    stub.set(ADDR(Module::IHttpClient, GetInstance), StubCommonClient);
    // 非成功的返回码
    DeleteVolumeGroupRequest request;
    DeleteVolumeGroupRequestBodyMsg body;
    body.m_delete.m_deleteVolumes = false;
    request.SetBody(body);
    request.SetApiVersion("3.14");
    request.SetGroupId("test_group_id");
    int tmpRetCode = g_httpStatusCode;
    g_httpStatusCode = 500;
    CinderClient cinder;
    std::shared_ptr<DeleteVolumeGroupResponse> response = cinder.DeleteGroup(request);
    EXPECT_TRUE((response->GetStatusCode() != 202));
    g_httpStatusCode = tmpRetCode;

    // 获取token失败
    stub.reset(ADDR(OpenStackTokenMgr, GetToken));
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenFailed);
    response = cinder.DeleteGroup(request);
    EXPECT_TRUE((response == nullptr));

    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例：获取快照列表成功
 * 前置条件：链路正常
 * CHECK点：删除卷成功
 */
TEST_F(OpenStackCinderClientTest, GetSnapshotList_Success)
{
    Stub stub;
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenSuccess);
    stub.set(ADDR(Module::IHttpClient, GetInstance), StubCommonClient);
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";

    GetSnapshotListRequest request;

    int tmpRetCode = g_httpStatusCode;
    g_httpStatusCode = 202;
    getHttpBodyString = g_snapshots;
    CinderClient cinder;
    std::shared_ptr<GetSnapshotListResponse> response = cinder.GetSnapshotList(request);
    EXPECT_TRUE((response != nullptr));
    if (response) {
        EXPECT_EQ(response->GetStatusCode(), 202);
    }
    g_httpStatusCode = tmpRetCode;
    getHttpBodyString = "";
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}


/*
 * 测试用例：获取快照列表
 * 前置条件：发送删除卷消息失败
 * CHECK点：Response为空
 */
TEST_F(OpenStackCinderClientTest, GetSnapshotList_FAIL)
{
    Stub stub;
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenSuccess);
    stub.set(ADDR(Module::IHttpClient, GetInstance), StubCommonClient);
    // 非成功的返回码
    GetSnapshotListRequest request;
    int tmpRetCode = g_httpStatusCode;
    g_httpStatusCode = 500;
    CinderClient cinder;
    std::shared_ptr<GetSnapshotListResponse> response = cinder.GetSnapshotList(request);
    EXPECT_TRUE((response == nullptr));
    g_httpStatusCode = tmpRetCode;

    // 获取token失败
    stub.reset(ADDR(OpenStackTokenMgr, GetToken));
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenFailed);
    response = cinder.GetSnapshotList(request);
    EXPECT_TRUE((response == nullptr));

    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例：获取group type成功
 * 前置条件：链路正常
 * CHECK点：删除卷成功
 */
TEST_F(OpenStackCinderClientTest, GetGroupType_Success)
{
    Stub stub;
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenSuccess);
    stub.set(ADDR(Module::IHttpClient, GetInstance), StubCommonClient);
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";

    GroupTypeRequest request;

    int tmpRetCode = g_httpStatusCode;
    g_httpStatusCode = 202;
    getHttpBodyString = g_groupType;
    CinderClient cinder;
    std::shared_ptr<GroupTypeResponse> response = cinder.GetGroupType(request);
    EXPECT_TRUE((response != nullptr));
    if (response) {
        EXPECT_EQ(response->GetStatusCode(), 202);
    }
    g_httpStatusCode = tmpRetCode;
    getHttpBodyString = "";
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}


/*
 * 测试用例：获取group type失败
 * 前置条件：发送删除卷消息失败
 * CHECK点：Response为空
 */
TEST_F(OpenStackCinderClientTest, GetGroupType_FAIL)
{
    Stub stub;
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenSuccess);
    stub.set(ADDR(Module::IHttpClient, GetInstance), StubCommonClient);
    // 非成功的返回码
    GroupTypeRequest request;
    int tmpRetCode = g_httpStatusCode;
    g_httpStatusCode = 500;
    CinderClient cinder;
    std::shared_ptr<GroupTypeResponse> response = cinder.GetGroupType(request);
    EXPECT_TRUE((response == nullptr));
    g_httpStatusCode = tmpRetCode;

    // 获取token失败
    stub.reset(ADDR(OpenStackTokenMgr, GetToken));
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenFailed);
    response = cinder.GetGroupType(request);
    EXPECT_TRUE((response == nullptr));

    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}


/*
 * 测试用例：获取卷组快照信息成功
 * 前置条件：链路正常
 * CHECK点：获得卷组信息成功
 */
TEST_F(OpenStackCinderClientTest, GetGroupSnapshot_Success)
{
    Stub stub;
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenSuccess);
    stub.set(ADDR(Module::IHttpClient, GetInstance), StubCommonClient);
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";

    GetSnapshotRequest request;

    int tmpRetCode = g_httpStatusCode;
    g_httpStatusCode = 202;
    getHttpBodyString = g_groupSnapshots;
    CinderClient cinder;
    std::shared_ptr<GetSnapshotResponse> response = cinder.GetGroupSnapshot(request);
    EXPECT_TRUE((response != nullptr));
    if (response) {
        EXPECT_EQ(response->GetStatusCode(), 202);
    }
    g_httpStatusCode = tmpRetCode;
    getHttpBodyString = "";
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}


/*
 * 测试用例：获取卷组快照信息失败
 * 前置条件：发送删除卷消息失败
 * CHECK点：Response为空
 */
TEST_F(OpenStackCinderClientTest, GetGroupSnapshot_FAIL)
{
    Stub stub;
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenSuccess);
    stub.set(ADDR(Module::IHttpClient, GetInstance), StubCommonClient);
    // 非成功的返回码
    GetSnapshotRequest request;
    int tmpRetCode = g_httpStatusCode;
    g_httpStatusCode = 500;
    CinderClient cinder;
    std::shared_ptr<GetSnapshotResponse> response = cinder.GetGroupSnapshot(request);
    EXPECT_TRUE((response == nullptr));
    g_httpStatusCode = tmpRetCode;

    // 获取token失败
    stub.reset(ADDR(OpenStackTokenMgr, GetToken));
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenFailed);
    response = cinder.GetGroupSnapshot(request);
    EXPECT_TRUE((response == nullptr));

    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例：删除GroupType信息成功
 * 前置条件：链路正常
 * CHECK点：删除GroupType成功
 */
TEST_F(OpenStackCinderClientTest, DeleteGroupType_Success)
{
    Stub stub;
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenSuccess);
    stub.set(ADDR(Module::IHttpClient, GetInstance), StubCommonClient);
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";

    GroupTypeRequest request;

    int tmpRetCode = g_httpStatusCode;
    g_httpStatusCode = 202;
    getHttpBodyString = "";
    CinderClient cinder;
    std::shared_ptr<GroupTypeResponse> response = cinder.DeleteGroupType(request);
    EXPECT_TRUE((response != nullptr));
    if (response) {
        EXPECT_EQ(response->GetStatusCode(), 202);
    }
    g_httpStatusCode = tmpRetCode;
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}


/*
 * 测试用例：删除GroupType信息失败
 * 前置条件：发送删除卷消息失败
 * CHECK点：Response为空
 */
TEST_F(OpenStackCinderClientTest, DeleteGroupType_FAIL)
{
    Stub stub;
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenSuccess);
    stub.set(ADDR(Module::IHttpClient, GetInstance), StubCommonClient);
    // 非成功的返回码
    GroupTypeRequest request;
    int tmpRetCode = g_httpStatusCode;
    g_httpStatusCode = 400;
    CinderClient cinder;
    std::shared_ptr<GroupTypeResponse> response = cinder.DeleteGroupType(request);
    EXPECT_TRUE((response->GetStatusCode() != 202));
    g_httpStatusCode = tmpRetCode;

    // 获取token失败
    stub.reset(ADDR(OpenStackTokenMgr, GetToken));
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenFailed);
    response = cinder.DeleteGroupType(request);
    EXPECT_TRUE((response == nullptr));

    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}


/*
 * 测试用例：删除GroupSnapshot信息成功
 * 前置条件：链路正常
 * CHECK点：获得卷组信息成功
 */
TEST_F(OpenStackCinderClientTest, DeleteGroupSnapShot_Success)
{
    Stub stub;
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenSuccess);
    stub.set(ADDR(Module::IHttpClient, GetInstance), StubCommonClient);
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";

    DeleteSnapshotRequest request;

    int tmpRetCode = g_httpStatusCode;
    g_httpStatusCode = 202;
    getHttpBodyString = "";
    CinderClient cinder;
    std::shared_ptr<DeleteSnapshotResponse> response = cinder.DeleteGroupSnapShot(request);
    EXPECT_TRUE((response != nullptr));
    if (response) {
        EXPECT_EQ(response->GetStatusCode(), 202);
    }
    g_httpStatusCode = tmpRetCode;
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}


/*
 * 测试用例：删除GroupSnapshot信息失败
 * 前置条件：发送删除卷消息失败
 * CHECK点：Response为空
 */
TEST_F(OpenStackCinderClientTest, DeleteGroupSnapShot_FAIL)
{
    Stub stub;
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenSuccess);
    stub.set(ADDR(Module::IHttpClient, GetInstance), StubCommonClient);
    // 非成功的返回码
    DeleteSnapshotRequest request;
    int tmpRetCode = g_httpStatusCode;
    g_httpStatusCode = 400;
    CinderClient cinder;
    std::shared_ptr<DeleteSnapshotResponse> response = cinder.DeleteGroupSnapShot(request);
    EXPECT_TRUE((response->GetStatusCode() != 202));
    g_httpStatusCode = tmpRetCode;

    // 获取token失败
    stub.reset(ADDR(OpenStackTokenMgr, GetToken));
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenFailed);
    response = cinder.DeleteGroupSnapShot(request);
    EXPECT_TRUE((response == nullptr));

    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例：获取卷组信息成功
 * 前置条件：链路正常
 * CHECK点：获得卷组信息成功
 */
TEST_F(OpenStackCinderClientTest, GetVolumeGroupStatus_Success)
{
    Stub stub;
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenSuccess);
    stub.set(ADDR(Module::IHttpClient, GetInstance), StubCommonClient);
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";

    VolumeGroupRequest request;

    int tmpRetCode = g_httpStatusCode;
    g_httpStatusCode = 202;
    getHttpBodyString = g_group;
    CinderClient cinder;
    std::shared_ptr<VolumeGroupResponse> response = cinder.GetVolumeGroupStatus(request);
    EXPECT_TRUE((response != nullptr));
    if (response) {
        EXPECT_EQ(response->GetStatusCode(), 202);
    }
    g_httpStatusCode = tmpRetCode;
    getHttpBodyString = "";
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}


/*
 * 测试用例：获取卷组信息失败
 * 前置条件：发送删除卷消息失败
 * CHECK点：Response为空
 */
TEST_F(OpenStackCinderClientTest, GetVolumeGroupStatus_FAIL)
{
    Stub stub;
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenSuccess);
    stub.set(ADDR(Module::IHttpClient, GetInstance), StubCommonClient);
    // 非成功的返回码
    VolumeGroupRequest request;
    int tmpRetCode = g_httpStatusCode;
    g_httpStatusCode = 500;
    CinderClient cinder;
    std::shared_ptr<VolumeGroupResponse> response = cinder.GetVolumeGroupStatus(request);
    EXPECT_TRUE((response == nullptr));
    g_httpStatusCode = tmpRetCode;

    // 获取token失败
    stub.reset(ADDR(OpenStackTokenMgr, GetToken));
    stub.set(ADDR(OpenStackTokenMgr, GetToken), StubGetTokenFailed);
    response = cinder.GetVolumeGroupStatus(request);
    EXPECT_TRUE((response == nullptr));

    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

}
