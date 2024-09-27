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
#include "common/CommonMock.h"
#include "protect_engines/openstack/utils/OpenStackTokenMgr.h"

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
using OpenStackPlugin::OpenStackTokenMgr;


namespace HDT_TEST {
std::string token = "MIIEhwYJKoZIhvcNAQcCoIIEeDCCBHQCAQExDTALBglghkgBZQMEAgEwggLoBgkqhkiG9w0BBwGgggLZBIIC1XsidG9rZW4iOnsiZXhwaXJlc19hdCI6IjIwMjItMDctMjJUMDc6NTk6NTkuODY5MDAwWiIsIm1ldGhvZHMiOlsicGFzc3dvcmQiXSwiY2F0YWxvZyI6W10sInJvbGVzIjpbeyJuYW1lIjoidmRjX2FkbSIsImlkIjoiY2E3MWU3NzFiYWZjNDI5OTkwOThmMDg4YTc4NGM3NTEifSx7Im5hbWUiOiJ0YWdfYWRtIiwiaWQiOiJkNmJiNWRiZjc0YjQ0Yjk1YWFmMmU3MGJmYzcyNTE0ZiJ9LHsibmFtZSI6ImFwcHJvdl9hZG0iLCJpZCI6IjBiZDBmZjlhMWZkNDRiNzc5MzZlNWUzNzExODZhMzI1In0seyJuYW1lIjoidmRjX293bmVyIiwiaWQiOiI0Nzg4ZjYyMzhmZDM0MWNjYmZkOGQwYzQzMzg4YjdlZSJ9LHsibmFtZSI6InRlX2FkbWluIiwiaWQiOiJhYzgyMWRkN2EwZDI0ZDI2OGI4ZGE2MDg0ZmRlNmQ3OCJ9XSwicHJvamVjdCI6eyJkb21haW4iOnsibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiOTkwNzYzNjFiOTVmNDIyNmIxOGRiMDAwMTU1NWJkMDAifSwibmFtZSI6InNjLWNkLTFfdGVzdCIsImlkIjoiZTM4ZDIyN2VkY2NlNDYzMWJlMjBiZmE1YWFkNzEzMGIifSwiaXNzdWVkX2F0IjoiMjAyMi0wNy0yMVQwNzo1OTo1OS44NjkwMDBaIiwidXNlciI6eyJkb21haW4iOnsibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiOTkwNzYzNjFiOTVmNDIyNmIxOGRiMDAwMTU1NWJkMDAifSwibmFtZSI6Imh1YW5ncm9uZyIsImlkIjoiZDQyMTZiN2QzYmE2NGE0ZWI2M2RiMzdjMmI5MTIyMmMifX19MYIBcjCCAW4CAQEwSTA9MQswCQYDVQQGEwJDTjEPMA0GA1UEChMGSHVhd2VpMR0wGwYDVQQDExRIdWF3ZWkgSVQgUHJvZHVjdCBDQQIIFamRIbpBmrcwCwYJYIZIAWUDBAIBMA0GCSqGSIb3DQEBAQUABIIBAGkKLMyXHOFwT4nqe4Iue5g59bBMsIAhW-bhq0MIiJklULEo8RDH+hX5e8AQ44K1Dv2KKXSctXqZoIjW+SeRFxSQm8Ifp-mw18gDn6F+DZRE1ZS+CeecSG8BmXutAfhd9YJQ2xRcw4tbOy21OY-WrXXqIkyyAW1kZpv1yejMm6d6QHDanObsrH9aMJkv79l9tpu0lk4kXM4ohAaUSbVJm47iOiRN2BNxnsHa4bymXFOCIkUYLtA+z0-BXjJIiZjem6Uhtqt6P97Z7MzyuTSFMw0fl6BGswajprEqrVvJg7tB2WCstsff2SPedA86-ufA39TrGuu1kWhLJeUWGQTf2PI=";
std::string getTokenRespBody = "{\
	\"token\": {\
		\"methods\": [\
			\"password\"\
		],\
		\"user\": {\
			\"domain\": {\
				\"id\": \"default\",\
				\"name\": \"Default\"\
			},\
			\"id\": \"4216fc46c862488e80ef891e1dea8f1a\",\
			\"name\": \"cloud_admin\",\
			\"password_expires_at\": null\
		},\
		\"audit_ids\": [\
			\"64osPo2-SuGxSmbja0oIVA\"\
		],\
		\"expires_at\": \"2022-11-29T14:35:27.000000Z\",\
		\"issued_at\": \"2022-11-29T08:35:27.000000Z\",\
		\"project\": {\
			\"domain\": {\
				\"id\": \"default\",\
				\"name\": \"Default\"\
			},\
			\"id\": \"9d40311132f840708ad10494f1429452\",\
			\"name\": \"admin\"\
		},\
		\"is_domain\": false,\
		\"roles\": [{\
			\"id\": \"40f6817c2b5f4d4f87d089d9d554041a\",\
			\"name\": \"admin\"\
		}],\
		\"catalog\": [{\
				\"endpoints\": [{\
						\"id\": \"57998f3b538240c9bca1949a320eab44\",\
						\"interface\": \"internal\",\
						\"region_id\": \"az1.dc1\",\
						\"url\": \"https://volume.localdomain.com:8776/v2/9d40311132f840708ad10494f1429452\",\
						\"region\": \"az1.dc1\"\
					},\
					{\
						\"id\": \"e5f73dcad98b4bce8d05aa5a3acab6b1\",\
						\"interface\": \"admin\",\
						\"region_id\": \"az1.dc1\",\
						\"url\": \"https://volume.az1.dc1.domainname.com:443/v2/9d40311132f840708ad10494f1429452\",\
						\"region\": \"az1.dc1\"\
					},\
					{\
						\"id\": \"f96efe4a425a4b778d4f7ad7435cd22a\",\
						\"interface\": \"public\",\
						\"region_id\": \"az1.dc1\",\
						\"url\": \"https://volume.az1.dc1.domainname.com:443/v2/9d40311132f840708ad10494f1429452\",\
						\"region\": \"az1.dc1\"\
					}\
				],\
				\"id\": \"bf77e5cefb36454895fe95c2a76ea7d1\",\
				\"type\": \"volumev2\",\
				\"name\": \"cinderv2\"\
			},\
			{\
				\"endpoints\": [{\
						\"id\": \"fdaa181faccf448d864f7d2efc9f4a1e\",\
						\"interface\": \"internal\",\
						\"region_id\": \"az1.dc1\",\
						\"url\": \"https://compute.localdomain.com:8001/v2.1/9d40311132f840708ad10494f1429452\",\
						\"region\": \"az1.dc1\"\
					},\
					{\
						\"id\": \"e2b80bdf6b1f46a7a6d5c5bf7545901d\",\
						\"interface\": \"admin\",\
						\"region_id\": \"az1.dc1\",\
						\"url\": \"https://compute.az1.dc1.domainname.com:443/v2.1/9d40311132f840708ad10494f1429452\",\
						\"region\": \"az1.dc1\"\
					},\
					{\
						\"id\": \"4bfe354be9b247e195b86b1b36eb9b08\",\
						\"interface\": \"public\",\
						\"region_id\": \"az1.dc1\",\
						\"url\": \"https://compute.az1.dc1.domainname.com:443/v2.1/9d40311132f840708ad10494f1429452\",\
						\"region\": \"az1.dc1\"\
					}\
				],\
				\"id\": \"cc7eb58f7e7948d8817381b5b658c15e\",\
				\"type\": \"compute\",\
				\"name\": \"nova\"\
			},\
			{\
				\"endpoints\": [{\
						\"id\": \"a8ae522705664b3e88b6025235fc5616\",\
						\"interface\": \"internal\",\
						\"region_id\": \"az1.dc1\",\
						\"url\": \"https://volume.localdomain.com:8776/v2/9d40311132f840708ad10494f1429452\",\
						\"region\": \"az1.dc1\"\
					},\
					{\
						\"id\": \"1adedf8ef7f5439bac7cf80ae779c71e\",\
						\"interface\": \"admin\",\
						\"region_id\": \"az1.dc1\",\
						\"url\": \"https://volume.az1.dc1.domainname.com:443/v2/9d40311132f840708ad10494f1429452\",\
						\"region\": \"az1.dc1\"\
					},\
					{\
						\"id\": \"2951ce922ea440efa629507a0eb1b863\",\
						\"interface\": \"public\",\
						\"region_id\": \"az1.dc1\",\
						\"url\": \"https://volume.az1.dc1.domainname.com:443/v2/9d40311132f840708ad10494f1429452\",\
						\"region\": \"az1.dc1\"\
					}\
				],\
				\"id\": \"ba0fe1f300ab4541a94edd7a70bc512e\",\
				\"type\": \"volume\",\
				\"name\": \"cinder\"\
			},\
			{\
				\"endpoints\": [{\
						\"id\": \"bb2f5398f6e1438f82c9f7888242b406\",\
						\"interface\": \"internal\",\
						\"region_id\": \"az1.dc1\",\
						\"url\": \"https://volume.localdomain.com:8776/v3/9d40311132f840708ad10494f1429452\",\
						\"region\": \"az1.dc1\"\
					},\
					{\
						\"id\": \"e114f06de6514feca92f14244c15e628\",\
						\"interface\": \"admin\",\
						\"region_id\": \"az1.dc1\",\
						\"url\": \"https://volume.az1.dc1.domainname.com:443/v3/9d40311132f840708ad10494f1429452\",\
						\"region\": \"az1.dc1\"\
					},\
					{\
						\"id\": \"3b6651030a054272ac358ac5cf798a64\",\
						\"interface\": \"public\",\
						\"region_id\": \"az1.dc1\",\
						\"url\": \"https://volume.az1.dc1.domainname.com:443/v3/9d40311132f840708ad10494f1429452\",\
						\"region\": \"az1.dc1\"\
					}\
				],\
				\"id\": \"3c994e6bfb8d43c3af958a5a7ea8bc90\",\
				\"type\": \"volumev3\",\
				\"name\": \"cinderv3\"\
			}\
		]\
	}\
}";

class OpenStackTokenMgrTest : public testing::Test {
protected:
    void SetUp() 
    {
        OpenStackTokenMgr::GetInstance().m_tokenMap.clear();
		m_stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CreateClient_SendRequest);
    };
    void TearDown()
    {
        m_stub.reset(ADDR(Module::IHttpClient, GetInstance));
    };
    Stub m_stub;
};


