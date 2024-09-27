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
#include "protect_engines/hcs/common/HcsCommonInfo.h"
#include "common/model/ResponseModelMock.h"
#include "common/model/ModelBaseMock.h"
#include "protect_engines/hcs/utils/IHttpResponseMock.h"
#include "protect_engines/hcs/utils/IHttpClientMock.h"
#include "protect_engines/hcs/api/ecs/EcsClient.h"
#include "common/Structs.h"
#include "common/CommonMock.h"

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
using namespace HcsPlugin;

namespace HDT_TEST {
class EcsClientTest : public testing::Test {
protected:
    void SetUp()
    {}
    void TearDown()
    {}
};

const std::string g_tokenStr =
    "MIIEhwYJKoZIhvcNAQcCoIIEeDCCBHQCAQExDTALBglghkgBZQMEAgEwggLoBgkqhkiG9w0BBwGgggLZBIIC1XsidG9rZW4iOnsiZXhwaXJlc19hdC"
    "I6IjIwMjItMDctMjJUMDc6NTk6NTkuODY5MDAwWiIsIm1ldGhvZHMiOlsicGFzc3dvcmQiXSwiY2F0YWxvZyI6W10sInJvbGVzIjpbeyJuYW1lIjoi"
    "dmRjX2FkbSIsImlkIjoiY2E3MWU3NzFiYWZjNDI5OTkwOThmMDg4YTc4NGM3NTEifSx7Im5hbWUiOiJ0YWdfYWRtIiwiaWQiOiJkNmJiNWRiZjc0Yj"
    "Q0Yjk1YWFmMmU3MGJmYzcyNTE0ZiJ9LHsibmFtZSI6ImFwcHJvdl9hZG0iLCJpZCI6IjBiZDBmZjlhMWZkNDRiNzc5MzZlNWUzNzExODZhMzI1In0s"
    "eyJuYW1lIjoidmRjX293bmVyIiwiaWQiOiI0Nzg4ZjYyMzhmZDM0MWNjYmZkOGQwYzQzMzg4YjdlZSJ9LHsibmFtZSI6InRlX2FkbWluIiwiaWQiOi"
    "JhYzgyMWRkN2EwZDI0ZDI2OGI4ZGE2MDg0ZmRlNmQ3OCJ9XSwicHJvamVjdCI6eyJkb21haW4iOnsibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiOTkw"
    "NzYzNjFiOTVmNDIyNmIxOGRiMDAwMTU1NWJkMDAifSwibmFtZSI6InNjLWNkLTFfdGVzdCIsImlkIjoiZTM4ZDIyN2VkY2NlNDYzMWJlMjBiZmE1YW"
    "FkNzEzMGIifSwiaXNzdWVkX2F0IjoiMjAyMi0wNy0yMVQwNzo1OTo1OS44NjkwMDBaIiwidXNlciI6eyJkb21haW4iOnsibmFtZSI6Imh1YW5ncm9u"
    "ZyIsImlkIjoiOTkwNzYzNjFiOTVmNDIyNmIxOGRiMDAwMTU1NWJkMDAifSwibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiZDQyMTZiN2QzYmE2NGE0ZW"
    "I2M2RiMzdjMmI5MTIyMmMifX19MYIBcjCCAW4CAQEwSTA9MQswCQYDVQQGEwJDTjEPMA0GA1UEChMGSHVhd2VpMR0wGwYDVQQDExRIdWF3ZWkgSVQg"
    "UHJvZHVjdCBDQQIIFamRIbpBmrcwCwYJYIZIAWUDBAIBMA0GCSqGSIb3DQEBAQUABIIBAGkKLMyXHOFwT4nqe4Iue5g59bBMsIAhW-"
    "bhq0MIiJklULEo8RDH+hX5e8AQ44K1Dv2KKXSctXqZoIjW+SeRFxSQm8Ifp-mw18gDn6F+DZRE1ZS+CeecSG8BmXutAfhd9YJQ2xRcw4tbOy21OY-"
    "WrXXqIkyyAW1kZpv1yejMm6d6QHDanObsrH9aMJkv79l9tpu0lk4kXM4ohAaUSbVJm47iOiRN2BNxnsHa4bymXFOCIkUYLtA+z0-"
    "BXjJIiZjem6Uhtqt6P97Z7MzyuTSFMw0fl6BGswajprEqrVvJg7tB2WCstsff2SPedA86-ufA39TrGuu1kWhLJeUWGQTf2PI=";
const std::string g_serverList = "{\"count\":3,\"servers\":[{\"id\":\"c982522d-c5ec-44f7-9919-25bb1587a48f\",\"name\":"
                                 "\"ecs-4ac8-0005\"},{\"id\":\"f0e97318-3ff6-49b8-90c3-6d5d1367af8b\",\"name\":\"ecs-"
                                 "4ac8-0002\"},{\"id\":\"660e57b4-e59d-4aa1-9be4-04ca179d67c5\",\"name\":\"bb-001\"}]}";
const std::string g_serverList1 =
    "{\"count\":1,\"servers\":[{\"id\":\"c982522d-c5ec-44f7-9919-25bb1587a48f\",\"name\":\"ecs-4ac8-0005\"}]}";
const std::string g_serverDetail =
    "{\"server\":{\"tenant_id\":\"e38d227edcce4631be20bfa5aad7130b\",\"addresses\":{\"subnet-8f61\":[{\"OS-EXT-IPS-MAC:"
    "mac_addr\":\"fa:16:3e:0b:7a:38\",\"OS-EXT-IPS:type\":\"fixed\",\"addr\":\"192.168.0.216\",\"version\":4}]},"
    "\"metadata\":{\"productId\":\"5b4ecaa32947446b824df4a6c60c8a04\",\"__instance_vwatchdog\":\"false\",\"_ha_policy_"
    "type\":\"remote_rebuild\",\"server_expiry\":\"0\",\"cascaded.instance_extrainfo\":\"max_cpu:254,current_mem:8192,"
    "org_mem:8192,iohang_timeout:720,pcibridge:2,system_serial_number:10a4e361-c981-46f2-b9ba-d7ff9c601693,max_mem:"
    "4194304,cpu_num_for_one_plug:1,org_cpu:4,xml_support_live_resize:True,current_cpu:4,uefi_mode_sysinfo_fields:"
    "version_serial_uuid_family_asset,num_of_mem_plug:57\"},\"OS-EXT-STS:task_state\":null,\"OS-DCF:diskConfig\":"
    "\"MANUAL\",\"OS-EXT-AZ:availability_zone\":\"az0.dc0\",\"links\":[{\"rel\":\"self\",\"href\":\"https://"
    "ecs.sc-cd-1.demo.com/v2/e38d227edcce4631be20bfa5aad7130b/servers/"
    "10a4e361-c981-46f2-b9ba-d7ff9c601693\"},{\"rel\":\"bookmark\",\"href\":\"https://ecs.sc-cd-1.demo.com/"
    "e38d227edcce4631be20bfa5aad7130b/servers/"
    "10a4e361-c981-46f2-b9ba-d7ff9c601693\"}],\"OS-EXT-STS:power_state\":4,\"id\":\"10a4e361-c981-46f2-b9ba-"
    "d7ff9c601693\",\"os-extended-volumes:volumes_attached\":[{\"id\":\"d5fb423e-9bd1-429a-8441-91efdef2b5f1\"}],\"OS-"
    "EXT-SRV-ATTR:host\":\"EA918B93-2561-E611-9B2A-049FCAD22DFC\",\"image\":{\"links\":[{\"rel\":\"bookmark\",\"href\":"
    "\"https://ecs.sc-cd-1.demo.com/e38d227edcce4631be20bfa5aad7130b/images/"
    "a0a24ff5-1899-4c38-843a-659fd9d3ac15\"}],\"id\":\"a0a24ff5-1899-4c38-843a-659fd9d3ac15\"},\"OS-SRV-USG:terminated_"
    "at\":null,\"accessIPv4\":\"\",\"accessIPv6\":\"\",\"created\":\"2022-05-11T03:00:26Z\",\"hostId\":"
    "\"0e2d4d8215b35d8b4eb632e9841d3fc9d1a3208749a15f34abb30b12\",\"OS-EXT-SRV-ATTR:hypervisor_hostname\":\"EA918B93-"
    "2561-E611-9B2A-049FCAD22DFC\",\"key_name\":null,\"flavor\":{\"links\":[{\"rel\":\"bookmark\",\"href\":\"https://"
    "ecs.sc-cd-1.demo.com/e38d227edcce4631be20bfa5aad7130b/flavors/"
    "ab2e658d-fdac-4bdf-aa3f-59f977c5e581\"}],\"id\":\"ab2e658d-fdac-4bdf-aa3f-59f977c5e581\"},\"security_groups\":[{"
    "\"name\":\"default\"}],\"config_drive\":\"\",\"OS-EXT-STS:vm_state\":\"stopped\",\"OS-EXT-SRV-ATTR:instance_"
    "name\":\"instance-00000030\",\"user_id\":\"d4216b7d3ba64a4eb63db37c2b91222c\",\"name\":\"ecs-4d45-0001\",\"OS-SRV-"
    "USG:launched_at\":\"2022-05-11T03:00:36.000000\",\"updated\":\"2022-06-28T09:54:06Z\",\"status\":\"SHUTOFF\"}}";
const std::string g_serverListDetail =
    "{\"count\": 1,\
    \"servers\": [{\
    \"id\": \"78f14666-1309-421c-ae3b-60649c616b92\",\
    \"name\": \"rigoujian-勿动\",\
    \"status\": \"ACTIVE\",\
    \"progress\": 0,\
    \"updated\": \"2022-10-19T11:50:54Z\",\
    \"created\": \"2022-10-17T07:28:08Z\",\
    \"metadata\": {\
    \"_ha_policy_type\": \"remote_rebuild\",\
    \"metering.imagetype\": \"gold\",\
    \"vpc_id\": \"4b92c9ca-5025-44ca-9779-b5e17e029ff5\"},\
    \"key_name\": null,\
    \"OS-EXT-STS:task_state\": null,\
    \"OS-EXT-STS:power_state\": 1}]}";

const std::string g_serverId = "10a4e361-c981-46f2-b9ba-d7ff9c601693";
const std::string g_projectId = "e38d227edcce4631be20bfa5aad7130b";



bool Stub_GetToken_Failed(void *obj, ModelBase &model, std::string &tokenStr, std::string &endPoint)
{
    return false;
}

static Module::IHttpClient *Stub_Send_Request_Failed()
{   
    std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
    EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(404));
    EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(404));
    EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));
    std::set<std::string> GetHeadByNameReturn = {};
    EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
    EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(""));
    std::set<std::string> headerValue;
    headerValue.insert(g_tokenStr);
    std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

    IHttpClientMock *httpClient = new (std::nothrow) IHttpClientMock();
    EXPECT_CALL(*httpClient, SendRequest(_, _)).WillRepeatedly(Return(httpRespone));
    EXPECT_CALL(*httpClient, SendRequest(_, _)).WillRepeatedly(Return(httpRespone));
    return httpClient;
}

