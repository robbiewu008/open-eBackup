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
#include <cstdio>
#include <openssl/evp.h>
#include "curl_http/CodeConvert.h"
#include "job_controller/jobs/restore/RestoreJob.h"
#include "ApplicationProtectPlugin_types.h"
#include "ApplicationProtectBaseDataType_types.h"
#include "protect_engines/ProtectEngineMock.h"
#include "volume_handlers/volumeHandlerMock.h"
#include "common/DirtyRanges.h"
#include "common/utils/Utils.h"
#include "repository_handlers/mock/FileSystemHandlerMock.h"
#include "repository_handlers/factory/RepositoryFactory.h"
#include <repository_handlers/mock/FileSystemHandlerMock.h>
#include "ClientInvoke.h"
#include <sstream>
#include <fstream>
#include <string.h>
#include "job_controller/jobs/restore/ArchiveRestoreJob.h"
#include "ArchiveStreamService.h"
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
const int BUFF_SIZE = 10;
const std::string volMetaData = "{\
    \"uuid\":\"1\",\
    \"moRef\":\"1-1\",\
}";
const std::string gSnapshotInfo = "{\
    \"createTime\":\"\",\
    \"moRef\":\"\",\
    \"preMoRef\":\"\",\
    \"reserved\":false,\
    \"snapshotName\":\"VIRTUAL_PLUGIN_5752D641-0A27-49B5-A0DA-FC2B74A535F1_1654402480\",\
    \"vmName\":\"\",\
    \"vmUuid\":\"\"\
}";

static int32_t FileSystemReadSnapshotSuccess_Invoke(std::string &buf, size_t size)
{
    buf = gSnapshotInfo;
    return gSnapshotInfo.length();
}

static int32_t FormVolStructListSuccess_Invoke(std::unordered_map<std::string, std::string> &volMetaData, 
                                               std::vector<VolInfo> &volList)
{
    VolInfo vol;
    vol.m_uuid = "1";
    vol.m_moRef = "1-1";
    volList.push_back(vol);
    return SUCCESS;
}

