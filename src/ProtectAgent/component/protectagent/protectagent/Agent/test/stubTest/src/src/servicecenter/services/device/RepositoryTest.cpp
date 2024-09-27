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
#include "servicecenter/services/device/RepositoryTest.h"
#include "servicecenter/services/device/PrepareFileSystem.h"
#include "servicecenter/services/device/RepositoryFactory.h"
#include "servicecenter/services/device/CacheRepository.h"
#include "servicecenter/services/device/LogRepository.h"
#include "servicecenter/services/device/Repository.h"
#include "securecom/RootCaller.h"
#include "securecom/CryptAlg.h"
#include "host/host.h"
#include "common/Log.h"
#include "common/File.h"
#include "common/Ip.h"
#include "common/Utils.h"
#include "common/ConfigXmlParse.h"
#include "taskmanager/externaljob/Job.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"
#include "taskmanager/externaljob/PluginAnyTimeRestore.h"
#include <vector>

using namespace std;
using namespace AppProtect;

namespace {
static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_int32 StubSuccess(mp_void* pthis)
{
    return MP_SUCCESS;
}

mp_int32 StubFailed(mp_void* pthis)
{
    return MP_FAILED;
}

mp_bool StubTrue(mp_void* pthis)
{
    return MP_TRUE;
}
mp_void StubReturnVoid(mp_void* pthis)
{
    return;
}

mp_string StubGetStartUserEmpty(const mp_string &pluginName)
{
    return "";
}

mp_string StubGetStartUser(const mp_string &pluginName)
{
    return "test";
}

mp_int32 StubGetMountIPTestSuccess(const StorageRepository& stRep, MountNasParam& param)
{
    return MP_SUCCESS;
}

mp_int32 StubGetMountIPTestFail(const StorageRepository& stRep, MountNasParam& param)
{
    return MP_FAILED;
}

mp_int32 StubMountNasFileSystemSuccess(void *obj, MountNasParam &mountNasParam, vector<mp_string> &successMountPath, 
    std::set<mp_string> &availStorageIp, bool multiFileSystem, const MountPermission &permit)
{
    successMountPath.push_back("192.168.1.1");
    return MP_SUCCESS;
}

mp_int32 StubMountNasFileSystemFail(MountNasParam &mountNasParam, vector<mp_string> &successMountPath, 
    std::set<mp_string> &availStorageIp, bool multiFileSystem, const MountPermission &permit)
{
    return MP_FAILED;
}

mp_int32 StubAssembleCacheRepositorySuccess(const AppProtect::PluginJobData &data, StorageRepository &stRep, Json::Value &JsonRep_new)
{
    return MP_SUCCESS;
}

mp_int32 StubAssembleCacheRepositoryFail(const AppProtect::PluginJobData &data, StorageRepository &stRep, Json::Value &JsonRep_new)
{
    return MP_FAILED;
}

mp_int32 StubUmountNasFileSystem(const vector<mp_string> &successMountPath)
{
    return MP_SUCCESS;
}

mp_int32 StubCreateJobDirTestSuccess(const AppProtect::PluginJobData &data)
{
    return MP_SUCCESS;
}

mp_int32 SStubCreateJobDirTestFail(const AppProtect::PluginJobData &data)
{
    return MP_FAILED;
}

mp_bool StubDirExist(const mp_char* pszDirPath)
{
    return MP_FALSE;
}

mp_int32 StubCreateDirTestSuccess(const mp_char* pszDirPath)
{
    return MP_SUCCESS;
}

mp_int32 StubCreateDirTestFail(const mp_char* pszDirPath)
{
    return MP_FAILED;
}

mp_int32 StubCheckIsDoradoEnvironmentFalse(mp_bool& isDorado)
{
    isDorado = MP_FALSE;
    return MP_SUCCESS;
}

mp_bool StubIsDataturboMountTestSuccess(PluginJobData &data)
{
    return MP_TRUE;
}

mp_bool StubIsDataturboMountTestFail(PluginJobData &data)
{
    return MP_FALSE;
}

std::vector<mp_string> StubGetInnerHostIps(mp_void* pThis)
{
    std::vector<mp_string> innerHostIps;
    innerHostIps.push_back("192.168.1.1");
    innerHostIps.push_back("192.168.1.2");
    innerHostIps.push_back("192.168.1.3");
    innerHostIps.push_back("192.168.1.4");
    return innerHostIps;
}

mp_int32 StubGetHostSN(mp_string &hostId)
{
    hostId="123456";
    return MP_SUCCESS;
}

mp_string StubGetLunInfo(const Json::Value &lunInfo, MountNasParam &param, const mp_string &resourceId)
{
    return "anystring";
}

mp_bool StubCheckMountedFalse(PluginJobData &data, const mp_string &repositoryType)
{
    return MP_FALSE;
}


}
/*
* 用例名称：创建仓库
* 前置条件：1、无
* check点：1、cache仓库创建成功,指针不为空
*/
TEST_F(RepositoryTest, CreateRepositoryTest)
{
    RepositoryDataType::type repositoryType = RepositoryDataType::type::CACHE_REPOSITORY;
    std::shared_ptr<RepositoryFactory> m_repositoryFactory;
    std::shared_ptr<Repository> pRepository = m_repositoryFactory->CreateRepository(repositoryType);
    EXPECT_NE(pRepository, nullptr);

    repositoryType = RepositoryDataType::type::DATA_REPOSITORY;
    pRepository = m_repositoryFactory->CreateRepository(repositoryType);
    EXPECT_NE(pRepository, nullptr);
}