static Module::IHttpClient *Stub_Get_Servers_List_Success()
{
    std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
    EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(200));
    EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(200));
    EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));
    std::set<std::string> GetHeadByNameReturn = {};
    EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
    EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(g_serverList));
    std::set<std::string> headerValue;
    headerValue.insert(g_tokenStr);
    std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

    IHttpClientMock *httpClient = new (std::nothrow) IHttpClientMock();
    EXPECT_CALL(*httpClient, SendRequest(_, _)).WillRepeatedly(Return(httpRespone));
    EXPECT_CALL(*httpClient, SendRequest(_, _)).WillRepeatedly(Return(httpRespone));
    return httpClient;
}

static Module::IHttpClient *Stub_Get_Servers_List1_Success()
{
    std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
    EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(200));
    EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(200));
    EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));
    std::set<std::string> GetHeadByNameReturn = {};
    EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
    EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(g_serverList1));
    std::set<std::string> headerValue;
    headerValue.insert(g_tokenStr);
    std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

    IHttpClientMock *httpClient = new (std::nothrow) IHttpClientMock();
    EXPECT_CALL(*httpClient, SendRequest(_, _)).WillRepeatedly(Return(httpRespone));
    EXPECT_CALL(*httpClient, SendRequest(_, _)).WillRepeatedly(Return(httpRespone));
    return httpClient;
}

