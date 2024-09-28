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
#include <job_controller/jobs/backup/BackupIoTask.h>
#include "protect_engines/hcs/common/HcsCommonInfo.h"
#include "common/model/ResponseModelMock.h"
#include "common/model/ModelBaseMock.h"
#include "protect_engines/hcs/utils/IHttpResponseMock.h"
#include "protect_engines/hcs/utils/IHttpClientMock.h"
#include "protect_engines/hcs/api/iam/IamClient.h"
#include "protect_engines/hcs/api/evs/EvsClient.h"
#include "common/Structs.h"

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using ::testing::Invoke;;

using namespace VirtPlugin;
using namespace HcsPlugin;

namespace HDT_TEST {
class EvsClientTest : public testing::Test {
protected:
    void SetUp() {}
    void TearDown() {}
};

const std::string g_tokenStr = "MIIEhwYJKoZIhvcNAQcCoIIEeDCCBHQCAQExDTALBglghkgBZQMEAgEwggLoBgkqhkiG9w0BBwGgggLZBIIC1XsidG9rZW4iOnsiZXhwaXJlc19hdCI6IjIwMjItMDctMjJUMDc6NTk6NTkuODY5MDAwWiIsIm1ldGhvZHMiOlsicGFzc3dvcmQiXSwiY2F0YWxvZyI6W10sInJvbGVzIjpbeyJuYW1lIjoidmRjX2FkbSIsImlkIjoiY2E3MWU3NzFiYWZjNDI5OTkwOThmMDg4YTc4NGM3NTEifSx7Im5hbWUiOiJ0YWdfYWRtIiwiaWQiOiJkNmJiNWRiZjc0YjQ0Yjk1YWFmMmU3MGJmYzcyNTE0ZiJ9LHsibmFtZSI6ImFwcHJvdl9hZG0iLCJpZCI6IjBiZDBmZjlhMWZkNDRiNzc5MzZlNWUzNzExODZhMzI1In0seyJuYW1lIjoidmRjX293bmVyIiwiaWQiOiI0Nzg4ZjYyMzhmZDM0MWNjYmZkOGQwYzQzMzg4YjdlZSJ9LHsibmFtZSI6InRlX2FkbWluIiwiaWQiOiJhYzgyMWRkN2EwZDI0ZDI2OGI4ZGE2MDg0ZmRlNmQ3OCJ9XSwicHJvamVjdCI6eyJkb21haW4iOnsibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiOTkwNzYzNjFiOTVmNDIyNmIxOGRiMDAwMTU1NWJkMDAifSwibmFtZSI6InNjLWNkLTFfdGVzdCIsImlkIjoiZTM4ZDIyN2VkY2NlNDYzMWJlMjBiZmE1YWFkNzEzMGIifSwiaXNzdWVkX2F0IjoiMjAyMi0wNy0yMVQwNzo1OTo1OS44NjkwMDBaIiwidXNlciI6eyJkb21haW4iOnsibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiOTkwNzYzNjFiOTVmNDIyNmIxOGRiMDAwMTU1NWJkMDAifSwibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiZDQyMTZiN2QzYmE2NGE0ZWI2M2RiMzdjMmI5MTIyMmMifX19MYIBcjCCAW4CAQEwSTA9MQswCQYDVQQGEwJDTjEPMA0GA1UEChMGSHVhd2VpMR0wGwYDVQQDExRIdWF3ZWkgSVQgUHJvZHVjdCBDQQIIFamRIbpBmrcwCwYJYIZIAWUDBAIBMA0GCSqGSIb3DQEBAQUABIIBAGkKLMyXHOFwT4nqe4Iue5g59bBMsIAhW-bhq0MIiJklULEo8RDH+hX5e8AQ44K1Dv2KKXSctXqZoIjW+SeRFxSQm8Ifp-mw18gDn6F+DZRE1ZS+CeecSG8BmXutAfhd9YJQ2xRcw4tbOy21OY-WrXXqIkyyAW1kZpv1yejMm6d6QHDanObsrH9aMJkv79l9tpu0lk4kXM4ohAaUSbVJm47iOiRN2BNxnsHa4bymXFOCIkUYLtA+z0-BXjJIiZjem6Uhtqt6P97Z7MzyuTSFMw0fl6BGswajprEqrVvJg7tB2WCstsff2SPedA86-ufA39TrGuu1kWhLJeUWGQTf2PI=";

const std::string g_endpoint = "https://ecs.sc-cd-1.demo.com/v2/e38d227edcce4631be20bfa5aad7130b";

const std::string g_volDetail = "{\
    \"volume\": {\
        \"id\": \"d5fb423e-9bd1-429a-8441-91efdef2b5f1\",\
        \"links\": [\
            {\
                \"href\": \"https://evs.sc-cd-1.demo.com/v2/e38d227edcce4631be20bfa5aad7130b/volumes/d5fb423e-9bd1-429a-8441-91efdef2b5f1\",\
                \"rel\": \"self\"\
            },\
            {\
                \"href\": \"https://evs.sc-cd-1.demo.com/e38d227edcce4631be20bfa5aad7130b/volumes/d5fb423e-9bd1-429a-8441-91efdef2b5f1\",\
                \"rel\": \"bookmark\"\
            }\
        ],\
        \"name\": \"ecs-4d45-0001-volume-0000\",\
        \"status\": \"in-use\",\
        \"attachments\": [\
            {\
                \"server_id\": \"10a4e361-c981-46f2-b9ba-d7ff9c601693\",\
                \"attachment_id\": \"5aa3989e-acd7-467d-a436-9afdeb59865b\",\
                \"attached_at\": \"2022-07-13T02:38:31.273770\",\
                \"host_name\": null,\
                \"volume_id\": \"d5fb423e-9bd1-429a-8441-91efdef2b5f1\",\
                \"device\": \"/dev/vda\",\
                \"id\": \"d5fb423e-9bd1-429a-8441-91efdef2b5f1\"\
            }\
        ],\
        \"availability_zone\": \"az0.dc0\",\
        \"os-vol-host-attr:host\": \"cinder-kvm002@typei_global_bussiness_01#StoragePool003-bu\",\
        \"source_volid\": null,\
        \"snapshot_id\": null,\
        \"description\": null,\
        \"created_at\": \"2022-05-11T02:59:15.824963\",\
        \"volume_type\": \"business_type_01\",\
        \"os-vol-tenant-attr:tenant_id\": \"e38d227edcce4631be20bfa5aad7130b\",\
        \"size\": 10,\
        \"metadata\": {\
            \"take_over_lun_wwn\": \"--\",\
            \"tenancy\": \"0\",\
            \"lun_wwn\": \"658f987100b749bcc5d447af00000081\",\
            \"StorageType\": \"OceanStorV5\",\
            \"__sys_is_server_vol__\": \"true\",\
            \"readonly\": \"False\",\
            \"attached_mode\": \"rw\"\
        },\
        \"volume_image_metadata\": {\
            \"file_name\": \"**.qcow2\",\
            \"min_ram\": \"0\",\
            \"describe\": \"\",\
            \"__os_bit\": \"64\",\
            \"size\": \"1461190656\",\
            \"__os_version\": \"Astra Linux 1.4 64bit\",\
            \"__support_static_ip\": \"False\",\
            \"file_format\": \"qcow2\",\
            \"disk_format\": \"qcow2\",\
            \"container_format\": \"bare\",\
            \"__isregistered\": \"true\",\
            \"hw_disk_bus\": \"virtio\",\
            \"ori_disk_format\": \"qcow2\",\
            \"__quick_start\": \"False\",\
            \"cloudinit\": \"False\",\
            \"image_name\": \"linux-rh\",\
            \"hw_firmware_type\": \"bios\",\
            \"image_id\": \"a0a24ff5-1899-4c38-843a-659fd9d3ac15\",\
            \"__os_type\": \"Linux\",\
            \"min_disk\": \"10\",\
            \"__system_encrypted\": \"false\",\
            \"__support_kvm\": \"true\",\
            \"virtual_env_type\": \"FusionCompute\",\
            \"__support_live_resize\": \"False\",\
            \"__admin_encrypted\": \"false\",\
            \"__image_source_type\": \"glance\",\
            \"checksum\": \"e54d250e6e3699df1cb662678c298fca\",\
            \"__imagetype\": \"gold\",\
            \"__platform\": \"Other\",\
            \"architecture\": \"x86_64\",\
            \"__virtual_size\": \"10\",\
            \"is_auto_config\": \"false\"\
        },\
        \"os-vol-mig-status-attr:migstat\": null,\
        \"os-vol-mig-status-attr:name_id\": null,\
        \"encrypted\": false,\
        \"replication_status\": \"disabled\",\
        \"user_id\": \"d4216b7d3ba64a4eb63db37c2b91222c\",\
        \"consistencygroup_id\": null,\
        \"bootable\": \"true\",\
        \"updated_at\": \"2022-07-13T02:38:31.311402\",\
        \"shareable\": false,\
        \"multiattach\": false,\
        \"os-volume-replication:extended_status\": null,\
        \"os-volume-replication:driver_data\": \"{\\\"lun_id\\\": \\\"129\\\", \\\"sn\\\": \\\"2102351NPT10J3000001\\\"}\"\
    }\
}";

const std::string g_volDetailList = 
    "{\"count\": 1,\
    \"volumes\": [{\
    \"attachments\": [{\
    \"server_id\": \"78f14666-1309-421c-ae3b-60649c616b92\",\
    \"attachment_id\": \"08ba7927-b5f9-4e95-982e-bdb0d77751d6\",\
    \"attached_at\": \"2022-10-19T11:50:26.035705\",\
    \"host_name\": null,\
    \"volume_id\": \"6682771a-0214-4bab-8699-b54e4815d439\",\
    \"device\": \"/dev/sdc\",\
    \"id\": \"6682771a-0214-4bab-8699-b54e4815d439\"}],\
    \"metadata\": {\
    \"take_over_lun_wwn\": \"--\",\
    \"lun_wwn\": \"658f987100b749bc10fb2c7800000630\",\
    \"StorageType\": \"OceanStorV5\",\
    \"readonly\": \"False\",\
    \"tenancy\": \"0\",\
    \"hw:passthrough\": \"true\",\
    \"__sys_is_server_vol__\": \"true\",\
    \"attached_mode\": \"rw\"},\
    \"status\": \"in-use\",\
    \"volume_image_metadata\": {},\
    \"backup_id\": null}]}";

static bool Stub_GetTokenSuccess(void *obj, ModelBase &model, std::string &tokenStr, std::string &endPoint)
{
    tokenStr = g_tokenStr;
    endPoint = g_endpoint;
    return true;
}

bool Stub_GetTokenFailed(void *obj, ModelBase &model, std::string &tokenStr, std::string &endPoint)
{
    return false;
}

static Module::IHttpClient *Stub_ShowVolumeDetail_Success()
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
    EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(g_volDetail));
    std::set<std::string> headerValue;
    headerValue.insert(g_tokenStr);
    std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

    IHttpClientMock *httpClient = new (std::nothrow) IHttpClientMock();
    EXPECT_CALL(*httpClient, SendRequest(_, _)).WillRepeatedly(Return(httpRespone));
    return httpClient;
}