/*
* 用例名称：挂载文件系统
* 前置条件：1、无
* check点：1、挂载文件系统成功，检查返回值
           2、挂载文件系统失败，检查返回值
*/
TEST_F(RepositoryTest, MountRepositoryTest)
{
    StorageRepository stRep;
    HostAddress hostAddressJson;
    hostAddressJson.ip = "192.168.1.2";
    hostAddressJson.port = 0;
    stRep.remoteHost.push_back(hostAddressJson);
    stRep.repositoryType = RepositoryDataType::type::CACHE_REPOSITORY;
    stRep.remotePath = "filesystem";
    MountPermission permit;

    AppProtect::PluginJobData jobData = {"pluginName", "mainID", "subID", Json::Value(), AppProtect::MainJobType::BACKUP_JOB};
    Json::Value JsonRep_new;

    stub.set(ADDR(Repository, GetMountIP), StubGetMountIPTestSuccess);
    stub.set(ADDR(PrepareFileSystem, MountNasFileSystem), StubMountNasFileSystemSuccess);
    stub.set(ADDR(CIP, CheckIsDoradoEnvironment), StubSuccess);
    stub.set(ADDR(Repository, IsDataturboMount), StubIsDataturboMountTestFail);

    RepositoryDataType::type repositoryType = RepositoryDataType::type::CACHE_REPOSITORY;
    std::shared_ptr<RepositoryFactory> repositoryFactory;
    std::shared_ptr<Repository> pRepository = repositoryFactory->CreateRepository(repositoryType);
//    EXPECT_EQ(MP_SUCCESS, pRepository->Mount(jobData, stRep, JsonRep_new));

    repositoryType = RepositoryDataType::type::DATA_REPOSITORY;
    pRepository = repositoryFactory->CreateRepository(repositoryType);
//    EXPECT_EQ(MP_SUCCESS, pRepository->Mount(jobData, stRep, JsonRep_new));

    stub.set(ADDR(Repository, GetMountIP), StubGetMountIPTestFail);
    repositoryType = RepositoryDataType::type::CACHE_REPOSITORY;
    pRepository = repositoryFactory->CreateRepository(repositoryType);
    EXPECT_EQ(MP_FAILED, pRepository->Mount(jobData, stRep, JsonRep_new, permit));

    stub.set(ADDR(Repository, GetMountIP), StubGetMountIPTestSuccess);
    stub.set(ADDR(PrepareFileSystem, MountNasFileSystem), StubMountNasFileSystemFail);
    repositoryType = RepositoryDataType::type::CACHE_REPOSITORY;
    pRepository = repositoryFactory->CreateRepository(repositoryType);
    EXPECT_EQ(MP_FAILED, pRepository->Mount(jobData, stRep, JsonRep_new, permit));

    stub.set(ADDR(Repository, IsDataturboMount), StubIsDataturboMountTestSuccess);
    stub.set(ADDR(PrepareFileSystem, MountDataturboFileSystem), StubMountNasFileSystemFail);
    stub.set(ADDR(PrepareFileSystem, MountNasFileSystem), StubMountNasFileSystemSuccess);
    repositoryType = RepositoryDataType::type::DATA_REPOSITORY;
    pRepository = repositoryFactory->CreateRepository(repositoryType);
    EXPECT_EQ(MP_FAILED, pRepository->Mount(jobData, stRep, JsonRep_new, permit));

    stub.set(ADDR(Repository, IsDataturboMount), StubIsDataturboMountTestSuccess);
    stub.set(ADDR(PrepareFileSystem, MountDataturboFileSystem), StubMountNasFileSystemFail);
    stub.set(ADDR(PrepareFileSystem, MountNasFileSystem), StubMountNasFileSystemFail);
    repositoryType = RepositoryDataType::type::DATA_REPOSITORY;
    pRepository = repositoryFactory->CreateRepository(repositoryType);
    EXPECT_EQ(MP_FAILED, pRepository->Mount(jobData, stRep, JsonRep_new, permit));

    Json::Value extendInfo;
    extendInfo["sanclientInvolved"] = "false";
    jobData.param["extendInfo"] = extendInfo;
    stub.set(ADDR(Repository, MountFileIoSystem), StubFailed);
    repositoryType = RepositoryDataType::type::DATA_REPOSITORY;
    pRepository = repositoryFactory->CreateRepository(repositoryType);
    EXPECT_EQ(MP_FAILED, pRepository->Mount(jobData, stRep, JsonRep_new, permit));

    extendInfo["sanclientInvolved"] = "true";
    jobData.param["extendInfo"] = extendInfo;
    stub.set(ADDR(Repository, MountFileIoSystem), StubSuccess);
    repositoryType = RepositoryDataType::type::DATA_REPOSITORY;
    pRepository = repositoryFactory->CreateRepository(repositoryType);
    EXPECT_EQ(MP_FAILED, pRepository->Mount(jobData, stRep, JsonRep_new, permit));

}



