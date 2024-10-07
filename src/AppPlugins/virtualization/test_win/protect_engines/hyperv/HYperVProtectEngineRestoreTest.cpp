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
#include "addr_pri.h"
#include "protect_engines/hyperv/HyperVProtectEngine.h"
#include <common/PluginTypes.h>
#include <protect_engines/engine_factory/EngineFactory.h>

bool g_isSuccess = true;
int g_count = 0;

class HyperVProtectEngineRestoreTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    // 恢复Handler
    std::shared_ptr<HyperVProtectEngine> m_HyperVRestoreProtectEngineHandler;
};

static void FormCopyData(AppProtect::RestoreJob &restoreInfo, bool isError)
{
    AppProtect::Copy copy;
    copy.id = "001";
    // 数据仓
    StorageRepository dataRepo;
    dataRepo.repositoryType = RepositoryDataType::DATA_REPOSITORY;
    dataRepo.path.push_back(isError ? "/datapath1" : "C:");
    copy.repositories.push_back(dataRepo);
    // 元数据仓
    StorageRepository metaRepo;
    metaRepo.repositoryType = RepositoryDataType::META_REPOSITORY;
    metaRepo.path.push_back(isError ? "/metapath1" : "C:");
    copy.repositories.push_back(metaRepo);
    // 缓存仓
    StorageRepository cacheRepo;
    cacheRepo.repositoryType = RepositoryDataType::CACHE_REPOSITORY;
    cacheRepo.path.push_back(isError ? "/cachepath1" : "C:");
    copy.repositories.push_back(cacheRepo);
    restoreInfo.copies.push_back(copy);
}

static void StubSetRestoreInfo(AppProtect::RestoreJob &restoreInfo)
{
    std::string t_volExtendInfo = "{\"targetVolumes\":\"[{\\\"uuid\\\":\\\"53f52bff-8ffa-46ae-98b4-a1f20aa99a7a\\\"},\
    {\\\"uuid\\\":\\\"2856e31e-242a-42e7-a992-713cd5bdc691\\\"}]\"}";
    restoreInfo.requestId = "123";
    restoreInfo.jobId = "123";
	restoreInfo.extendInfo = "{\"restoreLevel\":0}";
    restoreInfo.targetEnv.__set_id("136");
    restoreInfo.targetEnv.__set_name("HcsPlanet");
    restoreInfo.targetEnv.__set_type("Virtual");
    restoreInfo.targetEnv.__set_endpoint("demo.com");
    restoreInfo.targetEnv.auth.__set_authkey("bss_admin");
    restoreInfo.targetEnv.auth.__set_authPwd("xxxxxxxx");
    restoreInfo.targetEnv.auth.__set_extendInfo(
        "{\"certification\":\"cert\",\"enableCert\":\"1\",\"revocationlist\":\"\"}");
    restoreInfo.targetEnv.__set_extendInfo(
        "{\"projectId\":\"e38d227edcce4631be20bfa5aad7130b\",\"regionId\":\"sc-cdHyperVPlugin::FAILED\"}");
    // 卷参数
    ApplicationResource restoreVol;
    restoreVol.type = "volume";
    restoreVol.id = "1HyperVPlugin::FAILEDHyperVPlugin::FAILEDHyperVPlugin::FAILED"; // 源卷
    restoreVol.__set_extendInfo(t_volExtendInfo);                                    // 目标卷信息
    restoreInfo.restoreSubObjects = { restoreVol };
    restoreInfo.targetObject.__set_id("10a4e361-c981-46f2-b9ba-d7ff9c601693");
    restoreInfo.targetObject.__set_extendInfo("{\"projectId\":\"projectId\",\"domainName\":\"domains\"}");
}

bool Stub_JsonStringToStruct(const std::string &jsonString, VolumeInfo &t)
{
    return true;
}

bool Stub_JsonStringToJsonValue(const std::string &jsonString, Json::Value &value)
{
    return g_isSuccess;
}

