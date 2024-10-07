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
#include <functional>
#include "job_controller/jobs/restore/RestoreJob.h"
#include "ApplicationProtectPlugin_types.h"
#include "ApplicationProtectBaseDataType_types.h"
#include "protect_engines/ProtectEngineMock.h"
#include "volume_handlers/volumeHandlerMock.h"
#include "common/DirtyRanges.h"
#include "common/utils/Utils.h"
#include "common/CTime.h"
#include "common/CommonMock.h"
#include "repository_handlers/mock/FileSystemHandlerMock.h"
#include "repository_handlers/factory/RepositoryFactory.h"
#include <repository_handlers/mock/FileSystemHandlerMock.h>
#include <repository_handlers/mock/FileSystemMock.h>
#include "ClientInvoke.h"
#include "config_reader/ConfigIniReader.h"
#include <sstream>

using ::testing::_;
using ::testing::An;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using ::testing::Invoke;
using ::testing::A;

using namespace VirtPlugin;

namespace HDT_TEST {
class ProtectEngineMock;
const int BUFF_SIZE = 5 * DEFAULT_BLOCK_SIZE;

const std::string volMetaData = "{\
    \"uuid\":\"1\",\
    \"moRef\":\"1-1\",\
}";

const std::string volUUID = "1c0b87cf-18c5-4b90-8753-838d02f8d25c";
const std::string g_volumeContent = "FILE CONTENT IN THE TEST FILE.";

static int32_t FileSystemReadMetaDataSuccess_Invoke(std::string &buf, size_t size)
{
    buf = volMetaData;
    return volMetaData.length();
}

size_t FileSystemReadSuccess_Invoke(std::shared_ptr<uint8_t[]> buf, size_t count)
{
    return count;
}

static std::string Stub_FileSystemReadSnapshotInfo()
{
    SnapshotInfo snapshotInfo;
    VolSnapInfo volSnapshotInfo;
    volSnapshotInfo.m_volUuid = volUUID;
    snapshotInfo.m_volSnapList.push_back(volSnapshotInfo);
    std::string snapshotInfoStr;
    Module::JsonHelper::StructToJsonString(snapshotInfo, snapshotInfoStr);
    return snapshotInfoStr;
}

static int32_t FileSystemReadSnapshotForRestore_Invoke(std::string &buf, size_t size)
{
    std::string return_value = Stub_FileSystemReadSnapshotInfo();
    buf = return_value;
    return size;
}

static std::string Stub_GetVolPairInfo()
{
    VolMatchPairInfo volPair;
    VolInfo copyVol;
    VolInfo targetVol;
    copyVol.m_uuid = "1-1-1-1";
    targetVol.m_uuid = "1-1-1-1";
    targetVol.m_newCreate = true;  // 新创建卷
    volPair.AddVolPair(copyVol, targetVol); // 原卷恢复
    copyVol.m_uuid = "2-2-2-2";
    targetVol.m_uuid = "3-3-3-3";  // 非原卷恢复
    targetVol.m_newCreate = false;  // 新创建卷
    volPair.AddVolPair(copyVol, targetVol);
    std::string volPairStr;
    Module::JsonHelper::StructToJsonString(volPair, volPairStr);
    return volPairStr;
}

static int32_t FileSystemReadVolPair_Invoke(std::string &buf, size_t size)
{
    buf = Stub_GetVolPairInfo();
    return size;
}

static std::string Stub_GetVMInfo()
{
    VMInfo vmObj;
    vmObj.m_uuid = "8-8-8-8";
    std::string vmStr;
    Module::JsonHelper::StructToJsonString(vmObj, vmStr);
    return vmStr;
}

static int32_t FileSystemReadRestoreVolumePair_Invoke(std::string &buf, size_t size)
{
    buf = Stub_GetVolPairInfo();
    return size;
}

static int32_t FileSystemReadVMInfo_Invoke(std::string &buf, size_t size)
{
    buf = Stub_GetVMInfo();
    return size;
}

int FSReadInvokeTimes = 0;
static int32_t FileSystemReadVMandVolStub(std::string &buf, size_t size)
{
    if (FSReadInvokeTimes == 0) {
        buf = Stub_GetVolPairInfo();
        FSReadInvokeTimes ++;
    } else if (FSReadInvokeTimes == 1) {
        buf = Stub_GetVMInfo();
        FSReadInvokeTimes ++;
    }
    return size;
}

static int32_t FileSystemWriteSuccess_Invoke(const std::string &content)
{
    return content.length();
}


static bool FileExistStub(const std::string &fileName)
{
    if (fileName.find("main_task_status.info") != std::string::npos) {
        return false;
    }
    if (fileName.find("_block_bitmap.info") > fileName.length()) { // 不存在有效比特位图则返回true
        return true;
    }
    return false;
}

static bool CustomizeFileExistStub(const std::string &fileName)
{
    if (fileName.find("main_task_status.info") != std::string::npos) {
        return false;
    }
    if (fileName.find("vol_match.info") < fileName.length()) {
        return false;
    }
    if (fileName.find("_block_bitmap.info") > fileName.length()) { // 不存在有效比特位图则返回true
        return true;
    }
    return false;
}

static bool CustomizeVolPairFileExist(const std::string &fileName)
{
    if (fileName.find("main_task_status.info") != std::string::npos) {
        return false;
    }
    return true;
}

static std::shared_ptr<ProtectEngine> Stub_CreateEngine_GetVolumeHandler_OK(const JobType &jobType,
    std::shared_ptr<JobCommonInfo> jobInfo)
{
    std::cout << "test " << std::endl;
    std::shared_ptr<JobHandle> jobHandle = std::make_shared<JobHandle>(jobType, jobInfo);
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>(jobHandle);
	VolInfo volInfo;
    std::shared_ptr<VolumeHandlerMock> volHandlerMock = std::make_shared<VolumeHandlerMock>(jobHandle, volInfo);
    testing::Mock::AllowLeak(volHandlerMock.get());
    DirtyRanges dirtyRanges;
    dirtyRanges.AddRange(DirtyRange(0, BUFF_SIZE));
    EXPECT_CALL(*volHandlerMock, GetDirtyRanges(_,_,_,_,_)).WillRepeatedly(DoAll(SetArgReferee<2>(dirtyRanges),
        Return(SUCCESS)));
    EXPECT_CALL(*volHandlerMock, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandlerMock, Open(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandlerMock, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandlerMock, WriteBlocks(_,_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandlerMock, Flush()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandlerMock, CleanLeftovers()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, GetVolumeHandler(_,_)).WillRepeatedly(DoAll(SetArgReferee<1>(volHandlerMock),
        Return(SUCCESS)));
    EXPECT_CALL(*protectEngine, PreHook(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, PostHook(_)).WillRepeatedly(Return(SUCCESS));
    return protectEngine;
}

static std::shared_ptr<ProtectEngine> Stub_CreateEngine_GetVolumeHandler_DiffDirtyRange(const JobType &jobType,
    std::shared_ptr<JobCommonInfo> jobInfo)
{
    std::shared_ptr<JobHandle> jobHandle = std::make_shared<JobHandle>(jobType, jobInfo);
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>(jobHandle);
    VolInfo volInfo;
    std::shared_ptr<VolumeHandlerMock> volHandlerMock = std::make_shared<VolumeHandlerMock>(jobHandle, volInfo);
    testing::Mock::AllowLeak(protectEngine.get());
    testing::Mock::AllowLeak(volHandlerMock.get());
    EXPECT_CALL(*volHandlerMock, GetVolumeSize()).WillRepeatedly(Return(strlen(g_volumeContent.c_str())));
    int invokeTimes = 3;
    EXPECT_CALL(*volHandlerMock, GetDirtyRanges(_,_,_,_,_)).Times(invokeTimes).WillOnce(Invoke(
            [&](const VolSnapInfo &preVolSnapshot, const VolSnapInfo &curVolSnapshot, DirtyRanges &dirtyRanges,
                const uint64_t startOffset, uint64_t &endOffset) {
                dirtyRanges.AddRange(DirtyRange(0, DEFAULT_BLOCK_SIZE * 5));
                return SUCCESS;
            })).WillOnce(Invoke(
            [&](const VolSnapInfo &preVolSnapshot, const VolSnapInfo &curVolSnapshot, DirtyRanges &dirtyRanges,
                const uint64_t startOffset, uint64_t &endOffset) {
                dirtyRanges.AddRange(DirtyRange(DEFAULT_BLOCK_SIZE * 5, DEFAULT_BLOCK_SIZE * 5));
                return SUCCESS;
            })).WillOnce(Invoke(
            [&](const VolSnapInfo &preVolSnapshot, const VolSnapInfo &curVolSnapshot, DirtyRanges &dirtyRanges,
                const uint64_t startOffset, uint64_t &endOffset) {
                dirtyRanges.AddRange(DirtyRange(DEFAULT_BLOCK_SIZE * 10, 10 * 1024 * 1024ULL));
                return SUCCESS;
            }));
    EXPECT_CALL(*volHandlerMock, ReadBlocks(_,_,_,_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandlerMock, Open(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandlerMock, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandlerMock, WriteBlocks(_,_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandlerMock, Flush()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandlerMock, CleanLeftovers()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, GetVolumeHandler(_,_)).WillRepeatedly(DoAll(SetArgReferee<1>(volHandlerMock),
        Return(SUCCESS)));
    EXPECT_CALL(*protectEngine, PreHook(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, PostHook(_)).WillRepeatedly(Return(SUCCESS));
    return protectEngine;
}

static std::shared_ptr<ProtectEngine> Stub_CreateEngine_GetVolumeHandler_FAILED(const JobType &jobType,
    std::shared_ptr<JobCommonInfo> jobInfo)
{
    std::shared_ptr<JobHandle> jobHandle = std::make_shared<JobHandle>(jobType, jobInfo);
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>(jobHandle);
    EXPECT_CALL(*protectEngine, GetVolumeHandler(_,_)).WillRepeatedly(Return(FAILED));
    EXPECT_CALL(*protectEngine, PreHook(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, PostHook(_)).WillRepeatedly(Return(SUCCESS));
    return protectEngine;
}

static std::shared_ptr<ProtectEngine> Stub_CreateEngine_GetDirtyRangesr_FAILED(const JobType &jobType,
    std::shared_ptr<JobCommonInfo> jobInfo)
{
    std::shared_ptr<JobHandle> jobHandle = std::make_shared<JobHandle>(jobType, jobInfo);
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>(jobHandle);
	VolInfo volInfo;
    std::shared_ptr<VolumeHandlerMock> volHandlerMock = std::make_shared<VolumeHandlerMock>(jobHandle, volInfo);
    EXPECT_CALL(*volHandlerMock, GetDirtyRanges(_,_,_,_,_)).WillRepeatedly(Return((FAILED)));
    EXPECT_CALL(*volHandlerMock, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandlerMock, Open(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandlerMock, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandlerMock, Flush()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandlerMock, CleanLeftovers()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, GetVolumeHandler(_,_)).WillRepeatedly(DoAll(SetArgReferee<1>(volHandlerMock),
        Return(SUCCESS)));
    EXPECT_CALL(*protectEngine, PreHook(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, PostHook(_)).WillRepeatedly(Return(SUCCESS));
    return protectEngine;
}

/* Stub for FileSystemHandler for block bitmap test */
static std::shared_ptr<RepositoryHandler> Stub_CreateFSHandler_BitMapTest(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemMock> fsHandler = std::make_shared<FileSystemMock>();
    testing::Mock::AllowLeak(fsHandler.get());
    return fsHandler;
}

static std::shared_ptr<ProtectEngine> Stub_CreateEngine_FAILED(const JobType &jobType,
    std::shared_ptr<JobCommonInfo> jobInfo)
{
    return nullptr;
}

static uint64_t Stub_GetSegSizeFromConf()
{
    return 5 * DEFAULT_BLOCK_SIZE; // 50M
}

static bool Stub_ExecBlockTaskStart(void* obj)
{
    return true;
}


static int preHookRet {SUCCESS};
static int postHookRet {SUCCESS};
static int createSnapShotRet {SUCCESS};
static int deleteSnapShotRet {SUCCESS};
static int createMachineRet {SUCCESS};
static int deleteMachineRet {SUCCESS};
static int powerOnMachineRet{SUCCESS};
static int powerOffMachineRet{SUCCESS};
static int genVolPairRet{SUCCESS};
static int checkBeforeRecoverRet{SUCCESS};
static int detachVolumeRet{SUCCESS};
static int attachVolumeRet{SUCCESS};
static int deleteVolumeRet{SUCCESS};

static int fsOpenRet = SUCCESS;
static int fsCloseRet = SUCCESS;
static int fsfileSizeRet = volMetaData.length();
static bool fsRemoveRet = true;
static bool fsCreateDirRet = true;

void Stub_ReportTaskLabel_OK()
{
    return;
}

std::function<int32_t(std::string &, size_t)> stubReadFunc = nullptr;
std::function<bool(const std::string &)> fileExistStubFunc = nullptr;

class RestoreJobTest : public testing::Test {
protected:
    /**
    * @brief 组装Agent下发的副本数据
    *
    * @param vmRestoreParam
    * @return def
    */
    void FormCopyData(AppProtect::RestoreJob &vmRestoreParam)
    {
        AppProtect::Copy copy;
        copy.id = "001";
        // 数据仓
        StorageRepository dataRepo;
        dataRepo.repositoryType = RepositoryDataType::DATA_REPOSITORY;
        dataRepo.path.push_back("/datapath1");
        copy.repositories.push_back(dataRepo);
        // 元数据仓
        StorageRepository metaRepo;
        metaRepo.repositoryType = RepositoryDataType::META_REPOSITORY;
        metaRepo.path.push_back("/metapath1");
        copy.repositories.push_back(metaRepo);
        // 缓存仓
        StorageRepository cacheRepo;
        cacheRepo.repositoryType = RepositoryDataType::CACHE_REPOSITORY;
        cacheRepo.path.push_back("/cachepath1");
        copy.repositories.push_back(cacheRepo);
        vmRestoreParam.copies.push_back(copy);
    }

    /**
    * @brief 组装目录环境相关信息
    *
    * @param vmRestoreParam
    */
    void FormTargetEnv(AppProtect::RestoreJob &vmRestoreParam)
    {
        vmRestoreParam.targetEnv.__isset.auth = true;
        vmRestoreParam.targetEnv.auth.__set_authkey("admin");
        vmRestoreParam.targetEnv.auth.__set_authPwd("Admin@123");
        vmRestoreParam.targetEnv.__set_endpoint("8.40.102.115");
        int port = 8088;
        vmRestoreParam.targetEnv.__set_port(port);
    }

    /**
    * @brief 组装目标虚拟机信息
    *
    * @param vmRestoreParam
    * @return vid
    */
    void FormTargetObject(AppProtect::RestoreJob &vmRestoreParam, std::string power)
    {
        vmRestoreParam.targetObject.__set_id("147");
        vmRestoreParam.targetObject.__set_name("test_123456");
        std::stringstream extendInfo;
        extendInfo << "{\"powerState\":" << power << "}";
        vmRestoreParam.targetObject.__set_extendInfo(extendInfo.str());
    }

    void FormRestoreVolList(AppProtect::RestoreJob &vmRestoreParam)
    {
        ApplicationResource restoreVol;
        restoreVol.type = "volume";
        restoreVol.id = "1"; // 备份卷ID
        restoreVol.__set_extendInfo("{\"targetVolUUID\":\"2\", \"targetDSName\":\"ds1\", \"targetDSMoRef\":\"moref1\",\
            \"tagetStorageDC\":\"dc1\", \
            \"datastoreName\":\"test\", \"datastoreMoRef\":\"10\", \"dataCenter\":\"dc\", \"replaceMode\":\"1\"}");
        vmRestoreParam.restoreSubObjects = {restoreVol};
    }

    void FormRestoreAdvParam(AppProtect::RestoreJob &vmRestoreParam, std::string RestoreLevel,std::string forceRestore)
    {
        std::stringstream params;
        params << "{\"restoreLevel\":" << RestoreLevel << ",\"force_recovery\":" << forceRestore <<"}";
        vmRestoreParam.__set_extendInfo(params.str()); // restoreLevel: 0-虚拟机恢复 1-卷恢复
    }

    void SetRestoreParam(int RestoreLevel, const JobResult::type restoreJobResult, std::string powerState = "1",
                         std::string forceRestore = "false")
    {
        AppProtect::RestoreJob vmRestoreParam;
        vmRestoreParam.jobId = "1";
        vmRestoreParam.requestId = "000001";
        FormCopyData(vmRestoreParam);
        FormTargetEnv(vmRestoreParam);
        FormTargetObject(vmRestoreParam, powerState);
        FormRestoreVolList(vmRestoreParam);
        FormRestoreAdvParam(vmRestoreParam, std::to_string(RestoreLevel),forceRestore);
        m_RestoreJob->SetPostJobResultType(restoreJobResult);
        std::shared_ptr <ThriftDataBase> restorePtr = std::make_shared<AppProtect::RestoreJob>(vmRestoreParam);
        std::shared_ptr <JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
        jobInfo->SetJobInfo(restorePtr);
        m_RestoreJob->SetJobInfo(jobInfo);
    }

    VolInfo GetDefaultVolObj()
    {
        VolInfo volObj;
        volObj.m_moRef = "moref";
        volObj.m_uuid = volUUID;
        volObj.m_name = "name";
        volObj.m_type = "type";
        volObj.m_datastore.m_type = "storage";
        volObj.m_datastore.m_moRef = "ds_moref";
        volObj.m_datastore.m_name = "ds_name";
        volObj.m_volSizeInBytes = 50 * 1024 * 1024ULL; // 50M
        return volObj;
    }

    void SetSubJobVolInfo()
    {
        m_srcVolObj = GetDefaultVolObj();
        m_targetVolObj = m_srcVolObj;
    }

    void SetSubJob(std::string jobId="0", std::string subJobId="1", std::string jobName="subJob", int jobPri=1)
    {
        m_subJob.jobId = jobId;
        m_subJob.subJobId = subJobId;
        m_subJob.jobName = jobName;
        m_subJob.jobPriority = jobPri;
        std::string srcVolStr;
        std::string dstVolStr;
        Module::JsonHelper::StructToJsonString(m_srcVolObj, srcVolStr);
        Module::JsonHelper::StructToJsonString(m_targetVolObj, dstVolStr);
        SubJobExtendInfo subJobExtendInfo;
        subJobExtendInfo.m_subTaskType = 1;
        subJobExtendInfo.m_targetVolumeInfo = srcVolStr;
        subJobExtendInfo.m_originVolumeInfo = dstVolStr;
        std::string subJobExtendInfoStr;
        Module::JsonHelper::StructToJsonString(subJobExtendInfo, subJobExtendInfoStr);
        m_subJob.jobInfo = subJobExtendInfoStr;
        std::shared_ptr<AppProtect::SubJob> subJobInfo = std::make_shared<AppProtect::SubJob>(m_subJob);
        m_RestoreJob->SetSubJob(subJobInfo);
    }

    static void ReportJobDetailsSuc(void *obj, ActionResult& returnValue, const SubJobDetails& jobInfo)
    {
        return;
    }

    void SetUp()
    {
        stub.set(sleep, Stub_Sleep);
        m_RestoreJob = std::make_unique<VirtPlugin::RestoreJob>();
        InitLogger();
        FSReadInvokeTimes = 0;
        SetSubJobVolInfo();
        SetSubJob();
        checkBeforeRecoverRet = SUCCESS;
        preHookRet = SUCCESS;
        postHookRet = SUCCESS;
        createSnapShotRet = SUCCESS;
        deleteSnapShotRet = SUCCESS;
        createMachineRet = SUCCESS;
        deleteMachineRet = SUCCESS;
        powerOnMachineRet = SUCCESS;
        attachVolumeRet = SUCCESS;
        powerOffMachineRet = SUCCESS;
        genVolPairRet = SUCCESS;
        detachVolumeRet = SUCCESS;
        deleteVolumeRet = SUCCESS;
        fsOpenRet = SUCCESS;
        fsCloseRet = SUCCESS;
        fsfileSizeRet = volMetaData.length();
        fsRemoveRet = true;
        fsCreateDirRet = true;\
        stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
        stub.set(ADDR(VirtualizationBasicJob, ReportTaskLabel), Stub_ReportTaskLabel_OK);
        fileExistStubFunc = nullptr;
    }
    void InitLogger()
    {
        std::string logFileName = "virt_plugin_restore_job_test.log";
        std::string logFilePath = "/tmp/log/";
        int logLevel = DEBUG;
        int logFileCount = 10;
        int logFileSize = 30;
        Module::CLogger::GetInstance().Init(
            logFileName.c_str(), logFilePath, logLevel, logFileCount, logFileSize);
    }

    void TearDown()
    {
        stub.reset(ADDR(EngineFactory, CreateProtectEngine));
        stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
        stub.reset(ADDR(VirtualizationBasicJob, ReportTaskLabel));
    }
    VolInfo m_srcVolObj;
    VolInfo m_targetVolObj;
    AppProtect::SubJob m_subJob;
    std::unique_ptr<VirtPlugin::RestoreJob> m_RestoreJob;
    Stub stub;

private:
    typedef int (*JobServiceMethodPtr)(JobService*);
    JobServiceMethodPtr JobService_ReportJobDetails = (JobServiceMethodPtr)(&JobService::ReportJobDetails);
};



/* Stub for ProtectEngine */
static std::shared_ptr<ProtectEngine> SStub_Call_ProtectEngine_OK(const JobType &jobType,
    std::shared_ptr<JobCommonInfo> jobInfo, std::string jobId, std::string subJobId)
{
    VolMatchPairInfo volPairs;
    VolInfo originalVol;
    originalVol.m_uuid = "1-1-1-1";
    VolInfo targetVol;
    targetVol.m_uuid = "1-1-1-1";
    VolPair restoreVolPair;
    restoreVolPair.m_originVol = originalVol;
    restoreVolPair.m_targetVol = targetVol;
    volPairs.m_volPairList.push_back(restoreVolPair);
    std::shared_ptr<JobHandle> jobHandle = std::make_shared<JobHandle>(jobType, jobInfo);
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>(jobHandle);
    EXPECT_CALL(*protectEngine, PreHook(_)).WillRepeatedly(Return(preHookRet));
    EXPECT_CALL(*protectEngine, PostHook(_)).WillRepeatedly(Return(postHookRet));
    EXPECT_CALL(*protectEngine, CreateSnapshot(_,_)).WillRepeatedly(Return(createSnapShotRet));
    EXPECT_CALL(*protectEngine, DeleteSnapshot(_)).WillRepeatedly(Return(deleteSnapShotRet));
    EXPECT_CALL(*protectEngine, CreateMachine(_)).WillRepeatedly(Return(createMachineRet));
    EXPECT_CALL(*protectEngine, DeleteMachine(_)).WillRepeatedly(Return(deleteMachineRet));
    EXPECT_CALL(*protectEngine, PowerOnMachine(_)).WillRepeatedly(Return(powerOnMachineRet));
    EXPECT_CALL(*protectEngine, PowerOffMachine(_)).WillRepeatedly(Return(powerOffMachineRet));
    EXPECT_CALL(*protectEngine, GenVolPair(_,_,_,_)).WillRepeatedly(DoAll(SetArgReferee<3>(volPairs), Return(SUCCESS)));
    EXPECT_CALL(*protectEngine, CheckBeforeRecover(_)).WillRepeatedly(Return(checkBeforeRecoverRet));
    EXPECT_CALL(*protectEngine, DetachVolume(_)).WillRepeatedly(Return(detachVolumeRet));
    EXPECT_CALL(*protectEngine, AttachVolume(_)).WillRepeatedly(Return(attachVolumeRet));
    EXPECT_CALL(*protectEngine, DeleteVolume(_)).WillRepeatedly(Return(deleteVolumeRet));
    return protectEngine;
}

/* Stub for FileSystemHandler */
static std::shared_ptr<RepositoryHandler> Stub_CreateFSHandler(
    const AppProtect::StorageRepository &storageRepo)
{
    if (stubReadFunc == nullptr) {
        stubReadFunc = FileSystemReadMetaDataSuccess_Invoke;
    }
    if (fileExistStubFunc == nullptr) {
        fileExistStubFunc = FileExistStub;
    }
    static std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(fsOpenRet));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(fsCloseRet));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(fsfileSizeRet));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(stubReadFunc));
    EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(fileExistStubFunc)));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(fsRemoveRet));
    EXPECT_CALL(*fsHandler, CreateDirectory(_)).WillRepeatedly(Return(fsCreateDirRet));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    return fsHandler;
}

std::string Stub_GetStringSuccess(std::string s1, std::string s2)
{
    return "yes";
}
bool stubLoadMetaDataSuccess()
{
    return true;
}

int SaveFileStubSucc()
{
    return SUCCESS;
}

bool FlushToStorageSucc()
{
    return true;
}

static void Stub_JobServiceAddNewJob_OK(ActionResult& result, const std::vector<SubJob>& subs)
{
    std::cout << "Stub_JobServiceAddNewJob_OK call" << std::endl;
    result.__set_code(SUCCESS);
    return;
}

static void Stub_JobServiceAddNewJob_NO(ActionResult& result, const std::vector<SubJob>& subs)
{
    std::cout << "Stub_JobServiceAddNewJob_NO call" << std::endl;
    result.__set_code(FAILED);
    return;
}

static void Stub_JobServiceReportJobDetails(ActionResult& result, const SubJobDetails& subJobDetails)
{
    std::cout << "Stub_JobServiceReportJobDetails call" << std::endl;
    return;
}

static void NowTime(time_t &startTime)
{
    static int retryCount = 0;
    if (retryCount == 0) {
        retryCount += 1;
        startTime = 0;
        return;
    }
    startTime = 500;
    return;
}

static std::shared_ptr<RepositoryHandler> Stub_CreateFSHandler_Customize_CreateDir_failed(
    const AppProtect::StorageRepository &storageRepo)
{
    static std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    static bool initFlag = false;
    if (!initFlag) {
        EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(fsOpenRet));
        EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(fsCloseRet));
        EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(fsfileSizeRet));
        EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(FileSystemReadMetaDataSuccess_Invoke));
        EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
        EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(fsRemoveRet));
        EXPECT_CALL(*fsHandler, CreateDirectory(_)).WillRepeatedly(Return(fsCreateDirRet));
        int invokeTimes = 4;
        EXPECT_CALL(*fsHandler, Exists(_)).Times(invokeTimes).WillOnce(Invoke([&](
                    const std::string &fileName) {
                    return true;
                })).WillOnce(Invoke([&](
                    const std::string &fileName) {
                    return true;
                })).WillOnce(Invoke([&](
                    const std::string &fileName) {
                    return true;
                })).WillOnce(Invoke([&](
                    const std::string &fileName) {
                    return false;
                }));
    }
    initFlag = true;
    return fsHandler;
}