static int32_t FileSystemReadMetaDataSuccess_Invoke(std::string &buf, size_t size)
{
    buf = volMetaData;
    return volMetaData.length();
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

static int32_t FileSystemWriteSuccess_Invoke(const std::string &content)
{
    return content.length();
}

static int32_t RunShellCmdWithOutputSucc(const severity_level &severity, const std::string &moduleName,
    const std::size_t &requestID, const std::string &cmd, const std::vector<std::string> params,
    std::vector<std::string> &cmdoutput, std::string &stderroutput)
{
    cmdoutput.push_back("ret_code=0");
    cmdoutput.push_back("ret_desc=succ");
    cmdoutput.push_back("dev_addr=/tmp/virtual/");
    cmdoutput.push_back("ResultCode=0");
    cmdoutput.push_back("snapSize=123");
    cmdoutput.push_back("blockSize=123");
    cmdoutput.push_back("volSize=123");
    cmdoutput.push_back("status=0");
    return SUCCESS;
}

static int ArchiveStreamService_Init_SUCCESS(void* obj, const mp_string &backupId, const mp_string &taskID, const mp_string &dirList)
{
    return SUCCESS;
}

static int ArchiveStreamService_Init_FAILED(void* obj, const mp_string &backupId, const mp_string &taskID, const mp_string &dirList)
{
    return MP_FAILED;
}

static int ArchiveStreamService_Connect_SUCCESS(void* obj, const mp_string &busiIp, mp_int32 busiPort, bool openSsl)
{
    return SUCCESS;
}

static int ArchiveStreamService_Connect_FAILED(void* obj, const mp_string &busiIp, mp_int32 busiPort, bool openSsl)
{
    return FAILED;
}

static int ArchiveStreamService_DisConnect_SUCCESS(void* obj, const mp_string &busiIp, mp_int32 busiPort, bool openSsl)
{
    return SUCCESS;
}

static int ArchiveStreamService_PrepareRecovery_SUCCESS(void* obj, mp_string &metaFileDir)
{
    return SUCCESS;
}
static int ArchiveStreamService_PrepareRecovery_Failed(void* obj, mp_string &metaFileDir)
{
    return FAILED;
}
static int ArchiveStreamService_QueryPrepareStatus_SUCCESS(void* obj, mp_int32 &state)
{
    state = 1;
    return SUCCESS;
}
static int CommonInfoInit_SUCCESS(void* obj)
{
    return true;
}
static int CopyFileToCacheRepo_SUCCESS(void* obj, const std::string& file)
{
    return SUCCESS;
}
static int InitArchiveClient_SUCCESS(void* obj)
{
    return SUCCESS;
}
static int PrepareS3Client_SUCCESS(void* obj)
{
    return SUCCESS;
}

static int CloseArchiveClient_SUCCESS(void* obj)
{
    return SUCCESS;
}

static int LoadMetaData_SUCCESS(void* obj)
{
    return SUCCESS;
}

static int GenInitialRestoreVolList_SUCCESS(void* obj)
{
    return SUCCESS;
}
static int PrerequisitePreHook_SUCCESS(void* obj)
{
    return SUCCESS;
}

static int CreateAndPowerOffMachine_SUCCESS(void* obj)
{
    return SUCCESS;
}

static int GenFinalRestoreVolList_SUCCESS(void* obj)
{
    return SUCCESS;
}

static int DetachVolume_SUCCESS(void* obj)
{
    return SUCCESS;
}
static int PrerequisitePostHook_SUCCESS(void* obj)
{
    return SUCCESS;
}
static int CheckBeforeRecover_SUCCESS(void* obj)
{
    return SUCCESS;
}
static int GetCloudPathList_SUCCESS(void* obj)
{
    return SUCCESS;
}
static bool LoadArchiveMetaData_SUCCESS(void* obj)
{
    return true;
}
static bool InitS3Info_SUCCESS(void* obj)
{
    return true;
}
static int Stub_GetRecoverObjectList_SUCCESS_STATUS_2(void* obj, int64_t readCountLimit, std::string &checkpoint, std::string &splitFile,
        int64_t &objectNum, int32_t &status) {
    status = 2;
    checkpoint = "ll";
    splitFile = "/control_llttest.txt";
    return SUCCESS;
}
static int Stub_EndRecover_TRUE(void*)
{
    return SUCCESS;
}
static bool Stub_DecodeBase64(const uint64_t& bufferSize, const std::string& in, std::string& out)
{
    out = in;
    return true;
}
static bool Stub_GetSnapShotInfo_Success(void* obj, VolSnapInfo &snapInfo)
{
    VolSnapInfo m_snapInfo;
    m_snapInfo.m_snapshotId = "uuid";
    snapInfo = m_snapInfo;
    return true;
}
bool Dirtyranges_FlushSucc()
{
    return true;
}
static int Stub_GetFileData_SUCCESS(ArchiveStreamGetFileReq& req, ArchiveStreamGetFileRsq& retValue)
{
    retValue.readEnd = 1;
    /* char *res;
    res = (char*)calloc(1, 10);
    *res = "data";
    retValue.data = res; */
    return SUCCESS;
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
class ArchiveRestoreJobTest : public testing::Test {
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
    void FormTargetObject(AppProtect::RestoreJob &vmRestoreParam)
    {
        vmRestoreParam.targetObject.__set_id("147");
        vmRestoreParam.targetObject.__set_name("test_123456");
        vmRestoreParam.targetObject.__set_extendInfo("{\"targetFolderLocation\":\"d0\", \"powerState\":\"1\",\
        \"computeResourceMoRef\":\"148\",\"storagePoolType\":5986, \"metaData\":\"a\", \
        \"ifStartNetworkAdapter\":1, \"hostMoRef\":\"123456789\", \
        \"restorePlace\":1,\
        \"networkList\": [{\"adapterName\":\"adapter1\", \"networkName\": \"net1\", \"networkMoRef\":\"1\"}]}");
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

    void FormRestoreAdvParam(AppProtect::RestoreJob &vmRestoreParam, std::string RestoreLevel)
    {
        std::stringstream params;
        params << "{\"restoreLevel\":" << RestoreLevel << "}";
        vmRestoreParam.__set_extendInfo(params.str()); // restoreLevel: 0-虚拟机恢复 1-卷恢复
    }

    void SetRestoreParam(int RestoreLevel, const JobResult::type RestoreJobResult)
    {
        AppProtect::RestoreJob vmRestoreParam;
        vmRestoreParam.jobId = "1";
        vmRestoreParam.requestId = "000001";
        FormCopyData(vmRestoreParam);
        FormTargetEnv(vmRestoreParam);
        FormTargetObject(vmRestoreParam);
        FormRestoreVolList(vmRestoreParam);
        FormRestoreAdvParam(vmRestoreParam, std::to_string(RestoreLevel));
        m_ArchiveRestoreJob->SetPostJobResultType(RestoreJobResult);
        std::shared_ptr<ThriftDataBase> restorePtr = std::make_shared<AppProtect::RestoreJob>(vmRestoreParam);
        std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
        jobInfo->SetJobInfo(restorePtr);
        m_ArchiveRestoreJob->SetJobInfo(jobInfo);
    }
    void SetSubJob()
    {
         // 获取子任务
        AppProtect::SubJob subJob;
        subJob.jobId = "00000000-0000-0000-0000-000000000000";
        subJob.subJobId = "11111111-1111-1111-1111-111111111111";
        subJob.jobName = "XYZ";
        subJob.jobPriority = 1;
        VolInfo volInfo;
        volInfo.m_moRef = "moref";
        volInfo.m_uuid = "uuid";
        volInfo.m_name = "name";
        volInfo.m_type = "type";
        volInfo.m_datastore.m_type = "storage";
        volInfo.m_datastore.m_moRef = "ds_moref";
        volInfo.m_datastore.m_name = "ds_name";
        volInfo.m_volSizeInBytes = 0;
        std::string volStr;
        Module::JsonHelper::StructToJsonString(volInfo, volStr);
        SubJobExtendInfo subJobExtendInfo;
        subJobExtendInfo.m_subTaskType = 1;
        subJobExtendInfo.m_targetVolumeInfo = volStr;
        subJobExtendInfo.m_originVolumeInfo = volStr;
        std::string subJobExtendInfoStr;
        Module::JsonHelper::StructToJsonString(subJobExtendInfo, subJobExtendInfoStr);
        subJob.jobInfo = subJobExtendInfoStr;
        std::shared_ptr<AppProtect::SubJob> subJobInfo = std::make_shared<AppProtect::SubJob>(subJob);
        // 获取任务
        m_ArchiveRestoreJob->SetSubJob(subJobInfo);
    }

    static void ReportJobDetailsSuc(void *obj, ActionResult& returnValue, const SubJobDetails& jobInfo)
    {
        return;
    }


    void SetUp()
    {
        m_ArchiveRestoreJob = std::make_unique<VirtPlugin::ArchiveRestoreJob>();
        SetSubJob();
        preHookRet = SUCCESS;
        postHookRet = SUCCESS;
        createSnapShotRet = SUCCESS;
        deleteSnapShotRet = SUCCESS;
        createMachineRet = SUCCESS;
        deleteMachineRet = SUCCESS;
        powerOnMachineRet = SUCCESS;
        stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
        ArchiveS3Info s3Info;
        ArchiveS3Info::IpPort ipPort;
        ipPort.ip = "8.40.9.9";
        ipPort.port = 8088;
        s3Info.serviceInfo.push_back(ipPort);
        m_ArchiveRestoreJob->m_s3Info = s3Info;
        stub.set(ADDR(ArchiveRestoreJob, InitS3Info), InitS3Info_SUCCESS);
        stub.set(ADDR(ArchiveStreamService, Connect), ArchiveStreamService_Connect_SUCCESS);
        stub.set(ADDR(ArchiveStreamService, PrepareRecovery), ArchiveStreamService_PrepareRecovery_SUCCESS);
        stub.set(ADDR(ArchiveStreamService, QueryPrepareStatus), ArchiveStreamService_QueryPrepareStatus_SUCCESS);
        stub.set(Module::runShellCmdWithOutput, RunShellCmdWithOutputSucc);
        stub.set(ADDR(ArchiveStreamService, GetRecoverObjectList), Stub_GetRecoverObjectList_SUCCESS_STATUS_2);
        stub.set(ADDR(Module, CodeConvert::DecodeBase64), Stub_DecodeBase64);
    }

    void TearDown()
    {
        stub.reset(ADDR(EngineFactory, CreateProtectEngine));
        stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
    }
    std::unique_ptr<VirtPlugin::ArchiveRestoreJob> m_ArchiveRestoreJob;
    Stub stub;
private:
    typedef int (*JobServiceMethodPtr)(JobService*);
    JobServiceMethodPtr JobService_ReportJobDetails = (JobServiceMethodPtr)(&JobService::ReportJobDetails);
};

static std::shared_ptr<ProtectEngine> Stub_Call_ProtectEngine_OK(const JobType &jobType,
    std::shared_ptr<JobCommonInfo> jobInfo)
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
    std::shared_ptr<VolumeHandlerMock> volHandler = std::make_shared<VolumeHandlerMock>(jobHandle, originalVol);
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
    EXPECT_CALL(*protectEngine, GetVolumeHandler(_,_)).WillRepeatedly(DoAll(SetArgReferee<1>(volHandler),
        Return(SUCCESS)));
    EXPECT_CALL(*protectEngine, DetachVolume(_)).WillRepeatedly(Return(detachVolumeRet));
    EXPECT_CALL(*protectEngine, AttachVolume(_)).WillRepeatedly(Return(attachVolumeRet));
    EXPECT_CALL(*protectEngine, DeleteVolume(_)).WillRepeatedly(Return(deleteVolumeRet));
    return protectEngine;
}

static std::shared_ptr<RepositoryHandler> Stub_CreateFSHandler(
    const AppProtect::StorageRepository &storageRepo)
{
    static std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(fsOpenRet));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(fsCloseRet));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(fsfileSizeRet));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(FileSystemReadMetaDataSuccess_Invoke));
    EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(FileExistStub)));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(fsRemoveRet));
    EXPECT_CALL(*fsHandler, CreateDirectory(_)).WillRepeatedly(Return(fsCreateDirRet));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    return fsHandler;
}
static void Stub_AddNewJob_SUCCESS(ActionResult& returnValue, const std::vector<SubJob>& job)
{
    returnValue.code = 0;
    return;
}
/**
 * 用例名称：虚拟机恢复前置任务成功
 * 前置条件：无
 * check点：返回SUCCESS
 */
