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
#include <protect_engines/kubernetes/common/KubeErrorCodes.h>
#include <iostream>
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "stub.h"
#include "addr_pri.h"
#include "json/json.h"
#include "common/Constants.h"
#include "common/Structs.h"
#include "common/utils/Utils.h"
#include "protect_engines/kubernetes/rest/client/StorageClient.h"
#include "protect_engines/kubernetes/common/KubeCommonInfo.h"
#include "common/JsonUtils.h"
#include "stub.h"
#include <log/Log.h>

using  Module::HttpRequest;
using  Module::IHttpResponse;
using  Module::IHttpClient;
using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
using namespace KubernetesPlugin;

namespace HDT_TEST {
class StorageClientTest : public testing::Test {
public:
    void SetUp();

    void TearDown();

    static void SetUpTestCase();

    static void TearDownTestCase();
};

void StorageClientTest::SetUp(){}

void StorageClientTest::TearDown() {}

void StorageClientTest::SetUpTestCase() {}

void StorageClientTest::TearDownTestCase() {}

int32_t SendStorageApiSuccess(HttpRequest &httpParam, HttpResponseInfo &httpResponse)
{
    httpResponse.m_statusCode = 200;
    httpResponse.cookies = {
            "session=ismsession=EA27698A079EBAC777077D174395232FFE67D86ECB094E274D61E2015214467D;SameSite=Lax;path=/;httponly;secure;"};
    if (httpParam.url.find("/deviceManager/rest/xxxxx/sessions") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{\"accountstate\":1,\"deviceid\":\"2102353GTD10L9000007\",\"iBaseToken\":\"57B90CC0894098717CD5516EE5DDBCA79F072BABF60A4A6D936FE5FFDD01090C\",\"lastloginip\":\"8.40.99.131\",\"lastlogintime\":1656339028,\"pwdchangetime\":1654707090,\"roleId\":\"1\",\"usergroup\":\"\",\"userid\":\"admin\",\"username\":\"admin\",\"userscope\":\"0\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if (httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/system/") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{\"CACHEWRITEQUOTA\":\"333\",\"CONFIGMODEL\":\"1\",\"DESCRIPTION\":\"\",\"DOMAINNAME\":\"\",\"FREEDISKSCAPACITY\":\"0\",\"HEALTHSTATUS\":\"1\",\"HOTSPAREDISKSCAPACITY\":\"0\",\"ID\":\"2102353GTD10L9000007\",\"LOCATION\":\"\",\"MEMBERDISKSCAPACITY\":\"60000000000\",\"NAME\":\"Huawei.Storage\",\"PRODUCTMODE\":\"821\",\"PRODUCTVERSION\":\"V600R005C21\",\"RUNNINGSTATUS\":\"1\",\"SECTORSIZE\":\"512\",\"STORAGEPOOLCAPACITY\":\"33030913444\",\"STORAGEPOOLFREECAPACITY\":\"28565410974\",\"STORAGEPOOLHOSTSPARECAPACITY\":\"6606182688\",\"STORAGEPOOLRAWCAPACITY\":\"52849461510\",\"STORAGEPOOLUSEDCAPACITY\":\"4465502470\",\"THICKLUNSALLOCATECAPACITY\":\"0\",\"THICKLUNSUSEDCAPACITY\":\"-1\",\"THINLUNSALLOCATECAPACITY\":\"108406352\",\"THINLUNSMAXCAPACITY\":\"1887436800\",\"THINLUNSUSEDCAPACITY\":\"-1\",\"TOTALCAPACITY\":\"60000000000\",\"TYPE\":201,\"UNAVAILABLEDISKSCAPACITY\":\"0\",\"USEDCAPACITY\":\"4465502470\",\"VASA_ALTERNATE_NAME\":\"Huawei.Storage\",\"VASA_SUPPORT_BLOCK\":\"FC,FCOE,ISCSI,Others\",\"VASA_SUPPORT_FILESYSTEM\":\"\",\"VASA_SUPPORT_PROFILE\":\"BlockDeviceProfile,CapabilityProfile,VirtualVolumeProfile\",\"WRITETHROUGHSW\":\"1\",\"WRITETHROUGHTIME\":\"192\",\"mappedLunsCountCapacity\":\"0\",\"patchVersion\":\"\",\"pointRelease\":\"1.2.1RC1\",\"productModeString\":\"OceanProtect X8000\",\"unMappedLunsCountCapacity\":\"1887436800\",\"userFreeCapacity\":\"68233794713\",\"wwn\":\"21004cf55b9769b4\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if (httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/lun") != std::string::npos) {
        httpResponse.m_body = "{\"data\":[{\"ALLOCCAPACITY\":\"0\",\"ALLOCTYPE\":\"1\",\"CAPABILITY\":\"3\",\"CAPACITY\":\"209715200\",\"CAPACITYALARMLEVEL\":\"2\",\"COMPRESSION\":\"0\",\"COMPRESSIONSAVEDCAPACITY\":\"0\",\"COMPRESSIONSAVEDRATIO\":\"0\",\"DEDUPSAVEDCAPACITY\":\"0\",\"DEDUPSAVEDRATIO\":\"0\",\"DESCRIPTION\":\"\",\"DISGUISEREMOTEARRAYID\":\"--\",\"DISGUISESTATUS\":\"0\",\"DRS_ENABLE\":\"false\",\"ENABLECOMPRESSION\":\"true\",\"ENABLEISCSITHINLUNTHRESHOLD\":\"false\",\"ENABLESMARTDEDUP\":\"false\",\"EXPOSEDTOINITIATOR\":\"false\",\"EXTENDIFSWITCH\":\"false\",\"HASRSSOBJECT\":\"{\\\"SnapShot\\\":\\\"FALSE\\\",\\\"LunCopy\\\":\\\"FALSE\\\",\\\"RemoteReplication\\\":\\\"FALSE\\\",\\\"SplitMirror\\\":\\\"FALSE\\\",\\\"LunMigration\\\":\\\"FALSE\\\",\\\"LUNMirror\\\":\\\"FALSE\\\",\\\"HyperMetro\\\":\\\"FALSE\\\",\\\"LunClone\\\":\\\"FALSE\\\",\\\"HyperCopy\\\":\\\"FALSE\\\",\\\"HyperCDP\\\":\\\"FALSE\\\",\\\"CloudBackup\\\":\\\"FALSE\\\",\\\"drStar\\\":\\\"FALSE\\\"}\",\"HEALTHSTATUS\":\"1\",\"HYPERCDPSCHEDULEDISABLE\":\"0\",\"ID\":\"12\",\"IOCLASSID\":\"\",\"IOPRIORITY\":\"1\",\"ISADD2LUNGROUP\":\"false\",\"ISCHECKZEROPAGE\":\"true\",\"ISCLONE\":\"false\",\"ISCSITHINLUNTHRESHOLD\":\"90\",\"MIRRORPOLICY\":\"1\",\"MIRRORTYPE\":\"0\",\"NAME\":\"snap1\",\"NGUID\":\"71009769b4056cb54cf55bcd0000000c\",\"OWNINGCONTROLLER\":\"--\",\"PARENTID\":\"0\",\"PARENTNAME\":\"s0\",\"PREFETCHPOLICY\":\"0\",\"PREFETCHVALUE\":\"0\",\"REMOTELUNID\":\"--\",\"REPLICATION_CAPACITY\":\"0\",\"RUNNINGSTATUS\":\"27\",\"RUNNINGWRITEPOLICY\":\"1\",\"SC_HITRAGE\":\"0\",\"SECTORSIZE\":\"512\",\"SMARTCACHEPARTITIONID\":\"\",\"SNAPSHOTSCHEDULEID\":\"--\",\"SUBTYPE\":\"0\",\"THINCAPACITYUSAGE\":\"0\",\"TOTALSAVEDCAPACITY\":\"0\",\"TOTALSAVEDRATIO\":\"0\",\"TYPE\":11,\"USAGETYPE\":\"0\",\"WORKINGCONTROLLER\":\"--\",\"WORKLOADTYPEID\":\"1026\",\"WORKLOADTYPENAME\":\"San_Backup_Default\",\"WRITEPOLICY\":\"1\",\"WWN\":\"64cf55b1009769b4056cb5cd0000000c\",\"blockDeviceName\":\"--\",\"devController\":\"--\",\"functionType\":\"2\",\"grainSize\":\"64\",\"hyperCdpScheduleId\":\"0\",\"isShowDedupAndCompression\":\"true\",\"lunCgId\":\"0\",\"mapped\":\"false\",\"remoteLunWwn\":\"--\",\"serviceEnabled\":\"false\",\"takeOverLunWwn\":\"--\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if (httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/snapshot/stop") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":0,\"description\":\"0\"}}";
    }  else if (httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/snapshot/activate") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if (httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/snapshot") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{\"CASCADEDLEVEL\":\"0\",\"CASCADEDNUM\":\"0\",\"CONSUMEDCAPACITY\":\"0\",\"DESCRIPTION\":\"\",\"EXPOSEDTOINITIATOR\":\"false\",\"HEALTHSTATUS\":\"1\",\"HYPERCOPYIDS\":\"\",\"ID\":\"31\",\"IOCLASSID\":\"\",\"IOPRIORITY\":\"1\",\"ISSCHEDULEDSNAP\":\"0\",\"NAME\":\"LUN012_snap_212323212\",\"NGUID\":\"71009769b41364bb4cf55b660000001f\",\"PARENTID\":\"11\",\"PARENTNAME\":\"LUN012\",\"PARENTTYPE\":11,\"ROLLBACKENDTIME\":\"-1\",\"ROLLBACKRATE\":\"-1\",\"ROLLBACKSPEED\":\"2\",\"ROLLBACKSTARTTIME\":\"-1\",\"ROLLBACKTARGETOBJID\":\"4294967295\",\"ROLLBACKTARGETOBJNAME\":\"--\",\"RUNNINGSTATUS\":\"43\",\"SOURCELUNCAPACITY\":\"209715200\",\"SOURCELUNID\":\"11\",\"SOURCELUNNAME\":\"LUN012\",\"SUBTYPE\":\"0\",\"TIMESTAMP\":\"1657019535\",\"TYPE\":27,\"USERCAPACITY\":\"209715200\",\"WORKINGCONTROLLER\":\"--\",\"WORKLOADTYPEID\":\"1026\",\"WORKLOADTYPENAME\":\"San_Backup_Default\",\"WWN\":\"64cf55b1009769b41364bb660000001f\",\"coupleUuid\":\"1f00900891d94400b469975bf54c0021\",\"isReadOnly\":\"0\",\"replicationCapacity\":\"0\",\"snapCgId\":\"4294967295\",\"snapCgName\":\"--\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if (httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/sessions") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":0,\"description\":\"0\"}}";
    }
    return Module::SUCCESS;
}