/**
 * 用例名称：虚拟机恢复前置任务成功
 * 前置条件：应用所有接口均调用成功
 * check点：前置任务返回SUCCESS
 */
TEST_F(RestoreJobTest, VMResotrePrerequisiteSuc)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
    EXPECT_EQ(m_RestoreJob->PrerequisiteJob(), SUCCESS);
}

/**
 * 用例名称：卷恢复前置任务成功
 * 前置条件：应用所有接口均调用成功，存在卷匹配对 
 * check点：前置任务返回SUCCESS
 */
TEST_F(RestoreJobTest, VolResotrePrerequisiteSuc)
{
    fileExistStubFunc = CustomizeVolPairFileExist;
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_DISK), JobResult::SUCCESS);
    EXPECT_EQ(m_RestoreJob->PrerequisiteJob(), SUCCESS);
}

/**
 * 用例名称：虚拟机恢复前置任务未开启跳过坏块配置，强制恢复失败
 * 前置条件：CommonInfoInit执行成功，未开启跳过坏块配置
 * check点：前置任务返回FAILED
 */
 TEST_F(RestoreJobTest, ResotrePrerequisiteSuc_ForciblyRestoreFailed)
 {
 stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
 stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
 SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS, "1","true");
 EXPECT_EQ(m_RestoreJob->PrerequisiteJob(), FAILED);
 }