static Module::IHttpClient* StubShowVolumeDetailListSuccess()
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
    EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(g_volDetailList));
    std::set<std::string> headerValue;
    headerValue.insert(g_tokenStr);
    std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

    IHttpClientMock *httpClient = new (std::nothrow) IHttpClientMock();
    EXPECT_CALL(*httpClient, SendRequest(_, _)).WillRepeatedly(Return(httpRespone));
    return httpClient;
}

static Module::IHttpClient *Stub_ShowVolumeDetail_Failed()
{
    std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
    EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(200));
    EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(200));
    EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));
    std::set<std::string> GetHeadByNameReturn = {};
    EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
    EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(g_volDetail));
    std::set<std::string> headerValue;
    headerValue.insert(g_tokenStr);
    std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

    IHttpClientMock *httpClient = new (std::nothrow) IHttpClientMock();
    EXPECT_CALL(*httpClient, SendRequest(_, _)).WillRepeatedly(Return(httpRespone));
    return httpClient;
}

/*
 * 测试用例：获取卷信息成功
 * 前置条件：获取token成功、调用接口成功
 * CHECK点：获取到的卷id符合预期
 */
TEST_F(EvsClientTest, ShowVolumeDetail_Success)
{
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_ShowVolumeDetail_Success);
    ShowVolumeRequest request;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    request.SetUserInfo(auth);
    request.SetScopeValue("e38d227edcce4631be20bfa5aad7130b");
    request.SetDomain("domain");
    request.SetEndpoint("demo.com");
    request.SetRegion("sc-cd-1");
    request.SetVolumeId("volId");

    EvsClient evs;
    std::shared_ptr<ShowVolumeResponse> response = evs.ShowVolumeDetail(request);
    EXPECT_TRUE((response != nullptr));
    EXPECT_EQ(response->GetStatusCode(), 200);
    EXPECT_TRUE(response->Serial());
    HSCVolDetail volDetail = response->GetHSCVolDetail();
    EXPECT_EQ(volDetail.m_hostVolume.m_id, "d5fb423e-9bd1-429a-8441-91efdef2b5f1");
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例：获取卷信息失败
 * 前置条件：获取token失败
 * CHECK点：response返回nullptr
 */