// TEST_F(ArchiveRestoreJobTest, GenerateSubjob_SUCCESS)
// {
//     stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_Call_ProtectEngine_OK);
//     stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
//     SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
//     stub.set(ADDR(JobService, AddNewJob), Stub_AddNewJob_SUCCESS);
//     EXPECT_EQ(m_ArchiveRestoreJob->GenerateSubJob(), SUCCESS);
//     stub.reset(ADDR(JobService, AddNewJob));
//     stub.reset(ADDR(EngineFactory, CreateProtectEngine));
//     stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
// }

/**
 * 用例名称：虚拟机恢复前置任务成功
 * 前置条件：无
 * check点：返回SUCCESS
 */
// TEST_F(ArchiveRestoreJobTest, PrerequisiteJob_VM_Success)
// {
//     std::ofstream fout;
//     fout.open("/control_llttest.txt", ios::out);
//     fout << "f,dm,0,20230728095100_51.10.97.67_mysql-bin.000002,0,13206,120,0" << std::endl;
//     fout.close();
//     stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_Call_ProtectEngine_OK);
//     stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
//     SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
//     EXPECT_EQ(m_ArchiveRestoreJob->PrerequisiteJob(), SUCCESS);
//     remove("/control_llttest.txt");
//     stub.reset(ADDR(EngineFactory, CreateProtectEngine));
//     stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
// }