/**
 * 用例名称：虚拟机恢复前置任务开启跳过坏块配置，强制恢复成功
 * 前置条件：CommonInfoInit执行成功，开启跳过坏块配置
 * check点：前置任务返回SUCCESS
 */
 TEST_F(RestoreJobTest, ResotrePrerequisiteSuc_ForciblyRestoreSUCCESS)
 {
 stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
 stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
 stub.set(ADDR(Module::ConfigReader, getString), Stub_GetStringSuccess);

 SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS, "1","true");
 EXPECT_EQ(m_RestoreJob->PrerequisiteJob(), SUCCESS);
 }


/**
 * 用例名称：虚拟机恢复前置任务执行只执行Hook点，不执行框架流程
 * 前置条件：前置PreHook返回特殊流程错误码
 * check点：前置任务返回SUCCESS
 */
TEST_F(RestoreJobTest, ResotrePrerequisiteSuc_DiffFlow)
{
    preHookRet = DIFFERENT_FLOW;
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_DISK), JobResult::SUCCESS);
    EXPECT_EQ(m_RestoreJob->PrerequisiteJob(), SUCCESS);
}

/**
 * 用例名称：打开虚拟机ovf文件失败(文件默认不存在）,导致前置任务失败
 * 前置条件：虚拟机ovf文件不存在
 * check点：前置任务返回FAILED
 */
