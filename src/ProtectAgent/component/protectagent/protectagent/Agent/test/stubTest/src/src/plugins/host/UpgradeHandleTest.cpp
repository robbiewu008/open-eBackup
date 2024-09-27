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
#include "plugins/host/UpgradeHandleTest.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/ErrorCode.h"
#include "common/ConfigXmlParse.h" 
#include "common/File.h"
#include "securecom/RootCaller.h"
#include "securecom/CryptAlg.h"
#include <vector>


namespace {
mp_void LogReturn(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...)
{
    return;
}

mp_int32 ExecFailed(const mp_string& pszSourceFilePath, const mp_string& pszDestFilePath)
{
    return MP_FAILED;
}

mp_int32 ExecSucc(const mp_string& pszSourceFilePath, const mp_string& pszDestFilePath)
{
    return MP_SUCCESS;
}

mp_int32 CheckBeforeUpgradeFailed()
{
    return MP_FAILED;
}

mp_int32 CheckBeforeUpgradeSucc()
{
    return MP_SUCCESS;
}

mp_int32 ObtainUpgradePacFailed()
{
    return MP_FAILED;
}

mp_int32 ObtainUpgradePacSucc()
{
    return MP_SUCCESS;
}

mp_int32 PrepareForUpgradeFailed()
{
    return MP_FAILED;
}

mp_int32 PrepareForUpgradeSucc()
{
    return MP_SUCCESS;
}

mp_int32 CopyFileCoverDestFailed(const mp_string& pszSourceFilePath, const mp_string& pszDestFilePath)
{
    return MP_FAILED;
}

mp_int32 CopyFileCoverDestSucc(const mp_string& pszSourceFilePath, const mp_string& pszDestFilePath)
{
    return MP_SUCCESS;
}
}

mp_int32 StubGetValueString4_succ(const mp_string& strParentSection, const mp_string& strChildSection, 
    const mp_string& strKey, mp_string& strValue)
{
    return 0;
}

mp_int32 StubGetValueString4_fail(const mp_string& strParentSection, const mp_string& strChildSection, 
    const mp_string& strKey, mp_string& strValue)
{
    return -1;
}

mp_int32 StubGetValueString3_succ(const mp_string& strSection, const mp_string& strKey,
    mp_string& strValue)
{
    return 0;
}

mp_int32 StubGetValueString3_fail(const mp_string& strSection, const mp_string& strKey,
    mp_string& strValue)
{
    return -1;
}

mp_int32 StubGetDownloadInfo_succ()
{
    return 0;
}

mp_int32 StubGetDownloadInfo_fail()
{
    return -1;
}

mp_int32 StubSecurityConfiguration_succ(HttpRequest& req)
{
    return 0;
}

mp_int32 StubSecurityConfiguration_fail(HttpRequest& req)
{
    return -1;
}

mp_int32 StubInitRequest_fail()
{
    return -1;
}

mp_int32 StubInitRequest_succ()
{
    UpgradeHandle::m_vecPMIp.push_back("192.168.0.0");
    return 0;
}

mp_int32 StubSendRequest_fail(IHttpClient* httpClient, const HttpRequest& req, mp_string& disposition)
{
    return -1;
}

mp_int32 StubSendRequest_succ(IHttpClient* httpClient, const HttpRequest& req, mp_string& disposition)
{
    disposition = "DataProtect_client.zip_123456aaaa\"";
    return 0;
}

mp_int32 StubReadFile_fail(mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    return -1;
}

mp_int32 StubReadFile_fail1(mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    return 0;
}

mp_int32 StubReadFile_fail2(mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    vecOutput.push_back("downloadLink=https://[192.168.69.203,192.168.69.202:25082/v1/host-agent/update/download?uuid=crmdownloadlinkee763a7020164ef29d08f60d91e20a33");
    return 0;
}

mp_int32 StubReadFile_fail3(mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    vecOutput.push_back("downloadLink=https://[192.168.69.203,192.168.69.202]25082/v1/host-agent/update/download?uuid=crmdownloadlinkee763a7020164ef29d08f60d91e20a33");
    return 0;
}


mp_int32 StubReadFile_succ(mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    vecOutput.push_back("downloadLink=https://[192.168.69.203,192.168.69.202]:25082/v1/host-agent/update/download?uuid=crmdownloadlinkee763a7020164ef29d08f60d91e20a33");
    return 0;
}

