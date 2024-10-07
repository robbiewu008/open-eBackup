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
#include <iostream>
#include <list>
#include <cstdio>
#include <memory>
#include <system/System.hpp>
#include <log/Log.h>
#include <common/Macros.h>
#include <common/Constants.h>

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "common/CommonMock.h"
#include "common/JsonUtils.h"
#include <common/JsonHelper.h>

#include "volume_handlers/fusionstorage/FusionStorageRestApiOperator.h"
#include "volume_handlers/fusionstorage/FusionStorageRestApiErrorCode.h"
#include "volume_handlers/fusionstorage/client/FusionStorageRestApiRequest.h"
#include "volume_handlers/fusionstorage/client/FusionStorageRestClient.h"
#include "volume_handlers/fusionstorage/FusionStorageIscsiDiskScanner.h"

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

using ErrorCode = FusionStorageRestApiErrorCode;

using namespace VirtPlugin;

namespace HDT_TEST {
class FusionStorageRestApiOperatorTest : public testing::Test {
public:
    void SetUp()
    {
        fusionStorageRestApiOperatorTest = std::make_shared<FusionStorageRestApiOperator>("127.0.0.1", "123", "456", "789");
        fusionStorageRestApiOperatorTest->InitRestApiOperator();
    }
    void TearDown(){};

public:
    std::shared_ptr<FusionStorageRestApiOperator> fusionStorageRestApiOperatorTest;
    Stub stub;
    AppProtect::BackupJob m_backupInfo;
    AppProtect::RestoreJob m_restoreInfo;
};

static int32_t ExecStubSuccess()
{
    return SUCCESS;
}

static int32_t ExecStubFailed()
{
    return FAILED;
}

// -------------------------------------- JOBHANDLE ------------------------------------------------------
// 备份任务句柄
AppProtect::BackupJob m_backupJobInfo;
JobType jobType = JobType::BACKUP;
// m_backupJobInfo.protectEnv.id = "123";
std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupJobInfo);
// jobInfo->SetJobInfo(data);
std::shared_ptr<JobHandle> jobHandle = std::make_shared<JobHandle>(jobType, jobInfo);

// 恢复任务句柄

// -------------------------------------- global object ---------------------------------------------------

// -------------------------------------- responseBody ---------------------------------------------------

const std::string g_responseResult0 = "{\"result\":0}";
const std::string g_responseResultCode0 = "{\"result\": {\"code\": 0, \"description\": \"0\"}}";
const std::string g_responseResult1 = "{\"result\":1}";
const std::string g_responseResultCode1 = "{\"result\": {\"code\": 1, \"description\": \"1\"}}";
const std::string g_responseResult2 = "{\"result\":2}";
const std::string g_responseResultCode2 = "{\"result\": {\"code\": 2, \"description\": \"2\"}}";

const std::string g_responseResultSucces = "{\"result\":0}";
const std::string g_token = "{\
    \"data\": {\
        \"last_login_client_ip\": \"50.64.166.39\",\
        \"last_login_time\": 1686314313,\
        \"password_last_modified_time\": 1675712566,\
        \"password_last_modified_utc_time\": 1675683766,\
        \"password_status\": 1,\
        \"role_id\": \"1\",\
        \"scope\": \"0\",\
        \"system_current_time\": 1686532691,\
        \"system_esn\": \"18de91d663e0d573\",\
        \"user_id\": \"admin\",\
        \"user_name\": \"admin\",\
        \"x_auth_token\": \"MTAyMzc3MTUxNUEwMjM3NzIxNDE1NzNl\",\
        \"x_csrf_token\": \"iSGInddUYncxLpd8CEIdEyE9sr0pdiLJ\"\
    },\"result\": {\
        \"code\": 0,\
        \"description\": \"0\"\
    }}";

const uint32_t OKCODE = 200;
const uint32_t ERRCODE = 404;

const std::string g_getSnapshotOrVolumeInfoSuccess = "{\"result\": 0,\"data\":{\"wwn\":\"wwnExemple\"}}";
const std::string g_createHostSuccess = "{\"result\":0}";
const std::string g_getHostByNameSuccess = "{\"result\":0,\"hostList:\":[\"hostName\",\"host2\"]}";
const std::string g_createIscsiInitiatorSuccess = "{\"result\":0}";
const std::string g_getIscsiInitiatorByNameSuccess = "{\"result\":0}";
const std::string g_addIscsiInitiatorToHostSuccess = "{\"result\":0}";
const std::string g_deleteIscsiInitiatorFromHostSuccess = "{\"result\":0}";
// 获得主机列表，存在host1
const std::string g_getHostListSuccess = "{\
    \"result\": 0,\
    \"hostList\": [\
        {\
            \"hostName\": \"host2\",\
            \"createTime\": \"1685708782\",\
            \"osType\": \"Linux\",\
            \"ipAddress\": \"\",\
            \"hostId\": 12,\
            \"switchoverMode\": \"Disable_alua\",\
            \"pathType\": \"optimal_path\"\
        },{\
            \"hostName\": \"host1\",\
            \"createTime\": \"1685708779\",\
            \"osType\": \"Linux\",\
            \"ipAddress\": \"\",\
            \"hostId\": 11,\
            \"switchoverMode\": \"Disable_alua\",\
            \"pathType\": \"optimal_path\"\
        },{\
            \"hostName\": \"host3\",\
            \"createTime\": \"1685708786\",\
            \"osType\": \"Linux\",\
            \"ipAddress\": \"\",\
            \"hostId\": 13,\
            \"switchoverMode\": \"Disable_alua\",\
            \"pathType\": \"optimal_path\"\
        }],\
    \"hostCount\": 3}";
// 获得主机列表，不存在host1
const std::string g_getHostListFailed = "{\
    \"result\": 0,\
    \"hostListFailed\": [\
        {\
            \"hostName\": \"host2\",\
            \"createTime\": \"1685708782\",\
            \"osType\": \"Linux\",\
            \"ipAddress\": \"\",\
            \"hostId\": 12,\
            \"switchoverMode\": \"Disable_alua\",\
            \"pathType\": \"optimal_path\"\
        }],\
    \"hostCount\": 3}";
const std::string g_getTargetIscsiPortal = "{\
    \"result\": 0,\
    \"nodeResultList\": [{\
            \"errorCode\": 50155018,\
            \"nodeMgrIp\": \"88.1.1.186\",\
            \"status\": \"failed\",\
            \"desc\": \"The iSCSI service on the node is not enabled.\"\
        },{\
            \"errorCode\": 50155018,\
            \"nodeMgrIp\": \"88.1.1.160\",\
            \"status\": \"failed\",\
            \"desc\": \"The iSCSI service on the node is not enabled.\"\
        },{\
            \"errorCode\": 33760257,\
            \"nodeMgrIp\": \"88.1.1.171\",\
            \"status\": \"failed\",\
            \"desc\": \"The specified iSCSI IP address does not exist.\"\
        },{\
            \"errorCode\": 50155018,\
            \"nodeMgrIp\": \"88.7.1.1\",\
            \"status\": \"failed\",\
            \"desc\": \"The iSCSI service on the node is not enabled.\"\
        },{\
            \"errorCode\": 50155018,\
            \"nodeMgrIp\": \"88.7.1.2\",\
            \"status\": \"failed\",\
            \"desc\": \"The iSCSI service on the node is not enabled.\"\
        },{\
            \"errorCode\": 50155018,\
            \"nodeMgrIp\": \"88.1.1.236\",\
            \"status\": \"failed\",\
            \"desc\": \"The iSCSI service on the node is not enabled.\"\
        },{\
            \"iscsiPortalList\": [{\
                    \"targetName\": \"iqn.2012-10.com.huawei.dsware:fa168449642ec40b.vbs.131082\",\
                    \"iscsiPortal\": \"192.168.23.225:3260\",\
                    \"iscsiStatus\": \"active\"\
                }],\
            \"nodeMgrIp\": \"88.1.1.225\",\
            \"status\": \"successful\"\
        },{\
            \"errorCode\": 50155018,\
            \"nodeMgrIp\": \"88.7.1.3\",\
            \"status\": \"failed\",\
            \"desc\": \"The iSCSI service on the node is not enabled.\"\
        },{\
            \"errorCode\": 50155018,\
            \"nodeMgrIp\": \"88.1.1.145\",\
            \"status\": \"failed\",\
            \"desc\": \"The iSCSI service on the node is not enabled.\"\
        },{\
            \"errorCode\": 50155018,\
            \"nodeMgrIp\": \"88.1.1.231\",\
            \"status\": \"failed\",\
            \"desc\": \"The iSCSI service on the node is not enabled.\"\
        }]\
}";
const std::string g_targetIscsiPortal = "{\
            \"iscsiPortalList\": [{\
                    \"targetName\": \"iqn.2012-10.com.huawei.dsware:fa168449642ec40b.vbs.131082\",\
                    \"iscsiPortal\": \"192.168.23.225:3260\",\
                    \"iscsiStatus\": \"active\"\
                }],\
            \"nodeMgrIp\": \"88.1.1.225\",\
            \"status\": \"successful\"\
        }";