static Module::IHttpClient *Stub_Get_Servers_Detail_Success()
{
    std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
    EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(200));
    EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(200));
    EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));
    std::set<std::string> GetHeadByNameReturn = {};
    EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
    EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(g_serverDetail));
    std::set<std::string> headerValue;
    headerValue.insert(g_tokenStr);
    std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

    IHttpClientMock *httpClient = new (std::nothrow) IHttpClientMock();
    EXPECT_CALL(*httpClient, SendRequest(_, _)).WillRepeatedly(Return(httpRespone));
    EXPECT_CALL(*httpClient, SendRequest(_, _)).WillRepeatedly(Return(httpRespone));
    return httpClient;
}

static Module::IHttpClient *StubGetServersDetailListSuccess()
{
    std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
    EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(200));
    EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(200));
    EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));
    std::set<std::string> GetHeadByNameReturn = {};
    EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
    EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(g_serverListDetail));
    std::set<std::string> headerValue;
    headerValue.insert(g_tokenStr);
    std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

    IHttpClientMock *httpClient = new (std::nothrow) IHttpClientMock();
    EXPECT_CALL(*httpClient, SendRequest(_, _)).WillRepeatedly(Return(httpRespone));
    EXPECT_CALL(*httpClient, SendRequest(_, _)).WillRepeatedly(Return(httpRespone));
    return httpClient;
}