int Stub_LoadFileToStructWithRetry(std::shared_ptr<RepositoryHandler> repoHandler, const std::string &file, VMInfo &t)
{
    t.m_uuid = "46AA60A1-400E-4C83-8F71-FCC5D6334BCB";
    return HyperVPlugin::SUCCESS;
}

int32_t Stub_GetStringValueByKeyFromJsonString(const std::string &bodyStr, const std::string &key, std::string &result)
{
	result = "1";
    return g_isSuccess ? HyperVPlugin::SUCCESS : HyperVPlugin::FAILED;
}

int32_t Stub_GetStringValueByKeyFromJsonString_GenVolPair(const std::string &bodyStr, const std::string &key, std::string &result)
{
	result = bodyStr;
	return g_isSuccess ? HyperVPlugin::SUCCESS : HyperVPlugin::FAILED;
}

DWORD Stub_GetFileAttributesA(LPCSTR lpFileName)
{
	return 0;
}

DWORD Stub_GetLastError()
{
	return g_isSuccess ? ERROR_FILE_EXISTS : 0 ;
}

int Stub_CloseHandle(HANDLE hObject)
{
	return 0;
}

HANDLE Stub_CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	return g_count++ > 0 ? (void *) + 2 : INVALID_HANDLE_VALUE;
}

bool Stub_InitJobPara()
{
    return g_isSuccess;
}

int32_t Stub_InitWmiClient()
{
    return HyperVPlugin::FAILED;
}

int32_t Stub_AttachDiskToVM(const HyperVPlugin::VolumeInfo &diskInfo, const std::string &vmId)
{
    return g_isSuccess ? HyperVPlugin::SUCCESS : HyperVPlugin::FAILED;
}

int32_t Stub_CreateVM(const VmModifyParam &modifyParam, std::string &createdVmId, std::string &errorDetails)
{
    return g_isSuccess ? HyperVPlugin::SUCCESS : HyperVPlugin::FAILED;
}

int32_t Stub_DeleteVM(const std::string &vmId)
{
    return g_isSuccess ? HyperVPlugin::SUCCESS : HyperVPlugin::FAILED;
}

int32_t Stub_DetachDiskFromVM(const std::string &vmId, const std::vector<VolumeExtendInfo> &diskInfos)
{
    return g_isSuccess ? HyperVPlugin::SUCCESS : HyperVPlugin::FAILED;
}

int32_t Stub_ChangeVMPowerState(const std::string &vmId, bool powerOff)
{
	return g_isSuccess ? HyperVPlugin::SUCCESS : HyperVPlugin::FAILED;
}

// ACCESS_PRIVATE_FIELD(HyperVProtectEngine, std::shared_ptr<AppProtect::RestoreJob>, m_restorePara);
ACCESS_PRIVATE_FUN(HyperVProtectEngine, bool(), InitJobPara);
ACCESS_PRIVATE_FUN(HyperVProtectEngine, int32_t(), InitWmiClient);

void HyperVProtectEngineRestoreTest::SetUp()
{
	if (m_HyperVRestoreProtectEngineHandler == nullptr) {
		AppProtect::RestoreJob restoreJob;
		StubSetRestoreInfo(restoreJob);
		FormCopyData(restoreJob, false);
		std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(restoreJob);
		std::shared_ptr<JobCommonInfo> m_restoreJobInfo = std::make_shared<JobCommonInfo>();
		m_restoreJobInfo->SetJobInfo(data);
		std::shared_ptr<VirtPlugin::JobHandle> m_restoreJobHandle =
			std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, m_restoreJobInfo);
		m_HyperVRestoreProtectEngineHandler = std::make_shared<HyperVProtectEngine>(m_restoreJobHandle);
	}
}

void HyperVProtectEngineRestoreTest::TearDown() {}

void HyperVProtectEngineRestoreTest::SetUpTestCase() {}

void HyperVProtectEngineRestoreTest::TearDownTestCase() {}