static bool Stub_TokenExpired()
{
    return true;
}

static bool Stub_TokenNotExpired()
{
    return false;
}

void RequestMock(ModelBaseMock &request)
{
    AuthObj auth;
    auth.name = "name";
    auth.passwd = "passwd";
    request.SetUserInfo(auth);
    request.SetScopeValue("e38d227edcce4631be20bfa5aad7130b");
    request.SetDomain("domain");
    request.SetEndpoint("demo.com");
    EXPECT_CALL(request, GetScopeType()).WillRepeatedly(Return(Scope::USER_DOMAIN));
    EXPECT_CALL(request, GetApiType()).WillRepeatedly(Return(ApiType::NOVA));
}

/*
 * 测试用例： 1. 调用openstackmgr接口获取GetToken成功(发送消息获取token). 2. 从内存中获取到token
 * 前置条件： token过期，重新获取token成功，解析endpoint成功
 * CHECK点： 1. 接口调用成功  2. endpoint获取正确
 */
TEST_F(OpenStackTokenMgrTest, GetTokenSuccess)
{
    std::string tokenStr, endpoint;
    ModelBaseMock request;
    RequestMock(request);
    request.SetEnvAddress("identity.az1.dc1.domainname.com");
	g_httpStatusCode = 201;
	g_httpResponsebody = getTokenRespBody;
	g_httpResponseHeader = token;
    m_stub.set(ADDR(BaseTokenMgr, TokenIsExpired), Stub_TokenExpired);
    bool ret = OpenStackTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint);
    EXPECT_TRUE(ret);
    EXPECT_EQ(tokenStr, token);
    EXPECT_EQ(endpoint, "https://compute.az1.dc1.domainname.com:443/v2.1/9d40311132f840708ad10494f1429452");
    m_stub.reset(ADDR(BaseTokenMgr, TokenIsExpired));
    m_stub.set(ADDR(BaseTokenMgr, TokenIsExpired), Stub_TokenNotExpired);
    tokenStr = "";
    endpoint = "";
    // 再次获取token, 从内存中获取到token
    ret = OpenStackTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint);
    EXPECT_TRUE(ret);
    EXPECT_EQ(tokenStr, g_httpResponseHeader);
    EXPECT_EQ(endpoint, "https://compute.az1.dc1.domainname.com:443/v2.1/9d40311132f840708ad10494f1429452");
}