TEST_F(RestoreJobTest, ResotrePrerequisite_openvmOvf_Fail)
{
    fsOpenRet = FAILED;
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    EXPECT_EQ(m_RestoreJob->PrerequisiteJob(), FAILED);
}

/**
 * 用例名称：卷恢复，CheckBeforeRecover调用失败
 * 前置条件：应用调用CheckBeforeRecover失败
 * check点：前置任务返回FAILED
 */
TEST_F(RestoreJobTest, ResotrePrerequisite_CheckBeforeRecover_fail)
{
    checkBeforeRecoverRet = FAILED;
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_DISK), JobResult::SUCCESS);
    EXPECT_EQ(m_RestoreJob->PrerequisiteJob(), FAILED);
}

/**
 * 用例名称：InitRepo调用CreateDirectory失败
 * 前置条件：repoHandler调用CreateDirectory接口失败
 * check点：前置任务返回FAILED
 */
TEST_F(RestoreJobTest, ResotrePrerequisite_CreateDirectory_fail)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    fsCreateDirRet = false;
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_Customize_CreateDir_failed);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_DISK), JobResult::SUCCESS);
    EXPECT_EQ(m_RestoreJob->PrerequisiteJob(), FAILED);
}

/**
 * 用例名称：DetachVolume失败
 * 前置条件：DetachVolume接口失败
 * check点：前置任务返回FAILED
 */
TEST_F(RestoreJobTest, ResotrePrerequisite_DetachVolume_fail)
{
    detachVolumeRet = FAILED;
    fileExistStubFunc = CustomizeFileExistStub;
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_DISK), JobResult::SUCCESS);
    EXPECT_EQ(m_RestoreJob->PrerequisiteJob(), FAILED);
}

/**
 * 用例名称：创建虚拟机失败
 * 前置条件：调用应用创建虚拟机接口失败
 * check点：前置任务返回FAILED
 */
TEST_F(RestoreJobTest, ResotrePrerequisite_CreateVM_fail)
{
    createMachineRet = FAILED;
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
    SetSubJob("0", "1", "PostSubJob");
    EXPECT_EQ(m_RestoreJob->ExecuteSubJob(), FAILED);
    createMachineRet = SUCCESS;
}

/**
 * 用例名称：下电虚拟机失败
 * 前置条件：调用应用下电虚拟机接口失败
 * check点：前置任务返回FAILED
 */
TEST_F(RestoreJobTest, ResotrePrerequisite_PowerOffVM_fail)
{
    powerOffMachineRet = FAILED;
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_DISK), JobResult::SUCCESS);
    EXPECT_EQ(m_RestoreJob->PrerequisiteJob(), FAILED);
}

/*
 * 用例名称：子任务执行初始化任务失败，流程测试
 * 前置条件：无
 * check点：创建受保护对象引擎失败，流程返回FAILED
 */
TEST_F(RestoreJobTest, SubTaskInitializeCreateProtectengineFailed)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_FAILED);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
    int ret = m_RestoreJob->ExecuteSubJob();
    EXPECT_EQ(ret, FAILED);
}

/*
 * 用例名称：子任务执行初始化任务失败，流程测试
 * 前置条件：无
 * check点：创建读写handler失败，流程返回FAILED
 */
TEST_F(RestoreJobTest, SubTaskInitializeCreateIOHandlerFailed)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_GetVolumeHandler_FAILED);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
    int ret = m_RestoreJob->ExecuteSubJob();
    EXPECT_EQ(ret, FAILED);
}

/*
 * 用例名称：子任务执行初始化任务成功，流程测试
 * 前置条件：无
 * check点：获取dirtyrange，流程返回FAILED
 */
TEST_F(RestoreJobTest, SubTaskGetDirtyRangesFAILED)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_GetDirtyRangesr_FAILED);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
    int ret = m_RestoreJob->ExecuteSubJob();
    EXPECT_EQ(ret, FAILED);
}