/*
 * 测试用例： 虚拟机列表查询成功
 * 前置条件： 对返回值进行打桩
 * CHECK点： 不设置查询参数，判断response中的值是否和返回值一致
 */
TEST_F(EcsClientTest, GetSereverListSuccess)
{
    GetServerListRequest getServerListRequeset;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getServerListRequeset.SetUserInfo(auth);
    getServerListRequeset.SetScopeValue(g_projectId);
    getServerListRequeset.SetDomain("domain");
    getServerListRequeset.SetEndpoint("demo.com");
    getServerListRequeset.SetRegion("sc-cd-1");
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetToken_Success);
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Get_Servers_List_Success);
    EcsClient ecsClient;
    std::shared_ptr<GetServerListResponse> response = ecsClient.GetServerList(getServerListRequeset);
    EXPECT_TRUE((response != nullptr));
    EXPECT_EQ(response->GetStatusCode(), 200);
    EXPECT_TRUE(response->Serial());
    ProjectServerDetailList serverListDetail = response->GetServerListDetail();
    EXPECT_EQ(serverListDetail.m_serversInfo.size(), 3);
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 虚拟机列表查询成功
 * 前置条件： 对返回值进行打桩
 * CHECK点： 设置查询参数，判断response中的值是否和返回值一致
 */
TEST_F(EcsClientTest, GetSereverListSuccessWhenSetQueryParams)
{
    GetServerListRequest getServerListRequeset;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getServerListRequeset.SetUserInfo(auth);
    getServerListRequeset.SetScopeValue(g_projectId);
    getServerListRequeset.SetDomain("domain");
    getServerListRequeset.SetEndpoint("demo.com");
    getServerListRequeset.SetRegion("sc-cd-1");
    getServerListRequeset.SetServerName("ecs-4ac8-0005");
    getServerListRequeset.SetServerStatus("SHUTOFF");
    getServerListRequeset.SetServerOffset(1);
    getServerListRequeset.SetServerLimit(1);
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetToken_Success);
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Get_Servers_List1_Success);
    EcsClient ecsClient;
    std::shared_ptr<GetServerListResponse> response = ecsClient.GetServerList(getServerListRequeset);
    EXPECT_TRUE((response != nullptr));
    EXPECT_EQ(response->GetStatusCode(), 200);
    EXPECT_TRUE(response->Serial());
    ProjectServerDetailList serverListDetail = response->GetServerListDetail();
    EXPECT_EQ(serverListDetail.m_serversInfo.size(), 1);
    EXPECT_EQ(serverListDetail.m_serversInfo[0].m_name, "ecs-4ac8-0005");
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 虚拟机列表查询失败
 * 前置条件： 对返回值进行打桩
 * CHECK点： 获取token失败
 */