int32_t SendStorageApiSuccess2(HttpRequest &httpParam, HttpResponseInfo &httpResponse)
{
    httpResponse.m_statusCode = 200;
    httpResponse.cookies = {
            "session=ismsession=EA27698A079EBAC777077D174395232FFE67D86ECB094E274D61E2015214467D;SameSite=Lax;path=/;httponly;secure;"};
    if (httpParam.url.find("/deviceManager/rest/xxxxx/sessions") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{\"accountstate\":1,\"deviceid\":\"2102353GTD10L9000007\",\"iBaseToken\":\"57B90CC0894098717CD5516EE5DDBCA79F072BABF60A4A6D936FE5FFDD01090C\",\"lastloginip\":\"8.40.99.131\",\"lastlogintime\":1656339028,\"pwdchangetime\":1654707090,\"roleId\":\"1\",\"usergroup\":\"\",\"userid\":\"admin\",\"username\":\"admin\",\"userscope\":\"0\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if (httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/system/") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{\"CACHEWRITEQUOTA\":\"333\",\"CONFIGMODEL\":\"1\",\"DESCRIPTION\":\"\",\"DOMAINNAME\":\"\",\"FREEDISKSCAPACITY\":\"0\",\"HEALTHSTATUS\":\"1\",\"HOTSPAREDISKSCAPACITY\":\"0\",\"ID\":\"2102353GTD10L9000007\",\"LOCATION\":\"\",\"MEMBERDISKSCAPACITY\":\"60000000000\",\"NAME\":\"Huawei.Storage\",\"PRODUCTMODE\":\"92\",\"PRODUCTVERSION\":\"V600R005C21\",\"RUNNINGSTATUS\":\"1\",\"SECTORSIZE\":\"512\",\"STORAGEPOOLCAPACITY\":\"33030913444\",\"STORAGEPOOLFREECAPACITY\":\"28565410974\",\"STORAGEPOOLHOSTSPARECAPACITY\":\"6606182688\",\"STORAGEPOOLRAWCAPACITY\":\"52849461510\",\"STORAGEPOOLUSEDCAPACITY\":\"4465502470\",\"THICKLUNSALLOCATECAPACITY\":\"0\",\"THICKLUNSUSEDCAPACITY\":\"-1\",\"THINLUNSALLOCATECAPACITY\":\"108406352\",\"THINLUNSMAXCAPACITY\":\"1887436800\",\"THINLUNSUSEDCAPACITY\":\"-1\",\"TOTALCAPACITY\":\"60000000000\",\"TYPE\":201,\"UNAVAILABLEDISKSCAPACITY\":\"0\",\"USEDCAPACITY\":\"4465502470\",\"VASA_ALTERNATE_NAME\":\"Huawei.Storage\",\"VASA_SUPPORT_BLOCK\":\"FC,FCOE,ISCSI,Others\",\"VASA_SUPPORT_FILESYSTEM\":\"\",\"VASA_SUPPORT_PROFILE\":\"BlockDeviceProfile,CapabilityProfile,VirtualVolumeProfile\",\"WRITETHROUGHSW\":\"1\",\"WRITETHROUGHTIME\":\"192\",\"mappedLunsCountCapacity\":\"0\",\"patchVersion\":\"\",\"pointRelease\":\"1.2.1RC1\",\"productModeString\":\"OceanProtect X8000\",\"unMappedLunsCountCapacity\":\"1887436800\",\"userFreeCapacity\":\"68233794713\",\"wwn\":\"21004cf55b9769b4\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    }
    return Module::SUCCESS;
}

int32_t SendStorageApiFail1(HttpRequest &httpParam, HttpResponseInfo &httpResponse)
{
    httpResponse.m_statusCode = 200;
    httpResponse.cookies = {
            "session=ismsession=EA27698A079EBAC777077D174395232FFE67D86ECB094E274D61E2015214467D;SameSite=Lax;path=/;httponly;secure;"};
    httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":1077949061,\"description\":\"The user name or password is incorrect.\",\"errorParam\":\"\",\"suggestion\":\"Check the user name and password, and try again.\"}}";
    if (httpParam.url.find("/deviceManager/rest/xxxxx/sessions") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":1077949061,\"description\":\"The user name or password is incorrect.\",\"errorParam\":\"\",\"suggestion\":\"Check the user name and password, and try again.\"}}";
    } else if (httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/sessions") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":1077949061,\"description\":\"Logout fail.\",\"errorParam\":\"\",\"suggestion\":\"Check the user name and password, and try again.\"}}";
    }
    return Module::SUCCESS;
}

int32_t SendStorageApiFail2(HttpRequest &httpParam, HttpResponseInfo &httpResponse)
{
    httpResponse.m_statusCode = 502;
    httpResponse.cookies = {
            "session=ismsession=EA27698A079EBAC777077D174395232FFE67D86ECB094E274D61E2015214467D;SameSite=Lax;path=/;httponly;secure;"};
    if (httpParam.url.find("/deviceManager/rest/xxxxx/sessions") != std::string::npos) {
        httpResponse.m_body = "Connection fail";
    }
    return Module::FAILED;
}

int32_t SendStorageApiFail3(HttpRequest &httpParam, HttpResponseInfo &httpResponse)
{
    httpResponse.m_statusCode = 200;
    httpResponse.cookies = {
            "session=ismsession=EA27698A079EBAC777077D174395232FFE67D86ECB094E274D61E2015214467D;SameSite=Lax;path=/;httponly;secure;"};
    if (httpParam.url.find("/deviceManager/rest/xxxxx/sessions") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{\"accountstate\":1,\"deviceid\":\"2102353GTD10L9000007\",\"iBaseToken\":\"57B90CC0894098717CD5516EE5DDBCA79F072BABF60A4A6D936FE5FFDD01090C\",\"lastloginip\":\"8.40.99.131\",\"lastlogintime\":1656339028,\"pwdchangetime\":1654707090,\"roleId\":\"1\",\"usergroup\":\"\",\"userid\":\"admin\",\"username\":\"admin\",\"userscope\":\"0\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else {
        httpResponse.m_body = "{";
    }
    return Module::SUCCESS;
}

int32_t SendStorageApiFail4(HttpRequest &httpParam, HttpResponseInfo &httpResponse)
{
    httpResponse.m_statusCode = 200;
    httpResponse.cookies = {
            "session=ismsession=EA27698A079EBAC777077D174395232FFE67D86ECB094E274D61E2015214467D;SameSite=Lax;path=/;httponly;secure;"};
    if (httpParam.url.find("/deviceManager/rest/xxxxx/sessions") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{\"accountstate\":1,\"deviceid\":\"2102353GTD10L9000007\",\"iBaseToken\":\"57B90CC0894098717CD5516EE5DDBCA79F072BABF60A4A6D936FE5FFDD01090C\",\"lastloginip\":\"8.40.99.131\",\"lastlogintime\":1656339028,\"pwdchangetime\":1654707090,\"roleId\":\"1\",\"usergroup\":\"\",\"userid\":\"admin\",\"username\":\"admin\",\"userscope\":\"0\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":1077949061,\"description\":\"The user name or password is incorrect.\",\"errorParam\":\"\",\"suggestion\":\"Check the user name and password, and try again.\"}}";
    }
    return Module::SUCCESS;
}

void GetIpVectorFromString_stub(std::vector<std::string> &ipList, const std::string &ip, const std::string &ipListString)
{
    ipList.push_back("8.40.102.126");
    return;
}

void Storage_sleep_stub() { }

/*
 * 测试用例： 创建存储客户端认证成功，并且成功获取到
 * 前置条件： 1、存储设备连接正常；2、用户名和密码正确；
 * CHECK点： 获取到的存储设备版本信息与预期相同
 */
TEST_F(StorageClientTest, CheckAccessAuthentication_SUCCESS)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageApiSuccess);
    AccessAuthParam accessAuthParam("admin", "Admin@123", "0");
    std::string ipList;
    auto [ret, storageClient]= StorageClient::Create("8.40.102.115", 8088, accessAuthParam, ipList);
    StorageDeviceInfo storageDeviceInfo;
    std::tie(ret, storageDeviceInfo) = storageClient->GetDeviceBaseInfo();
    EXPECT_EQ(ret, Module::SUCCESS);
    EXPECT_EQ(storageDeviceInfo.version, "V6");
    EXPECT_EQ(storageDeviceInfo.productMode, "821");
    EXPECT_EQ(storageDeviceInfo.productVersion, "V600R005C21");
}

/*
 * 测试用例： 创建V5存储客户端认证成功，并且成功获取到
 * 前置条件： 1、存储设备连接正常；2、用户名和密码正确；
 * CHECK点： 获取到的存储设备版本信息与预期相同
 */
TEST_F(StorageClientTest, CheckAccessAuthentication_fail_when_response_error_code_not_zore)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageApiSuccess2);
    AccessAuthParam accessAuthParam("admin", "Admin@123", "0");
    std::string ipList;
    auto [ret, storageClient]= StorageClient::Create("8.40.102.115", 8088, accessAuthParam, ipList);
    StorageDeviceInfo storageDeviceInfo;
    std::tie(ret, storageDeviceInfo) = storageClient->GetDeviceBaseInfo();
    EXPECT_EQ(ret, Module::SUCCESS);
    EXPECT_EQ(storageDeviceInfo.version, "V5");
    EXPECT_EQ(storageDeviceInfo.productMode, "92");
    EXPECT_EQ(storageDeviceInfo.productVersion, "V600R005C21");
}