/*
* 用例名称：获得挂载Sanclient参数
* 前置条件：1、无
* check点：1、获取成功，检查返回值
           2、获取失败，检查返回值
*/
TEST_F(RepositoryTest, GetSanclientParamTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    MountNasParam param;
    PluginJobData data;
    Json::Value lunInfo;
    lunInfo["lunName"] = "0";
    lunInfo["lunId"] = "4";
    lunInfo["sanclientWwpn"] = "C050760BAFC600C1";
    lunInfo["path"] = "/any/test";
    Json::Value sanClient;
    sanClient["luninfo"].append(lunInfo);
    Json::Value agent;
    agent["sanClients"].append(sanClient);
    agent["id"] = "123456";
    data.param["agents"].append(agent);

    Json::Value appInfoJsonValue;
    appInfoJsonValue["uuid"] =  "9876a21";
    data.param["appInfo"] = appInfoJsonValue;

    stub.set(ADDR(CHost, GetHostSN), StubGetHostSN);
    stub.set(ADDR(Repository, GetMountLunInfo), StubGetLunInfo);
    Repository pRepository;
    mp_int32 iRet = pRepository.GetSanclientParam(param, data);
    EXPECT_EQ(MP_FAILED, iRet);
}

mp_void StubDecryptStr(const mp_string& inStr, mp_string& outStr)
{
    outStr = "test";
    return;
}