TEST_F(EcsClientTest, GetSereverListFailedWhenGetTokenFailed)
{
    GetServerListRequest getServerListRequeset;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getServerListRequeset.SetUserInfo(auth);
    getServerListRequeset.SetScopeValue(g_projectId);
    getServerListRequeset.SetDomain("domain");
    getServerListRequeset.SetEndpoint("demo.com");
    getServerListRequeset.SetRegion("sc-cd-1");
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetToken_Failed);
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Get_Servers_List_Success);
    EcsClient ecsClient;
    std::shared_ptr<GetServerListResponse> response = ecsClient.GetServerList(getServerListRequeset);
    EXPECT_TRUE((response == nullptr));
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 虚拟机列表查询失败
 * 前置条件： 对返回值进行打桩
 * CHECK点： 发送失败
 */
TEST_F(EcsClientTest, GetSereverListFailedWhenSendFailed)
{
    GetServerListRequest getServerListRequeset;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getServerListRequeset.SetUserInfo(auth);
    getServerListRequeset.SetScopeValue(g_projectId);
    getServerListRequeset.SetDomain("domain");
    getServerListRequeset.SetEndpoint("demo.com");
    getServerListRequeset.SetRegion("sc-cd-1");
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetToken_Success);
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Send_Request_Failed);
    EcsClient ecsClient;
    std::shared_ptr<GetServerListResponse> response = ecsClient.GetServerList(getServerListRequeset);
    EXPECT_TRUE((response == nullptr));
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 虚拟机列表查询失败
 * 前置条件： 对返回值进行打桩
 * CHECK点： 未设置必传参数UserInfo，参数校验失败，返回nullptr
 */
TEST_F(EcsClientTest, GetSereverListFailedWhenUserInfoNotSet)
{
    GetServerListRequest getServerListRequeset;
    getServerListRequeset.SetScopeValue(g_projectId);
    getServerListRequeset.SetDomain("domain");
    getServerListRequeset.SetEndpoint("demo.com");
    getServerListRequeset.SetRegion("sc-cd-1");
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetToken_Success);
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Get_Servers_List_Success);
    EcsClient ecsClient;
    std::shared_ptr<GetServerListResponse> response = ecsClient.GetServerList(getServerListRequeset);
    EXPECT_TRUE((response == nullptr));
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 虚拟机列表查询失败
 * 前置条件： 对返回值进行打桩
 * CHECK点： 未设置必传参数ScopeValue，参数校验失败，返回nullptr
 */
TEST_F(EcsClientTest, GetSereverListFailedWhenScopeValueNotSet)
{
    GetServerListRequest getServerListRequeset;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getServerListRequeset.SetUserInfo(auth);
    getServerListRequeset.SetDomain("domain");
    getServerListRequeset.SetEndpoint("demo.com");
    getServerListRequeset.SetRegion("sc-cd-1");
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetToken_Success);
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Get_Servers_List_Success);
    EcsClient ecsClient;
    std::shared_ptr<GetServerListResponse> response = ecsClient.GetServerList(getServerListRequeset);
    EXPECT_TRUE((response == nullptr));
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 虚拟机列表查询失败
 * 前置条件： 对返回值进行打桩
 * CHECK点： 未设置必传参数Domain，参数校验失败，返回nullptr
 */
TEST_F(EcsClientTest, GetSereverListFailedWhenDomainNotSet)
{
    GetServerListRequest getServerListRequeset;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getServerListRequeset.SetUserInfo(auth);
    getServerListRequeset.SetScopeValue(g_projectId);
    getServerListRequeset.SetEndpoint("demo.com");
    getServerListRequeset.SetRegion("sc-cd-1");
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetToken_Success);
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Get_Servers_List_Success);
    EcsClient ecsClient;
    std::shared_ptr<GetServerListResponse> response = ecsClient.GetServerList(getServerListRequeset);
    EXPECT_TRUE((response == nullptr));
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 虚拟机列表查询失败
 * 前置条件： 对返回值进行打桩
 * CHECK点： 未设置必传参数Endpoint，参数校验失败，返回nullptr
 */
