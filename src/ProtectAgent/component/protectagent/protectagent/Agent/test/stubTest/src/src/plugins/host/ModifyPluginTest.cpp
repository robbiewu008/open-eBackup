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
#include "plugins/host/ModifyPluginHandleTest.h"
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

mp_int32 CheckBeforeModifyPluginFailed()
{
    return MP_FAILED;
}

mp_int32 CheckBeforeModifyPluginSucc()
{
    return MP_SUCCESS;
}

mp_int32 ObtainModifyPluginPacFailed()
{
    return MP_FAILED;
}

mp_int32 ObtainModifyPluginPacSucc()
{
    return MP_SUCCESS;
}

mp_int32 PrepareForModifyPluginFailed()
{
    return MP_FAILED;
}

mp_int32 PrepareForModifyPluginSucc()
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

mp_int32 StubGetValueString_succ(const mp_string& strParentSection, const mp_string& strChildSection, 
    const mp_string& strKey, mp_string& strValue)
{
    return 0;
}

mp_int32 StubGetValueString_fail(const mp_string& strParentSection, const mp_string& strChildSection, 
    const mp_string& strKey, mp_string& strValue)
{
    return -1;
}

mp_int32 StubGetValueStr_succ(const mp_string& strSection, const mp_string& strKey,
    mp_string& strValue)
{
    return 0;
}

mp_int32 StubGetValueStr_fail(const mp_string& strSection, const mp_string& strKey,
    mp_string& strValue)
{
    return -1;
}

mp_int32 StubGetDownloadLink_succ()
{
    return 0;
}

mp_int32 StubGetDownloadLink_fail()
{
    return -1;
}

mp_int32 StubSecurityConf_succ(HttpRequest& req)
{
    return 0;
}

mp_int32 StubSecurityConf_fail(HttpRequest& req)
{
    return -1;
}

mp_int32 StubInitReq_fail()
{
    return -1;
}

mp_int32 StubInitReq_succ()
{
    ModifyPluginHandle::m_vecPMIp.push_back("192.168.0.0");
    return 0;
}

mp_int32 StubSendReq_fail(IHttpClient* httpClient, const HttpRequest& req, mp_string& disposition)
{
    return -1;
}

mp_int32 StubSendReq_succ(IHttpClient* httpClient, const HttpRequest& req, mp_string& disposition)
{
    disposition = "DataProtect_client.zip_123456aaaa\"";
    return 0;
}

mp_int32 StubReadFileInfo_fail(mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    return -1;
}

mp_int32 StubReadFileInfo_fail1(mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    return 0;
}

mp_int32 StubReadFileInfo_fail2(mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    vecOutput.push_back("downloadLink=https://[192.168.69.203,192.168.69.202:25082/v1/host-agent/update/download?uuid=crmdownloadlinkee763a7020164ef29d08f60d91e20a33");
    return 0;
}

mp_int32 StubReadFileInfo_fail3(mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    vecOutput.push_back("downloadLink=https://[192.168.69.203,192.168.69.202]25082/v1/host-agent/update/download?uuid=crmdownloadlinkee763a7020164ef29d08f60d91e20a33");
    return 0;
}


mp_int32 StubReadFileInfo_succ(mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    vecOutput.push_back("downloadLink=https://[192.168.69.203,192.168.69.202]:25082/v1/host-agent/update/download?uuid=crmdownloadlinkee763a7020164ef29d08f60d91e20a33");
    return 0;
}

mp_string StubGetFile(const mp_string& strFileName)
{
    std::string parentPath = get_current_dir_name();
    return parentPath + "/" + "modify_signature.sign";
}

