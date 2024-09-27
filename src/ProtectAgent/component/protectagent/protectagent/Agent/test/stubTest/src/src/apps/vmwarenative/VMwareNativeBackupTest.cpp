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
#include "apps/vmwarenative/VMwareNativeBackupTest.h"

#define DoLogTest()                                                                                                    \
    do {                                                                                                               \
        stub.set(ADDR(CLogger, Log), LogTest);                                                                         \
    } while (0)

namespace {
mp_int32 RETRY_NUM = 0;
const mp_int32 CURLE_OK = 0;
const mp_int32 CURLE_OTHER = 1;
const mp_int32 CURLE_COULDNT_RESOLVE_HOST = 6;
const mp_int32 CURLE_SSL_CIPHER = 59;
}

mp_int32 StubFailed()
{
    return MP_FAILED;
}

mp_int32 StubSuccess()
{
    return MP_SUCCESS;
}

mp_int32 stubParsejsonSUCCESS_CAInfoIsEmpty(const mp_string &msgBody, mp_string &taskId, VerifyVcenterParam &VerifyParam)
{
    mp_string strTestCAInfo = "";
    VerifyParam.vecCAInfo.push_back(strTestCAInfo);
    return MP_SUCCESS;
}

mp_int32 stubParsejsonSUCCESS_CAInfoIsNoEmpty(const mp_string &msgBody, mp_string &taskId, VerifyVcenterParam &VerifyParam)
{
    mp_string strTestCAInfo = "test";
    VerifyParam.vecCAInfo.push_back(strTestCAInfo);
    return MP_SUCCESS;
}

// Parsejson用例的桩函数
const mp_string TestMsgBody =
    "{"\
        "\"body\":{"\
            "\"ProductManager\":{"\
                "\"Certs\":[\"test\"],"\
                "\"Cls\":\"test\","\
                "\"IP\":\"1.1.1.1\","\
                "\"Password\":\"test\","\
                "\"Port\":1234,"\
                "\"Protocol\":0,"\
                "\"RevocationList\":\"test\","\
                "\"UserName\":\"test\","\
                "\"Version\":\"test\"\
                "\"TlsCompatible\":false,"\
                "},\
            "\"parentTaskId\":\"test\","\
            "\"taskId\":\"test\","\
            "\"vmInfo\":{"\
                "\"vmID\":\"test\","\
                "\"vmName\":\"test\","\
                "\"vmRef\":\"test\"\
            "}"\
        "},"\
        "\"cmd\":1"\
    "}";

const mp_string TestMsgBodyNoProductManager =
    "{"\
        "\"body\":{"\
            "\"parentTaskId\":\"test\","\
            "\"taskId\":\"test\","\
            "\"vmInfo\":{"\
                "\"vmID\":\"test\","\
                "\"vmName\":\"test\","\
                "\"vmRef\":\"test\""\
            "}"\
        "},"\
        "\"cmd\":1"\
    "}";

// PreVerify用例桩函数