TEST_F(EvsClientTest, ShowVolumeDetail_Failed)
{
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenFailed);
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_ShowVolumeDetail_Failed);
    ShowVolumeRequest request;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    request.SetUserInfo(auth);
    request.SetScopeValue("e38d227edcce4631be20bfa5aad7130b");
    request.SetDomain("domain");
    request.SetRegion("sc-cd-1");
    request.SetVolumeId("volId");

    EvsClient evs;
    std::shared_ptr<ShowVolumeResponse> response = evs.ShowVolumeDetail(request);
    EXPECT_TRUE((response == nullptr));
}

/*
 * 测试用例：获取卷信息成功
 * 前置条件：获取token成功、调用接口成功
 * CHECK点：获取到的卷id符合预期
 */
TEST_F(EvsClientTest, ShowVolumeDetailList_Success)
{
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenSuccess);
    stub.set(ADDR(Module::IHttpClient, GetInstance), StubShowVolumeDetailListSuccess);
    ShowVolumeListRequest request;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    request.SetUserInfo(auth);
    request.SetScopeValue("e38d227edcce4631be20bfa5aad7130b");
    request.SetDomain("domain");
    request.SetEndpoint("demo.com");
    request.SetRegion("sc-cd-1");
    request.SetVolumeStatus("in-use");
    request.SetVolumeOffset(0);
    request.SetVolumeLimit(1);

    EvsClient evs;
    std::shared_ptr<ShowVolumeDetailResponse> response = evs.ShowVolumeDetailList(request);
    EXPECT_TRUE((response != nullptr));
    EXPECT_EQ(response->GetStatusCode(), 200);
    EXPECT_TRUE(response->Serial());
    VolumeDetailList volDetail = response->GetHSCVolDetail();
    EXPECT_EQ(volDetail.m_volumeInfo.size(), 1);
    stub.reset(ADDR(Module::IHttpClient, GetInstance));
}

/*
 * 测试用例：获取卷信息失败
 * 前置条件：获取token失败
 * CHECK点：response返回nullptr
 */
TEST_F(EvsClientTest, ShowVolumeDetailList_Failed)
{
    Stub stub;
    stub.set(ADDR(HCSTokenMgr, GetToken), Stub_GetTokenFailed);
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_ShowVolumeDetail_Failed);
    ShowVolumeListRequest request;
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    request.SetUserInfo(auth);
    request.SetScopeValue("e38d227edcce4631be20bfa5aad7130b");
    request.SetDomain("domain");
    request.SetRegion("sc-cd-1");

    EvsClient evs;
    std::shared_ptr<ShowVolumeDetailResponse> response = evs.ShowVolumeDetailList(request);
    EXPECT_TRUE((response == nullptr));
}
}