TEST_F(CModifyPluginHandleTest, ModifyPluginAgentHandleTest)
{
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    mp_void *param = NULL;
    ModifyPluginHandle modifyHandle;
    mp_void *iRet = NULL;

    stub.set(ADDR(ModifyPluginHandle, CheckBeforeModify), CheckBeforeModifyPluginFailed);
    iRet = modifyHandle.ModifyPluginAgentHandle(param);
    EXPECT_TRUE(iRet == NULL);

    stub.set(ADDR(ModifyPluginHandle, CheckBeforeModify), CheckBeforeModifyPluginSucc);
    stub.set(ADDR(ModifyPluginHandle, ObtainModifyPluginPac), ObtainModifyPluginPacFailed);
    iRet = modifyHandle.ModifyPluginAgentHandle(param);
    EXPECT_TRUE(iRet == NULL);

    stub.set(ADDR(ModifyPluginHandle, ObtainModifyPluginPac), ObtainModifyPluginPacSucc);
    stub.set(ADDR(ModifyPluginHandle, PrepareForModifyPlugin), PrepareForModifyPluginFailed);
    iRet = modifyHandle.ModifyPluginAgentHandle(param);
    EXPECT_TRUE(iRet == NULL);

    stub.set(ADDR(ModifyPluginHandle, PrepareForModifyPlugin), PrepareForModifyPluginSucc);
    stub.set(ADDR(CRootCaller, Exec), ExecFailed);
    iRet = modifyHandle.ModifyPluginAgentHandle(param);
    EXPECT_TRUE(iRet == NULL);

    stub.set(ADDR(CRootCaller, Exec), ExecSucc);
    iRet = modifyHandle.ModifyPluginAgentHandle(param);
    EXPECT_TRUE(iRet == NULL);
}

TEST_F(CModifyPluginHandleTest, CheckBeforeModifyPluginTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    ModifyPluginHandle modifyHandle;

    stub.set(ADDR(CRootCaller, Exec), ExecFailed);
    mp_int32 iRet = modifyHandle.CheckBeforeModify();
    
    stub.set(ADDR(CRootCaller, Exec), ExecSucc);
    iRet = modifyHandle.CheckBeforeModify();
}

/*
* Description  : 从下载点获取升级包
* 前置条件：存在记录升级信息的文件
* check点：1、记录升级信息的文件是否存在，2、获取记录升级信息是否成功
*/
TEST_F(CModifyPluginHandleTest, ObtainModifyPluginPacTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger,Log), LogReturn);
    ModifyPluginHandle modifyHandle;

    stub.set(ADDR(ModifyPluginHandle, InitRequest), StubInitReq_fail);
    mp_int32 iRet = modifyHandle.ObtainModifyPluginPac();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(ModifyPluginHandle, InitRequest), StubInitReq_succ);
    stub.set(ADDR(ModifyPluginHandle, SendRequest), StubSendReq_fail);
    iRet = modifyHandle.ObtainModifyPluginPac();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(ModifyPluginHandle, SendRequest), StubSendReq_succ);
    iRet = modifyHandle.ObtainModifyPluginPac();
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
* Description  : 准备开始升级校验
* 前置条件：存在包
* check点：1、校验包的完整性，2、能否解压包
*/
TEST_F(CModifyPluginHandleTest, PrepareForModifyPluginTest)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), LogReturn);
    ModifyPluginHandle modifyHandle;
    modifyHandle.m_signature = "vm3M9mwG/ddx+6sABqX+z+gTYeAxa1iFiGJwg3lb+K9dTupVqiyzYt7Z2nmQRjlmbkLB8/jpx/DeMLU/+CA2VTNqWET/"
                                "F4Uw9CRijUiMx0FfJ1vV1fM2tJrqgqfnrpiC6RxOHqqA6Cxi9HQLL8pQGuwYdavqDNHgKhCPLZQTnMMV9I9yt0T7qIwo"
                                "fb6Puiv79C35HM5dyxm547zWyzafynpqLATYIzv3yyXSLXrP6sccup3N1FZV5pqgOZjQeIxuqkXx2PG4rsdLcRwzQdJM"
                                "jyzLZeJK9bjW75vW8N8RxpuO1ckS+nxW+MsMePwwurK8T6cNma8oyyoX/LB74pz/zQWDU405YWPD6l29NyReSZ7pzXfi"
                                "q1JuqQO1a2225VTPiWpCv3Q1pf8oGxAzae481JsZpR/59dkwxOiv29uMyEh2E8tCVm6vQ7KjpUr3InudzQBJFWUgwi1R"
                                "RRzAyTPbaRNw2pSnG2IKkdM4NLXzJ/Bufo/w85mWedGs/QDNxKip";


    stub.set((mp_string(CPath::*)(mp_string))ADDR(CPath, GetStmpFilePath), StubGetFile);
    stub.set(ADDR(CRootCaller, Exec), ExecFailed);
    mp_int32 iRet = modifyHandle.PrepareForModifyPlugin();
    EXPECT_EQ(iRet, MP_FAILED);
    
    stub.set(ADDR(CRootCaller, Exec), ExecSucc);
    iRet = modifyHandle.PrepareForModifyPlugin();
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
* Description  : 升级流程之前检查主机资源
* 前置条件：可用空间大小满足条件
* check点：1、检查可用空间大小，2、检查升级是否成功
*/
TEST_F(CModifyPluginHandleTest, UpdateModifyPluginStatusTest)
{
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), LogReturn);
    mp_string strModifyPluginStatus;
    ModifyPluginHandle modifyHandle;
    mp_int32 iRet;

    stub.set(ADDR(CMpFile, CopyFileCoverDest), CopyFileCoverDestFailed);
    iRet = modifyHandle.UpdateModifyPluginStatus(strModifyPluginStatus);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CMpFile, CopyFileCoverDest), CopyFileCoverDestSucc);
    iRet = modifyHandle.UpdateModifyPluginStatus(strModifyPluginStatus);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