mp_int32 stubWriteFile(const mp_string& strFilePath, const vector<mp_string>& vecInput)
{
    if (!strFilePath.empty()) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

// UseCurl2VerifyVcenter用例桩函数
CURL* StubCurlEasyInit()
{
    return nullptr;
}

mp_int32 StubCurlEasyPerformOK()
{
    if (RETRY_NUM++ < 1) {
        return CURLE_COULDNT_RESOLVE_HOST;
    }
    return CURLE_OK;
}

mp_int32 StubCurlEasyPerformCIPHER()
{
    return CURLE_SSL_CIPHER;
}

mp_int32 StubCurlEasyPerformOTHER()
{
    return CURLE_OTHER;
}

// CleanUpTmpCertFile用例桩函数

mp_int32 stubDelFile(const mp_string& pszFilePath)
{
    if (!pszFilePath.empty()) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

/*
 * 用例名称：认证Vcenter证书测试
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(VMwareNativeBackupTest, VerifyVcenterCert)
{
    VMwareNativeBackup vmwareNativeBackup;
    mp_int32 iRet;
    mp_string msgBody = "";
    mp_string taskId = "";
    stub.set(ADDR(VMwareNativeBackup, Parsejson), StubFailed);
    iRet = vmwareNativeBackup.VerifyVcenterCert(msgBody, taskId);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(ADDR(VMwareNativeBackup, Parsejson), stubParsejsonSUCCESS_CAInfoIsEmpty);
    iRet = vmwareNativeBackup.VerifyVcenterCert(msgBody, taskId);
    EXPECT_EQ(MP_SUCCESS, iRet);
    stub.set(ADDR(VMwareNativeBackup, Parsejson), stubParsejsonSUCCESS_CAInfoIsNoEmpty);

    stub.set(ADDR(VMwareNativeBackup, PreVerify), StubFailed);
    iRet = vmwareNativeBackup.VerifyVcenterCert(msgBody, taskId);
    EXPECT_EQ(MP_FAILED, iRet);
    stub.set(ADDR(VMwareNativeBackup, PreVerify), StubSuccess);

    stub.set(ADDR(VMwareNativeBackup, UseCurl2VerifyVcenter), StubFailed);
    iRet = vmwareNativeBackup.VerifyVcenterCert(msgBody, taskId);
    EXPECT_EQ(MP_FAILED, iRet);
    stub.set(ADDR(VMwareNativeBackup, UseCurl2VerifyVcenter), StubSuccess);

    stub.set(ADDR(VMwareNativeBackup, CleanUpTmpCertFile), StubFailed);
    iRet = vmwareNativeBackup.VerifyVcenterCert(msgBody, taskId);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(ADDR(VMwareNativeBackup, CleanUpTmpCertFile), StubSuccess);
    iRet = vmwareNativeBackup.VerifyVcenterCert(msgBody, taskId);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

/*
 * 用例名称：解析DME的json体获取
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(VMwareNativeBackupTest, Parsejson)
{
    VMwareNativeBackup vmwareNativeBackup;
    mp_int32 iRet;
    mp_string taskId = "";
    VerifyVcenterParam VerifyParam;

    stub.set(ADDR(VMwareNativeBackup, GetRequestJsonBody), StubFailed);
    iRet = vmwareNativeBackup.Parsejson(TestMsgBody, taskId, VerifyParam);
    EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);
    stub.reset(ADDR(VMwareNativeBackup, GetRequestJsonBody), StubSuccess);

    stub.set(ADDR(VMwareNativeBackup, GetTaskId), StubFailed);
    iRet = vmwareNativeBackup.Parsejson(TestMsgBody, taskId, VerifyParam);
    EXPECT_EQ(MP_FAILED, iRet);
    stub.reset(ADDR(VMwareNativeBackup, GetTaskId), StubSuccess);

    iRet = vmwareNativeBackup.Parsejson(TestMsgBodyNoProductManager, taskId, VerifyParam);
    EXPECT_EQ(MP_FAILED, iRet);

    iRet = vmwareNativeBackup.Parsejson(TestMsgBody, taskId, VerifyParam);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

/*
 * 用例名称：认证Vcenter证书前置准备
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(VMwareNativeBackupTest, PreVerify)
{
    VMwareNativeBackup vmwareNativeBackup;
    mp_int32 iRet;
    mp_string taskId = "";
    VerifyVcenterParam VerifyParam;
    VerifyParam.strIP = "1.1.1.1";
    VerifyParam.uintProt = 1234

    stub.set(ADDR(CIPCFile, WriteFile), stubWriteFile);

    VerifyParam.strTmpVcenterCAFile = "";
    iRet = vmwareNativeBackup.PreVerify(VerifyParam);
    EXPECT_EQ(MP_FAILED, iRet);

    VerifyParam.strTmpVcenterCAFile = "test";
    VerifyParam.vecCRLInfo.clear();
    iRet = vmwareNativeBackup.PreVerify(VerifyParam);
    EXPECT_EQ(MP_FAILED, iRet);

    VerifyParam.vecCRLInfo.emplace_back("test")
    VerifyParam.strTmpVcenterCRLFile = "";
    iRet = vmwareNativeBackup.PreVerify(VerifyParam);
    EXPECT_EQ(MP_FAILED, iRet);

    VerifyParam.strTmpVcenterCRLFile = "test";
    iRet = vmwareNativeBackup.PreVerify(VerifyParam);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

/*
 * 用例名称：执行认证Vcenter证书
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(VMwareNativeBackupTest, UseCurl2VerifyVcenter)
{
    VMwareNativeBackup vmwareNativeBackup;
    mp_int32 iRet;
    mp_string taskId = "";
    VerifyVcenterParam VerifyParam;
    VerifyParam.strTmpVcenterCAFile = "test";
    VerifyParam.strTmpVcenterCRLFile = "test";
    VerifyParam.bCRLIsEmpty = MP_FALSE;
    VerifyParam.bTLSCompatible = MP_FALSE;

    stub.set(curl_easy_init, StubCurlEasyInit);
    iRet = vmwareNativeBackup.UseCurl2VerifyVcenter(taskId, VerifyParam);
    EXPECT_EQ(MP_FAILED, iRet);
    stub.reset(curl_easy_init);

    stub.set(curl_easy_perform, StubCurlEasyPerformCIPHER);
    iRet = vmwareNativeBackup.UseCurl2VerifyVcenter(taskId, VerifyParam);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(curl_easy_perform, StubCurlEasyPerformOTHER);
    iRet = vmwareNativeBackup.UseCurl2VerifyVcenter(taskId, VerifyParam);
    EXPECT_EQ(MP_FAILED, iRet);

    RETRY_NUM = 0;
    stub.set(curl_easy_perform, StubCurlEasyPerformOK);
    iRet = vmwareNativeBackup.UseCurl2VerifyVcenter(taskId, VerifyParam);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

/*
 * 用例名称：删除临时的Vcenter证书
 * 前置条件：无
 * check点：检查返回值
 */
TEST_F(VMwareNativeBackupTest, CleanUpTmpCertFile)
{
    VMwareNativeBackup vmwareNativeBackup;
    mp_int32 iRet;
    VerifyVcenterParam VerifyParam;

    stub.set(ADDR(CMpFile, DelFile), stubDelFile);
    VerifyParam.strTmpVcenterCAFile = "";
    iRet = vmwareNativeBackup.CleanUpTmpCertFile(testFileInfoEmpty, testFileInfoEmpty);
    EXPECT_EQ(MP_FAILED, iRet);
    VerifyParam.strTmpVcenterCAFile = "test";

    VerifyParam.bCRLIsEmpty = MP_FALSE
    VerifyParam.strTmpVcenterCRLFile = "";
    iRet = vmwareNativeBackup.CleanUpTmpCertFile(testFileInfoNoEmpty, testFileInfoEmpty);
    EXPECT_EQ(MP_FAILED, iRet);
    VerifyParam.strTmpVcenterCRLFile = "test";

    iRet = vmwareNativeBackup.CleanUpTmpCertFile(testFileInfoNoEmpty, testFileInfoNoEmpty);
    EXPECT_EQ(MP_SUCCESS, iRet);

    VerifyParam.bCRLIsEmpty = MP_TRUE
    iRet = vmwareNativeBackup.CleanUpTmpCertFile(testFileInfoNoEmpty, testFileInfoNoEmpty);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

/*
* 测试用例：调用CheckTool
* 前置条件：GetTaskId失败
* CHECK点：调用CheckTool返回失败
* 前置条件：GetTaskId成功
* 前置条件：CheckTool返回失败
* CHECK点：调用CheckTool返回失败
* 前置条件：CheckTool返回成功
* CHECK点：调用CheckTool返回成功
*/
TEST_F(VMwareNativeBackupTest, VmfsCheckTool)
{
    mp_int32 iRet;
    VMwareNativeBackup VMBObj;
    mp_string msgBody;
    mp_string taskId;

    stub.set(ADDR(VMwareNativeBackup, GetTaskId), StubFailed);
    iRet = VMBObj.VmfsCheckTool(msgBody, taskId);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.reset(ADDR(VMwareNativeBackup, GetTaskId));
    stub.set(ADDR(VMwareNativeBackup, GetTaskId), StubSuccess);
    stub.set(ADDR(VmfsHandler, CheckTool), StubFailed);
    iRet = VMBObj.VmfsCheckTool(msgBody, taskId);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.reset(ADDR(VmfsHandler, CheckTool));
    stub.set(ADDR(VmfsHandler, CheckTool), StubSuccess);
    iRet = VMBObj.VmfsCheckTool(msgBody, taskId);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
* 测试用例：调用VmfsMount
* 前置条件：不合法入参
* CHECK点：调用VmfsMount失败
* 前置条件：合法入参
* 前置条件：Mount调用失败
* CHECK点：调用VmfsMount失败
* 前置条件：Mount调用成功
* CHECK点：调用VmfsMount成功
*/
TEST_F(VMwareNativeBackupTest, VmfsMount)
{
    mp_int32 iRet;
    VMwareNativeBackup VMBObj;
    mp_string msgBody = "";
    mp_string taskId;
    Json::Value respBody;

    iRet = VMBObj.VmfsMount(msgBody, taskId, respBody);
    EXPECT_EQ(iRet, MP_FAILED);

    msg_body = "{ \
	\"taskId\": \"123\",\
	\"wwn\": [\
		\"xxxx-xxx-A\",\
		\"xxxx-xxx-B\",\
		\"xxxx-xxx-C\"\
	]\
    }";

    stub.set(ADDR(VmfsHandler, Mount), StubFailed);
    iRet = VMBObj.VmfsMount(msgBody, taskId, respBody);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.reset(ADDR(VmfsHandler, Mount));
    stub.set(ADDR(VmfsHandler, Mount), StubSuccess);
    iRet = VMBObj.VmfsMount(msgBody, taskId, respBody);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
* 测试用例：调用VmfsUmount
* 前置条件：不合法入参
* CHECK点：调用VmfsUmount失败
* 前置条件：合法入参
* 前置条件：Umount调用失败
* CHECK点：调用VmfsUmount失败
* 前置条件：Umount调用成功
* CHECK点：调用VmfsUmount成功
*/
TEST_F(VMwareNativeBackupTest, VmfsUmount)
{
    mp_int32 iRet;
    VMwareNativeBackup VMBObj;
    mp_string msgBody = "";
    mp_string taskId;

    iRet = VMBObj.VmfsUmount(msgBody, taskId);
    EXPECT_EQ(iRet, MP_FAILED);

    msg_body = "{ \
	\"taskId\": \"123\",\
	\"mountPath\": [\
		\"xxxx-xxx-A\",\
		\"xxxx-xxx-B\",\
		\"xxxx-xxx-C\"\
	]\
    }";

    stub.set(ADDR(VmfsHandler, Umount), StubFailed);
    iRet = VMBObj.VmfsUmount(msgBody, taskId);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.reset(ADDR(VmfsHandler, Umount));
    stub.set(ADDR(VmfsHandler, Umount), StubSuccess);
    iRet = VMBObj.VmfsUmount(msgBody, taskId);
    EXPECT_EQ(iRet, MP_SUCCESS);
}