const std::string g_currentsession = "{\
    \"data\": [{\
            \"client_ip\": \"50.64.191.171\",\
            \"role_id\": \"1\",\
            \"role_name\": \"\",\
            \"scope\": \"0\",\
            \"status\": \"1\",\
            \"user_id\": \"1\",\
            \"user_name\": \"admin\"\
        }],\
    \"result\": {\
        \"code\": 0,\
        \"description\": \"0\"\
    }\
}";
const std::string g_currentsessionFailed = "{\
    \"data\": [{\
            \"clientip\": \"50.64.191.171\",\
            \"role_id\": \"1\",\
            \"role_name\": \"\",\
            \"scope\": \"0\",\
            \"status\": \"1\",\
            \"user_id\": \"1\",\
            \"user_name\": \"admin\"\
        }],\
    \"result\": {\
        \"code\": 0,\
        \"description\": \"0\"\
    }\
}";
const std::string g_queryhostfromvolumeSuccess = "{\
    \"result\": 0,\
    \"hostList\": [{\
            \"hostName\": \"zl_test\",\
            \"lunId\": 1\
        },{\
            \"hostName\": \"Host20230607144412\",\
            \"lunId\": 1\
        },{\
            \"hostName\": \"host1\",\
            \"lunId\": 3\
        },{\
            \"hostName\": \"host3\",\
            \"lunId\": 1\
        }]\
}";
const std::string g_queryhostfromvolumeFailed = "{\
    \"result\": 0,\
    \"hostLists\": [{\
            \"hostName\": \"zl_test\",\
            \"lunId\": 1\
        },{\
            \"hostName\": \"Host20230607144412\",\
            \"lunId\": 1\
        },{\
            \"hostName\": \"host1\",\
            \"lunId\": 3\
        },{\
            \"hostName\": \"host3\",\
            \"lunId\": 1\
        }]\
}";
const std::string g_hcsAuthExtendInfo =
    "{ 	\"enableCert\" : \"0\", 	\"storages\" : "
    "\"[{\\\"username\\\":\\\"zl_admin\\\",\\\"password\\\":\\\"admin@storage4\\\",\\\"ip\\\":\\\"88.1.1.21\\\","
    "\\\"port\\\":8088,\\\"enableCert\\\":\\\"0\\\",\\\"certification\\\":\\\"\\\",\\\"revocationList\\\":\\\"\\\","
    "\\\"certName\\\":\\\"\\\",\\\"certSize\\\":\\\"\\\",\\\"crlName\\\":\\\"\\\",\\\"crlSize\\\":\\\"\\\",\\\"sn\\\":"
    "\\\"88.1.1.21\\\",\\\"storageType\\\":\\\"1\\\",\\\"ipList\\\":\\\"88.1.1.20,88.1.1.21\\\"},{\\\"username\\\":\\\"admin\\\",\\\"password\\\":\\\"Admin@"
    "1234\\\",\\\"ip\\\":\\\"8.40.99.81\\\",\\\"port\\\":8088,\\\"enableCert\\\":\\\"0\\\",\\\"certification\\\":"
    "\\\"\\\",\\\"revocationList\\\":\\\"\\\",\\\"certName\\\":\\\"\\\",\\\"certSize\\\":\\\"\\\",\\\"crlName\\\":"
    "\\\"\\\",\\\"crlSize\\\":\\\"\\\",\\\"sn\\\":\\\"2102351NPT10J3000001\\\",\\\"storageType\\\":\\\"0\\\",\\\"ipList\\\":\\\"8.40.99.81,8.40.99.82\\\"}]\", 	"
    "\"vdcInfo\" : "
    "\"{\\\"name\\\":\\\"admin_zl\\\",\\\"passwd\\\":\\\"admin@storage1\\\",\\\"domainName\\\":\\\"zl_hcs_test\\\"}\" "
    "}";
const std::string g_storageInfoTestJsonString1 =
    "{ 	\"certName\" : \"\", 	\"certSize\" : \"\", 	\"certification\" : \"\", 	\"crlName\" : \"\", 	"
    "\"crlSize\" : \"\", 	\"enableCert\" : \"0\", 	\"ip\" : \"88.1.1.21\", 	\"password\" : \"admin@storage4\", "
    "	\"port\" : 8088, 	\"revocationList\" : \"\", 	\"sn\" : \"88.1.1.21\",    \"storageType\" : \"1\", 	"
    "\"username\" : \"zl_admin\" }";
const std::string g_storageInfoTestJsonString2 =
    "{ 	\"certName\" : \"\", 	\"certSize\" : \"\", 	\"certification\" : \"\", 	\"crlName\" : \"\", 	"
    "\"crlSize\" : \"\", 	\"enableCert\" : \"0\", 	\"ip\" : \"8.40.99.81\", 	\"password\" : \"Admin@1234\", 	"
    "\"port\" : 8088, 	\"revocationList\" : \"\", 	\"sn\" : \"2102351NPT10J3000001\", 	\"storageType\" : \"0\", 	"
    "\"username\" : \"admin\" }";

const std::string g_restapiResponseFailed1 = "{\"result\":1}";
const std::string g_restapiResponseFailed2 = "{\"result\":2}";
const std::string g_restapiResponseFailedOther = "{\"results\":2}";

const std::string g_hostSN = "6d1af429-57c0-40ec-bd3e-3c6aa7e75769";
const std::string g_loginedTargetIp1 = "192.168.23.225";
const std::string g_loginedTargetIp2 = "8.1.1.187";
const std::string g_loginedTargetIp3 = "8.1.1.188";

// --------------------------------------- Stub IHttpClient -------------------------------------------------

std::string getHttpBodyString = "";
uint32_t getHttpCode = OKCODE;
static Module::IHttpClient *Stub_CallApi()
{
    std::shared_ptr<IHttpResponseMock> httpRespone = std::make_shared<IHttpResponseMock>();
    EXPECT_CALL(*httpRespone, Success()).WillRepeatedly(Return(true));
    EXPECT_CALL(*httpRespone, GetStatusCode()).WillRepeatedly(Return(g_httpStatusCode));
    EXPECT_CALL(*httpRespone, GetErrCode()).WillRepeatedly(Return(0));
    EXPECT_CALL(*httpRespone, GetErrString()).WillRepeatedly(Return(""));
    EXPECT_CALL(*httpRespone, Busy()).WillRepeatedly(Return(false));
    EXPECT_CALL(*httpRespone, GetHttpStatusCode()).WillRepeatedly(Return(getHttpCode));
    EXPECT_CALL(*httpRespone, GetHttpStatusDescribe()).WillRepeatedly(Return(""));

    std::set<std::string> GetHeadByNameReturn = {};
    EXPECT_CALL(*httpRespone, GetHeadByName(_)).WillRepeatedly(Return(GetHeadByNameReturn));
    std::string bodyString = getHttpBodyString;
    EXPECT_CALL(*httpRespone, GetBody()).WillRepeatedly(Return(bodyString));
    std::set<std::string> headerValue;
    headerValue.insert(g_token);

    std::map<std::string, std::set<std::string> > getHeadersReturn = {{"X-Subject-Token", headerValue}};
    EXPECT_CALL(*httpRespone, GetHeaders()).WillRepeatedly(Return(getHeadersReturn));
    std::set<std::string> getCookiesReturn = {};
    EXPECT_CALL(*httpRespone, GetCookies()).WillRepeatedly(Return(getCookiesReturn));

    IHttpClientMock *httpClient = new (std::nothrow) IHttpClientMock();
    EXPECT_CALL(*httpClient, SendMemCertRequest(_, _)).WillRepeatedly(Return(httpRespone));
    EXPECT_CALL(*httpClient, SendRequest(_, _)).WillRepeatedly(Return(httpRespone));
    return httpClient;
}

std::vector<std::string> g_loginedIps;
int32_t Stub_GetLoginedTargetIp(std::vector<std::string> loginedIps){
    loginedIps = g_loginedIps;
    return SUCCESS;
}

std::string Stub_GetFusionStorageMgrPort()
{
    return "8088";
}

std::string Stub_GetFusionStorageMgrIp()
{
    return "192.168.1.1";
}

StorageInfo Stub_GetStorageType(
    std::string p1, std::string p2, int32_t p3, std::string p4, std::string p5, std::string p6, std::string p7, std::string p8)
{
    StorageInfo g_storageInfoTest;
    g_storageInfoTest.m_sn = p1;
    g_storageInfoTest.m_ip = p2;

    g_storageInfoTest.m_port = p3;

    g_storageInfoTest.m_userName = p4;
    g_storageInfoTest.m_passWd = p5;

    g_storageInfoTest.m_enableCert = p6;
    g_storageInfoTest.m_storageType = p7;

    g_storageInfoTest.m_ipList = p8;
    return g_storageInfoTest;
}
StorageInfo g_storageInfoTest1 = Stub_GetStorageType("88.1.1.21", "88.1.1.21", 8088, "admin", "Huawei@123", "0", "1", "88.1.1.21,88.1.1.88");
StorageInfo g_storageInfoTest2 = Stub_GetStorageType("8.40.99.81", "2102351NPT10J3000001", 8088, "user", "Admin@1234", "0", "0", "8.40.99.81,8.40.99.82");

int32_t Stub_CallApiFailed(RequestInfo &requestInfo, std::shared_ptr<ResponseModel> response, ModelBase &model)
{
    return FAILED;
}

std::vector<std::string> stub_cmdOut1;
static int Stub_RunShell_Success(const std::string &moduleName, const std::size_t &requestID, const std::string &cmd,
    const std::vector<std::string> params, std::vector<std::string> &cmdoutput, std::vector<std::string> &stderroutput,
    std::stringstream &outstring, const unsigned int &runShellType)
{
    cmdoutput = stub_cmdOut1;
    return SUCCESS;
}

static int Stub_RunCommand_Success(const std::string& cmdName, const std::vector<Module::CmdParam>& cmdVec, std::vector<std::string>& result,
    const Module::CmdRunUser& cmdUser)
{
    result = stub_cmdOut1;
    return SUCCESS;
}

static int Stub_RunShell_Failed(const std::string &moduleName, const std::size_t &requestID, const std::string &cmd,
    const std::vector<std::string> params, std::vector<std::string> &cmdoutput, std::vector<std::string> &stderroutput,
    std::stringstream &outstring, const unsigned int &runShellType)
{
    cmdoutput = stub_cmdOut1;
    return FAILED;
}

int32_t StubGetVolumeWwnByNameSuccess(const std::string &volumeName, std::string &errorDes)
{
    errorDes = "success";
    return SUCCESS;
}

int32_t StubCreateHostLunMappingSuccess(
    const std::string &hostName, const std::string &volumeName, std::string &errorDes)
{
    errorDes = "success";
    return SUCCESS;
}

int32_t StubScanDiskSuccess(const std::string &volumeName, const std::string &volumeWwn, std::string &diskDevicePath)
{
    diskDevicePath = "/dev/disk/../../sdb";
    return SUCCESS;
}

int32_t Stub_GetResponseResult_Offline(const std::string &responseBody, bool codeFlag)
{
    return static_cast<int32_t>(ErrorCode::RESTAPI_ERR_WRONG_REQUEST_PARAMETER);
}

int32_t Stub_GetResponseResult_Not_Exist(const std::string &responseBody, bool codeFlag)
{
    return static_cast<int32_t>(ErrorCode::RESTAPI_ERR_INITIATOR_IS_NOT_EXIST);
}

int32_t responseResult = static_cast<int32_t>(ErrorCode::RESTAPI_OK);
int32_t Stub_GetResponseResult(const std::string &responseBody, bool codeFlag)
{
    return responseResult;
}

int32_t Stub_GetVolumeMappedHostSuccess(const std::string &volumeName, std::vector<std::string> &hostNameList, std::string &errorDes){
    return SUCCESS;
}

int32_t Stub_GetVolumeMappedHostFailed(const std::string &volumeName, std::vector<std::string> &hostNameList, std::string &errorDes){
    return FAILED;
}

int32_t Stub_ParseMappedHostListSuccess(const std::string &responseBody, std::vector<std::string> &hostNameList){
    return SUCCESS;
}

int32_t Stub_ParseMappedHostListFailed(const std::string &responseBody, std::vector<std::string> &hostNameList){
    return FAILED;
}

/**
 * 测试用例： 实例化ApiOperator对象
 * 前置条件： 通过Api创建对应的Api客户端后，需要使用构造函数实例化
 * CHECK点： 实例化RestApiOperator对象成功
 */
TEST_F(FusionStorageRestApiOperatorTest, CreateFusionStorageRestApiOperatorSuccess)
{
    bool ret = false;

    ret = (fusionStorageRestApiOperatorTest.get() == nullptr);
    EXPECT_EQ(ret, false);
}