/*
 * 测试用例： 创建存储客户端认证失败
 * 前置条件： 1、存储设备连接正常；2、用户名和密码不正确；
 * CHECK点： 返回账户密码错误错误码
 */
TEST_F(StorageClientTest, CheckAccessAuthentication_Falied1)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageApiFail1);
    stub.set(VirtPlugin::Utils::GetIpVectorFromString, GetIpVectorFromString_stub);
    AccessAuthParam accessAuthParam("admin", "Admin@123", "11");
    std::string ipList = "8.40.102.126";
    auto [ret, storageClient]= StorageClient::Create("8.40.102.126", 8088, accessAuthParam, ipList, true);
    EXPECT_EQ(ret, Module::FAILED);
}

//*
// * 测试用例： 创建存储客户端认证失败
// * 前置条件： 1、存储设备连接正常；2、调用接口http code返回502，网络原因；
// * CHECK点： 返回失败错误码15
// */
TEST_F(StorageClientTest, CheckAccessAuthentication_Falied2)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageApiFail2);
    AccessAuthParam accessAuthParam("admin", "Admin@123", "0");
    std::string ipList;
    auto[ret, storageClient] = StorageClient::Create("8.40.102.127", 8088, accessAuthParam, ipList);
    EXPECT_EQ(ret, Module::FAILED);
}