TEST_F(RepositoryTest, GetMountLunInfoTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);

    Json::Value lunS;
    Json::Value sanclientS;
    Json::Value agentS;
    lunS["lunName"] = "1";
    lunS["lunId"] = "1";
    lunS["sanclientWwpn"] = "123456789";
    lunS["agentWwpn"] = "anyalkds";
    lunS["path"] = "anypath";
    lunS["filesystemsize"] = 1024;
    sanclientS["luninfo"].append(lunS);
    agentS["sanClients"].append(sanclientS);

    MountNasParam param;
    param.repositoryType = "data";
    mp_string rsId = "12345";

    Repository pRepository;
    mp_string res = pRepository.GetMountLunInfo(agentS, param, rsId);
    EXPECT_EQ(res,"12345:123456789,/1,data,1023,anyalkds,,,//");
 
    stub.set(DecryptStr, StubDecryptStr);
    Json::Value lunS1;
    Json::Value sanclientS1;
    Json::Value agentS1;
    lunS1["lunName"] = "1";
    lunS1["lunId"] = "1";
    lunS1["sanclientWwpn"] = "123456789";
    lunS1["agentWwpn"] = "anyalkds";
    lunS1["path"] = "anypath";
    lunS1["filesystemsize"] = 1024;
    lunS1["UnidirectionalAuthPwd"] = "tttt";
    sanclientS1["luninfo"].append(lunS1);
    sanclientS1["iqns"].append("iqnstest");
    sanclientS1["iqns"].append("iqnstest1");
    agentS1["sanClients"].append(sanclientS1);
 
    mp_string res1 = pRepository.GetMountLunInfo(agentS1, param, rsId);
    EXPECT_EQ(res1,"12345:123456789,test/1,data,1023,anyalkds,,,//");
 
    mp_string res2 = pRepository.GetMountLunInfo(agentS, param, rsId);
    EXPECT_EQ(res2,"12345:123456789,/1,data,1023,anyalkds,,,//");
}



/*
* 用例名称：挂载sanclient文件系统
* 前置条件：1、无
* check点：1、挂载文件系统成功，检查返回值
           2、挂载文件系统失败，检查返回值
*/
TEST_F(RepositoryTest, MountFileIoRepositoryTest)
{
    StorageRepository stRep;
    HostAddress hostAddressJson;
    hostAddressJson.ip = "192.168.1.2";
    hostAddressJson.port = 0;
    stRep.remoteHost.push_back(hostAddressJson);
    stRep.repositoryType = RepositoryDataType::type::DATA_REPOSITORY;
    stRep.remotePath = "filesystem";
    std::vector<mp_string> vecStorageIp;
    std::vector<mp_string> vecDataturboIP;
    mp_string storageFs = "/202101041911";
    vecStorageIp.push_back("8.40.99.244");
    vecStorageIp.push_back("8.40.99.245");
    MountNasParam mountNasParam = {"11111", "data", vecStorageIp, vecDataturboIP, storageFs, "vmware_type"};
    mountNasParam.lunInfo = "a4813:21000024ff2f4190/150,meta,32767//";
    AppProtect::PluginJobData jobData = {"pluginName", "mainID", "subID", Json::Value(), AppProtect::MainJobType::BACKUP_JOB};
    Json::Value JsonRep_new;
    RepositoryDataType::type repositoryType = RepositoryDataType::type::DATA_REPOSITORY;

    std::shared_ptr<RepositoryFactory> repositoryFactory;
    std::shared_ptr<Repository> pRepository = repositoryFactory->CreateRepository(repositoryType);
    stub.set(ClearString, StubReturnVoid);
    stub.set(ADDR(Repository, CheckSanclientMounted), StubCheckMountedFalse);
    stub.set(ADDR(Repository, GetSanclientParam), StubFailed);
    EXPECT_EQ(MP_FAILED, pRepository->MountFileIoSystem(mountNasParam, jobData, stRep, JsonRep_new));

    stub.set(ADDR(Repository, CheckSanclientMounted), StubCheckMountedFalse);
    stub.set(ADDR(Repository, GetSanclientParam), StubSuccess);
    stub.set(ADDR(Repository, RecordSanclientMounted), StubSuccess);
    stub.set(&PrepareFileSystem::MountFileIoSystem, StubFailed);
    EXPECT_EQ(MP_FAILED, pRepository->MountFileIoSystem(mountNasParam, jobData, stRep, JsonRep_new));

    stub.set(ADDR(Repository, CheckSanclientMounted), StubCheckMountedFalse);
    stub.set(ADDR(Repository, GetSanclientParam), StubSuccess);
    stub.set(ADDR(Repository, RecordSanclientMounted), StubSuccess);
    stub.set(&PrepareFileSystem::MountFileIoSystem, StubSuccess);
    EXPECT_EQ(MP_SUCCESS, pRepository->MountFileIoSystem(mountNasParam, jobData, stRep, JsonRep_new));
}