/**
 * 用例名称：虚拟机恢复前置任务因未找到control文件失败
 * 前置条件：无
 * check点：返回SUCCESS
 */
// TEST_F(ArchiveRestoreJobTest, PrerequisiteJob_VM_Control_Failed)
// {
//     stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_Call_ProtectEngine_OK);
//     stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
//     SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
//     EXPECT_EQ(m_ArchiveRestoreJob->PrerequisiteJob(), FAILED);
//     stub.reset(ADDR(EngineFactory, CreateProtectEngine));
//     stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
// } 
/**
 * 用例名称：虚拟机恢复前置任务未成功建立连接，恢复失败
 * 前置条件：无
 * check点：返回SUCCESS
 */
// TEST_F(ArchiveRestoreJobTest, PrerequisiteJob_VM_Connect_Failed)
// {
//     stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_Call_ProtectEngine_OK);
//     stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
//     stub.set(ADDR(ArchiveStreamService, Connect), ArchiveStreamService_Connect_SUCCESS);
//     SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
//     EXPECT_EQ(m_ArchiveRestoreJob->PrerequisiteJob(), FAILED);
//     stub.reset(ADDR(EngineFactory, CreateProtectEngine));
//     stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
//     stub.reset(ADDR(ArchiveStreamService, Connect));
// }

/**
 * 用例名称：虚拟机恢复前置任务未成功初始化S3信息，恢复失败
 * 前置条件：无
 * check点：返回SUCCESS
 */
// TEST_F(ArchiveRestoreJobTest, PrerequisiteJob_VM_S3InitFailed_Failed)
// {
//     stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_Call_ProtectEngine_OK);
//     stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
//     stub.set(ADDR(ArchiveRestoreJob, InitS3Info), InitS3Info_SUCCESS);
//     SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
//     EXPECT_EQ(m_ArchiveRestoreJob->PrerequisiteJob(), FAILED);
//     stub.reset(ADDR(EngineFactory, CreateProtectEngine));
//     stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
//     stub.reset(ADDR(ArchiveRestoreJob, InitS3Info));
// }
/**
 * 用例名称：执行子任务成功
 * 前置条件：前置任务成功
 * check点：返回SUCCESS
 */