/*
 * 用例名称：源卷恢复子任务执行源机恢复成功
 * 前置条件：源卷恢复
 * check点：执行子任务成功
 */
TEST_F(RestoreJobTest, SubTaskExecOriginalVolRestoreSucc)
{
    stubReadFunc = FileSystemReadSnapshotForRestore_Invoke;
    fsfileSizeRet = Stub_FileSystemReadSnapshotInfo().length();
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_GetVolumeHandler_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    stub.set(ADDR(DirtyRanges, FlushToStorage), FlushToStorageSucc);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_DISK), JobResult::SUCCESS);
    int ret = m_RestoreJob->ExecuteSubJob();
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * 用例名称：子任务测试块比特MAP加载以及检查有效数据是否正常，流程及功能测试
 * 前置条件：1. 恢复到新位置 2. 存在块比特MAP表 3. 段大小为20M 4. 源卷及目标卷大小为50M
 * check点: 执行子任务，流程返回SUCCESS
 */
TEST_F(RestoreJobTest, SubTaskExecute_CheckBlockBitMapOK)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_GetVolumeHandler_DiffDirtyRange);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_BitMapTest);
    stub.set(ADDR(VirtPlugin::VirtualizationBasicJob, GetSegementSizeFromConf), Stub_GetSegSizeFromConf);
    stub.set(ADDR(VirtualizationBasicJob, ReportTaskLabel), Stub_ReportTaskLabel_OK);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_DISK), JobResult::SUCCESS);
    const std::string filename = "/tmp/volumes_block_bitmap.info";
    std::string bitmap = "1111101100111";
    const std::shared_ptr<RepositoryHandler> fileHandler = std::make_shared<FileSystemHandler>();
    int ret = Utils::SaveToFile(fileHandler, filename, bitmap);
    m_targetVolObj.m_uuid = "1-1-1-1"; // diff from src vol uuid
    SetSubJob();
    ret = m_RestoreJob->ExecuteSubJob();
    EXPECT_EQ(ret, SUCCESS);
    stub.reset(ADDR(EngineFactory, CreateProtectEngine));
    stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
    stub.reset(ADDR(VirtualizationBasicJob, ReportTaskLabel));
}

/*
 * 用例名称：上电虚拟机失败导致导致PostSubJob失败
 * 前置条件：卷恢复
 * check点：PostSubJob返回FAIL
 */
TEST_F(RestoreJobTest, VolPostSubJobFail_PowerOnVmFail)
{
    powerOnMachineRet = FAILED;
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_DISK), JobResult::SUCCESS);
    SetSubJob("0", "1", "PostSubJob");
    EXPECT_EQ(m_RestoreJob->ExecuteSubJob(), FAILED);
}

/*
 * 用例名称：上电虚拟机失败导致导致PostSubJob失败
 * 前置条件：虚拟机恢复
 * check点：PostSubJob返回FAIL
 */
TEST_F(RestoreJobTest, VmRestorePostSubJobFail_PowerOnVmFail)
{
    powerOnMachineRet = FAILED;
    stubReadFunc = FileSystemReadVMandVolStub;
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
    SetSubJob("0", "1", "PostSubJob");
    EXPECT_EQ(m_RestoreJob->ExecuteSubJob(), FAILED);
}

/*
 * 用例名称：PostSubJob成功
 * 前置条件：所有调用接口均成功
 * check点：PostSubJob返回FAIL
 */
TEST_F(RestoreJobTest, VmRestorePostSubJobSucc)
{
    createMachineRet = SUCCESS;
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
    SetSubJob("0", "1", "PostSubJob");
    EXPECT_EQ(m_RestoreJob->ExecuteSubJob(), FAILED);
}

/*
 * 用例名称：卷恢复PostSubJob成功
 * 前置条件：所有接口调用成功
 * check点：PostSubJob返回SUCCESS
 */
TEST_F(RestoreJobTest, PostSubJobSucc)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_DISK), JobResult::SUCCESS, "0");
    SetSubJob("0", "1", "PostSubJob");
    EXPECT_EQ(m_RestoreJob->ExecuteSubJob(), FAILED);
}

/*
 * 用例名称：子任务挂载卷失败导致PostSubJob失败
 * 前置条件：应用AttachVolue调用失败
 * check点：PostSubJob返回FAILED
 */
TEST_F(RestoreJobTest, PostSubJob_attachVolFailed)
{
    attachVolumeRet = FAILED;
    stubReadFunc = FileSystemReadVolPair_Invoke;
    fsfileSizeRet = Stub_GetVolPairInfo().length();
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_DISK), JobResult::SUCCESS);
    SetSubJob("0", "1", "PostSubJob");
    EXPECT_EQ(m_RestoreJob->ExecuteSubJob(), FAILED);
}

static bool Stub_CommonInfoInit()
{
    return true;
}

static bool Stub_CommonInfoInit_Fail()
{
    return false;
}

static void CreateVolMatchPairInfo(std::string& contentStr, size_t& strSize)
{
    std::vector<VolInfo> originVolList;
    originVolList.resize(3);
    originVolList[0].m_slotId = "1";
    originVolList[0].m_datastore.m_type = "Config-vVol";
    originVolList[1].m_slotId = "2";
    originVolList[1].m_datastore.m_type = "Data-vVol";
    originVolList[2].m_slotId = "3";
    originVolList[2].m_datastore.m_type = "Mem-vVol";
    std::vector<VolInfo> targetVolList;
    targetVolList.resize(3);
    targetVolList[2].m_slotId = "1";
    targetVolList[2].m_datastore.m_type = "Config- vVol";
    targetVolList[0].m_slotId = "2";
    targetVolList[0].m_datastore.m_type = "Data-vVol";
    targetVolList[1].m_slotId = "3";
    targetVolList[1].m_datastore.m_type = "Mem-vVol";

    VolMatchPairInfo volPairInfo;
    volPairInfo.AddVolPair(originVolList[0], targetVolList[0]);
    volPairInfo.AddVolPair(originVolList[1], targetVolList[1]);
    volPairInfo.AddVolPair(originVolList[2], targetVolList[2]);

    Module::JsonHelper::StructToJsonString(volPairInfo, contentStr);
    strSize = contentStr.size();
}
 static bool Main_Task_Status_Exists_Invoke(const std::string &fileName)
 {
     if (fileName.find("main_task_status.info") != std::string::npos) {
         return false;
     }
     return true;
 }

// Stub for FileSystemHandler: vol-match-info read success
static std::shared_ptr<RepositoryHandler> Stub_CreateFSHandler_VolMatch_OK(
    const AppProtect::StorageRepository &storageRepo)
{
    std::string contentStr;
    size_t strSize;
    CreateVolMatchPairInfo(contentStr, strSize);

    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(strSize));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(DoAll(SetArgReferee<0>(contentStr), Return(strSize)));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(Main_Task_Status_Exists_Invoke)));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    return fsHandler;
}

// Stub for FileSystemHandler: vol-match-info read fail
static std::shared_ptr<RepositoryHandler> Stub_CreateFSHandler_VolMatch_Read_Fail(
    const AppProtect::StorageRepository &storageRepo)
{
    std::string contentStr;
    size_t strSize = -1;

    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(strSize));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(DoAll(SetArgReferee<0>(contentStr), Return(strSize)));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(Main_Task_Status_Exists_Invoke)));
    return fsHandler;
}

/*
 * 用例名称：分解任务调用公共的信息初始化失败
 * 前置条件：初始化失败
 * check点：分解任务失败
 */
TEST_F(RestoreJobTest, Gen_JobInitFail)
{
    stub.set(ADDR(VirtPlugin::RestoreJob, CommonInfoInit), Stub_CommonInfoInit_Fail);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
    EXPECT_EQ(m_RestoreJob->GenerateSubJob(), FAILED);
    stub.reset(ADDR(VirtPlugin::RestoreJob, CommonInfoInit));
}

/*
 * 用例名称：分解任务只执行Hook点，不执行框架流程
 * 前置条件：PreHook返回特殊流程错误码
 * check点：分解任务失败
 */
TEST_F(RestoreJobTest, Gen_DiffFlow)
{
    preHookRet = DIFFERENT_FLOW;
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_VolMatch_OK);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_DISK), JobResult::SUCCESS);
    EXPECT_EQ(m_RestoreJob->GenerateSubJob(), SUCCESS);
    stub.reset(ADDR(EngineFactory, CreateProtectEngine));
    stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
}

/*
 * 用例名称：分解任务获取不到vol-match-info
 * 前置条件：读不到vol-match-info
 * check点：分解任务失败
 */