/*
 * 测试用例： 创建存储客户端认证成功，并且获取卷信息成功
 * 前置条件： 1、存储设备连接正常；2、用户名和密码正确；3、获取卷信息成功；
 * CHECK点： 获取到的卷信息与预期相同
 */
TEST_F(StorageClientTest, GetLunInfoData_Success)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageApiSuccess);
    AccessAuthParam accessAuthParam("admin", "Admin@123", "0");
    std::string ipList;
    auto [ret, storageClient]= StorageClient::Create("8.40.102.115", 8088, accessAuthParam, ipList);
    LunInfoData lunInfoData;
    std::tie(ret, lunInfoData) = storageClient->GetLunInfoData("snap1");
    EXPECT_EQ(ret, Module::SUCCESS);
    EXPECT_EQ(lunInfoData.m_id, "12");
}

/*
 * 测试用例： 创建存储客户端认证成功，并且获取卷信息失败
 * 前置条件： 1、存储设备连接正常；2、用户名和密码正确；3、获取卷信息报文异常；
 * CHECK点： 返回失败码
 */
TEST_F(StorageClientTest, GetLunInfoData_fail_when_response_error)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageApiFail3);
    AccessAuthParam accessAuthParam("admin", "Admin@123", "0");
    std::string ipList;
    auto [ret, storageClient]= StorageClient::Create("8.40.102.115", 8088, accessAuthParam, ipList);
    LunInfoData lunInfoData;
    std::tie(ret, lunInfoData) = storageClient->GetLunInfoData("snap1");
    EXPECT_EQ(ret, -1);
}

