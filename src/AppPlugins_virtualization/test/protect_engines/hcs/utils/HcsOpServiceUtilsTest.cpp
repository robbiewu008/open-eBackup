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
#include <string.h>
#include <iostream>
#include <map>
#include <sys/socket.h> 
#include "curl_http/HttpClientInterface.h"
#include "common/client/RestClient.h"
#include <common/model/ModelBase.h>
#include <common/model/ResponseModel.h>
#include <common/httpclient/HttpClient.h>
#include "common/CommonMock.h"
#include "common/Structs.h"
#include "protect_engines/hcs/utils/HcsOpServiceUtils.h"
#include "volume_handlers/common/ControlDevice.h"
#include <log/Log.h>
#include "protect_engines/hcs/common/HcsHttpStatus.h"
#include "protect_engines/hcs/common/HcsCommonInfo.h"

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
const std::string g_hcsOpServiceAppEnvExtendInfoTest =
    "{ 	\"domain\" : \"demo3301.com\", 	\"hcs_token\" : "
    "\"{\\\"expires_at\\\":\\\"2023-09-05T23:37:25.105000Z\\\",\\\"token\\\":"
    "\\\"MIIE1QYJKoZIhvcNAQcCoIIExjCCBMICAQExDTALBglghkgBZQMEAgEwggKjBgkqhkiG9w0BBwGgggKUBIICkHsidG9rZW4iOnsiZXhwaXJlc1"
    "9hdCI6IjIwMjMtMDktMDVUMjM6Mzc6MjUuMTA1MDAwWiIsIm1ldGhvZHMiOlsiaHdfYXNzdW1lX3JvbGUiXSwicm9sZXMiOlt7Im5hbWUiOiJvcF9z"
    "ZXJ2aWNlIiwiaWQiOiJmYTE2M2ViMmFkYTY5NzRjMGFjZjdmYTkzNjAzYmQ3OCJ9XSwicHJvamVjdCI6eyJkb21haW4iOnsibmFtZSI6Ind1eXVhbn"
    "hpIiwiaWQiOiJjMDZhZmVlMWFmNDk0ZWNiYjM1MmFhMDVhMTZhMjc3NyJ9LCJuYW1lIjoic2EtZmItMV9jZXNoaSIsImlkIjoiZTVjNDM4YzQyNThk"
    "NDM0ODg4M2IzZDU1ZjVmNWMyMGUifSwiaXNzdWVkX2F0IjoiMjAyMy0wOS0wNFQyMzozNzoyNS4xMDUwMDBaIiwidXNlciI6eyJkb21haW4iOnsibm"
    "FtZSI6Ind1eXVhbnhpIiwiaWQiOiJjMDZhZmVlMWFmNDk0ZWNiYjM1MmFhMDVhMTZhMjc3NyJ9LCJuYW1lIjoid3V5dWFueGkvb3Bfc2VydmljZSIs"
    "ImlkIjoiY2NlOWI5MjYyYzJjNDViMWJmZGNmYzk2YzFmMWRkMzgifSwiYXNzdW1lZF9ieSI6eyJ1c2VyIjp7ImRvbWFpbiI6eyJuYW1lIjoib3Bfc2"
    "VydmljZSIsImlkIjoiZmExNjNlYjJhZGE2OTc0YzBhY2Y3ZmE5NDA3M2JkOGEifSwibmFtZSI6Im9wX3NlcnZpY2Vfb2NlYW5wcm90ZWN0IiwiaWQi"
    "OiI3ZGQwZDQwYTM2ZmI0OWNmOTQ4ZjY0N2E4NTY5NzhjOCJ9fX19MYICBTCCAgECAQEwXDBWMQswCQYDVQQGEwJDTjELMAkGA1UECAwCc2MxCzAJBg"
    "NVBAcMAmNkMQswCQYDVQQKDAJIVzEQMA4GA1UECwwHQ2xvdWRCVTEOMAwGA1UEAwwFdG9rZW4CAhAAMAsGCWCGSAFlAwQCATANBgkqhkiG9w0BAQEF"
    "AASCAYA4kw4ww2Ukc2uIXFiJ2KmTuWHqIOaqmsr8p57S26Vey0o-3mbeiKcDtbbeFiYocf+"
    "NtslPJ26NHTd7Ke5kZIEHs2heqClB83v2KZ5DnnDJCqsqIHWIoilYLR3mi6EXi0OUBHNE-v5Nxmjkg55My-"
    "KBZucDaBoBy8Qegj3I74O7liGaHYtmNiKTbbup8f6ak3zRDoxOnHlybbKn67WArejOr5eAq-"
    "DPH4X9QoB962Ol22TAeAC55DOempotMaPICGFKMtjPpPfB48Oc0s9GVOxQX1SG2Ri23P0kmP7745f3pvOTTwcuSGtlgnLYX8-M49YklFsQzhsIHT-"
    "cWXtMNy6oZi+MNmjZUx5zQVLbLVtpXBZJJITP5qDNexWRu5RTwTgbLLd31c5ibQ+DG75u+aMg4deyjDfsC9Ri2jSSAF13u+"
    "PckvACXqOC8c1sKC48tvg4ngYp6Rpq-7YLruu0c4hLolbRDIuLFb4x4alni9CgzLbz4Qn11xcdW9G7pCTr7UY=\\\"}\", 	\"projectId\" "
    ": null, 	\"project_id\" : \"e5c438c4258d4348883b3d55f5f5c20e\", 	\"regionId\" : null, 	\"region_id\" : "
    "\"sa-fb-1\" }";