TEST_F(EcsClientTest, GetSereverListFailedWhenEndpointNotSet)
{
    GetServerListRequest getServerListRequeset;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getServerListRequeset.SetUserInfo(auth);
    getServerListRequeset.SetScopeValue(g_projectId);
    getServerListRequeset.SetDomain("domain");
    getServerListRequeset.SetRegion("sc-cd-1");
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetToken_Success);
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Get_Servers_List_Success);
    EcsClient ecsClient;
    std::shared_ptr<GetServerListResponse> response = ecsClient.GetServerList(getServerListRequeset);
    EXPECT_TRUE((response == nullptr));
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 虚拟机列表查询失败
 * 前置条件： 对返回值进行打桩
 * CHECK点： 未设置必传参数Region，参数校验失败，返回nullptr
 */
TEST_F(EcsClientTest, GetSereverListFailedWhenRegionNotSet)
{
    GetServerListRequest getServerListRequeset;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getServerListRequeset.SetUserInfo(auth);
    getServerListRequeset.SetScopeValue(g_projectId);
    getServerListRequeset.SetDomain("domain");
    getServerListRequeset.SetEndpoint("demo.com");
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetToken_Success);
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Get_Servers_List_Success);
    EcsClient ecsClient;
    std::shared_ptr<GetServerListResponse> response = ecsClient.GetServerList(getServerListRequeset);
    EXPECT_TRUE((response == nullptr));
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 虚拟机详情查询成功
 * 前置条件： 对返回值进行打桩
 * CHECK点： 判断response中的值是否和返回值一致
 */
TEST_F(EcsClientTest, GetSereverDetailsSuccess)
{
    GetServerDetailsRequest getServerDetailsRequeset;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getServerDetailsRequeset.SetUserInfo(auth);
    getServerDetailsRequeset.SetScopeValue(g_projectId);
    getServerDetailsRequeset.SetDomain("domain");
    getServerDetailsRequeset.SetEndpoint("demo.com");
    getServerDetailsRequeset.SetRegion("sc-cd-1");
    getServerDetailsRequeset.SetServerId(g_serverId);
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetToken_Success);
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Get_Servers_Detail_Success);
    EcsClient ecsClient;
    std::shared_ptr<GetServerDetailsResponse> response = ecsClient.GetServerDetails(getServerDetailsRequeset);
    EXPECT_TRUE((response != nullptr));
    EXPECT_EQ(response->GetStatusCode(), 200);
    EXPECT_TRUE(response->Serial());
    ServerDetail serverDetail = response->GetServerDetails();
    EXPECT_EQ(serverDetail.m_hostServerInfo.m_uuid, g_serverId);
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 虚拟机详情查询失败
 * 前置条件： 对返回值进行打桩
 * CHECK点： 未设置必传参数ServerId，参数校验失败，返回nullptr
 */
TEST_F(EcsClientTest, GetSereverDetailsFailedWhenServerIdNotSet)
{
    GetServerDetailsRequest getServerDetailsRequeset;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getServerDetailsRequeset.SetUserInfo(auth);
    getServerDetailsRequeset.SetScopeValue(g_projectId);
    getServerDetailsRequeset.SetDomain("domain");
    getServerDetailsRequeset.SetEndpoint("demo.com");
    getServerDetailsRequeset.SetRegion("sc-cd-1");
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetToken_Success);
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Get_Servers_Detail_Success);
    EcsClient ecsClient;
    std::shared_ptr<GetServerDetailsResponse> response = ecsClient.GetServerDetails(getServerDetailsRequeset);
    EXPECT_TRUE((response == nullptr));
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 虚拟机详情查询失败
 * 前置条件： 对返回值进行打桩
 * CHECK点： 获取token失败
 */
TEST_F(EcsClientTest, GetSereverDetailsFailedWhenGetTokenFailed)
{
    GetServerDetailsRequest getServerDetailsRequeset;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getServerDetailsRequeset.SetUserInfo(auth);
    getServerDetailsRequeset.SetScopeValue(g_projectId);
    getServerDetailsRequeset.SetDomain("domain");
    getServerDetailsRequeset.SetEndpoint("demo.com");
    getServerDetailsRequeset.SetRegion("sc-cd-1");
    getServerDetailsRequeset.SetServerId(g_serverId);
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetToken_Failed);
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Get_Servers_Detail_Success);
    EcsClient ecsClient;
    std::shared_ptr<GetServerDetailsResponse> response = ecsClient.GetServerDetails(getServerDetailsRequeset);
    EXPECT_TRUE((response == nullptr));
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 虚拟机详情查询失败
 * 前置条件： 对返回值进行打桩
 * CHECK点： 发送失败
 */