/*
 * 测试用例： 创建存储客户端认证成功，并且获取卷信息失败
 * 前置条件： 1、存储设备连接正常；2、用户名和密码正确；3、获取卷信息报文异常；
 * CHECK点： 返回失败码
 */
TEST_F(StorageClientTest, QuerySnapshot_fail_when_response_error_code_not_zore)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageApiFail4);
    AccessAuthParam accessAuthParam("admin", "Admin@123", "0");
    std::string ipList;
    auto [ret, storageClient]= StorageClient::Create("8.40.102.115", 8088, accessAuthParam, ipList);
    SnapshotInfoData snapshotInfoData;
    std::tie(ret, snapshotInfoData) = storageClient->QuerySnapshot("1");
    EXPECT_EQ(ret, 1077949061);
}

/*
 * 测试用例： 发送rest请求
 * 前置条件： 内存加载双向证书成功
 * CHECK点： rest请求结果是否成功
 */
TEST_F(StorageClientTest, SendMemCertRequest_fail_when_http_status_404)
{
    Stub stub;
    stub.set(sleep, Storage_sleep_stub);
    AccessAuthParam accessAuthParam("admin", "Admin@123", "0");
    std::string ipList;
    auto[ret, storageClient]= StorageClient::Create("127.0.0.1", 8089, accessAuthParam, ipList);
    EXPECT_EQ(ret, Module::FAILED);
}