/**
 * 测试用例： 初始化成员变量
 * 前置条件：
 * CHECK点： 初始化成功，指针不为空
 */
TEST_F(FusionStorageRestApiOperatorTest, InitRestApiOperatorSuccess)
{
    Stub stub;
    stub.set((int(*)(const std::string&, const std::vector<Module::CmdParam>&, std::vector<std::string>&,
        const Module::CmdRunUser&))(Module::RunCommand), Stub_RunCommand_Success);
    stub_cmdOut1.push_back(g_hostSN);

    int32_t ret = fusionStorageRestApiOperatorTest->InitRestApiOperator();
    EXPECT_EQ(ret, SUCCESS);
}

TEST_F(FusionStorageRestApiOperatorTest, InitRestApiOperatorFailed)
{
    Stub stub;
    stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_Failed);
    stub_cmdOut1.push_back(g_hostSN);

    int32_t ret = fusionStorageRestApiOperatorTest->InitRestApiOperator();
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 获得存储信息
 * 前置条件：在检查存储连通性或进行备份恢复时，需要检查和存储的连通性
 * CHECK点： 获得存储信息成功
 */
TEST_F(FusionStorageRestApiOperatorTest, GetStorageInfoSuccess)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 获得存储信息
 * 前置条件：在检查存储连通性或进行备份恢复时，需要检查和存储的连通性
 * CHECK点： 获得存储信息失败
 */
TEST_F(FusionStorageRestApiOperatorTest, GetStorageInfoFailed)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.22";
    volInfo.m_datastore.m_moRef = "88.1.1.22";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);
    EXPECT_EQ(ret, FAILED);
}

TEST_F(FusionStorageRestApiOperatorTest, GetStorageInfoFailed2)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.22";
    volInfo.m_datastore.m_moRef = "88.1.1.22";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo("", volInfo, isBackUp);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例：录入controlDeviceInfo
 * 前置条件：在检查存储连通性或进行备份恢复时，当解析出存储信息后需要将符合参数录入controldeviceinfo
 * CHECK点： 录入controlDeviceInfo成功
 */
TEST_F(FusionStorageRestApiOperatorTest, GetControlDeviceInfoSuccess)
{
    Stub stub;
    stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_Success);
    stub_cmdOut1.push_back(g_hostSN);
    bool isBackUp = true;
    std::vector<StorageInfo> storageVectorTest;
    storageVectorTest.push_back(g_storageInfoTest1);
    storageVectorTest.push_back(g_storageInfoTest2);
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";
    ControlDeviceInfo controlDeviceInfoTest;
    int32_t ret = fusionStorageRestApiOperatorTest->GetControlDeviceInfo(storageVectorTest, controlDeviceInfoTest, volInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例：录入controlDeviceInfo
 * 前置条件：在检查存储连通性或进行备份恢复时，当解析出存储信息后需要将符合参数录入controldeviceinfo
 * CHECK点： 录入controlDeviceInfo失败
 */
TEST_F(FusionStorageRestApiOperatorTest, GetControlDeviceInfoFailed)
{
    Stub stub;
    stub.set(Module::BaseRunShellCmdWithOutputWithOutLock, Stub_RunShell_Success);
    stub_cmdOut1.push_back(g_hostSN);
    bool isBackUp = true;
    std::vector<StorageInfo> storageVectorTest;
    storageVectorTest.push_back(g_storageInfoTest1);
    storageVectorTest.push_back(g_storageInfoTest2);
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.22";
    volInfo.m_datastore.m_moRef = "88.1.1.22";
    ControlDeviceInfo controlDeviceInfoTest;

    int32_t ret = fusionStorageRestApiOperatorTest->GetControlDeviceInfo(storageVectorTest, controlDeviceInfoTest, volInfo);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 处理响应结果result=0
 * 前置条件： 收到响应体
 * CHECK点： 解析响应成功
 */
TEST_F(FusionStorageRestApiOperatorTest, GetResponseRusult0)
{
    bool ret = false;

    ret = (fusionStorageRestApiOperatorTest->GetResponseResult(g_responseResult0, false) == RESTAPISUCCESS);
    EXPECT_EQ(ret, true);
}

/**
 * 测试用例： 处理响应结果result=1
 * 前置条件： 收到响应体
 * CHECK点： 解析响应成功
 */
TEST_F(FusionStorageRestApiOperatorTest, GetResponseRusult1)
{
    bool ret = false;

    ret = (fusionStorageRestApiOperatorTest->GetResponseResult(g_responseResult1, false) == RESTAPIWRONGACION);
    EXPECT_EQ(ret, true);
}

/**
 * 测试用例： 处理响应结果result=2
 * 前置条件： 收到响应体
 * CHECK点： 解析响应成功
 */
TEST_F(FusionStorageRestApiOperatorTest, GetResponseRusult2)
{
    bool ret = false;

    ret = (fusionStorageRestApiOperatorTest->GetResponseResult(g_responseResult2, false) == RESTAPIFAILED);
    EXPECT_EQ(ret, true);
}

/**
 * 测试用例： 处理响应结果result=0
 * 前置条件： 收到响应体
 * CHECK点： 解析响应失败
 */
TEST_F(FusionStorageRestApiOperatorTest, GetResponseRusultFailed)
{
    const std::string responseBody = "{\"result\":{\"code\":\"0\"}}";
    bool codeFlag;
    int32_t ret = fusionStorageRestApiOperatorTest->GetResponseResult(responseBody, true);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 返回错误描述
 * 前置条件： 收到响应体
 * CHECK点： 返回描述failed
 */
TEST_F(FusionStorageRestApiOperatorTest, GetErrStringFailed)
{
    bool ret = false;
    int32_t errorCode = -1;
    const std::string responseOK = "Unknown error";

    ret = (fusionStorageRestApiOperatorTest->GetErrString(errorCode) == responseOK);
    EXPECT_EQ(ret, false);
}

TEST_F(FusionStorageRestApiOperatorTest, GetErrStringFailed2)
{
    bool ret = false;
    int32_t errorCode = 0;
    const std::string responseOK = "OK";

    ret = (fusionStorageRestApiOperatorTest->GetErrString(errorCode) == responseOK);
    EXPECT_EQ(ret, true);
}

/**
 * 测试用例： 处理响应结果解析失败1
 * 前置条件： 收到响应体
 * CHECK点： 返回failed
 */
TEST_F(FusionStorageRestApiOperatorTest, GetResponseRusultFailed1)
{
    bool ret = false;

    ret = (fusionStorageRestApiOperatorTest->GetResponseResult("{\"fad\":0}", false) == FAILED);
    EXPECT_EQ(ret, true);
}
/**
 * 测试用例： 处理响应结果解析失败2
 * 前置条件： 收到响应体
 * CHECK点： 返回failed
 */
TEST_F(FusionStorageRestApiOperatorTest, GetResponseRusultFailed2)
{
    bool ret = false;

    ret = (fusionStorageRestApiOperatorTest->GetResponseResult("", false) == FAILED);
    EXPECT_EQ(ret, true);
}

/**
 * 测试用例： 处理响应结果解析失败1
 * 前置条件： 收到响应体
 * CHECK点： 返回failed
 */
TEST_F(FusionStorageRestApiOperatorTest, GetResponseRusultFailed3)
{
    bool ret = false;

    ret = (fusionStorageRestApiOperatorTest->GetResponseResult("{\"fad\":0}", true) == FAILED);
    EXPECT_EQ(ret, true);
}

/**
 * 测试用例： 返回错误描述
 * 前置条件： 收到响应体
 * CHECK点： 返回正常描述
 */
TEST_F(FusionStorageRestApiOperatorTest, GetErrStringSuccess)
{
    bool ret = false;
    int32_t errorCode = 0;

    const std::string responseOK =
        fusionStorageRestApiOperatorTest->m_errorCodeToErrorDes[static_cast<int32_t>(ErrorCode::RESTAPI_OK)];

    ret = (fusionStorageRestApiOperatorTest->GetErrString(errorCode) == responseOK);
    EXPECT_EQ(ret, true);
}

TEST_F(FusionStorageRestApiOperatorTest, GetErrStringFalied)
{
    bool ret = false;
    int32_t errorCode = 123;

    const std::string responseOK =
        fusionStorageRestApiOperatorTest->m_errorCodeToErrorDes[static_cast<int32_t>(ErrorCode::RESTAPI_OK)];

    ret = (fusionStorageRestApiOperatorTest->GetErrString(errorCode) == responseOK);
    EXPECT_EQ(ret, false);
}

/**
 * 用例名称：尝试获得token时，检查当前token的session是否过期
 * 前置条件：需要发送api请求时，尝试获得token
 * check点：当前session有效
 */
TEST_F(FusionStorageRestApiOperatorTest, GetCurrentSessionSuccess)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);

    getHttpBodyString = g_responseResultCode0;
    getHttpCode = OKCODE;

    ret = fusionStorageRestApiOperatorTest->GetCurrentSession();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 用例名称：尝试获得token时，检查当前token的session是否过期
 * 前置条件：需要发送api请求时，尝试获得token
 * check点：当前session无效
 */
TEST_F(FusionStorageRestApiOperatorTest, GetCurrentSessionFailed)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);

    getHttpBodyString = g_responseResultCode2;
    getHttpCode = OKCODE;

    int32_t ret = fusionStorageRestApiOperatorTest->GetCurrentSession();
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：检查存储连通性成功后，删除当前会话
 * 前置条件：检查存储连通性成功
 * check点：注销会话成功
 */
TEST_F(FusionStorageRestApiOperatorTest, DeleteCurrentSessionSuccess)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);

    getHttpBodyString = g_responseResultCode0;
    getHttpCode = OKCODE;

    ret = fusionStorageRestApiOperatorTest->DeleteCurrentSession();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 用例名称：检查存储连通性成功后，删除当前会话
 * 前置条件：检查存储连通性成功
 * check点：注销会话失败
 */
TEST_F(FusionStorageRestApiOperatorTest, DeleteCurrentSessionFailed)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);

    getHttpBodyString = g_responseResultCode2;
    getHttpCode = OKCODE;

    int32_t ret = fusionStorageRestApiOperatorTest->DeleteCurrentSession();
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：从查询当前session的返回体中拿到当前发送请求的客户端的IP
 * 前置条件：查询当前session成功
 * check点：获得IP成功
 */