TEST_F(EcsClientTest, GetSereverDetailsFailedWhenSendFailed)
{
    GetServerDetailsRequest getServerDetailsRequeset;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getServerDetailsRequeset.SetUserInfo(auth);
    getServerDetailsRequeset.SetScopeValue(g_projectId);
    getServerDetailsRequeset.SetDomain("domain");
    getServerDetailsRequeset.SetEndpoint("demo.com");
    getServerDetailsRequeset.SetRegion("sc-cd-1");
    getServerDetailsRequeset.SetServerId(g_serverId);
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetToken_Success);
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Send_Request_Failed);
    EcsClient ecsClient;
    std::shared_ptr<GetServerDetailsResponse> response = ecsClient.GetServerDetails(getServerDetailsRequeset);
    EXPECT_TRUE((response == nullptr));
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 虚拟机详情查询成功
 * 前置条件： 对返回值进行打桩
 * CHECK点： 判断response中的值是否和返回值一致
 */
TEST_F(EcsClientTest, GetSereverDetailsListSuccess)
{
    GetServerListRequest getServerListRequeset;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getServerListRequeset.SetUserInfo(auth);
    getServerListRequeset.SetScopeValue(g_projectId);
    getServerListRequeset.SetDomain("domain");
    getServerListRequeset.SetEndpoint("demo.com");
    getServerListRequeset.SetRegion("sc-cd-1");
    getServerListRequeset.SetServerOffset(0);
    getServerListRequeset.SetServerLimit(1);
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetToken_Success);
    stub.set(ADDR(Module::IHttpClient, GetInstance), StubGetServersDetailListSuccess);
    EcsClient ecsClient;
    std::shared_ptr<GetServerListResponse> response = ecsClient.GetServerDetailList(getServerListRequeset);
    EXPECT_TRUE((response != nullptr));
    EXPECT_EQ(response->GetStatusCode(), 200);
    EXPECT_TRUE(response->Serial());
    ProjectServerDetailList serverListDetail = response->GetServerListDetail();
    EXPECT_EQ(serverListDetail.m_serversInfo.size(), 1);
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 虚拟机详情查询失败
 * 前置条件： 对返回值进行打桩
 * CHECK点： 获取token失败
 */
TEST_F(EcsClientTest, GetSereverDetailsListFailedWhenGetTokenFailed)
{
    GetServerListRequest getServerListRequeset;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getServerListRequeset.SetUserInfo(auth);
    getServerListRequeset.SetScopeValue(g_projectId);
    getServerListRequeset.SetDomain("domain");
    getServerListRequeset.SetEndpoint("demo.com");
    getServerListRequeset.SetRegion("sc-cd-1");
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetToken_Failed);
    stub.set(ADDR(Module::IHttpClient, GetInstance), StubGetServersDetailListSuccess);
    EcsClient ecsClient;
    std::shared_ptr<GetServerListResponse> response = ecsClient.GetServerDetailList(getServerListRequeset);
    EXPECT_TRUE((response == nullptr));
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例： 虚拟机详情查询失败
 * 前置条件： 对返回值进行打桩
 * CHECK点： 发送失败
 */
TEST_F(EcsClientTest, GetSereverDetailsListFailedWhenSendFailed)
{
    GetServerListRequest getServerListRequeset;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    getServerListRequeset.SetUserInfo(auth);
    getServerListRequeset.SetScopeValue(g_projectId);
    getServerListRequeset.SetDomain("domain");
    getServerListRequeset.SetEndpoint("demo.com");
    getServerListRequeset.SetRegion("sc-cd-1");
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetToken_Success);
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_Send_Request_Failed);
    EcsClient ecsClient;
    std::shared_ptr<GetServerListResponse> response = ecsClient.GetServerDetailList(getServerListRequeset);
    EXPECT_TRUE((response == nullptr));
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

}  // namespace HDT_TEST