/*
 * 测试用例： 创建存储客户端认证成功，并且获取卷信息失败
 * 前置条件： 1、存储设备连接正常；2、用户名和密码正确；3、获取卷信息存储响应异常错误码；
 * CHECK点： 返回预期错误码
 */
TEST_F(StorageClientTest, GetLunInfoData_fail_when_response_error_code_not_zore)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageApiFail4);
    AccessAuthParam accessAuthParam("admin", "Admin@123", "0");
    std::string ipList;
    auto [ret, storageClient]= StorageClient::Create("8.40.102.115", 8088, accessAuthParam, ipList);
    LunInfoData lunInfoData;
    std::tie(ret, lunInfoData) = storageClient->GetLunInfoData("snap1");
    EXPECT_EQ(ret, 1077949061);
}

/*
 * 测试用例： 创建存储客户端认证成功，并且创建快照成功
 * 前置条件： 1、存储设备连接正常；2、用户名和密码正确；3、获取卷信息成功；
 * CHECK点： 获取到的快照与预期相同
 */
TEST_F(StorageClientTest, CreateSnapshot_Success)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageApiSuccess);
    AccessAuthParam accessAuthParam("admin", "Admin@123", "0");
    std::string ipList;
    auto[ret, storageClient] = StorageClient::Create("8.40.102.115", 8088, accessAuthParam, ipList);
    SnapshotCreateParam snapshotCreateParam;
    snapshotCreateParam.m_lunId = "11";
    snapshotCreateParam.m_name = "LUN012_snap_212323212";
    snapshotCreateParam.m_description = "";
    SnapshotInfoData snapshotInfoData;
    std::tie(ret, snapshotInfoData) = storageClient->CreateSnapshot(snapshotCreateParam);
    EXPECT_EQ(ret, Module::SUCCESS);
    EXPECT_EQ(snapshotInfoData.m_id, "31");
    EXPECT_EQ(snapshotInfoData.m_name, "LUN012_snap_212323212");
    EXPECT_EQ(snapshotInfoData.m_runningStatus, "43");
}