TEST_F(RestoreJobTest, Gen_GetVolMatchPairInfoFail)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_VolMatch_Read_Fail);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
    EXPECT_EQ(m_RestoreJob->GenerateSubJob(), FAILED);
    stub.reset(ADDR(EngineFactory, CreateProtectEngine));
    stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
}

/*
 * 用例名称：分解任务上传拆分的子任务到框架失败
 * 前置条件：上传子任务失败
 * check点：分解任务失败
 */
TEST_F(RestoreJobTest, Gen_PutSubTasksToFrameFail)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_VolMatch_OK);
    stub.set(ADDR(JobService, AddNewJob), Stub_JobServiceAddNewJob_NO);
    stub.set(ADDR(JobService, ReportJobDetails), Stub_JobServiceReportJobDetails);
    stub.set(ADDR(Module::CTime, Now), NowTime);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_DISK), JobResult::SUCCESS);
    EXPECT_EQ(m_RestoreJob->GenerateSubJob(), FAILED);
    stub.reset(ADDR(EngineFactory, CreateProtectEngine));
    stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
    stub.reset(ADDR(JobService, AddNewJob));
    stub.reset(ADDR(JobService, ReportJobDetails));
}

/*
 * 用例名称：分解任务成功
 * 前置条件：成功
 * check点：分解任务成功
 */
TEST_F(RestoreJobTest, Gen_GenerateSubJob)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_VolMatch_OK);
    stub.set(ADDR(JobService, AddNewJob), Stub_JobServiceAddNewJob_OK);
    stub.set(ADDR(JobService, ReportJobDetails), Stub_JobServiceReportJobDetails);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_DISK), JobResult::SUCCESS);
    EXPECT_EQ(m_RestoreJob->GenerateSubJob(), SUCCESS);
    stub.reset(ADDR(EngineFactory, CreateProtectEngine));
    stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
    stub.reset(ADDR(JobService, AddNewJob));
    stub.reset(ADDR(JobService, ReportJobDetails));
}

/*
 * 用例名称：虚拟机恢复任务执行成功，后置任务执行成功
 * 前置条件：生产引擎所有操作成功
 * check点：PostJob返回SUCCESS
 */
TEST_F(RestoreJobTest, VMPostJobSucc_RestoreSucc)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
    EXPECT_EQ(m_RestoreJob->PostJob(), SUCCESS);
}

/*
 * 用例名称：虚拟机恢复任务执行失败，后置任务执行成功
 * 前置条件：生产引擎所有操作成功
 * check点：PostJob返回SUCCESS
 */
TEST_F(RestoreJobTest, VMPostJobSucc_RestoreFail)
{
    deleteMachineRet = SUCCESS;
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::FAILED);
    EXPECT_EQ(m_RestoreJob->PostJob(), FAILED);
}

/*
 * 用例名称：卷恢复任务执行成功，后置任务执行成功
 * 前置条件：生产引擎所有操作成功
 * check点：PostJob返回SUCCESS
 */
TEST_F(RestoreJobTest, VolPostJobSucc_RestoreSucc)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_DISK), JobResult::SUCCESS);
    EXPECT_EQ(m_RestoreJob->PostJob(), SUCCESS);
}

/*
 * 用例名称：卷恢复任务执行失败，后置任务执行成功
 * 前置条件：生产引擎所有操作成功
 * check点：PostJob返回SUCCESS
 */
TEST_F(RestoreJobTest, VolPostJobSucc_RestoreFail)
{
    stubReadFunc = FileSystemReadVolPair_Invoke;
    fsfileSizeRet = Stub_GetVolPairInfo().length();
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_DISK), JobResult::FAILED);
    EXPECT_EQ(m_RestoreJob->PostJob(), SUCCESS);
}

/*
 * 用例名称：后置删除新卷失败/成功
 * 前置条件：恢复任务失败/成功
 * check点：PostJob返回FAILED/SUCCESS
 */
TEST_F(RestoreJobTest, VolPostJob_DelVolFail)
{
    deleteVolumeRet = FAILED;
    stubReadFunc = FileSystemReadVolPair_Invoke;
    fsfileSizeRet = Stub_GetVolPairInfo().length();
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_DISK), JobResult::FAILED);
    EXPECT_EQ(m_RestoreJob->PostJob(), FAILED);
}

/*
 * 用例名称：后置挂载卷失败
 * 前置条件：恢复任务失败
 * check点：PostJob返回FAILED
 */
TEST_F(RestoreJobTest, VolPostJob_AttachVolFail)
{
    attachVolumeRet = FAILED;
    stubReadFunc = FileSystemReadVolPair_Invoke;
    fsfileSizeRet = Stub_GetVolPairInfo().length();
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_DISK), JobResult::FAILED);
    EXPECT_EQ(m_RestoreJob->PostJob(), FAILED);
}

/*
 * 用例名称：后置删除虚拟机失败
 * 前置条件：恢复任务失败
 * check点：PostJob返回FAILED
 */
TEST_F(RestoreJobTest, VolPostJob_DeleteVMFail)
{
    deleteMachineRet = FAILED;
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stubReadFunc = FileSystemReadVMInfo_Invoke;
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::FAILED);
    EXPECT_EQ(m_RestoreJob->PostJob(), FAILED);
}

int Stub_SaveToFileWithRetry_Failed(const std::shared_ptr<RepositoryHandler> &repoHandler, const std::string &filename, const std::string &content)
{
    static int SaveToFileWithRetryCount = 0;
    if (SaveToFileWithRetryCount == 0)
    {
        SaveToFileWithRetryCount++;
        return SUCCESS;
    }
    return FAILED;
}

/*
 * 用例名称：前置任务无法正确读取卷匹配对路径
 * 前置条件：
 * check点:
 */
TEST_F(RestoreJobTest, PrerequisiteJob_SaveToFileWithRetry_failed)
{
    fileExistStubFunc = CustomizeFileExistStub;
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
    stub.set(Utils::SaveToFileWithRetry, Stub_SaveToFileWithRetry_Failed);
    EXPECT_EQ(m_RestoreJob->PrerequisiteJob(), FAILED);
    stub.reset(Utils::SaveToFileWithRetry);
}

static std::shared_ptr<ProtectEngine> Stub_Call_ProtectEngine_GenVolPair_FAILED(const JobType &jobType,
    std::shared_ptr<JobCommonInfo> jobInfo)
{
    VolMatchPairInfo volPairs;
    std::shared_ptr<JobHandle> jobHandle = std::make_shared<JobHandle>(jobType, jobInfo);
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>(jobHandle);
    EXPECT_CALL(*protectEngine, PreHook(_)).WillRepeatedly(Return(preHookRet));
    EXPECT_CALL(*protectEngine, PostHook(_)).WillRepeatedly(Return(postHookRet));
    EXPECT_CALL(*protectEngine, CreateSnapshot(_,_)).WillRepeatedly(Return(createSnapShotRet));
    EXPECT_CALL(*protectEngine, DeleteSnapshot(_)).WillRepeatedly(Return(deleteSnapShotRet));
    EXPECT_CALL(*protectEngine, CreateMachine(_)).WillRepeatedly(Return(createMachineRet));
    EXPECT_CALL(*protectEngine, DeleteMachine(_)).WillRepeatedly(Return(deleteMachineRet));
    EXPECT_CALL(*protectEngine, PowerOnMachine(_)).WillRepeatedly(Return(powerOnMachineRet));
    EXPECT_CALL(*protectEngine, PowerOffMachine(_)).WillRepeatedly(Return(powerOffMachineRet));
    EXPECT_CALL(*protectEngine, GenVolPair(_,_,_,_)).WillRepeatedly(DoAll(SetArgReferee<3>(volPairs), Return(FAILED)));
    EXPECT_CALL(*protectEngine, CheckBeforeRecover(_)).WillRepeatedly(Return(checkBeforeRecoverRet));
    EXPECT_CALL(*protectEngine, DetachVolume(_)).WillRepeatedly(Return(detachVolumeRet));
    EXPECT_CALL(*protectEngine, AttachVolume(_)).WillRepeatedly(Return(attachVolumeRet));
    EXPECT_CALL(*protectEngine, DeleteVolume(_)).WillRepeatedly(Return(deleteVolumeRet));
    return protectEngine;
}

/*
 * 用例名称：前置任务无法正确读取卷匹配对路径
 * 前置条件：
 * check点:
 */