/*
* 用例名称：挂载文件系统
* 前置条件：无
* check点：1.挂载文件系统成功 2.挂载文件系统失败
*/
TEST_F(RepositoryTest, RepositoryMount)
{
    PluginJobData data;
    StorageRepository stRep;
    Json::Value jsonRep_new;
    MountPermission permit;
    stRep.repositoryType = RepositoryDataType::type::DATA_REPOSITORY;

    std::shared_ptr<RepositoryFactory> repositoryFactory;
    RepositoryDataType::type repositoryType = RepositoryDataType::type::DATA_REPOSITORY;
    std::shared_ptr<Repository> pRepository = repositoryFactory->CreateRepository(repositoryType);

    stub.set(ClearString, StubReturnVoid);

    stub.set(ADDR(Repository, GetMountNasParam), StubSuccess);
    stub.set(ADDR(Repository, IsDataturboMount), StubTrue);
    stub.set(ADDR(PrepareFileSystem, MountDataturboFileSystem), StubFailed);
    stub.set(ADDR(PrepareFileSystem, MountNasFileSystem), StubFailed);
    repositoryType = RepositoryDataType::type::DATA_REPOSITORY;
    pRepository = repositoryFactory->CreateRepository(repositoryType);
    EXPECT_EQ(MP_FAILED, pRepository->Mount(data, stRep, jsonRep_new, permit));
}

/*
* 用例名称：卸载文件系统
* 前置条件：1、无
* check点：1、卸载文件系统成功
*/
TEST_F(RepositoryTest, UmountRepositoryTest)
{
    std::vector<mp_string> mountPoints;
    mountPoints.push_back("192.168.1.1");
    mp_string jobID = "123";
    stub.set(ADDR(PrepareFileSystem, UmountNasFileSystem), StubUmountNasFileSystem);
    RepositoryDataType::type repositoryType = RepositoryDataType::type::CACHE_REPOSITORY;
    std::shared_ptr<RepositoryFactory> repositoryFactory;
    std::shared_ptr<Repository> pRepository = repositoryFactory->CreateRepository(repositoryType);

    repositoryType = RepositoryDataType::type::DATA_REPOSITORY;
    pRepository = repositoryFactory->CreateRepository(repositoryType);
    EXPECT_EQ(1, mountPoints.size());

    stub.set(ADDR(PrepareFileSystem, UmountNasFileSystem), StubFailed);
    repositoryType = RepositoryDataType::type::DATA_REPOSITORY;
    pRepository = repositoryFactory->CreateRepository(repositoryType);
    EXPECT_EQ(MP_FAILED, pRepository->Umount(mountPoints, jobID));
}

/*
* 用例名称：获取挂载IP
* 前置条件：1、无
* check点：1、获取成功，检查返回值
           2、获取失败，检查返回值
*/
TEST_F(RepositoryTest, GetMountIPTest)
{
    StorageRepository stRep;
    MountNasParam param;
    stub.set(ADDR(CIP, CheckIsDoradoEnvironment), StubSuccess);
    stub.set(ADDR(AppProtectJobHandler, GetContainerBackendIps), StubGetInnerHostIps);
    Repository pRepository;
    mp_int32 ret = pRepository.GetMountIP(stRep, param);
    EXPECT_EQ(ret, MP_SUCCESS);
    EXPECT_EQ(param.vecStorageIp.size(), 4);

    stub.set(ADDR(CIP, CheckIsDoradoEnvironment), StubFailed);
    ret = pRepository.GetMountIP(stRep, param);
    EXPECT_EQ(ret, MP_FAILED);

    HostAddress host;
    HostAddress host1;
    host.supportProtocol = 1024;
    stRep.remoteHost.push_back(host);
    stRep.remoteHost.push_back(host1);
    stub.set(ADDR(CIP, CheckIsDoradoEnvironment), StubSuccess);
    ret = pRepository.GetMountIP(stRep, param);
    EXPECT_EQ(ret, MP_SUCCESS);
}