/*
 * 测试用例： 创建存储客户端认证成功，并且创建快照失败
 * 前置条件： 1、存储设备连接正常；2、用户名和密码正确；3、获取卷信息报文异常；
 * CHECK点： 返回预期错误码
 */
TEST_F(StorageClientTest, CreateSnapshot_fail_when_response_error_code_not_zore)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageApiFail4);
    AccessAuthParam accessAuthParam("admin", "Admin@123", "0");
    std::string ipList;
    auto[ret, storageClient] = StorageClient::Create("8.40.102.115", 8088, accessAuthParam, ipList);
    SnapshotCreateParam snapshotCreateParam;
    snapshotCreateParam.m_lunId = "11";
    snapshotCreateParam.m_name = "LUN012_snap_212323212";
    snapshotCreateParam.m_description = "";
    SnapshotInfoData snapshotInfoData;
    std::tie(ret, snapshotInfoData) = storageClient->CreateSnapshot(snapshotCreateParam);
    EXPECT_EQ(ret, 1077949061);
}

/*
 * 测试用例： 创建存储客户端认证成功，并且取消激活成功
 * 前置条件： 1、存储设备连接正常；2、用户名和密码正确；3、获取卷信息成功；
 * CHECK点： 返回码成功
 */
TEST_F(StorageClientTest, StopSnapshot_Success)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageApiSuccess);
    AccessAuthParam accessAuthParam("admin", "Admin@123", "0");
    std::string ipList;
    auto[ret, storageClient] = StorageClient::Create("8.40.102.115", 8088, accessAuthParam, ipList);
    ret = storageClient->StopSnapshot("32");
    EXPECT_EQ(ret, Module::SUCCESS);
}