* Description  : 下载升级包，初始化请求
* 前置条件：链接信息获取成功
* check点：1、链接信息解析成功，2、证书信息读取成功
*/
TEST_F(CModifyPluginHandleTest, InitRequestTest)
{
    Stub stub;
    ModifyPluginHandle modifyHandle;
    HttpRequest req;
    stub.set(ADDR(ModifyPluginHandle, GetDownloadInfo), StubGetDownloadLink_fail);
    mp_int32 iRet = modifyHandle.InitRequestCommon();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(ModifyPluginHandle, GetDownloadInfo), StubGetDownloadLink_succ);
    ModifyPluginHandle::m_vecPMIp.clear();
    stub.set(ADDR(ModifyPluginHandle, SecurityConfiguration), StubSecurityConf_fail);
    iRet = modifyHandle.InitRequest(req);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(ModifyPluginHandle, SecurityConfiguration), StubSecurityConf_succ);
    ModifyPluginHandle::m_vecPMIp.push_back("192.168.0.0");
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubGetValueStr_fail);
    iRet = modifyHandle.InitRequest(req);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubGetValueStr_succ);
    iRet = modifyHandle.InitRequest(req);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
* Description  : 下载升级包，设置证书配置
* 前置条件： 证书信息读取成功
* check点：1、证书信息读取成功
*/
TEST_F(CModifyPluginHandleTest, SecurityConfigurationTest)
{
    Stub stub;
    ModifyPluginHandle modifyHandle;
    HttpRequest req;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubGetValueStr_fail);
    mp_int32 iRet = modifyHandle.SecurityConfiguration(req);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubGetValueStr_succ);
    iRet = modifyHandle.SecurityConfiguration(req);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

/*
* Description  : 下载升级包，解析链接信息
* 前置条件： 升级信息文件存在
* check点：链接信息解析成功
*/
TEST_F(CModifyPluginHandleTest, GetDownloadInfo)
{
    Stub stub;
    ModifyPluginHandle modifyHandle;
    stub.set(ADDR(CIPCFile, ReadFile), StubReadFileInfo_fail);
    mp_int32 iRet = modifyHandle.GetDownloadInfo();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CIPCFile, ReadFile), StubReadFileInfo_fail1);
    iRet = modifyHandle.GetDownloadInfo();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CIPCFile, ReadFile), StubReadFileInfo_fail2);
    iRet = modifyHandle.GetDownloadInfo();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CIPCFile, ReadFile), StubReadFileInfo_fail3);
    iRet = modifyHandle.GetDownloadInfo();
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CIPCFile, ReadFile), StubReadFileInfo_succ);
    iRet =  modifyHandle.GetDownloadInfo();
    EXPECT_EQ(iRet, MP_SUCCESS);
}