TEST_F(RestoreJobTest, PrerequisiteJob_GenVolPair_failed)
{
    fileExistStubFunc = CustomizeFileExistStub;
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_Call_ProtectEngine_GenVolPair_FAILED);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
    EXPECT_EQ(m_RestoreJob->PrerequisiteJob(), FAILED);
}

/*
 * 用例名称：前置任务无法正确读取卷匹配对路径
 * 前置条件：
 * check点:
 */
TEST_F(RestoreJobTest, PrerequisiteJob_FindVol_failed)
{
    fileExistStubFunc = CustomizeFileExistStub;
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_Call_ProtectEngine_GenVolPair_FAILED);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    AppProtect::RestoreJob vmRestoreParam;
    vmRestoreParam.jobId = "-1";
    vmRestoreParam.requestId = "000001";
    FormCopyData(vmRestoreParam);
    FormTargetEnv(vmRestoreParam);
    FormTargetObject(vmRestoreParam, "1");
    FormRestoreVolList(vmRestoreParam);
    FormRestoreAdvParam(vmRestoreParam, std::to_string(int(RestoreLevel::RESTORE_TYPE_VM)), "false");
    m_RestoreJob->SetPostJobResultType(JobResult::SUCCESS);
    std::shared_ptr <ThriftDataBase> restorePtr = std::make_shared<AppProtect::RestoreJob>(vmRestoreParam);
    std::shared_ptr <JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(restorePtr);
    m_RestoreJob->SetJobInfo(jobInfo);
    EXPECT_EQ(m_RestoreJob->PrerequisiteJob(), FAILED);
}
/*
 * 用例名称：前置任务无法正确读取卷匹配对路径
 * 前置条件：
 * check点:
 */
TEST_F(RestoreJobTest, GenFinalRestoreVolList_findvol_failed)
{
    static std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(Return(false));
    m_RestoreJob->m_cacheRepoHandler = fsHandler;
    VolInfo vol;
    m_RestoreJob->m_initialRestoreVolsMap["1"] = vol;
    AppProtect::RestoreJob REJobInfo;
    m_RestoreJob->m_restorePara = std::make_shared<AppProtect::RestoreJob>(REJobInfo);
    EXPECT_EQ(m_RestoreJob->GenFinalRestoreVolList(), FAILED);
}


/*
 * 用例名称：前置任务无法正确读取卷匹配对路径
 * 前置条件：
 * check点:
 */
TEST_F(RestoreJobTest, GenFinalRestoreVolList_Loadfile_failed)
{
    static std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(FAILED));
    m_RestoreJob->m_cacheRepoHandler = fsHandler;
    EXPECT_EQ(m_RestoreJob->GenFinalRestoreVolList(), FAILED);
}

/*
 * 用例名称：前置任务无法正确读取卷匹配对路径
 * 前置条件：
 * check点:
 */
TEST_F(RestoreJobTest, createMachine_restorevm_empty_failed)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    AppProtect::RestoreJob vmRestoreParam;
    vmRestoreParam.jobId = "1";
    vmRestoreParam.requestId = "000001";
    FormCopyData(vmRestoreParam);
    FormTargetEnv(vmRestoreParam);
    FormRestoreVolList(vmRestoreParam);
    FormRestoreAdvParam(vmRestoreParam, std::to_string(int(RestoreLevel::RESTORE_TYPE_VM)),"false");
    m_RestoreJob->SetPostJobResultType(JobResult::SUCCESS);
    std::shared_ptr <ThriftDataBase> restorePtr = std::make_shared<AppProtect::RestoreJob>(vmRestoreParam);
    std::shared_ptr <JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(restorePtr);
    m_RestoreJob->SetJobInfo(jobInfo);
    EXPECT_EQ(m_RestoreJob->PrerequisiteJob(), FAILED);
}

int Stub_Create_SaveToFileWithRetry_Failed(const std::shared_ptr<RepositoryHandler> &repoHandler, const std::string &filename, const std::string &content)
{
    return FAILED;
}
/*
 * 用例名称：前置任务无法正确读取卷匹配对路径
 * 前置条件：
 * check点:
 */
TEST_F(RestoreJobTest, createMachine_SaveToFile_failed)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
    stub.set(Utils::SaveToFileWithRetry, Stub_Create_SaveToFileWithRetry_Failed);
    EXPECT_EQ(m_RestoreJob->PrerequisiteJob(), FAILED);
    stub.reset(Utils::SaveToFileWithRetry);
}

static bool stub_LoadMetaData_failed()
{
    return false;
}

static bool stub_CheckMainTaskStatusFileExist_failed()
{
    return true;
}

/*
 * 用例名称：前置任务无法正确读取卷匹配对路径
 * 前置条件：
 * check点:
 */
TEST_F(RestoreJobTest, PrerequisiteJob_CheckMainTaskStatus_failed)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    stub.set(ADDR(VirtPlugin::RestoreJob, CheckMainTaskStatusFileExist), stub_CheckMainTaskStatusFileExist_failed);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
    EXPECT_EQ(m_RestoreJob->PrerequisiteJob(), SUCCESS);
    stub.reset(ADDR(VirtPlugin::RestoreJob, CheckMainTaskStatusFileExist));
}

/*
 * 用例名称：前置任务无法正确读取卷匹配对路径
 * 前置条件：
 * check点:
 */
// TEST_F(RestoreJobTest, PrerequisiteJob_LoadMetaData_failed)
// {
//     stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
//     stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
//     stub.set(ADDR(VirtPlugin::RestoreJob, LoadMetaData), stub_LoadMetaData_failed);
//     SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
//     EXPECT_EQ(m_RestoreJob->PrerequisiteJob(), FAILED);
//     stub.reset(ADDR(VirtPlugin::RestoreJob, LoadMetaData));
// }

/*
 * 用例名称：前置任务无法正确读取卷匹配对路径
 * 前置条件：
 * check点:
 */
TEST_F(RestoreJobTest, Inithandlers_restorepara_empty)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    AppProtect::RestoreJob vmRestoreParam;
    vmRestoreParam.jobId = "1";
    vmRestoreParam.requestId = "000001";
    std::shared_ptr <ThriftDataBase> restorePtr = std::make_shared<AppProtect::RestoreJob>(vmRestoreParam);
    std::shared_ptr <JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(restorePtr);
    m_RestoreJob->m_restorePara = std::dynamic_pointer_cast<AppProtect::RestoreJob>(jobInfo->GetJobInfo());
    EXPECT_EQ(m_RestoreJob->InitHandlers(), false);
}

int Stub_InitProtectEngineHandler_SUCCESS(JobType jobType)
{
    return SUCCESS;
}

bool Stub_DoInitHandlers_FAILED(const AppProtect::StorageRepository &storageRepo,
    std::shared_ptr<RepositoryHandler> &repoHandler, std::string &repoPath)
{
    return false;
}
/*
 * 用例名称：前置任务无法正确读取卷匹配对路径
 * 前置条件：
 * check点:
 */
TEST_F(RestoreJobTest, Inithandlers_datainit_failed)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    stub.set(ADDR(VirtualizationBasicJob, DoInitHandlers), Stub_DoInitHandlers_FAILED);
    stub.set(ADDR(VirtualizationBasicJob, InitProtectEngineHandler), Stub_InitProtectEngineHandler_SUCCESS);
    AppProtect::RestoreJob vmRestoreParam;
    vmRestoreParam.jobId = "1";
    vmRestoreParam.requestId = "000001";
    AppProtect::Copy copy;
    copy.id = "001";
    StorageRepository dataRepo;
    dataRepo.repositoryType = RepositoryDataType::DATA_REPOSITORY;
    dataRepo.path.push_back("/datapath1");
    copy.repositories.push_back(dataRepo);
    vmRestoreParam.copies.push_back(copy);
    std::shared_ptr <ThriftDataBase> restorePtr = std::make_shared<AppProtect::RestoreJob>(vmRestoreParam);
    std::shared_ptr <JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(restorePtr);
    m_RestoreJob->m_restorePara = std::dynamic_pointer_cast<AppProtect::RestoreJob>(jobInfo->GetJobInfo());
    EXPECT_EQ(m_RestoreJob->InitHandlers(), false);
    stub.reset(ADDR(VirtualizationBasicJob, DoInitHandlers));
    stub.reset(ADDR(VirtualizationBasicJob, InitProtectEngineHandler));
}

/*
 * 用例名称：前置任务无法正确读取卷匹配对路径
 * 前置条件：
 * check点:
 */