/*
 * 测试用例： 创建存储客户端认证成功，并且取消激活时报
 * 前置条件： 1、存储设备连接正常；2、用户名和密码正确；3、取消激活快照存储响应失败；
 * CHECK点： 返回码成功
 */
TEST_F(StorageClientTest, StopSnapshot_fail_when_response_error_code_not_zore)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageApiFail4);
    AccessAuthParam accessAuthParam("admin", "Admin@123", "0");
    std::string ipList;
    auto[ret, storageClient] = StorageClient::Create("8.40.102.115", 8088, accessAuthParam, ipList);
    ret = storageClient->StopSnapshot("32");
    EXPECT_EQ(ret, 1077949061);
}

/*
 * 测试用例： 创建存储客户端认证成功，并且批量激活成功
 * 前置条件： 1、存储设备连接正常；2、用户名和密码正确；3、获取卷信息成功；
 * CHECK点： 返回码成功
 */
TEST_F(StorageClientTest, ActivateSnapshot_Success)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageApiSuccess);
    AccessAuthParam accessAuthParam("admin", "Admin@123", "0");
    std::string ipList;
    auto[ret, storageClient]= StorageClient::Create("8.40.102.115", 8088, accessAuthParam, ipList);
    std::vector<std::string> snapshotIds = {"32"};
    ret = storageClient->ActivateSnapshot(snapshotIds);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/*
 * 测试用例： 创建存储客户端认证成功，并且批量激活失败
 * 前置条件： 1、存储设备连接正常；2、用户名和密码正确；3、激活存储响应失败；
 * CHECK点： 返回码成功
 */
TEST_F(StorageClientTest, ActivateSnapshot_fail_when_response_error_code_not_zore)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageApiFail4);
    AccessAuthParam accessAuthParam("admin", "Admin@123", "0");
    std::string ipList;
    auto[ret, storageClient]= StorageClient::Create("8.40.102.115", 8088, accessAuthParam, ipList);
    std::vector<std::string> snapshotIds = {"32"};
    ret = storageClient->ActivateSnapshot(snapshotIds);
    EXPECT_EQ(ret, 1077949061);
}

/*
 * 测试用例： 存储客户端登出成功
 * 前置条件： 1、存储设备已正常连接；
 * CHECK点： 返回码成功
 */
TEST_F(StorageClientTest, Logout_Success)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageApiSuccess);
    AccessAuthParam accessAuthParam("admin", "Admin@123", "0");
    std::string ipList;
    auto[ret, storageClient]= StorageClient::Create("8.40.102.115", 8088, accessAuthParam, ipList);
    ret = storageClient->Logout();
    EXPECT_EQ(ret, Module::SUCCESS);
}

/*
 * 测试用例： 存储客户端登出失败
 * 前置条件： 1、存储设备已正常连接；
 * CHECK点： 返回码失败
 */
TEST_F(StorageClientTest, Logout_Fail)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageApiSuccess);
    AccessAuthParam accessAuthParam("admin", "Admin@123", "0");
    std::string ipList;
    auto[ret, storageClient]= StorageClient::Create("8.40.102.115", 8088, accessAuthParam, ipList);

    stub.set(ADDR(StorageClient, Send), SendStorageApiFail1);
    ret = storageClient->Logout();
    EXPECT_EQ(ret, 1077949061);
}

/*
 * 测试用例： 存储客户端登出失败
 * 前置条件： 1、存储设备已正常连接；2、登出存储响应异常
 * CHECK点： 返回码失败
 */
TEST_F(StorageClientTest, Logout_fail_when_response_error)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageApiSuccess);
    AccessAuthParam accessAuthParam("admin", "Admin@123", "0");
    std::string ipList;
    auto[ret, storageClient]= StorageClient::Create("8.40.102.115", 8088, accessAuthParam, ipList);

    stub.set(ADDR(StorageClient, Send), SendStorageApiFail3);
    ret = storageClient->Logout();
    EXPECT_EQ(ret, Module::FAILED);
}
}