const std::string g_hcsOpServiceAppEnvAuthExtendInfoTest =
    "{ 	\"enableCert\" : \"0\", 	\"storages\" : "
    "\"[{\\\"certName\\\":\\\"\\\",\\\"certSize\\\":\\\"\\\",\\\"crlName\\\":\\\"\\\",\\\"crlSize\\\":\\\"\\\","
    "\\\"enableCert\\\":\\\"0\\\",\\\"ip\\\":\\\"33.3.100.100\\\",\\\"password\\\":\\\"Admin@storage1\\\",\\\"port\\\":"
    "8088,\\\"storageType\\\":\\\"0\\\",\\\"username\\\":\\\"admin\\\"},{\\\"certName\\\":\\\"\\\",\\\"certSize\\\":"
    "\\\"\\\",\\\"crlName\\\":\\\"\\\",\\\"crlSize\\\":\\\"\\\",\\\"enableCert\\\":\\\"0\\\",\\\"ip\\\":\\\"8.46.251."
    "145\\\",\\\"password\\\":\\\"AAAAAgAAAAAAAAAAAAAAAQAAAAliNek1uiVU4an0grP0FyGhwZ/6rUb/"
    "n0q7UPNNAAAAAAAAAAAAAAAAAAAAFCdZGU1zApXOm6nzpbZSDkjHbLIT\\\",\\\"port\\\":8088,\\\"storageType\\\":\\\"0\\\","
    "\\\"username\\\":\\\"admin\\\"}]\" } ";

class HcsOpServiceUtilsTest : public testing::Test {
protected:
    void SetUp()
    {
        AppProtect::ApplicationEnvironment g_appEnvTest;
        g_appEnvTest.__set_extendInfo(g_hcsOpServiceAppEnvExtendInfoTest);
        g_appEnvTest.auth.__set_extendInfo(g_hcsOpServiceAppEnvAuthExtendInfoTest);
        hcsOpServiceUtilsTest = HcsOpServiceUtils::GetInstance();
        hcsOpServiceUtilsTest->GetAppEnv(g_appEnvTest);
    }
    void TearDown()
    {
        hcsOpServiceUtilsTest->m_isOpServiceEnv = false;
    }

public:
    HcsOpServiceUtils *hcsOpServiceUtilsTest;
};