/*
 * 测试用例： 调用openstackmgr接口获取GetToken失败
 * 前置条件： 1. 发送接口获取token失败返回400
 * CHECK点： 1. GetToken返回false
 */
TEST_F(OpenStackTokenMgrTest, GetTokenFailed_SendHttpFailed)
{
    std::string tokenStr, endpoint;
    ModelBaseMock request;
    RequestMock(request);
    request.SetEnvAddress("identity.az1.dc1.domainname.com");
	g_httpStatusCode = 400;
	g_httpResponseHeader = token;
	g_httpResponsebody = getTokenRespBody;
    m_stub.set(ADDR(BaseTokenMgr, TokenIsExpired), Stub_TokenExpired);
    bool ret = OpenStackTokenMgr::GetInstance().GetToken(request, tokenStr, endpoint);
    EXPECT_FALSE(ret);
    m_stub.reset(ADDR(BaseTokenMgr, TokenIsExpired));
}

/*
 * 测试用例： 调用TokenIsExpired接口判断Token未过期
 * 前置条件： TokenIsExpired返回false
 * CHECK点
 */
TEST_F(OpenStackTokenMgrTest, TokenNotExpiredTest)
{
	std::string date = "2033-01-10T14:13:37.977000Z";
	bool ret = OpenStackTokenMgr::GetInstance().TokenIsExpired(date);
	EXPECT_FALSE(ret);
}

/*
 * 测试用例： 调用TokenIsExpired接口判断Token已过期
 * 前置条件： TokenIsExpired返回true
 * CHECK点
 */
TEST_F(OpenStackTokenMgrTest, TokenExpiredTest)
{
	std::string date = "2022-01-10T14:13:37.977000Z";
	bool ret = OpenStackTokenMgr::GetInstance().TokenIsExpired(date);
	EXPECT_TRUE(ret);
}
}