TEST_F(FusionStorageRestApiOperatorTest, GetClientIpSuccess)
{
    std::string clientIp;

    int32_t ret = fusionStorageRestApiOperatorTest->GetClientIp(g_currentsession, clientIp);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 用例名称：从查询当前session的返回体中拿到当前发送请求的客户端的IP
 * 前置条件：查询当前session成功
 * check点：解析json失败
 */
TEST_F(FusionStorageRestApiOperatorTest, GetClientIpFailed)
{
    std::string clientIp;

    int32_t ret = fusionStorageRestApiOperatorTest->GetClientIp(g_currentsessionFailed, clientIp);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：用账密发送session请求获得token,成功
 * 前置条件：当前session失效后通过api获得新token
 * check点：用账密发送session请求获得token
 */
TEST_F(FusionStorageRestApiOperatorTest, GetTokenByAppEnvSuccess)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = static_cast<int32_t>(ErrorCode::RESTAPI_OK);
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    getHttpBodyString = "{}";
    getHttpCode = ERRCODE;
    fusionStorageRestApiOperatorTest->m_controlDeviceInfo.m_ip = "8.0.8.8";
    fusionStorageRestApiOperatorTest->m_controlDeviceInfo.m_ipList = {"8.0.8.8","8.0.8.9"};
    fusionStorageRestApiOperatorTest->m_controlDeviceInfo.m_password = "fadfadfa";
    fusionStorageRestApiOperatorTest->m_controlDeviceInfo.m_userName = "ffadfaadfadfa";
    std::string responseBody;
    std::string errorDes;
    int32_t ret = fusionStorageRestApiOperatorTest->GetTokenByAppEnv(responseBody, errorDes);
    EXPECT_EQ(ret, static_cast<int32_t>(ErrorCode::RESTAPI_OK));
}

/**
 * 用例名称：用账密发送session请求获得token,失败
 * 前置条件：当前session失效后通过api获得新token
 * check点：用账密发送session请求获得token
 */
TEST_F(FusionStorageRestApiOperatorTest, GetTokenByAppEnvFailed1)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    getHttpBodyString = "{}";
    getHttpCode = ERRCODE;
    HcsPlugin::StoragInfoReq storageInfoReq;
    storageInfoReq.m_userName = "normal";
    storageInfoReq.m_password = "123456";
    std::string responseBody;
    std::string errorDes;
    int32_t ret = fusionStorageRestApiOperatorTest->GetTokenByAppEnv(responseBody, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：用账密发送session请求获得token失败
 * 前置条件：当前session失效后通过api获得新token
 * check点：用账密发送session请求获得token
 */
TEST_F(FusionStorageRestApiOperatorTest, GetTokenByAppEnvFailed2)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = static_cast<int32_t>(ErrorCode::RESTAPI_ERR_WRONG_REQUEST_PARAMETER);
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    getHttpBodyString = "{}";
    getHttpCode = ERRCODE;
    HcsPlugin::StoragInfoReq storageInfoReq;
    storageInfoReq.m_userName = "normal";
    storageInfoReq.m_password = "123456";
    std::string responseBody;
    std::string errorDes;
    int32_t ret = fusionStorageRestApiOperatorTest->GetTokenByAppEnv(responseBody, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：用账密发送session请求获得token失败
 * 前置条件：当前session失效后通过api获得新token
 * check点：用账密发送session请求获得token
 */
TEST_F(FusionStorageRestApiOperatorTest, GetTokenByAppEnvFailed3)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = static_cast<int32_t>(ErrorCode::RESTAPI_ERR_WRONG_REQUEST_PARAMETER);
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    getHttpBodyString = "{}";
    getHttpCode = ERRCODE;
    HcsPlugin::StoragInfoReq storageInfoReq;
    storageInfoReq.m_userName = "normal";
    storageInfoReq.m_password = "123456";
    std::string responseBody;
    std::string errorDes;
    int32_t ret = fusionStorageRestApiOperatorTest->GetTokenByAppEnv(responseBody, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：从响应体中执行解析，解析出的就是token,成功
 * 前置条件：成功获得带token的响应体
 * check点：从响应体中执行解析，解析出的就是token
 */
TEST_F(FusionStorageRestApiOperatorTest, DoGetTokenSuccess)
{
    const std::string responseBody = "{\"data\":{\"x_auth_token\":\"MTI4NTgwMzc0NEEyODU4MGNhZDg5ZDcw\",\
        \"x_csrf_token\":\"4QMXzg9ajw2NUf0T287SLeMTXt30NeD5\"},\"result\":\
        {\"code\":0,\"description\":\"0\"}}";
    std::string token;
    int32_t ret = fusionStorageRestApiOperatorTest->DoGetToken(responseBody, token);
    EXPECT_EQ(ret, static_cast<int32_t>(ErrorCode::RESTAPI_OK));
}

/**
 * 用例名称：从响应体中执行解析，解析出的就是token,失败
 * 前置条件：成功获得带token的响应体
 * check点：从响应体中执行解析，解析出的就是token
 */
TEST_F(FusionStorageRestApiOperatorTest, DoGetTokenFailed1)
{
    const std::string responseBody = "{\"dt\":{\"x_auth_token\":\"MTI4NTgwMzc0NEEyODU4MGNhZDg5ZDcw\",\
        \"x_csrf_token\":\"4QMXzg9ajw2NUf0T287SLeMTXt30NeD5\"},\"result\":\
        {\"code\":0,\"description\":\"0\"}}";
    std::string token;
    int32_t ret = fusionStorageRestApiOperatorTest->DoGetToken(responseBody, token);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：从响应体中执行解析，解析出的就是token,失败
 * 前置条件：成功获得带token的响应体
 * check点：从响应体中执行解析，解析出的就是token
 */
TEST_F(FusionStorageRestApiOperatorTest, DoGetTokenFailed2)
{
    const std::string responseBody = "{\"data\":{\"x_auth\":\"MTI4NTgwMzc0NEEyODU4MGNhZDg5ZDcw\",\
        \"x_csrf_token\":\"4QMXzg9ajw2NUf0T287SLeMTXt30NeD5\"},\"result\":\
        {\"code\":0,\"description\":\"0\"}}";
    std::string token;
    int32_t ret = fusionStorageRestApiOperatorTest->DoGetToken(responseBody, token);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：从响应体中执行解析，解析出的就是token,失败
 * 前置条件：成功获得带token的响应体
 * check点：从响应体中执行解析，解析出的就是token
 */
TEST_F(FusionStorageRestApiOperatorTest, DoGetTokenFailed3)
{
    const std::string responseBody = "{\"data\":{\\\"x_auth_token\\\":\\\"MTI4NTgwMzc0NEEyODU4MGNhZDg5ZDcw\\\",\
        \\\"x_csrf_token\\\":\\\"4QMXzg9ajw2NUf0T287SLeMTXt30NeD5\\\"},\
        \"result\":{\\\"code\\\":0,\\\"description\\\":\\\"0\\\"}}";
    std::string token;
    int32_t ret = fusionStorageRestApiOperatorTest->DoGetToken(responseBody, token);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：挂载卷
 * 前置条件：任务检查连通性成功，开始进行子任务进行挂载卷
 * check点：挂载卷成功
 */
TEST_F(FusionStorageRestApiOperatorTest, AttachVolumeSuccess)
{
    Stub stub;
    stub.set(ADDR(FusionStorageRestApiOperator, GetVolumeWwnByName), StubScanDiskSuccess);
    stub.set(ADDR(FusionStorageRestApiOperator, CreateHostLunMapping), StubCreateHostLunMappingSuccess);
    stub.set(ADDR(FusionStorageRestApiOperator, DoScanDisk), StubScanDiskSuccess);

    const std::string volumeName = "volume123";
    std::string diskDevicePath = "/test123/temp";
    std::string errorDes = "success";
    fusionStorageRestApiOperatorTest->m_volumeWwn = "tempWwn";

    int32_t ret = fusionStorageRestApiOperatorTest->AttachVolume(volumeName, diskDevicePath, errorDes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 用例名称：挂载卷
 * 前置条件：任务检查连通性成功，开始进行子任务进行挂载卷
 * check点：挂载卷失败于获得WWN
 */
TEST_F(FusionStorageRestApiOperatorTest, AttachVolumeFailed1)
{
    Stub stub;
    stub.set(ADDR(FusionStorageRestApiOperator, GetVolumeWwnByName), ExecStubFailed);
    stub.set(ADDR(FusionStorageRestApiOperator, CreateHostLunMapping), StubCreateHostLunMappingSuccess);
    stub.set(ADDR(FusionStorageRestApiOperator, DoScanDisk), StubScanDiskSuccess);
    stub.set(ADDR(FusionStorageRestApiOperator, DeleteHostLunMapping), ExecStubSuccess);

    const std::string volumeName = "volume123";
    std::string diskDevicePath = "/test123/temp";
    std::string errorDes;
    fusionStorageRestApiOperatorTest->m_volumeWwn = "tempWwn";

    int32_t ret = fusionStorageRestApiOperatorTest->AttachVolume(volumeName, diskDevicePath, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：挂载卷
 * 前置条件：任务检查连通性成功，开始进行子任务进行挂载卷
 * check点：挂载卷失败于建立映射
 */
TEST_F(FusionStorageRestApiOperatorTest, AttachVolumeFailed2)
{
    Stub stub;
    stub.set(ADDR(FusionStorageRestApiOperator, GetVolumeWwnByName), StubScanDiskSuccess);
    stub.set(ADDR(FusionStorageRestApiOperator, CreateHostLunMapping), ExecStubFailed);
    stub.set(ADDR(FusionStorageRestApiOperator, DoScanDisk), StubScanDiskSuccess);
    stub.set(ADDR(FusionStorageRestApiOperator, DeleteHostLunMapping), ExecStubSuccess);

    const std::string volumeName = "volume123";
    std::string diskDevicePath = "/test123/temp";
    std::string errorDes;
    fusionStorageRestApiOperatorTest->m_volumeWwn = "tempWwn";

    int32_t ret = fusionStorageRestApiOperatorTest->AttachVolume(volumeName, diskDevicePath, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：挂载卷
 * 前置条件：任务检查连通性成功，开始进行子任务进行挂载卷
 * check点：挂载卷失败于扫盘
 */
TEST_F(FusionStorageRestApiOperatorTest, AttachVolumeFailed3)
{
    Stub stub;
    stub.set(ADDR(FusionStorageRestApiOperator, GetVolumeWwnByName), StubScanDiskSuccess);
    stub.set(ADDR(FusionStorageRestApiOperator, CreateHostLunMapping), StubCreateHostLunMappingSuccess);
    stub.set(ADDR(FusionStorageRestApiOperator, DoScanDisk), ExecStubFailed);
    stub.set(ADDR(FusionStorageRestApiOperator, DeleteHostLunMapping), ExecStubSuccess);

    const std::string volumeName = "volume123";
    std::string diskDevicePath = "/test123/temp";
    std::string errorDes;
    fusionStorageRestApiOperatorTest->m_volumeWwn = "tempWwn";

    int32_t ret = fusionStorageRestApiOperatorTest->AttachVolume(volumeName, diskDevicePath, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 卸载卷
 * 前置条件： 完成卷IO读写
 * CHECK点： 卸载成功
 */
TEST_F(FusionStorageRestApiOperatorTest, DetachVolumeSuccess)
{
    Stub stub;
    stub.set(ADDR(FusionStorageRestApiOperator, DeleteVolumeAllMapping), ExecStubSuccess);
    stub.set(ADDR(FusionStorageIscsiDiskScanner, DeleteDiskFromPathSet), ExecStubSuccess);

    const std::string volumeName = "volume1";
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->DetachVolume(volumeName, errorDes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 卸载卷
 * 前置条件： 完成卷IO读写
 * CHECK点： 卸载失败
 */
TEST_F(FusionStorageRestApiOperatorTest, DetachVolumeFailed)
{
    Stub stub;
    stub.set(ADDR(FusionStorageRestApiOperator, DeleteVolumeAllMapping), ExecStubFailed);
    stub.set(ADDR(FusionStorageIscsiDiskScanner, DeleteDiskFromPathSet), ExecStubSuccess);

    const std::string volumeName = "volume1";
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->DetachVolume(volumeName, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 查询bitmap，ISCSI无bitmap接口，通过尝试创建主机检查连通性
 * 前置条件： 任务进入检查存储连通性阶段
 * CHECK点： 创建主机不返回FAILED
 */
TEST_F(FusionStorageRestApiOperatorTest, QueryBitmapVolSuccess){
    Stub stub;
    stub.set(ADDR(FusionStorageRestApiOperator, QueryBitmapVol), ExecStubSuccess);
    
    BitmapVolumeInfo info;
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->QueryBitmapVol(info, errorDes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 创建启动器，当启动器创建成功时返回成功
 * 前置条件： 启动器不存在，需要创建启动器
 * CHECK点： 创建启动器成功
 */
TEST_F(FusionStorageRestApiOperatorTest, CreateIscsiInitiatorSuccess1)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = static_cast<int32_t>(ErrorCode::RESTAPI_OK);
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    getHttpBodyString = g_createIscsiInitiatorSuccess;
    getHttpCode = OKCODE;
    std::string iqnNumber = "iqnNumber";
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->CreateIscsiInitiator(iqnNumber, errorDes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 创建启动器，当启动器创建时发现已经有了时，依旧返回成功
 * 前置条件： 启动器不存在，需要创建启动器
 * CHECK点： 启动器已经创建
 */
TEST_F(FusionStorageRestApiOperatorTest, CreateIscsiInitiatorSuccess2)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIWRONGACION;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    getHttpBodyString = g_restapiResponseFailed1;
    getHttpCode = OKCODE;
    std::string iqnNumber = "iqnNumber";
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->CreateIscsiInitiator(iqnNumber, errorDes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 创建启动器，分布式返回result不等于0时失败
 * 前置条件： 启动器不存在，需要创建启动器
 * CHECK点： 创建启动器失败
 */
TEST_F(FusionStorageRestApiOperatorTest, CreateIscsiInitiatorFailed1)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIFAILED;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    getHttpBodyString = g_restapiResponseFailedOther;
    getHttpCode = OKCODE;
    std::string iqnNumber = "1";
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->CreateIscsiInitiator(iqnNumber, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 创建启动器，分布式返回result不等于0时失败
 * 前置条件： 启动器不存在，需要创建启动器
 * CHECK点： 创建启动器失败
 */
TEST_F(FusionStorageRestApiOperatorTest, CreateIscsiInitiatorFailed2)
{
    Stub stub;
    stub.set(ADDR(FusionStorageRestClient, CallApi), ExecStubFailed);
    responseResult = RESTAPIFAILED;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    getHttpBodyString = g_restapiResponseFailedOther;
    getHttpCode = OKCODE;
    std::string iqnNumber = "1";
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->CreateIscsiInitiator(iqnNumber, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 通过iqn查询启动器，当启动器存在时返回成功
 * 前置条件： 查询启动
 * CHECK点： 查询启动器成功
 */
TEST_F(FusionStorageRestApiOperatorTest, GetIscsiInitiatorByNameSuccess)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPISUCCESS;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    getHttpBodyString = g_getIscsiInitiatorByNameSuccess;
    getHttpCode = OKCODE;
    std::string iqnNumber = "iqnNumber";
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->GetIscsiInitiatorByName(iqnNumber, errorDes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 通过iqn查询启动器，当启动器不存在时返回失败
 * 前置条件： 查询启动器
 * CHECK点： 查询启动器失败
 */
TEST_F(FusionStorageRestApiOperatorTest, GetIscsiInitiatorByNameFailed1)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIWRONGACION;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    getHttpBodyString = g_restapiResponseFailed1;
    getHttpCode = OKCODE;
    std::string iqnNumber = "iqnNumber";
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->GetIscsiInitiatorByName(iqnNumber, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 通过iqn查询启动器，当启动器不存在时返回失败
 * 前置条件： 查询启动器
 * CHECK点： 查询启动器失败
 */
TEST_F(FusionStorageRestApiOperatorTest, GetIscsiInitiatorByNameFailed2)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIFAILED;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    std::string iqnNumber = "iqnNumber";
    std::string errorDes;

    getHttpCode = OKCODE;
    ret = fusionStorageRestApiOperatorTest->GetIscsiInitiatorByName(iqnNumber, errorDes);

    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 通过iqn查询启动器，当启动器不存在时返回失败
 * 前置条件： 查询启动器
 * CHECK点： 查询启动器失败
 */
TEST_F(FusionStorageRestApiOperatorTest, GetIscsiInitiatorByNameFailed3)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIFAILED;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    std::string iqnNumber = "iqnNumber";
    std::string errorDes;

    getHttpCode = OKCODE;
    int32_t ret = fusionStorageRestApiOperatorTest->GetIscsiInitiatorByName(iqnNumber, errorDes);

    EXPECT_EQ(ret, FAILED);
}

/** 测试用例：从存储上删除启动器
 * 前置条件：
 * CHECK点：删除成功
 */
TEST_F(FusionStorageRestApiOperatorTest, DeleteIscsiInitiatorSuccess)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPISUCCESS;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    getHttpBodyString = g_responseResult0;
    getHttpCode = OKCODE;
    const std::string portName = "iqn.1997-xxxx";
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->DeleteIscsiInitiator(portName, errorDes);
    EXPECT_EQ(ret, SUCCESS);
}

/** 测试用例： 从存储上删除启动器
 * 前置条件：
 * CHECK点：删除失败，已经不在
 */
TEST_F(FusionStorageRestApiOperatorTest, DeleteIscsiInitiatorFailed1)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIWRONGACION;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    const std::string portName = "iqn.1997-xxxx";
    std::string errorDes;
    getHttpBodyString = g_responseResult1;
    getHttpCode = OKCODE;

    int32_t ret = fusionStorageRestApiOperatorTest->DeleteIscsiInitiator(portName, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/** 测试用例： 从存储上删除启动器
 * 前置条件：
 * CHECK点：删除行动失败
 */
TEST_F(FusionStorageRestApiOperatorTest, DeleteIscsiInitiatorFailed2)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIFAILED;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    getHttpBodyString = g_responseResult2;
    getHttpCode = OKCODE;
    const std::string portName = "iqn.1997-xxxx";
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->DeleteIscsiInitiator(portName, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/** 测试用例： 将启动器添加到主机，当返回0时成功
 * 前置条件： 启动器和主机均存在，需要将启动器添加到主机
 * CHECK点： 添加启动器成功
 */
TEST_F(FusionStorageRestApiOperatorTest, AddIscsiInitiatorToHostSuccess)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPISUCCESS;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    getHttpBodyString = g_addIscsiInitiatorToHostSuccess;
    getHttpCode = OKCODE;
    std::string iqnNumber = "iqnNumber";
    std::string hostName = "hostName";
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->AddIscsiInitiatorToHost(hostName, iqnNumber, errorDes);
    EXPECT_EQ(ret, static_cast<int32_t>(ErrorCode::RESTAPI_OK));
}

/**
 * 测试用例： 将启动器添加到主机，当返回1时表示没有
 * 前置条件： 启动器和主机均存在，需要将启动器添加到主机
 * CHECK点： 添加启动器到主机发现根本没有
 */
TEST_F(FusionStorageRestApiOperatorTest, AddIscsiInitiatorToHostFailed1)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIWRONGACION;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    getHttpBodyString = g_restapiResponseFailed1;
    getHttpCode = OKCODE;
    std::string iqnNumber = "iqnNumber";
    std::string hostName = "hostName";
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->AddIscsiInitiatorToHost(hostName, iqnNumber, errorDes);
    EXPECT_EQ(ret, static_cast<int32_t>(ErrorCode::RESTAPI_OK));
}

/**
 * 测试用例： 将启动器添加到主机，当返回1时表示没有
 * 前置条件： 启动器和主机均存在，需要将启动器添加到主机
 * CHECK点： 添加启动器到主机发现根本没有
 */
TEST_F(FusionStorageRestApiOperatorTest, AddIscsiInitiatorToHostFailed2)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIFAILED;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    getHttpBodyString = g_restapiResponseFailed1;
    getHttpCode = OKCODE;
    std::string iqnNumber = "iqnNumber";
    std::string hostName = "hostName";
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->AddIscsiInitiatorToHost(hostName, iqnNumber, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 将启动器添加到主机，当返回1时表示没有
 * 前置条件： 启动器和主机均存在，需要将启动器添加到主机
 * CHECK点： 添加启动器到主机发现根本没有
 */
TEST_F(FusionStorageRestApiOperatorTest, AddIscsiInitiatorToHostFailed3)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIFAILED;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    std::string iqnNumber = "iqnNumber";
    std::string hostName = "hostName";
    std::string errorDes;

    getHttpBodyString = g_restapiResponseFailed1;
    getHttpCode = OKCODE;

    int32_t ret = fusionStorageRestApiOperatorTest->AddIscsiInitiatorToHost(hostName, iqnNumber, errorDes);

    EXPECT_EQ(ret, FAILED);
}

/** 测试用例： 将启动器从主机删除
 * 前置条件： 没有任务需要进行时，清理环境，将启动器从主机删除
 * CHECK点： 从主机删除启动器成功
 */
TEST_F(FusionStorageRestApiOperatorTest, DeleteIscsiInitiatorFromHostSuccess1)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPISUCCESS;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    getHttpBodyString = g_deleteIscsiInitiatorFromHostSuccess;
    getHttpCode = OKCODE;
    std::string iqnNumber = "iqnNumber";
    std::string hostName = "hostName";
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->DeleteIscsiInitiatorFromHost(hostName, iqnNumber, errorDes);
    EXPECT_EQ(ret, SUCCESS);
}

/** 测试用例： 将启动器从主机删除
 * 前置条件： 没有任务需要进行时，清理环境，将启动器从主机删除
 * CHECK点： 从主机删除启动器成功
 */
TEST_F(FusionStorageRestApiOperatorTest, DeleteIscsiInitiatorFromHostSuccess2)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIWRONGACION;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
   
    getHttpBodyString = g_deleteIscsiInitiatorFromHostSuccess;
    getHttpCode = OKCODE;
    std::string iqnNumber = "iqnNumber";
    std::string hostName = "hostName";
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->DeleteIscsiInitiatorFromHost(hostName, iqnNumber, errorDes);
    EXPECT_EQ(ret, SUCCESS);
}

/** 测试用例： 将启动器从主机删除
 * 前置条件： 没有任务需要进行时，清理环境，将启动器从主机删除
 * CHECK点： 从主机删除启动器失败
 */
TEST_F(FusionStorageRestApiOperatorTest, DeleteIscsiInitiatorFromHostFailed)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIFAILED;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    getHttpBodyString = g_deleteIscsiInitiatorFromHostSuccess;
    getHttpCode = OKCODE;
    std::string iqnNumber = "iqnNumber";
    std::string hostName = "hostName";
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->DeleteIscsiInitiatorFromHost(hostName, iqnNumber, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/** 测试用例： 创建主机
 * 前置条件：在存储上建立Agent主机，用于建立ISCSI连接
 * CHECK点：创建主机成功
 */
TEST_F(FusionStorageRestApiOperatorTest, CreateHostSuccess)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPISUCCESS;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    getHttpBodyString = g_createHostSuccess;
    getHttpCode = OKCODE;
    std::string hostName = "hostName";
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->CreateHost(hostName, errorDes);
    EXPECT_EQ(ret, SUCCESS);
}

/** 测试用例： 创建主机
 * 前置条件：在存储上建立Agent主机，用于建立ISCSI连接
 * CHECK点：创建主机失败，主机已经存在
 */
TEST_F(FusionStorageRestApiOperatorTest, CreateHostFailed1)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIWRONGACION;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    getHttpBodyString = g_createHostSuccess;
    getHttpCode = OKCODE;
    std::string hostName = "hostName";
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->CreateHost(hostName, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/** 测试用例： 创建主机
 * 前置条件：在存储上建立Agent主机，用于建立ISCSI连接
 * CHECK点：创建主机失败
 */
TEST_F(FusionStorageRestApiOperatorTest, CreateHostFailed2)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIFAILED;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    getHttpBodyString = g_createHostSuccess;
    getHttpCode = OKCODE;
    std::string hostName = "hostName";
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->CreateHost(hostName, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：检查主机是否存在,成功
 * 前置条件：无
 * check点：检查主机是否存在
 */
TEST_F(FusionStorageRestApiOperatorTest, GetHostByNameSuccess)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPISUCCESS;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    getHttpBodyString = g_getHostListSuccess;
    getHttpCode = OKCODE;
    const std::string hostName = "host1";
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->GetHostByName(hostName, errorDes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 用例名称：检查主机是否存在,失败
 * 前置条件：无
 * check点：检查主机是否存在
 */
TEST_F(FusionStorageRestApiOperatorTest, GetHostByNameFailed)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIFAILED;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    getHttpBodyString = g_getHostListSuccess;
    getHttpCode = OKCODE;
    const std::string hostName = "host100";
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->GetHostByName(hostName, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 创建lun和主机映射
 * 前置条件： 收到响应体
 * CHECK点： 返回描述success
 */
TEST_F(FusionStorageRestApiOperatorTest, AddLunToHostSuccess)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPISUCCESS;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    getHttpBodyString = g_responseResult0;
    getHttpCode = OKCODE;
    const std::string hostName = "host1";
    const std::string volumeName = "volume1";
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->AddLunToHost(hostName, volumeName, errorDes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 创建lun和主机映射
 * 前置条件： 收到响应体
 * CHECK点： 返回描述success
 */
TEST_F(FusionStorageRestApiOperatorTest, AddLunToHostSuccess2)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIWRONGACION;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    getHttpBodyString = "{}";
    getHttpCode = OKCODE;

    const std::string hostName = "host1";
    const std::string volumeName = "volume1";
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->AddLunToHost(hostName, volumeName, errorDes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 创建lun和主机映射
 * 前置条件： 收到响应体
 * CHECK点： 返回描述failed
 */
TEST_F(FusionStorageRestApiOperatorTest, AddLunToHostFailed)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    getHttpBodyString = "{}";
    getHttpCode = OKCODE;

    const std::string hostName = "host1";
    const std::string volumeName = "volume1";
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->AddLunToHost(hostName, volumeName, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 通过这个获得相应的iscsi服务端口信息
 * 前置条件：
 * CHECK点： success
 */
TEST_F(FusionStorageRestApiOperatorTest, QueryIscsiPortalSuccess)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPISUCCESS;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    std::string responseBody;
    std::string errorDes;

    getHttpBodyString = g_responseResult2;
    getHttpCode = OKCODE;

    ret = fusionStorageRestApiOperatorTest->QueryIscsiPortal(responseBody, errorDes);

    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 通过这个获得相应的iscsi服务端口信息
 * 前置条件：
 * CHECK点： failed
 */
TEST_F(FusionStorageRestApiOperatorTest, QueryIscsiPortalFailed1)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = FAILED;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    std::string responseBody;
    std::string errorDes;

    getHttpBodyString = g_responseResult2;
    getHttpCode = OKCODE;

    int32_t ret = fusionStorageRestApiOperatorTest->QueryIscsiPortal(responseBody, errorDes);

    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 通过这个获得相应的iscsi服务端口信息
 * 前置条件：
 * CHECK点： failed
 */
TEST_F(FusionStorageRestApiOperatorTest, QueryIscsiPortalFailed2)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIFAILED;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    std::string responseBody;
    std::string errorDes;

    getHttpBodyString = g_responseResult2;
    getHttpCode = OKCODE;

    int32_t ret = fusionStorageRestApiOperatorTest->QueryIscsiPortal(responseBody, errorDes);

    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例：根据查询的响应体，根据管理ip、获得目标端口、启动器名称、和状态
 * 前置条件： 收到响应体
 * CHECK点： 解析响应失败
 */
TEST_F(FusionStorageRestApiOperatorTest, GetTargetIscsiPortalSuccess)
{
    const std::string responseBody = g_getTargetIscsiPortal;
    std::vector<Json::Value> targetIscsiPortalList;
    std::string errorDes;
    int32_t ret = fusionStorageRestApiOperatorTest->GetTargetIscsiPortal(responseBody, targetIscsiPortalList, errorDes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例：根据查询的响应体，根据管理ip、获得目标端口、启动器名称、和状态
 * 前置条件： 收到响应体
 * CHECK点： 解析响应失败
 */
TEST_F(FusionStorageRestApiOperatorTest, GetTargetIscsiPortalFailed)
{
    const std::string responseBody = "{\"nodeResultList\": {}}";
    std::vector<Json::Value> targetIscsiPortalList;
    std::string errorDes;
    int32_t ret = fusionStorageRestApiOperatorTest->GetTargetIscsiPortal(responseBody, targetIscsiPortalList, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 挂载卷1：建立映射关系
 * 前置条件： 收到响应体
 * CHECK点： 返回描述failed
 */
TEST_F(FusionStorageRestApiOperatorTest, CreateHostLunMappingFailed)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    getHttpBodyString = "{}";
    getHttpCode = OKCODE;

    const std::string hostName = "host1";
    const std::string volumeName = "volume1";
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->CreateHostLunMapping(hostName, volumeName, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 检查是否有ISCSI连接
 * 前置条件： 收到响应体
 * CHECK点： 返回描述Success
 */
TEST_F(FusionStorageRestApiOperatorTest, QueryIscsiSessionSuccess)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    responseResult = RESTAPISUCCESS;
    getHttpBodyString = g_responseResultCode0;
    getHttpCode = OKCODE;

    const std::string hostName = "host1";
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->QueryIscsiSession(hostName, errorDes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 检查是否有ISCSI连接
 * 前置条件： 收到响应体
 * CHECK点： 返回描述failed1
 */
TEST_F(FusionStorageRestApiOperatorTest, QueryIscsiSessionFailed)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    responseResult = FAILED;
    getHttpBodyString = g_responseResultCode2;
    getHttpCode = OKCODE;

    const std::string hostName = "host1";
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->QueryIscsiSession(hostName, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 挂载卷2：扫盘获得文件信息
 * 前置条件： 收到响应体
 * CHECK点： 返回描述failed
 */
TEST_F(FusionStorageRestApiOperatorTest, CheckAndCreateIscsiInitiatorFailed)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    getHttpBodyString = "{}";
    getHttpCode = OKCODE;

    const std::string hostName = "host1";
    std::string portName = "port1";
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->CheckAndCreateIscsiInitiator(hostName, portName, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 建立ICSSI连接1：检查并登录目标器
 * 前置条件： 收到响应体
 * CHECK点： 返回描述failed
 */
TEST_F(FusionStorageRestApiOperatorTest, CheckAndLoginIscsiTargetFailed)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    getHttpBodyString = "{}";
    getHttpCode = OKCODE;

    const std::string hostName = "host1";
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->CheckAndLoginIscsiTarget(hostName, errorDes);
    EXPECT_EQ(ret, FAILED);
}

// /**
//  * 测试用例： 检查已登录的IP和可用ISCSI端口是否可用
//  * 前置条件： 任务进入登录target阶段，获得到iscsi端口列表
//  * CHECK点： 检查到已有可用端口登录
//  */
// TEST_F(FusionStorageRestApiOperatorTest, CheckExistLoginedTargetSuccess)
// {
//     Stub stub;
//     stub.set(ADDR(FusionStorageIscsiDiskScanner, GetLoginedTargetIP), Stub_GetLoginedTargetIp);
//     stub.set(ADDR(FusionStorageRestApiOperator, CheckContain), ExecStubSuccess);
//     g_loginedIps.clear();
//     g_loginedIps.push_back(g_loginedTargetIp1);

//     Json::Value targetIscsiPortal;
//     Json::Reader reader;
//     reader.parse(g_targetIscsiPortal, targetIscsiPortal);
//     std::vector<Json::Value> targetIscsiPortalList = {targetIscsiPortal};

//     int32_t ret = fusionStorageRestApiOperatorTest->CheckExistLoginedTarget(targetIscsiPortalList);
//     EXPECT_EQ(ret, SUCCESS);
// }

/**
 * 测试用例： 检查已登录的IP和可用ISCSI端口是否可用
 * 前置条件： 任务进入登录target阶段，获得到iscsi端口列表
 * CHECK点： 检查到没有可用端口登录
 */
TEST_F(FusionStorageRestApiOperatorTest, CheckExistLoginedTargetFailed)
{
    Stub stub;
    stub.set(ADDR(FusionStorageIscsiDiskScanner, GetLoginedTargetIP), ExecStubSuccess);
    stub.set(ADDR(FusionStorageRestApiOperator, CheckContain), ExecStubFailed);

    g_loginedIps.push_back(g_loginedTargetIp2);

    Json::Value targetIscsiPortal;
    Json::Reader reader;
    reader.parse(g_targetIscsiPortal, targetIscsiPortal);
    std::vector<Json::Value> targetIscsiPortalList;
    targetIscsiPortalList.push_back(targetIscsiPortal);

    int32_t ret = fusionStorageRestApiOperatorTest->CheckExistLoginedTarget(targetIscsiPortalList);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 执行登录Target操作
 * 前置条件： 收到响应体
 * CHECK点： 返回描述failed
 */
TEST_F(FusionStorageRestApiOperatorTest, LoginTargetFailed)
{
    Stub stub;
    stub.set(ADDR(FusionStorageRestApiOperator, CheckLoginedTarget), ExecStubFailed);
    stub.set(ADDR(FusionStorageRestApiOperator, DoLoginTarget), ExecStubFailed);

    Json::Value targetIscsiPortal;
    Json::Reader reader;
    reader.parse(g_targetIscsiPortal, targetIscsiPortal);
    std::vector<Json::Value> targetIscsiPortalList = {targetIscsiPortal};

    int32_t ret = fusionStorageRestApiOperatorTest->LoginTarget();
    EXPECT_EQ(ret, FAILED);
}
/**
 * 测试用例： 执行登录目标器
 * 前置条件： 收到响应体
 * CHECK点： 返回描述failed
 */
TEST_F(FusionStorageRestApiOperatorTest, DoLoginTargetFailed)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    getHttpBodyString = "{}";
    getHttpCode = OKCODE;

    const std::string targetPortalIp = "127.0.0.1";

    int32_t ret = fusionStorageRestApiOperatorTest->DoLoginTarget(targetPortalIp);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 主机-LUN 建立映射关系2：建立ISCSI连接
 * 前置条件： 收到响应体
 * CHECK点： 返回描述failed
 */
TEST_F(FusionStorageRestApiOperatorTest, CreateIscsiInitiatorAndHostFailed)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    getHttpBodyString = "{}";
    getHttpCode = OKCODE;

    const std::string hostName = "host1";
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->CreateIscsiInitiatorAndHost(hostName, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 创建lun和主机映射
 * 前置条件： 收到响应体
 * CHECK点： 返回描述failed
 */
TEST_F(FusionStorageRestApiOperatorTest, AddLunToHostFailed1)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIFAILED;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    getHttpBodyString = "{}";
    getHttpCode = OKCODE;

    const std::string hostName = "host1";
    const std::string volumeName = "volume1";
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->AddLunToHost(hostName, volumeName, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 执行删除映射操作
 * 前置条件： 收到响应体
 * CHECK点： 返回描述success
 */
TEST_F(FusionStorageRestApiOperatorTest, DeleteLunFromHostSuccess)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = static_cast<int32_t>(ErrorCode::RESTAPI_OK);
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    getHttpBodyString = "{}";
    getHttpCode = OKCODE;

    const std::string hostName = "host1";
    const std::string volumeName = "volume1";
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->DeleteLunFromHost(hostName, volumeName, errorDes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 执行删除映射操作
 * 前置条件： 收到响应体
 * CHECK点： 返回描述failed
 */
TEST_F(FusionStorageRestApiOperatorTest, DeleteLunFromHostFailed)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    getHttpBodyString = "{}";
    getHttpCode = OKCODE;

    const std::string hostName = "host1";
    const std::string volumeName = "volume1";
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->DeleteLunFromHost(hostName, volumeName, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 解除映射关系
 * 前置条件： 收到响应体
 * CHECK点： 返回描述failed
 */
TEST_F(FusionStorageRestApiOperatorTest, DeleteHostLunMappingFailed)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    getHttpBodyString = "{}";
    getHttpCode = OKCODE;

    const std::string hostName = "host1";
    const std::string volumeName = "volume1";
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->DeleteHostLunMapping(hostName, volumeName, errorDes);
    EXPECT_EQ(ret, static_cast<int32_t>(ErrorCode::RESTAPI_OK));
}

/**
 * 测试用例： 主机-LUN 建立映射关系2：建立ISCSI连接
 * 前置条件： 收到响应体
 * CHECK点： 返回描述success
 */
TEST_F(FusionStorageRestApiOperatorTest, QueryHostFromVolumeSuccess)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = static_cast<int32_t>(ErrorCode::RESTAPI_OK);
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    getHttpBodyString = "{}";
    getHttpCode = OKCODE;

    const std::string volumeName = "volume1";
    std::string responseBody;
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->QueryHostFromVolume(volumeName, responseBody, errorDes);
    EXPECT_EQ(ret, static_cast<int32_t>(ErrorCode::RESTAPI_OK));
}

/**
 * 测试用例： 主机-LUN 建立映射关系2：建立ISCSI连接
 * 前置条件： 收到响应体
 * CHECK点： 返回描述failed
 */
TEST_F(FusionStorageRestApiOperatorTest, QueryHostFromVolumeFailed1)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIFAILED;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    getHttpBodyString = "{}";
    getHttpCode = OKCODE;

    const std::string volumeName = "volume1";
    std::string responseBody;
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->QueryHostFromVolume(volumeName, responseBody, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 主机-LUN 建立映射关系2：建立ISCSI连接
 * 前置条件： 收到响应体
 * CHECK点： 返回描述failed
 */
TEST_F(FusionStorageRestApiOperatorTest, QueryHostFromVolumeFailed2)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIWRONGACION;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    getHttpBodyString = "{}";
    getHttpCode = OKCODE;

    const std::string volumeName = "volume1";
    std::string responseBody;
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->QueryHostFromVolume(volumeName, responseBody, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 查询主机和卷的映射关系
 * 前置条件： 创建或删除主机卷映射前后需要查询映射状态
 * CHECK点： 确认查询到映射
 */
TEST_F(FusionStorageRestApiOperatorTest, QueryHostLunMappingSuccess)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    getHttpBodyString = g_getHostListSuccess;
    getHttpCode = OKCODE;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPISUCCESS;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    const std::string volumeName = "volume1";
    const std::string hostName = "host1";
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->QueryHostLunMapping(hostName, volumeName, errorDes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 查询主机和卷的映射关系
 * 前置条件：创建或删除主机卷映射前后需要查询映射状态
 * CHECK点： 获得主机列表失败
 */
TEST_F(FusionStorageRestApiOperatorTest, QueryHostLunMappingFailed1)
{
    Stub stub;
    getHttpBodyString = g_responseResult2;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIFAILED;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    getHttpCode = OKCODE;
    const std::string volumeName = "volume1";
    const std::string hostName = "host1";
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->QueryHostLunMapping(hostName, volumeName, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 查询主机和卷的映射关系
 * 前置条件：创建或删除主机卷映射前后需要查询映射状态
 * CHECK点： 没有确认到主机列表中有目标主机
 */
TEST_F(FusionStorageRestApiOperatorTest, QueryHostLunMappingFailed2)
{
    Stub stub;
    getHttpBodyString = g_getHostListSuccess;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPISUCCESS;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    getHttpCode = OKCODE;
    const std::string volumeName = "volume1";
    const std::string hostName = "hostNotExists";
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->QueryHostLunMapping(hostName, volumeName, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 删除卷的所有映射关系
 * 前置条件： 后置任务清理时卸载卷，需要解除映射
 * CHECK点： 解除所有映射关系成功
 */
TEST_F(FusionStorageRestApiOperatorTest, DeleteVolumeAllMappingSuccess)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    stub.set(ADDR(FusionStorageRestApiOperator, GetVolumeMappedHost), Stub_GetVolumeMappedHostSuccess);
    responseResult = RESTAPISUCCESS;
    getHttpBodyString = g_getHostListSuccess;
    getHttpCode = OKCODE;
    const std::string volumeName = "volume1";
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->DeleteVolumeAllMapping(volumeName, errorDes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 删除卷的所有映射关系
 * 前置条件： 后置任务清理时卸载卷，需要解除映射
 * CHECK点： 解除所有映射关系失败
 */
TEST_F(FusionStorageRestApiOperatorTest, DeleteVolumeAllMappingFailed)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    stub.set(ADDR(FusionStorageRestApiOperator, GetVolumeMappedHost), Stub_GetVolumeMappedHostFailed);
    responseResult = RESTAPISUCCESS;
    getHttpBodyString = g_getHostListSuccess;
    getHttpCode = OKCODE;
    const std::string volumeName = "volume1";
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->DeleteVolumeAllMapping(volumeName, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 获得卷的所有映射关系
 * 前置条件： 后置任务清理时卸载卷，需要解除映射，此时需要先获取
 * CHECK点： 获得卷映射的主机列表成功
 */
TEST_F(FusionStorageRestApiOperatorTest, GetVolumeMappedHostSuccess)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    stub.set(ADDR(FusionStorageRestApiOperator, ParseMappedHostList), Stub_ParseMappedHostListSuccess);
    responseResult = RESTAPISUCCESS;
    getHttpBodyString = g_getHostListSuccess;
    getHttpCode = OKCODE;
    const std::string volumeName = "volume1";
    std::vector<std::string> hostNameList;
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->GetVolumeMappedHost(volumeName, hostNameList, errorDes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 获得卷的所有映射关系
 * 前置条件： 后置任务清理时卸载卷，需要解除映射，此时需要先获取
 * CHECK点： 获得卷映射的主机列表失败
 */
TEST_F(FusionStorageRestApiOperatorTest, GetVolumeMappedHostFailed)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    stub.set(ADDR(FusionStorageRestApiOperator, ParseMappedHostList), Stub_ParseMappedHostListFailed);
    responseResult = RESTAPISUCCESS;
    getHttpBodyString = g_getHostListSuccess;
    getHttpCode = OKCODE;
    const std::string volumeName = "volume1";
    std::vector<std::string> hostNameList;
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->GetVolumeMappedHost(volumeName, hostNameList, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 从查询卷映射的主机信息列表中解析出所有主机名
 * 前置条件： 后置任务清理时卸载卷，需要解除映射，此时需要先获取映射的主机列表
 * CHECK点： 解析成功
 */
TEST_F(FusionStorageRestApiOperatorTest, ParseMappedHostListSuccess)
{
    std::string responseBody = g_queryhostfromvolumeSuccess;
    std::vector<std::string> hostNameList;

    int32_t ret = fusionStorageRestApiOperatorTest->ParseMappedHostList(responseBody, hostNameList);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 从查询卷映射的主机信息列表中解析出所有主机名
 * 前置条件： 后置任务清理时卸载卷，需要解除映射，此时需要先获取映射的主机列表
 * CHECK点： 解析失败
 */
TEST_F(FusionStorageRestApiOperatorTest, ParseMappedHostListFailed)
{
    std::string responseBody = g_queryhostfromvolumeFailed;
    std::vector<std::string> hostNameList;

    int32_t ret = fusionStorageRestApiOperatorTest->ParseMappedHostList(responseBody, hostNameList);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 获得主机列表
 * 前置条件： 调用主机检查，通过查询获得主机列表
 * CHECK点： 获得主机列表成功
 */
TEST_F(FusionStorageRestApiOperatorTest, GetHostListSuccess)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPISUCCESS;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    getHttpBodyString = g_getHostListSuccess;
    getHttpCode = OKCODE;
    std::string hostList;
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->GetHostList(hostList, errorDes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 获得主机列表
 * 前置条件： 调用主机检查，通过查询获得主机列表
 * CHECK点： 获得主机列表失败1
 */
TEST_F(FusionStorageRestApiOperatorTest, GetHostListFailed1)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIWRONGACION;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    getHttpBodyString = g_responseResult1;
    getHttpCode = OKCODE;
    std::string hostList;
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->GetHostList(hostList, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 获得主机列表
 * 前置条件： 调用主机检查，通过查询获得主机列表
 * CHECK点： 获得主机列表失败2
 */
TEST_F(FusionStorageRestApiOperatorTest, GetHostListFailed2)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPIFAILED;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    getHttpBodyString = g_responseResult2;
    getHttpCode = OKCODE;
    std::string hostList;
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->GetHostList(hostList, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 获得主机列表
 * 前置条件： 调用主机检查，通过查询获得主机列表
 * CHECK点： 获得主机列表失败2
 */
TEST_F(FusionStorageRestApiOperatorTest, GetHostListFailed3)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = FAILED;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);

    std::string hostList;
    std::string errorDes;

    getHttpBodyString = g_responseResult2;
    getHttpCode = OKCODE;

    int32_t ret = fusionStorageRestApiOperatorTest->GetHostList(hostList, errorDes);

    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 检查主机是否在列表中
 * 前置条件： 获得主机列表后，在主机列表中查询主机名
 * CHECK点： 主机在列表中
 */
TEST_F(FusionStorageRestApiOperatorTest, CheckHostInListSuccess)
{
    int32_t ret = fusionStorageRestApiOperatorTest->CheckHostInList(g_getHostListSuccess, "host1");
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 检查主机是否在列表中
 * 前置条件： 获得主机列表后，在主机列表中查询主机名
 * CHECK点： 主机不在列表中
 */
TEST_F(FusionStorageRestApiOperatorTest, CheckHostInListFailed1)
{
    int32_t ret = fusionStorageRestApiOperatorTest->CheckHostInList(g_getHostListSuccess, "host4");
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 检查主机是否在列表中
 * 前置条件： 获得主机列表后，在主机列表中查询主机名
 * CHECK点： 检查失败，未能解析
 */
TEST_F(FusionStorageRestApiOperatorTest, CheckHostInListFailed2)
{
    int32_t ret = fusionStorageRestApiOperatorTest->CheckHostInList(g_getHostListFailed, "host1");
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 返回错误描述
 * 前置条件： 收到响应体
 * CHECK点： 返回描述success
 */
TEST_F(FusionStorageRestApiOperatorTest, QuerySnapshotInfoByNameSuccess)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = static_cast<int32_t>(ErrorCode::RESTAPI_OK);
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    getHttpBodyString = "{}";
    getHttpCode = OKCODE;

    const std::string volumeName;
    std::string responseBody;
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->QuerySnapshotInfoByName(volumeName, responseBody, errorDes);
    EXPECT_EQ(ret, static_cast<int32_t>(ErrorCode::RESTAPI_OK));
}

/**
 * 测试用例： 返回错误描述
 * 前置条件： 收到响应体
 * CHECK点： 返回描述failed
 */
TEST_F(FusionStorageRestApiOperatorTest, QuerySnapshotInfoByNameFailed)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = static_cast<int32_t>(ErrorCode::RESTAPI_ERR_UNKNOWN);
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    getHttpBodyString = "{}";
    getHttpCode = OKCODE;

    const std::string volumeName;
    std::string responseBody;
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->QuerySnapshotInfoByName(volumeName, responseBody, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 返回错误描述
 * 前置条件： 收到响应体
 * CHECK点： 返回描述failed
 */
TEST_F(FusionStorageRestApiOperatorTest, QuerySnapshotInfoByNameFailed2)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    getHttpBodyString = "{}";
    getHttpCode = OKCODE;

    const std::string volumeName;
    std::string responseBody;
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->QuerySnapshotInfoByName(volumeName, responseBody, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 查询卷信息，尺寸、WWN都在这里获得
 * 前置条件： 收到响应体
 * CHECK点： 返回描述success
 */
TEST_F(FusionStorageRestApiOperatorTest, QueryVolumeInfoByNameSuccess)
{
    bool isBackUp = true;
    VolInfo volInfo;
    volInfo.m_datastore.m_ip = "88.1.1.21";
    volInfo.m_datastore.m_moRef = "88.1.1.21";

    int32_t ret = fusionStorageRestApiOperatorTest->GetStorageInfo(g_hcsAuthExtendInfo, volInfo, isBackUp);

    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = static_cast<int32_t>(ErrorCode::RESTAPI_OK);
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    getHttpBodyString = "{}";
    getHttpCode = OKCODE;

    const std::string volumeName;
    std::string responseBody;
    std::string errorDes;

    ret = fusionStorageRestApiOperatorTest->QueryVolumeInfoByName(volumeName, responseBody, errorDes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 查询卷信息，尺寸、WWN都在这里获得
 * 前置条件： 收到响应体
 * CHECK点： 返回描述failed
 */
TEST_F(FusionStorageRestApiOperatorTest, QueryVolumeInfoByNameFailed1)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    getHttpBodyString = "{}";
    getHttpCode = OKCODE;

    const std::string volumeName;
    std::string responseBody;
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->QueryVolumeInfoByName(volumeName, responseBody, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 查询卷信息，尺寸、WWN都在这里获得
 * 前置条件： 收到响应体
 * CHECK点： 返回描述failed
 */
TEST_F(FusionStorageRestApiOperatorTest, QueryVolumeInfoByNameFailed2)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = -1;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    getHttpBodyString = "{}";
    getHttpCode = OKCODE;

    const std::string volumeName;
    std::string responseBody;
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->QueryVolumeInfoByName(volumeName, responseBody, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 从响应体中获取WWN
 * 前置条件： 收到响应体
 * CHECK点： 获取成功
 */
TEST_F(FusionStorageRestApiOperatorTest, GetSnapshotOrVolumeInfoSuccess)
{
    std::string errorDes;
    int32_t ret = fusionStorageRestApiOperatorTest->GetSnapshotOrVolumeInfo(g_getSnapshotOrVolumeInfoSuccess, errorDes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 测试用例： 从响应体中获取WWN
 * 前置条件： 收到响应体
 * CHECK点： 获取失败
 */
TEST_F(FusionStorageRestApiOperatorTest, GetSnapshotOrVolumeInfoFailed)
{
    std::string errorDes;
    int32_t ret = fusionStorageRestApiOperatorTest->GetSnapshotOrVolumeInfo(g_responseResult0, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 测试用例： 通过查询卷名的信息获得wwn，用于获取磁盘路径和校验挂载的卷是否正确
 * 前置条件： 收到响应体
 * CHECK点： 返回描述failed
 */
TEST_F(FusionStorageRestApiOperatorTest, GetVolumeWwnByNameFailed)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    getHttpBodyString = "{}";
    getHttpCode = OKCODE;

    const std::string volumeName = "volume1";
    std::string errorDes;

    int32_t ret = fusionStorageRestApiOperatorTest->GetVolumeWwnByName(volumeName, errorDes);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：从Json对象中读取一个字符串字段,成功
 * 前置条件：无
 * check点：从Json对象中读取一个字符串字段
 */
TEST_F(FusionStorageRestApiOperatorTest, GetStringFieldSuccess)
{
    Json::Value jsonBody;
    jsonBody["name"] = "k8s";
    const string field = "name";
    std::string result;
    int32_t ret = fusionStorageRestApiOperatorTest->GetStringField(jsonBody, field, result);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 用例名称：从Json对象中读取一个字符串字段,失败
 * 前置条件：无
 * check点：从Json对象中读取一个字符串字段
 */
TEST_F(FusionStorageRestApiOperatorTest, GetStringFieldFailed)
{
    Json::Value jsonBody;
    jsonBody["name"] = "k8s";
    const string field = "abc";
    std::string result;
    int32_t ret = fusionStorageRestApiOperatorTest->GetStringField(jsonBody, field, result);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：从Json对象中读取一个整数字段,失败
 * 前置条件：无
 * check点：从Json对象中读取一个整数字段
 */
TEST_F(FusionStorageRestApiOperatorTest, GetIntFieldFailed)
{
    Json::Value jsonBody;
    jsonBody["name"] = "k8s";
    const string field = "abc";
    int32_t result;
    int32_t ret = fusionStorageRestApiOperatorTest->GetIntField(jsonBody, field, result);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：检查已登录的端口列表是否包含target,成功
 * 前置条件：无
 * check点：检查已登录的端口列表是否包含target
 */
TEST_F(FusionStorageRestApiOperatorTest, CheckContainSuccess)
{
    const std::string targetIP = "192.168.1.1";
    std::vector<std::string> loginIP;
    loginIP.push_back("192.168.1.1");
    loginIP.push_back("192.168.1.2");
    loginIP.push_back("192.168.1.3");
    int32_t ret = fusionStorageRestApiOperatorTest->CheckContain(targetIP, loginIP);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 用例名称：检查已登录的端口列表是否包含target，失败
 * 前置条件：无
 * check点：检查已登录的端口列表是否包含target
 */
TEST_F(FusionStorageRestApiOperatorTest, CheckContainFailed)
{
    const std::string targetIP = "192.168.1.4";
    std::vector<std::string> loginIP;
    loginIP.push_back("192.168.1.1");
    loginIP.push_back("192.168.1.2");
    loginIP.push_back("192.168.1.3");
    int32_t ret = fusionStorageRestApiOperatorTest->CheckContain(targetIP, loginIP);
    EXPECT_EQ(ret, FAILED);
}

/**
 * 用例名称：发送api请求，并返回是否成功拿到响应
 * 前置条件：需要发送api请求
 * check点：获得响应成功
 */
TEST_F(FusionStorageRestApiOperatorTest, checkResponseResultSuccess)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = RESTAPISUCCESS;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    getHttpBodyString = g_responseResult0;
    getHttpCode = OKCODE;
    RequestInfo requestInfo;
    std::shared_ptr<ResponseModel> response = std::make_shared<ResponseModel>();

    int32_t ret = fusionStorageRestApiOperatorTest->checkResponseResult(requestInfo, response, false);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * 用例名称：发送api请求，并返回是否成功拿到响应
 * 前置条件：需要发送api请求
 * check点：获得响应失败
 */
TEST_F(FusionStorageRestApiOperatorTest, checkResponseResultFailed)
{
    Stub stub;
    stub.set(ADDR(Module::IHttpClient, GetInstance), Stub_CallApi);
    responseResult = FAILED;
    stub.set(ADDR(FusionStorageRestApiOperator, GetResponseResult), Stub_GetResponseResult);
    getHttpBodyString = g_responseResult0;
    getHttpCode = OKCODE;
    RequestInfo requestInfo;
    std::shared_ptr<ResponseModel> response = std::make_shared<ResponseModel>();

    int32_t ret = fusionStorageRestApiOperatorTest->checkResponseResult(requestInfo, response, false);
    EXPECT_EQ(ret, FAILED);
}

}  // namespace HDT_TEST