const std::string g_hcsOpServiceToken =
    "MIIE1QYJKoZIhvcNAQcCoIIExjCCBMICAQExDTALBglghkgBZQMEAgEwggKjBgkqhkiG9w0BBwGgggKUBIICkHsidG9rZW4iOnsiZXhwaXJlc19hdC"
    "I6IjIwMjMtMDktMDVUMjM6Mzc6MjUuMTA1MDAwWiIsIm1ldGhvZHMiOlsiaHdfYXNzdW1lX3JvbGUiXSwicm9sZXMiOlt7Im5hbWUiOiJvcF9zZXJ2"
    "aWNlIiwiaWQiOiJmYTE2M2ViMmFkYTY5NzRjMGFjZjdmYTkzNjAzYmQ3OCJ9XSwicHJvamVjdCI6eyJkb21haW4iOnsibmFtZSI6Ind1eXVhbnhpIi"
    "wiaWQiOiJjMDZhZmVlMWFmNDk0ZWNiYjM1MmFhMDVhMTZhMjc3NyJ9LCJuYW1lIjoic2EtZmItMV9jZXNoaSIsImlkIjoiZTVjNDM4YzQyNThkNDM0"
    "ODg4M2IzZDU1ZjVmNWMyMGUifSwiaXNzdWVkX2F0IjoiMjAyMy0wOS0wNFQyMzozNzoyNS4xMDUwMDBaIiwidXNlciI6eyJkb21haW4iOnsibmFtZS"
    "I6Ind1eXVhbnhpIiwiaWQiOiJjMDZhZmVlMWFmNDk0ZWNiYjM1MmFhMDVhMTZhMjc3NyJ9LCJuYW1lIjoid3V5dWFueGkvb3Bfc2VydmljZSIsImlk"
    "IjoiY2NlOWI5MjYyYzJjNDViMWJmZGNmYzk2YzFmMWRkMzgifSwiYXNzdW1lZF9ieSI6eyJ1c2VyIjp7ImRvbWFpbiI6eyJuYW1lIjoib3Bfc2Vydm"
    "ljZSIsImlkIjoiZmExNjNlYjJhZGE2OTc0YzBhY2Y3ZmE5NDA3M2JkOGEifSwibmFtZSI6Im9wX3NlcnZpY2Vfb2NlYW5wcm90ZWN0IiwiaWQiOiI3"
    "ZGQwZDQwYTM2ZmI0OWNmOTQ4ZjY0N2E4NTY5NzhjOCJ9fX19MYICBTCCAgECAQEwXDBWMQswCQYDVQQGEwJDTjELMAkGA1UECAwCc2MxCzAJBgNVBA"
    "cMAmNkMQswCQYDVQQKDAJIVzEQMA4GA1UECwwHQ2xvdWRCVTEOMAwGA1UEAwwFdG9rZW4CAhAAMAsGCWCGSAFlAwQCATANBgkqhkiG9w0BAQEFAASC"
    "AYA4kw4ww2Ukc2uIXFiJ2KmTuWHqIOaqmsr8p57S26Vey0o-3mbeiKcDtbbeFiYocf+"
    "NtslPJ26NHTd7Ke5kZIEHs2heqClB83v2KZ5DnnDJCqsqIHWIoilYLR3mi6EXi0OUBHNE-v5Nxmjkg55My-"
    "KBZucDaBoBy8Qegj3I74O7liGaHYtmNiKTbbup8f6ak3zRDoxOnHlybbKn67WArejOr5eAq-"
    "DPH4X9QoB962Ol22TAeAC55DOempotMaPICGFKMtjPpPfB48Oc0s9GVOxQX1SG2Ri23P0kmP7745f3pvOTTwcuSGtlgnLYX8-M49YklFsQzhsIHT-"
    "cWXtMNy6oZi+MNmjZUx5zQVLbLVtpXBZJJITP5qDNexWRu5RTwTgbLLd31c5ibQ+DG75u+aMg4deyjDfsC9Ri2jSSAF13u+"
    "PckvACXqOC8c1sKC48tvg4ngYp6Rpq-7YLruu0c4hLolbRDIuLFb4x4alni9CgzLbz4Qn11xcdW9G7pCTr7UY=";
const std::string g_hcsOpServiceExpireTime = "2023-09-05T23:37:25.105000Z";
const std::string g_hcsOpServiceRegion = "sa-fb-1";
const std::string g_hcsOpServiceDomain = "demo3301.com";
const std::string g_hcsOpServiceProjectId = "e5c438c4258d4348883b3d55f5f5c20e";