/*
* 用例名称：获得挂载Nas参数
* 前置条件：1、无
* check点：1、获取成功，检查返回值
           2、获取失败，检查返回值
*/
TEST_F(RepositoryTest, GetMountNasParamTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    MountNasParam param;
    PluginJobData data;
    StorageRepository stRep;
    MountPermission permit;
    Repository pRepository;
    mp_int32 iRet;

    stub.set(ADDR(Repository, GetMountIP), StubSuccess);
    stub.set(ADDR(ExternalPluginManager, GetPluginNameByAppType), StubSuccess);
    stub.set(ADDR(ExternalPluginManager, GetStartUser), StubGetStartUserEmpty);
    iRet = pRepository.GetMountNasParam(param, data, stRep, permit);
    EXPECT_EQ(MP_FAILED, iRet);
}

TEST_F(RepositoryTest, CacheRepository_Umount)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CacheRepository pRepository;
    std::vector<mp_string> mountPoints;
    mountPoints.push_back("path");
    mp_string jobID;

    stub.set(ADDR(PrepareFileSystem, UmountNasFileSystem), StubFailed);
    mp_int32 iRet = pRepository.Umount(mountPoints, jobID);
    EXPECT_EQ(MP_FAILED, iRet);

    mountPoints.clear();
    stub.set(ADDR(PrepareFileSystem, UmountNasFileSystem), StubSuccess);
    iRet = pRepository.Umount(mountPoints, jobID);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(RepositoryTest, CacheRepository_QueryRepoSubPath)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CacheRepository pRepository;
    std::vector<mp_string> path;
    PluginJobData data;

    pRepository.QueryRepoSubPath(data, path);
}

TEST_F(RepositoryTest, CacheRepository_AssembleRepository)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    CacheRepository pRepository;
    PluginJobData data;
    StorageRepository stRep;
    Json::Value jsonRep_new;
    pRepository.m_mountPoint.clear();
    pRepository.m_mountPoint.push_back("test");

    pRepository.AssembleRepository(data, stRep, jsonRep_new);
}

TEST_F(RepositoryTest, AssembleRemoteHost)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    Repository pRepository;
    std::set<mp_string> availStorageIp;
    availStorageIp.insert("192.168.1.1");
    StorageRepository stRep;
    HostAddress stHost;
    stHost.ip = "192.168.1.2";
    stRep.remoteHost.push_back(stHost);

    stub.set(ADDR(CIP, CheckIsDoradoEnvironment), StubFailed);
    EXPECT_EQ(MP_FAILED, pRepository.AssembleRemoteHost(availStorageIp, stRep));

    stub.set(ADDR(CIP, CheckIsDoradoEnvironment), StubSuccess);
    EXPECT_EQ(MP_SUCCESS, pRepository.AssembleRemoteHost(availStorageIp, stRep));

    stub.set(ADDR(CIP, CheckIsDoradoEnvironment), StubCheckIsDoradoEnvironmentFalse);
    EXPECT_EQ(MP_SUCCESS, pRepository.AssembleRemoteHost(availStorageIp, stRep));
}

TEST_F(RepositoryTest, LogRepository_QueryRepoSubPath)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    LogRepository pRepository;
    PluginJobData data;
    std::vector<mp_string> path;

    pRepository.QueryRepoSubPath(data, path);

    data.mainType = MainJobType::BACKUP_JOB;
    pRepository.QueryRepoSubPath(data, path);
}