/*
 * 测试用例： 挂载卷
 * 前置条件： 初始化失败
 * CHECK点： 挂载卷失败
 */
TEST_F(HyperVProtectEngineRestoreTest, AttachVolume_Init_Failed)
{
    Stub stub;
	g_isSuccess = false;
    auto F_Init = get_private_fun::HyperVProtectEngineInitJobPara();
    stub.set(F_Init, Stub_InitJobPara);
    int32_t retValue = HyperVPlugin::SUCCESS;
    VolInfo volInfo;
    retValue = m_HyperVRestoreProtectEngineHandler->AttachVolume(volInfo);
    EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 挂载卷
 * 前置条件： m_jobHandle GetStorageRepos 为空, GetRepoPathByType失败
 * CHECK点： 挂载卷失败
 */
TEST_F(HyperVProtectEngineRestoreTest, AttachVolume_GetRepoPathByType_Failed)
{
    AppProtect::RestoreJob restoreJob;
    StubSetRestoreInfo(restoreJob);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(restoreJob);
    std::shared_ptr<JobCommonInfo> m_restoreJobInfo = std::make_shared<JobCommonInfo>(data);
    std::shared_ptr<VirtPlugin::JobHandle> m_restoreJobHandle =
        std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, m_restoreJobInfo);
    std::shared_ptr<HyperVProtectEngine> hyperVEngine = std::make_shared<HyperVProtectEngine>(m_restoreJobHandle);
    int32_t retValue = HyperVPlugin::SUCCESS;
    VolInfo volInfo;
    retValue = hyperVEngine->AttachVolume(volInfo);
    EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 挂载卷
 * 前置条件： repoPath 为空, GetRepoPathByType失败
 * CHECK点： 挂载卷失败
 */
TEST_F(HyperVProtectEngineRestoreTest, AttachVolume_GetRepoPathByType_RepoPath_Failed)
{
    AppProtect::RestoreJob restoreJob;
    StubSetRestoreInfo(restoreJob);
    FormCopyData(restoreJob, true);
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(restoreJob);
    std::shared_ptr<JobCommonInfo> m_restoreJobInfo = std::make_shared<JobCommonInfo>(data);
    std::shared_ptr<VirtPlugin::JobHandle> m_restoreJobHandle =
        std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, m_restoreJobInfo);
    std::shared_ptr<HyperVProtectEngine> hyperVEngine = std::make_shared<HyperVProtectEngine>(m_restoreJobHandle);
    int32_t retValue = HyperVPlugin::SUCCESS;
    VolInfo volInfo;
    retValue = hyperVEngine->AttachVolume(volInfo);
    EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 挂载卷
 * 前置条件： JsonStringToJsonValue失败
 * CHECK点：  挂载卷失败
 */
TEST_F(HyperVProtectEngineRestoreTest, AttachVolume_JsonStringToJsonValue_Failed)
{
    std::cout << "aaaaaaaa" << std::endl;
    int32_t retValue = 1;
    VolInfo volInfo;
    volInfo.m_extendInfo = "incorrect";
    retValue = m_HyperVRestoreProtectEngineHandler->AttachVolume(volInfo);
    EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 挂载卷
 * 前置条件： wmiClient 挂载卷失败
 * CHECK点： 挂载卷失败
 */
TEST_F(HyperVProtectEngineRestoreTest, AttachVolume_WmiClient_Failed)
{
    Stub stub;
	g_isSuccess = true;
    stub.set(Utils::GetStringValueByKeyFromJsonString, Stub_GetStringValueByKeyFromJsonString);
	g_isSuccess = false;
	stub.set(ADDR(WMIClient, AttachDiskToVM), Stub_AttachDiskToVM);

    int32_t retValue = 1;
    VolInfo volInfo;
	volInfo.m_extendInfo = "{\"DiskIdentifier\":\"3850E712-9B7C-4A73-8999-3E5B766B0E44\",\"VMId\":\"3850E712-9B7C-4A73-8999-3E5B766B0E45\"}"; 
	retValue = m_HyperVRestoreProtectEngineHandler->AttachVolume(volInfo);
    EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 挂载卷
 * 前置条件： wmiClient 挂载卷成功
 * CHECK点： 挂载卷成功
 */
TEST_F(HyperVProtectEngineRestoreTest, AttachVolume_Success)
{
    Stub stub;
	g_isSuccess = true;
    stub.set(Utils::GetStringValueByKeyFromJsonString, Stub_GetStringValueByKeyFromJsonString);
    stub.set(ADDR(WMIClient, AttachDiskToVM), Stub_AttachDiskToVM);

    int32_t retValue = 1;
    VolInfo volInfo;
    volInfo.m_extendInfo = "{\"DiskIdentifier\":\"3850E712-9B7C-4A73-8999-3E5B766B0E44\",\"VMId\":\"3850E712-9B7C-4A73-8999-3E5B766B0E45\"}";
    retValue = m_HyperVRestoreProtectEngineHandler->AttachVolume(volInfo);
    EXPECT_EQ(retValue, HyperVPlugin::SUCCESS);
}

/*
 * 测试用例： 创建虚拟机
 * 前置条件： 初始化失败
 * CHECK点： 创建虚拟机失败
 */
TEST_F(HyperVProtectEngineRestoreTest, CreateMachine_Init_Failed)
{
    Stub stub;
	g_isSuccess = false;
    auto F_Init = get_private_fun::HyperVProtectEngineInitJobPara();
    stub.set(F_Init, Stub_InitJobPara);

    int32_t retValue = 1;
    VMInfo vmInfo;
    retValue = m_HyperVRestoreProtectEngineHandler->CreateMachine(vmInfo);
    EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 创建虚拟机
 * 前置条件： 准备虚拟机配置参数失败
 * CHECK点： 创建虚拟机失败
 */
TEST_F(HyperVProtectEngineRestoreTest, CreateMachine_Prepare_Failed)
{
	Stub stub;
	g_isSuccess = false;
	stub.set(Utils::GetStringValueByKeyFromJsonString, Stub_GetStringValueByKeyFromJsonString);
    int32_t retValue = 1;
    VMInfo vmInfo;
    retValue = m_HyperVRestoreProtectEngineHandler->CreateMachine(vmInfo);
    EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 创建虚拟机
 * 前置条件： 初始化wmiClient失败
 * CHECK点： 创建虚拟机失败
 */
TEST_F(HyperVProtectEngineRestoreTest, CreateMachine_InitWmiClient_Failed)
{
    Stub stub;
    auto F_Init = get_private_fun::HyperVProtectEngineInitWmiClient();
    stub.set(F_Init, Stub_InitWmiClient);
    stub.set(Utils::GetStringValueByKeyFromJsonString, Stub_GetStringValueByKeyFromJsonString);

    int32_t retValue = 1;
    VMInfo vmInfo;
    retValue = m_HyperVRestoreProtectEngineHandler->CreateMachine(vmInfo);
    EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 创建虚拟机
 * 前置条件： wmi创建虚拟机失败
 * CHECK点： 创建虚拟机失败
 */
TEST_F(HyperVProtectEngineRestoreTest, CreateMachine_WmiClient_Failed)
{
    Stub stub;
	g_isSuccess = true;
    stub.set(Utils::GetStringValueByKeyFromJsonString, Stub_GetStringValueByKeyFromJsonString);
    g_isSuccess = false;
    stub.set(ADDR(WMIClient, CreateVM), Stub_CreateVM);

    int32_t retValue = 1;
    VMInfo vmInfo;
    retValue = m_HyperVRestoreProtectEngineHandler->CreateMachine(vmInfo);
    EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 恢复前检查
 * 前置条件： 初始化失败
 * CHECK点： 恢复前检查失败
 */
TEST_F(HyperVProtectEngineRestoreTest, CheckBeforeRecover_Init_Failed)
{
    Stub stub;
	g_isSuccess = false;
    auto F_Init = get_private_fun::HyperVProtectEngineInitJobPara();
    stub.set(F_Init, Stub_InitJobPara);

    int32_t retValue = 1;
    VMInfo vmInfo;
    retValue = m_HyperVRestoreProtectEngineHandler->CheckBeforeRecover(vmInfo);
    EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 恢复前检查
 * 前置条件： GetStringValueByKeyFromJsonString失败
 * CHECK点： 恢复前检查失败
 */
TEST_F(HyperVProtectEngineRestoreTest, CheckBeforeRecover_Utils_Failed)
{
	Stub stub;
	g_isSuccess = false;
	stub.set(Utils::GetStringValueByKeyFromJsonString, Stub_GetStringValueByKeyFromJsonString);
    int32_t retValue = 1;
    VMInfo vmInfo;
    retValue = m_HyperVRestoreProtectEngineHandler->CheckBeforeRecover(vmInfo);
    EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 恢复前检查
 * 前置条件： 初始化wmiClient失败
 * CHECK点： 恢复前检查失败
 */
TEST_F(HyperVProtectEngineRestoreTest, CheckBeforeRecover_InitWmiClient_Failed)
{
    Stub stub;
    auto F_Init = get_private_fun::HyperVProtectEngineInitWmiClient();
    stub.set(F_Init, Stub_InitWmiClient);
	g_isSuccess = true;
    stub.set(Utils::GetStringValueByKeyFromJsonString, Stub_GetStringValueByKeyFromJsonString);

    int32_t retValue = 1;
    VMInfo vmInfo;
    vmInfo.m_uuid = "abcd";
    retValue = m_HyperVRestoreProtectEngineHandler->CheckBeforeRecover(vmInfo);
    EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 恢复前检查
 * 前置条件： wmi删除虚拟机失败
 * CHECK点： 恢复前检查失败
 */
TEST_F(HyperVProtectEngineRestoreTest, CheckBeforeRecover_WmiClient_Failed)
{
    Stub stub;
	g_isSuccess = true;
    stub.set(Utils::GetStringValueByKeyFromJsonString, Stub_GetStringValueByKeyFromJsonString);
    g_isSuccess = false;
    stub.set(ADDR(WMIClient, DeleteVM), Stub_DeleteVM);

    int32_t retValue = 1;
    VMInfo vmInfo;
    vmInfo.m_uuid = "abcd";
    retValue = m_HyperVRestoreProtectEngineHandler->CheckBeforeRecover(vmInfo);
    EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 恢复前检查
 * 前置条件： wmi删除虚拟机成功
 * CHECK点： 恢复前检查成功
 */
TEST_F(HyperVProtectEngineRestoreTest, CheckBeforeRecover_Success)
{
    Stub stub;
	g_isSuccess = true;
    stub.set(Utils::GetStringValueByKeyFromJsonString, Stub_GetStringValueByKeyFromJsonString);
    g_isSuccess = true;
    stub.set(ADDR(WMIClient, DeleteVM), Stub_DeleteVM);

    int32_t retValue = 1;
    VMInfo vmInfo;
    vmInfo.m_uuid = "abcd";
    retValue = m_HyperVRestoreProtectEngineHandler->CheckBeforeRecover(vmInfo);
    EXPECT_EQ(retValue, HyperVPlugin::SUCCESS);
}


/*
 * 测试用例： 卸载卷
 * 前置条件： 初始化失败
 * CHECK点： 卸载卷失败
 */
TEST_F(HyperVProtectEngineRestoreTest, DetachVolume_Init_Failed)
{
    Stub stub;
	g_isSuccess = false;
    auto F_Init = get_private_fun::HyperVProtectEngineInitJobPara();
    stub.set(F_Init, Stub_InitJobPara);

    int32_t retValue = 1;
    VolInfo volObj;
    retValue = m_HyperVRestoreProtectEngineHandler->DetachVolume(volObj);
    EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 卸载卷
 * 前置条件： id为空
 * CHECK点： 卸载卷失败
 */
TEST_F(HyperVProtectEngineRestoreTest, DetachVolume_NotDisk_Failed)
{
	Stub stub;
    AppProtect::RestoreJob restoreJob;
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(restoreJob);
    std::shared_ptr<JobCommonInfo> restoreJobInfo = std::make_shared<JobCommonInfo>();
    restoreJobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> restoreJobHandle =
        std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, restoreJobInfo);
    std::shared_ptr<HyperVProtectEngine> hyperVHandler = std::make_shared<HyperVProtectEngine>(restoreJobHandle);

    int32_t retValue = 1;
    VolInfo volObj;
    retValue = hyperVHandler->DetachVolume(volObj);
    EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 卸载卷
 * 前置条件： GetStringValueByKeyFromJsonString失败
 * CHECK点： 卸载卷失败
 */
TEST_F(HyperVProtectEngineRestoreTest, DetachVolume_GetStringValue_Failed)
{
	Stub stub;
	g_isSuccess = false;
	stub.set(Utils::GetStringValueByKeyFromJsonString, Stub_GetStringValueByKeyFromJsonString);
    int32_t retValue = 1;
    VolInfo volObj;
    retValue = m_HyperVRestoreProtectEngineHandler->DetachVolume(volObj);
    EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 卸载卷
 * 前置条件： JsonStringToJsonValue失败
 * CHECK点： 卸载卷失败
 */
TEST_F(HyperVProtectEngineRestoreTest, DetachVolume_Json_Failed)
{
    Stub stub;
	g_isSuccess = true;
    stub.set(Utils::GetStringValueByKeyFromJsonString, Stub_GetStringValueByKeyFromJsonString);
	g_isSuccess = false;
    stub.set(Module::JsonHelper::JsonStringToJsonValue, Stub_JsonStringToJsonValue);

    int32_t retValue = 1;
    VolInfo volObj;
    retValue = m_HyperVRestoreProtectEngineHandler->DetachVolume(volObj);
    EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 卸载卷
 * 前置条件： 初始化wmiClient失败
 * CHECK点： 卸载卷失败
 */
TEST_F(HyperVProtectEngineRestoreTest, DetachVolume_InitWmiClient_Failed)
{
    AppProtect::RestoreJob restoreJob;
    restoreJob.targetObject.__set_id("10a4e361-c981-46f2-b9ba-d7ff9c601693");
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(restoreJob);
    std::shared_ptr<JobCommonInfo> restoreJobInfo = std::make_shared<JobCommonInfo>();
    restoreJobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> restoreJobHandle =
        std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, restoreJobInfo);
    std::shared_ptr<HyperVProtectEngine> hyperVHandler = std::make_shared<HyperVProtectEngine>(restoreJobHandle);

    Stub stub;
    auto F_Init = get_private_fun::HyperVProtectEngineInitWmiClient();
    stub.set(F_Init, Stub_InitWmiClient);

    int32_t retValue = 1;
    VolInfo volObj;
    retValue = hyperVHandler->DetachVolume(volObj);
    EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 卸载卷
 * 前置条件： wmi删除虚拟机失败
 * CHECK点： 卸载卷失败
 */
TEST_F(HyperVProtectEngineRestoreTest, DetachVolume_DetachDiskFromVM_Failed)
{
    AppProtect::RestoreJob restoreJob;
    restoreJob.targetObject.__set_id("10a4e361-c981-46f2-b9ba-d7ff9c601693");
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(restoreJob);
    std::shared_ptr<JobCommonInfo> restoreJobInfo = std::make_shared<JobCommonInfo>();
    restoreJobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> restoreJobHandle =
        std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, restoreJobInfo);
    std::shared_ptr<HyperVProtectEngine> hyperVHandler = std::make_shared<HyperVProtectEngine>(restoreJobHandle);

    Stub stub;
    g_isSuccess = false;
    stub.set(ADDR(WMIClient, DetachDiskFromVM), Stub_DetachDiskFromVM);

    int32_t retValue = 1;
    VolInfo volObj;
    retValue = hyperVHandler->DetachVolume(volObj);
    EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}


/*
 * 测试用例： 卸载卷
 * 前置条件： wmi删除虚拟机成功
 * CHECK点： 卸载卷成功
 */
TEST_F(HyperVProtectEngineRestoreTest, DetachVolume_Success)
{
    AppProtect::RestoreJob restoreJob;
    restoreJob.targetObject.__set_id("10a4e361-c981-46f2-b9ba-d7ff9c601693");
	restoreJob.extendInfo = "{\"restoreLevel\":0}";
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::RestoreJob>(restoreJob);
    std::shared_ptr<JobCommonInfo> restoreJobInfo = std::make_shared<JobCommonInfo>();
    restoreJobInfo->SetJobInfo(data);
    std::shared_ptr<VirtPlugin::JobHandle> restoreJobHandle =
        std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, restoreJobInfo);
    std::shared_ptr<HyperVProtectEngine> hyperVHandler = std::make_shared<HyperVProtectEngine>(restoreJobHandle);

    Stub stub;
    g_isSuccess = true;
    stub.set(ADDR(WMIClient, DetachDiskFromVM), Stub_DetachDiskFromVM);

    int32_t retValue = 1;
    VolInfo volObj;
    retValue = hyperVHandler->DetachVolume(volObj);
    EXPECT_EQ(retValue, HyperVPlugin::SUCCESS);
}

/*
 * 测试用例： 生成卷映射匹配对信息
 * 前置条件： 获取TargetDisk失败
 * CHECK点： 生成卷对信息失败
 */
TEST_F(HyperVProtectEngineRestoreTest, GenVolPair_GetTargetDisk_Failed)
{
	int32_t retValue = 1;
	VMInfo vmObj;
	VolInfo volObj;
	ApplicationResource targetVol;
	VolMatchPairInfo volPairs;
	retValue = m_HyperVRestoreProtectEngineHandler->GenVolPair(vmObj, volObj, targetVol, volPairs);
	EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 生成卷映射匹配对信息
 * 前置条件： JsonStringToJsonValue失败
 * CHECK点： 生成卷对信息失败
 */
TEST_F(HyperVProtectEngineRestoreTest, GenVolPair_JsonString_Failed)
{
	Stub stub;
	g_isSuccess = true;
	stub.set(Utils::GetStringValueByKeyFromJsonString, Stub_GetStringValueByKeyFromJsonString);
	g_isSuccess = false;
	stub.set(Module::JsonHelper::JsonStringToJsonValue, Stub_JsonStringToJsonValue);

	int32_t retValue = 1;
	VMInfo vmObj;
	VolInfo volObj;
	ApplicationResource targetVol;
	VolMatchPairInfo volPairs;
	retValue = m_HyperVRestoreProtectEngineHandler->GenVolPair(vmObj, volObj, targetVol, volPairs);
	EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 生成卷映射匹配对信息
 * 前置条件： 创建磁盘空文件失败
 * CHECK点： 生成卷对信息失败
 */
TEST_F(HyperVProtectEngineRestoreTest, GenVolPair_CreateDisk_Failed)
{
	Stub stub;
	g_isSuccess = true;
	stub.set(Utils::GetStringValueByKeyFromJsonString, Stub_GetStringValueByKeyFromJsonString);
	stub.set(GetFileAttributesA, Stub_GetFileAttributesA);
	g_isSuccess = false;
	stub.set(GetLastError, Stub_GetLastError);
	stub.set(CloseHandle, Stub_CloseHandle);
	stub.set(CreateFileA, Stub_CreateFileA);

	int32_t retValue = 1;
	VMInfo vmObj;
	VolInfo volObj;
	ApplicationResource targetVol;
	targetVol.extendInfo = "{\"extendInfo\":{\"Format\":\"VHD\",\"Path\":\"C:/zyq.vhdx\"}}";
	VolMatchPairInfo volPairs;
	retValue = m_HyperVRestoreProtectEngineHandler->GenVolPair(vmObj, volObj, targetVol, volPairs);
	EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 生成卷映射匹配对信息
 * 前置条件： 创建磁盘空文件成功
 * CHECK点： 生成卷对信息成功
 */
TEST_F(HyperVProtectEngineRestoreTest, GenVolPair_Success)
{
	Stub stub;
	g_isSuccess = true;
	stub.set(Utils::GetStringValueByKeyFromJsonString, Stub_GetStringValueByKeyFromJsonString_GenVolPair);
	stub.set(GetFileAttributesA, Stub_GetFileAttributesA);
	g_isSuccess = true;
	stub.set(GetLastError, Stub_GetLastError);
	stub.set(CloseHandle, Stub_CloseHandle);
	stub.set(CreateFileA, Stub_CreateFileA);

	int32_t retValue = 1;
	VMInfo vmObj;
	VolInfo volObj;
	ApplicationResource targetVol;
	targetVol.extendInfo = "{\"extendInfo\":{\"Format\":\"VHD\",\"Path\":\"C:/zyq.vhdx\"}}";
	VolMatchPairInfo volPairs;
	retValue = m_HyperVRestoreProtectEngineHandler->GenVolPair(vmObj, volObj, targetVol, volPairs);
	EXPECT_EQ(retValue, HyperVPlugin::SUCCESS);
}

/*
 * 测试用例： 上电虚拟机
 * 前置条件： wmi上电失败
 * CHECK点： 上电虚拟机失败
 */
TEST_F(HyperVProtectEngineRestoreTest, PowerOnMachine_WmiPower_Failed)
{
	Stub stub;
	g_isSuccess = false;
	stub.set(ADDR(WMIClient, ChangeVMPowerState), Stub_ChangeVMPowerState);
	int32_t retValue = 1;
	VMInfo vmObj;
	retValue = m_HyperVRestoreProtectEngineHandler->PowerOnMachine(vmObj);
	EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 上电虚拟机
 * 前置条件： wmi上电成功
 * CHECK点： 上电虚拟机成功
 */
TEST_F(HyperVProtectEngineRestoreTest, PowerOnMachine_Success)
{
	Stub stub;
	g_isSuccess = true;
	stub.set(ADDR(WMIClient, ChangeVMPowerState), Stub_ChangeVMPowerState);
	int32_t retValue = 1;
	VMInfo vmObj;
	retValue = m_HyperVRestoreProtectEngineHandler->PowerOnMachine(vmObj);
	EXPECT_EQ(retValue, HyperVPlugin::SUCCESS);
}

/*
 * 测试用例： 下电虚拟机
 * 前置条件： wmi上电失败
 * CHECK点： 下电虚拟机失败
 */
TEST_F(HyperVProtectEngineRestoreTest, PowerOffMachine_WmiPower_Failed)
{
	Stub stub;
	g_isSuccess = false;
	stub.set(ADDR(WMIClient, ChangeVMPowerState), Stub_ChangeVMPowerState);
	int32_t retValue = 1;
	VMInfo vmObj;
	retValue = m_HyperVRestoreProtectEngineHandler->PowerOffMachine(vmObj);
	EXPECT_EQ(retValue, HyperVPlugin::FAILED);
}

/*
 * 测试用例： 下电虚拟机
 * 前置条件： wmi上电成功
 * CHECK点： 下电虚拟机成功
 */
TEST_F(HyperVProtectEngineRestoreTest, PowerOffMachine_Success)
{
	Stub stub;
	g_isSuccess = true;
	stub.set(ADDR(WMIClient, ChangeVMPowerState), Stub_ChangeVMPowerState);
	int32_t retValue = 1;
	VMInfo vmObj;
	retValue = m_HyperVRestoreProtectEngineHandler->PowerOffMachine(vmObj);
	EXPECT_EQ(retValue, HyperVPlugin::SUCCESS);
}