static bool ExecStubTrue()
{
    return true;
}

static bool ExecStubFalse()
{
    return false;
}

template<typename T>
static bool Stub_JsonStringToStructFalse(const std::string &inputString, T &outputStruct)
{
    return false;
}

/*
 * 测试用例：获得op服务化参数
 * 前置条件：OP服务化场景需要获得相应的参数，实际上根据是否有token传入判断是不是OP服务化场景
 * CHECK点：获得OP服务化参数成功
 */
TEST_F(HcsOpServiceUtilsTest, GetOpServiceInfoSuccess)
{
    bool ret = hcsOpServiceUtilsTest->GetOpServiceInfo();
    EXPECT_EQ(ret, true);
}

/*
 * 测试用例：获得op服务化参数
 * 前置条件：OP服务化场景需要获得相应的参数，实际上根据是否有token传入判断是不是OP服务化场景
 * CHECK点：获得OP服务化参数失败
 */
TEST_F(HcsOpServiceUtilsTest, GetOpServiceInfoFailed)
{
    Stub stub;
    stub.set(ADDR(HcsOpServiceUtils, SetTokenInfo), ExecStubFalse);
    bool ret = hcsOpServiceUtilsTest->GetOpServiceInfo();
    EXPECT_EQ(ret, false);
}

/*
 * 测试用例：解析OP服务化场景的token信息
 * 前置条件：OP服务化场景的token是PM直接下发
 * CHECK点：解析OP服务化场景的token信息成功
 */
TEST_F(HcsOpServiceUtilsTest, SetTokenInfoSuccess)
{
    bool ret = hcsOpServiceUtilsTest->SetTokenInfo();
    EXPECT_EQ(ret, true);
}

/*
 * 测试用例：获得OP服务化工具类的token
 * 前置条件：需要获得HCS环境的token
 * CHECK点：获得token
 */
TEST_F(HcsOpServiceUtilsTest, GetTokenSuccess)
{
    std::string retString = hcsOpServiceUtilsTest->GetToken();
    EXPECT_EQ(retString, g_hcsOpServiceToken);
}

/*
 * 测试用例：获得OP服务化工具类的token过期时间
 * 前置条件：需要获得HCS环境的token的过期时间
 * CHECK点：获得token过期时间
 */
TEST_F(HcsOpServiceUtilsTest, GetExpireAtTimeSuccess)
{
    std::string retString = hcsOpServiceUtilsTest->GetExpireAtTime();
    EXPECT_EQ(retString, g_hcsOpServiceExpireTime);
}

/*
 * 测试用例：获得OP服务化工具类的region
 * 前置条件：需要获得HCS环境的region
 * CHECK点：获得region
 */
TEST_F(HcsOpServiceUtilsTest, GetRegionSuccess)
{
    std::string retString = hcsOpServiceUtilsTest->GetRegion();
    EXPECT_EQ(retString, g_hcsOpServiceRegion);
}

/*
 * 测试用例：获得OP服务化工具类的域名
 * 前置条件：需要获得HCS环境的域名
 * CHECK点：获得hcs的域名
 */
TEST_F(HcsOpServiceUtilsTest, GetDomainSuccess)
{
    std::string retString = hcsOpServiceUtilsTest->GetDomain();
    EXPECT_EQ(retString, g_hcsOpServiceDomain);
}

/*
 * 测试用例：获得OP服务化工具类的region
 * 前置条件：需要获得HCS环境的region
 * CHECK点：获得ProjectId
 */
TEST_F(HcsOpServiceUtilsTest, GetProjectIdSuccess)
{
    std::string retString = hcsOpServiceUtilsTest->GetProjectId();
    EXPECT_EQ(retString, g_hcsOpServiceProjectId);
}

/*
 * 测试用例：判断是否为OP服务化场景
 * 前置条件：需要判断是不是OP服务化场景的任务
 * CHECK点：获得是否是OP服务化场景
 */
TEST_F(HcsOpServiceUtilsTest, GetIsOpServiceEnvSuccess)
{
    bool ret = hcsOpServiceUtilsTest->GetIsOpServiceEnv();
    EXPECT_EQ(ret, true);
}
}