mp_string StubGetStmpFilePath(const mp_string& strFileName)
{
    std::string parentPath = get_current_dir_name();
    return parentPath + "/" + "upgrade_signature.sign";
}

TEST_F(CUpgradeHandleTest, UpgradeAgentHandleTest)
{
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    mp_void *param = NULL;
    UpgradeHandle upgradeHandle;
    mp_void *iRet = NULL;

    stub.set(ADDR(UpgradeHandle, CheckBeforeUpgrade), CheckBeforeUpgradeFailed);
    iRet = upgradeHandle.UpgradeAgentHandle(param);
    EXPECT_TRUE(iRet == NULL);

    stub.set(ADDR(UpgradeHandle, CheckBeforeUpgrade), CheckBeforeUpgradeSucc);
    stub.set(ADDR(UpgradeHandle, ObtainUpgradePac), ObtainUpgradePacFailed);
    iRet = upgradeHandle.UpgradeAgentHandle(param);
    EXPECT_TRUE(iRet == NULL);

    stub.set(ADDR(UpgradeHandle, ObtainUpgradePac), ObtainUpgradePacSucc);
    stub.set(ADDR(UpgradeHandle, PrepareForUpgrade), PrepareForUpgradeFailed);
    iRet = upgradeHandle.UpgradeAgentHandle(param);
    EXPECT_TRUE(iRet == NULL);

    stub.set(ADDR(UpgradeHandle, PrepareForUpgrade), PrepareForUpgradeSucc);
    stub.set(ADDR(CRootCaller, Exec), ExecFailed);
    iRet = upgradeHandle.UpgradeAgentHandle(param);
    EXPECT_TRUE(iRet == NULL);

    stub.set(ADDR(CRootCaller, Exec), ExecSucc);
    iRet = upgradeHandle.UpgradeAgentHandle(param);
    EXPECT_TRUE(iRet == NULL);
}

TEST_F(CUpgradeHandleTest, CheckBeforeUpgradeTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    UpgradeHandle upgradeHandle;

    stub.set(ADDR(CRootCaller, Exec), ExecFailed);
    mp_int32 iRet = upgradeHandle.CheckBeforeUpgrade();
    
    stub.set(ADDR(CRootCaller, Exec), ExecSucc);
    iRet = upgradeHandle.CheckBeforeUpgrade();
}

/*
* Description  : 从下载点获取升级包
* 前置条件：存在记录升级信息的文件
* check点：1、记录升级信息的文件是否存在，2、获取记录升级信息是否成功
*/
TEST_F(CUpgradeHandleTest, ObtainUpgradePacTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    UpgradeHandle upgradeHandle;

    stub.set(ADDR(UpgradeHandle, InitRequest), StubInitRequest_fail);
    mp_int32 iRet = upgradeHandle.ObtainUpgradePac();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(UpgradeHandle, InitRequest), StubInitRequest_succ);
    stub.set(ADDR(UpgradeHandle, SendRequest), StubSendRequest_fail);
    iRet = upgradeHandle.ObtainUpgradePac();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(UpgradeHandle, SendRequest), StubSendRequest_succ);
    iRet = upgradeHandle.ObtainUpgradePac();
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
* Description  : 准备开始升级校验
* 前置条件：存在包
* check点：1、校验包的完整性，2、能否解压包
*/
TEST_F(CUpgradeHandleTest, PrepareForUpgradeTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), LogReturn);
    UpgradeHandle upgradeHandle;
    upgradeHandle.m_signature = "vm3M9mwG/ddx+6sABqX+z+gTYeAxa1iFiGJwg3lb+K9dTupVqiyzYt7Z2nmQRjlmbkLB8/jpx/DeMLU/+CA2VTNqWET/"
                                "F4Uw9CRijUiMx0FfJ1vV1fM2tJrqgqfnrpiC6RxOHqqA6Cxi9HQLL8pQGuwYdavqDNHgKhCPLZQTnMMV9I9yt0T7qIwo"
                                "fb6Puiv79C35HM5dyxm547zWyzafynpqLATYIzv3yyXSLXrP6sccup3N1FZV5pqgOZjQeIxuqkXx2PG4rsdLcRwzQdJM"
                                "jyzLZeJK9bjW75vW8N8RxpuO1ckS+nxW+MsMePwwurK8T6cNma8oyyoX/LB74pz/zQWDU405YWPD6l29NyReSZ7pzXfi"
                                "q1JuqQO1a2225VTPiWpCv3Q1pf8oGxAzae481JsZpR/59dkwxOiv29uMyEh2E8tCVm6vQ7KjpUr3InudzQBJFWUgwi1R"
                                "RRzAyTPbaRNw2pSnG2IKkdM4NLXzJ/Bufo/w85mWedGs/QDNxKip";


    stub.set((mp_string(CPath::*)(mp_string))ADDR(CPath, GetStmpFilePath), StubGetStmpFilePath);
    stub.set(ADDR(CRootCaller, Exec), ExecFailed);
    mp_int32 iRet = upgradeHandle.PrepareForUpgrade();
    EXPECT_EQ(iRet, MP_FAILED);
    
    stub.set(ADDR(CRootCaller, Exec), ExecSucc);
    iRet = upgradeHandle.PrepareForUpgrade();
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
* Description  : 升级流程之前检查主机资源
* 前置条件：可用空间大小满足条件
* check点：1、检查可用空间大小，2、检查升级是否成功
*/
TEST_F(CUpgradeHandleTest, UpdateUpgradeStatusTest)
{
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), LogReturn);
    mp_string strUpgradeStatus;
    UpgradeHandle upgradeHandle;
    mp_int32 iRet;

    stub.set(ADDR(CMpFile, CopyFileCoverDest), CopyFileCoverDestFailed);
    iRet = upgradeHandle.UpdateUpgradeStatus(strUpgradeStatus);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CMpFile, CopyFileCoverDest), CopyFileCoverDestSucc);
    iRet = upgradeHandle.UpdateUpgradeStatus(strUpgradeStatus);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