TEST_F(RepositoryTest, LogRepository_AssembleRepository)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet;
    LogRepository pRepository;
    PluginJobData data;
    StorageRepository stRep;
    Json::Value jsonRep_new;
    pRepository.m_mountPoint.push_back("test");

    data.mainType = MainJobType::RESTORE_JOB;
    stub.set(&PluginAnyTimeRestore::ComputerAnyTimeRestoreLogPath, StubFailed);
    iRet = pRepository.AssembleRepository(data, stRep, jsonRep_new);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(&PluginAnyTimeRestore::ComputerAnyTimeRestoreLogPath, StubSuccess);
    iRet = pRepository.AssembleRepository(data, stRep, jsonRep_new);
    EXPECT_EQ(MP_SUCCESS, iRet);

    data.mainType = MainJobType::BACKUP_JOB;
    iRet = pRepository.AssembleRepository(data, stRep, jsonRep_new);
    EXPECT_EQ(MP_SUCCESS, iRet);

    data.mainType = MainJobType::LIVEMOUNT_JOB;
    iRet = pRepository.AssembleRepository(data, stRep, jsonRep_new);
    EXPECT_EQ(MP_SUCCESS, iRet);

}

mp_int32 StubCConfigXmlParserGetValueStringHostAndStorageMap(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    strValue = "192.168.123.123@1234sxcvfgs;192.168.123.124@1234xxxxxx";
    return MP_SUCCESS;
}

mp_int32 StubCConfigXmlParserGetValueStringHostAndStorageMap1(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    strValue = "";
    return MP_SUCCESS;
}

mp_int32 StubCConfigXmlParserGetValueStringHostAndStorageMap2(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    strValue = "";
    return MP_FAILED;
}

TEST_F(RepositoryTest, GetHostAndStorageMapTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    Repository pRepository;
    std::map<mp_string, mp_string> ipAndEsnMap;

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubCConfigXmlParserGetValueStringHostAndStorageMap);
    EXPECT_EQ(MP_SUCCESS, pRepository.GetHostAndStorageMap(ipAndEsnMap));

    ipAndEsnMap.clear();
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubCConfigXmlParserGetValueStringHostAndStorageMap1);
    EXPECT_EQ(MP_FAILED, pRepository.GetHostAndStorageMap(ipAndEsnMap));

    ipAndEsnMap.clear();
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubCConfigXmlParserGetValueStringHostAndStorageMap2);
    EXPECT_EQ(MP_FAILED, pRepository.GetHostAndStorageMap(ipAndEsnMap)); 

    stub.reset((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString));
}


mp_int32 StubGetHostIPListSuccess(std::vector<mp_string>& ipv4List, std::vector<mp_string>& ipv6List)
{
    ipv4List.push_back("192.168.123.123");
    ipv6List.push_back("fe80::c63f:7884:eb7b:9679");
    return MP_SUCCESS;
}

mp_int32 StubGetHostIPListSuccess1(std::vector<mp_string>& ipv4List, std::vector<mp_string>& ipv6List)
{
    ipv4List.push_back("192.168.123.127");
    ipv6List.push_back("fe80::c63f:7884:eb7b:9679");
    return MP_SUCCESS;
}

TEST_F(RepositoryTest, ClusterSelectStorageTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    Repository pRepository;
    mp_string esn = "123456";
    mp_string subType = "test";

    EXPECT_EQ(MP_TRUE, pRepository.ClusterSelectStorage(esn, subType));

    subType = "DWS-Databases";
    esn = "1234sxcvfgs";
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubCConfigXmlParserGetValueStringHostAndStorageMap);
    stub.set(&CIP::GetHostIPList, StubGetHostIPListSuccess);
    EXPECT_EQ(MP_TRUE, pRepository.ClusterSelectStorage(esn, subType));

    esn = "xxxxs";
    EXPECT_EQ(MP_FALSE, pRepository.ClusterSelectStorage(esn, subType));

    esn = "1234sxcvfgs";
    stub.set(&CIP::GetHostIPList, StubGetHostIPListSuccess1);
    EXPECT_EQ(MP_FALSE, pRepository.ClusterSelectStorage(esn, subType));
    stub.reset((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString));
    stub.reset(&CIP::GetHostIPList);
}