// TEST_F(ArchiveRestoreJobTest, ExecuteSubJob_Success)
// {
//     stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_Call_ProtectEngine_OK);
//     stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
//     stub.set(ADDR(VirtPlugin::RestoreJob, GetSnapshotInfo), Stub_GetSnapShotInfo_Success);
//     stub.set(ADDR(DirtyRanges, FlushToStorage), Dirtyranges_FlushSucc);
//     SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
//     EXPECT_EQ(m_ArchiveRestoreJob->ExecuteSubJob(), SUCCESS);
//     stub.reset(ADDR(EngineFactory, CreateProtectEngine));
//     stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
//     stub.reset(ADDR(VirtPlugin::RestoreJob, GetSnapshotInfo));
//     stub.reset(ADDR(DirtyRanges, FlushToStorage));
// }
/**
 * 用例名称：执行后置任务成功
 * 前置条件：任务成功
 * check点：返回SUCCESS
 */
// TEST_F(ArchiveRestoreJobTest, PostJob_Success)
// {
//     stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_Call_ProtectEngine_OK);
//     stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler);
//     stub.set(ADDR(ArchiveStreamService, EndRecover), Stub_EndRecover_TRUE);
//     stub.set(ADDR(VirtPlugin::RestoreJob, GetSnapshotInfo), Stub_GetSnapShotInfo_Success);
//     stub.set(ADDR(DirtyRanges, FlushToStorage), Dirtyranges_FlushSucc);
//     SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
//     EXPECT_EQ(m_ArchiveRestoreJob->PostJob(), SUCCESS);
//     stub.reset(ADDR(VirtPlugin::RestoreJob, GetSnapshotInfo));
//     stub.reset(ADDR(DirtyRanges, FlushToStorage));
//     stub.reset(ADDR(ArchiveStreamService, EndRecover));
//     stub.reset(ADDR(EngineFactory, CreateProtectEngine));
//     stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
// }

/**
 * 用例名称：初始化s3信息成功
 * 前置条件：任务下发成功
 * check点：返回SUCCESS
 */

/* TEST_F(ArchiveRestoreJobTest, InitArchiveClient_Success)
{
    ArchiveS3Info s3Info;
    ArchiveS3Info::IpPort ipPort;
    ipPort.ip = "8.40.9.9";
    ipPort.port = 8088;
    s3Info.serviceInfo.push_back(ipPort);
    m_ArchiveRestoreJob->m_s3Info = s3Info;
    stub.set(ADDR(ArchiveRestoreJob, InitS3Info), InitS3Info_SUCCESS);
    stub.set(ADDR(ArchiveStreamService, Connect), ArchiveStreamService_Connect_SUCCESS);
    SetRestoreParam(int(RestoreLevel::RESTORE_TYPE_VM), JobResult::SUCCESS);
    EXPECT_EQ(m_ArchiveRestoreJob->InitArchiveClient(), true);
    stub.reset(ADDR(ArchiveRestoreJob, InitS3Info));
    stub.reset(ADDR(ArchiveStreamService, Connect));   
}  */

/**
 * 用例名称：初始化s3信息成功
 * 前置条件：任务下发成功
 * check点：返回SUCCESS
 */
TEST_F(ArchiveRestoreJobTest, CloseArchiveClient_Success)
{
    m_ArchiveRestoreJob->isInitArchiveClient = false;
    Stub stub;
    EXPECT_EQ(m_ArchiveRestoreJob->CloseArchiveClient(), true);
}

/**
 * 用例名称：初始化s3信息成功
 * 前置条件：任务下发成功
 * check点：返回SUCCESS
 */

/* TEST_F(ArchiveRestoreJobTest, PrepareS3Client_Success)
{
    Stub stub;
    stub.set(ADDR(ArchiveStreamService, PrepareRecovery), ArchiveStreamService_PrepareRecovery_Failed);
    stub.set(ADDR(ArchiveStreamService, QueryPrepareStatus), ArchiveStreamService_QueryPrepareStatus_SUCCESS);
    stub.set(ADDR(ArchiveStreamService, Disconnect), ArchiveStreamService_Connect_SUCCESS);
    stub.set(ADDR(ArchiveRestoreJob, CopyFileToCacheRepo), CopyFileToCacheRepo_SUCCESS);
    EXPECT_EQ(m_ArchiveRestoreJob->PrepareS3Client(), false);
} */

}