* Description  : 下载升级包，初始化请求
* 前置条件：链接信息获取成功
* check点：1、链接信息解析成功，2、证书信息读取成功
*/
TEST_F(CUpgradeHandleTest, InitRequestTest)
{
    Stub stub;
    UpgradeHandle upgradeHandle;
    HttpRequest req;
    stub.set(ADDR(UpgradeHandle, GetDownloadInfo), StubGetDownloadInfo_fail);
    mp_int32 iRet = upgradeHandle.InitRequestCommon();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(UpgradeHandle, GetDownloadInfo), StubGetDownloadInfo_succ);
    UpgradeHandle::m_vecPMIp.clear();
    stub.set(ADDR(UpgradeHandle, SecurityConfiguration), StubSecurityConfiguration_fail);
    iRet = upgradeHandle.InitRequest(req);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(UpgradeHandle, SecurityConfiguration), StubSecurityConfiguration_succ);
    UpgradeHandle::m_vecPMIp.push_back("192.168.0.0");
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubGetValueString3_fail);
    iRet = upgradeHandle.InitRequest(req);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubGetValueString3_succ);
    iRet = upgradeHandle.InitRequest(req);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
* Description  : 下载升级包，设置证书配置
* 前置条件： 证书信息读取成功
* check点：1、证书信息读取成功
*/
TEST_F(CUpgradeHandleTest, SecurityConfigurationTest)
{
    Stub stub;
    UpgradeHandle upgradeHandle;
    HttpRequest req;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubGetValueString3_fail);
    mp_int32 iRet = upgradeHandle.SecurityConfiguration(req);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubGetValueString3_succ);
    iRet = upgradeHandle.SecurityConfiguration(req);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
* Description  : 下载升级包，解析链接信息
* 前置条件： 升级信息文件存在
* check点：链接信息解析成功
*/
TEST_F(CUpgradeHandleTest, GetDownloadInfo)
{
    Stub stub;
    UpgradeHandle upgradeHandle;
    stub.set(ADDR(CIPCFile, ReadFile), StubReadFile_fail);
    mp_int32 iRet = upgradeHandle.GetDownloadInfo();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CIPCFile, ReadFile), StubReadFile_fail1);
    iRet = upgradeHandle.GetDownloadInfo();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CIPCFile, ReadFile), StubReadFile_fail2);
    iRet = upgradeHandle.GetDownloadInfo();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CIPCFile, ReadFile), StubReadFile_fail3);
    iRet = upgradeHandle.GetDownloadInfo();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CIPCFile, ReadFile), StubReadFile_succ);
    iRet =  upgradeHandle.GetDownloadInfo();
    EXPECT_EQ(iRet, MP_SUCCESS);
}