TEST_F(RestoreJobTest, Inithandlers_metainit_failed)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    stub.set(ADDR(VirtualizationBasicJob, DoInitHandlers), Stub_DoInitHandlers_FAILED);
    stub.set(ADDR(VirtualizationBasicJob, InitProtectEngineHandler), Stub_InitProtectEngineHandler_SUCCESS);
    AppProtect::RestoreJob vmRestoreParam;
    vmRestoreParam.jobId = "1";
    vmRestoreParam.requestId = "000001";
    AppProtect::Copy copy;
    copy.id = "001";
    StorageRepository metaRepo;
    metaRepo.repositoryType = RepositoryDataType::META_REPOSITORY;
    metaRepo.path.push_back("/metapath1");
    copy.repositories.push_back(metaRepo);
    vmRestoreParam.copies.push_back(copy);
    std::shared_ptr <ThriftDataBase> restorePtr = std::make_shared<AppProtect::RestoreJob>(vmRestoreParam);
    std::shared_ptr <JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(restorePtr);
    m_RestoreJob->m_restorePara = std::dynamic_pointer_cast<AppProtect::RestoreJob>(jobInfo->GetJobInfo());
    EXPECT_EQ(m_RestoreJob->InitHandlers(), false);
    stub.reset(ADDR(VirtualizationBasicJob, DoInitHandlers));
    stub.reset(ADDR(VirtualizationBasicJob, InitProtectEngineHandler));
}

/*
 * 用例名称：前置任务无法正确读取卷匹配对路径
 * 前置条件：
 * check点:
 */
TEST_F(RestoreJobTest, Inithandlers_cacheinit_failed)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    stub.set(ADDR(VirtualizationBasicJob, DoInitHandlers), Stub_DoInitHandlers_FAILED);
    stub.set(ADDR(VirtualizationBasicJob, InitProtectEngineHandler), Stub_InitProtectEngineHandler_SUCCESS);
    AppProtect::RestoreJob vmRestoreParam;
    vmRestoreParam.jobId = "1";
    vmRestoreParam.requestId = "000001";
    AppProtect::Copy copy;
    copy.id = "001";
    StorageRepository cacheRepo;
    cacheRepo.repositoryType = RepositoryDataType::CACHE_REPOSITORY;
    cacheRepo.path.push_back("/cachepath1");
    copy.repositories.push_back(cacheRepo);
    vmRestoreParam.copies.push_back(copy);
    std::shared_ptr <ThriftDataBase> restorePtr = std::make_shared<AppProtect::RestoreJob>(vmRestoreParam);
    std::shared_ptr <JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(restorePtr);
    m_RestoreJob->m_restorePara = std::dynamic_pointer_cast<AppProtect::RestoreJob>(jobInfo->GetJobInfo());
    EXPECT_EQ(m_RestoreJob->InitHandlers(), false);
    stub.reset(ADDR(VirtualizationBasicJob, DoInitHandlers));
    stub.reset(ADDR(VirtualizationBasicJob, InitProtectEngineHandler));
}

/*
 * 用例名称：前置任务无法正确读取卷匹配对路径
 * 前置条件：
 * check点:
 */
TEST_F(RestoreJobTest, Inithandlers_cache_empty)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    AppProtect::RestoreJob vmRestoreParam;
    vmRestoreParam.jobId = "1";
    vmRestoreParam.requestId = "000001";
    AppProtect::Copy copy;
    copy.id = "001";
    // 数据仓
    StorageRepository dataRepo;
    dataRepo.repositoryType = RepositoryDataType::DATA_REPOSITORY;
    dataRepo.path.push_back("/datapath1");
    copy.repositories.push_back(dataRepo);
    // 元数据仓
    StorageRepository metaRepo;
    metaRepo.repositoryType = RepositoryDataType::META_REPOSITORY;
    metaRepo.path.push_back("/metapath1");
    copy.repositories.push_back(metaRepo);
    vmRestoreParam.copies.push_back(copy);
    std::shared_ptr <ThriftDataBase> restorePtr = std::make_shared<AppProtect::RestoreJob>(vmRestoreParam);
    std::shared_ptr <JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(restorePtr);
    m_RestoreJob->m_restorePara = std::dynamic_pointer_cast<AppProtect::RestoreJob>(jobInfo->GetJobInfo());
    EXPECT_EQ(m_RestoreJob->InitHandlers(), false);
}

static bool Stub_JsonStringToJsonValue_tf(const std::string& jsonString, Json::Value& value)
{
    static int jsoncount = 0;
    if (jsoncount == 0)
    {
        jsoncount++;
        return Module::JsonHelper::JsonStringToJsonValue(jsonString, value);
    }
    else
    {
        jsoncount++;
        return false;
    }
}

static bool Stub_JsonStringToJsonValue_f(const std::string& jsonString, Json::Value& value)
{
   return false;
}

/*
 * 用例名称：前置任务无法正确读取卷匹配对路径
 * 前置条件：
 * check点:
 */
TEST_F(RestoreJobTest, InitRestoreParams_restorepara_empty)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    AppProtect::RestoreJob vmRestoreParam;
    vmRestoreParam.jobId = "1";
    vmRestoreParam.requestId = "000001";
    std::shared_ptr <ThriftDataBase> restorePtr = std::make_shared<AppProtect::RestoreJob>(vmRestoreParam);
    std::shared_ptr <JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(restorePtr);
    m_RestoreJob->m_restorePara = std::dynamic_pointer_cast<AppProtect::RestoreJob>(jobInfo->GetJobInfo());
    EXPECT_EQ(m_RestoreJob->InitRestoreParams(), false);
}

/*
 * 用例名称：前置任务无法正确读取卷匹配对路径
 * 前置条件：
 * check点:
 */
TEST_F(RestoreJobTest, InitRestoreParams_targetobjinfo_failed)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    stub.set(ADDR(Module::JsonHelper, JsonStringToJsonValue), Stub_JsonStringToJsonValue_f);
    AppProtect::RestoreJob vmRestoreParam;
    vmRestoreParam.jobId = "1";
    vmRestoreParam.requestId = "000001";
    FormCopyData(vmRestoreParam);
    FormTargetEnv(vmRestoreParam);
    FormTargetObject(vmRestoreParam, "1");
    FormRestoreVolList(vmRestoreParam);
    FormRestoreAdvParam(vmRestoreParam, std::to_string(int(RestoreLevel::RESTORE_TYPE_VM)), "false");
    m_RestoreJob->SetPostJobResultType(JobResult::SUCCESS);
    std::shared_ptr <ThriftDataBase> restorePtr = std::make_shared<AppProtect::RestoreJob>(vmRestoreParam);
    std::shared_ptr <JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(restorePtr);
    m_RestoreJob->SetJobInfo(jobInfo);
    m_RestoreJob->m_restorePara = std::dynamic_pointer_cast<AppProtect::RestoreJob>(jobInfo->GetJobInfo());
    EXPECT_EQ(m_RestoreJob->InitRestoreParams(), false);
}

/*
 * 用例名称：前置任务无法正确读取卷匹配对路径
 * 前置条件：
 * check点:
 */
TEST_F(RestoreJobTest, InitRestoreParams_jobAdvance_failed)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), SStub_Call_ProtectEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
    stub.set(ADDR(Module::JsonHelper, JsonStringToJsonValue), Stub_JsonStringToJsonValue_tf);
    AppProtect::RestoreJob vmRestoreParam;
    vmRestoreParam.jobId = "1";
    vmRestoreParam.requestId = "000001";
    FormCopyData(vmRestoreParam);
    FormTargetEnv(vmRestoreParam);
    FormTargetObject(vmRestoreParam, "1");
    FormRestoreVolList(vmRestoreParam);
    FormRestoreAdvParam(vmRestoreParam, std::to_string(int(RestoreLevel::RESTORE_TYPE_VM)), "false");
    m_RestoreJob->SetPostJobResultType(JobResult::SUCCESS);
    std::shared_ptr <ThriftDataBase> restorePtr = std::make_shared<AppProtect::RestoreJob>(vmRestoreParam);
    std::shared_ptr <JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    jobInfo->SetJobInfo(restorePtr);
    m_RestoreJob->SetJobInfo(jobInfo);
    m_RestoreJob->m_restorePara = std::dynamic_pointer_cast<AppProtect::RestoreJob>(jobInfo->GetJobInfo());
    EXPECT_EQ(m_RestoreJob->InitRestoreParams(), false);
    stub.reset(ADDR(Module::JsonHelper, JsonStringToJsonValue));
}
}
