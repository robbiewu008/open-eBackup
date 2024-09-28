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
#include <vector>
#include <utility>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"

#include <securec.h>
#include <job/JobCommonInfo.h>
#include "ClientInvoke.h"
#include <log/Log.h>

#include <common/Macros.h>
#include <common/Structs.h>
#include <common/DirtyRanges.h>
#include "common/CTime.h"
#include "common/CommonMock.h"
#include <job_controller/jobs/backup/BackupJob.h>
#include <protect_engines/engine_factory/EngineFactory.h>
#include <repository_handlers/RepositoryHandler.h>
#include <repository_handlers/factory/RepositoryFactory.h>

#include <protect_engines/ProtectEngineMock.h>
#include <repository_handlers/mock/FileSystemHandlerMock.h>
#include <repository_handlers/mock/FileSystemMock.h>
#include <volume_handlers/volumeHandlerMock.h>
#include "common/sha256/Sha256.h"
#include "repository_handlers/filesystem/FileSystemHandler.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;
using ::testing::Invoke;
using ::testing::Matcher;
using ::testing::A;

using namespace VirtPlugin;
namespace {
const std::string MODULE_NAME = "BackupJobTest";
const int32_t MAX_TEST_SIZE = 100;
const std::string DEFAULT_META_PATH = "/tmp/meta_repo";
const std::string DEFAULT_UUID = "1111-2222-333";
}

namespace HDT_TEST {
class ProtectEngineMock;
using FuncPtr = int32_t (*)(ProtectEngineMock*);
const std::string g_volumeContent = "FILE CONTENT IN THE TEST FILE.";

class BackupJobTest : public testing::Test {
public:
    void SetUp();

    void SetTestPara()
    {
        AppProtect::StorageRepository metaRepo;
        metaRepo.repositoryType = RepositoryDataType::META_REPOSITORY;
        metaRepo.path.push_back(DEFAULT_META_PATH);
        metaRepo.protocol = RepositoryProtocolType::NFS;
        m_job.m_metaRepoHandler = std::make_shared<FileSystemHandler>();
        m_job.m_metaRepoPath = DEFAULT_META_PATH;
        m_job.m_metaRepoHandler->CreateDirectory(DEFAULT_META_PATH);
    }

    void TearDown() {}

    void InitLogger()
    {
        std::string logFileName = "virt_plugin_test.log";
        std::string logFilePath = "/tmp/log/";
        int logLevel = DEBUG;
        int logFileCount = 10;
        int logFileSize = 30;
        Module::CLogger::GetInstance().Init(
            logFileName.c_str(), logFilePath, logLevel, logFileCount, logFileSize);
    }

public:
    VirtPlugin::BackupJob m_job;
    AppProtect::BackupJob m_backupJobInfo;
    AppProtect::SubJob m_subJob;
    BackupSubJobInfo m_subJobInfo;
    typedef int (*JobServiceMethodPtr)(JobService*);
    JobServiceMethodPtr JobService_ReportJobDetails = (JobServiceMethodPtr)(&JobService::ReportJobDetails);
    JobServiceMethodPtr JobService_AddNewJob = (JobServiceMethodPtr)(&JobService::AddNewJob);
    JobServiceMethodPtr JobService_ReportCopyAdditionalInfo = (JobServiceMethodPtr)(&JobService::ReportCopyAdditionalInfo);
    Stub stub;
};

const std::string gSnapshotInfo = "{\
    \"extendInfo\": \"\",\
    \"moRef\": \"\",\
    \"preMoRef\": \"\",\
    \"vmName\": \"\",\
    \"vmUuid\": \"\",\
    \"volSnapList\": null\
}";

const std::string gCheckpoint = "{\
    \"completedSegmentSize\": 666\
}";

const std::string gVmInfoContent = "{\
    \"computeResourceMoRef\": \"\",\
    \"extendInfo\": \"\",\
    \"hostMoRef\": \"\",\
    \"ifStartNetworkAdapter\": false,\
    \"isUndervApp\": false,\
    \"metaData\": \"\",\
    \"moRef\": \"\",\
    \"netCardList\": null,\
    \"poweron\": false,\
    \"restorePlace\": 0,\
    \"storagePoolType\": 0,\
    \"storageResource\": {\
        \"extendInfo\": \"\",\
        \"storageMoref\": \"\",\
        \"storageName\": \"\"\
    },\
    \"targetFolderLocation\": \"\",\
    \"volList\": [\
        {\
            \"busNumber\":\"\",\
            \"changeID\":\"\",\
            \"controllerType\":\"\",\
            \"dataCenter\":\"\",\
            \"datastoreMoRef\":\"\",\
            \"datastoreName\":\"\",\
            \"diskKey\":\"\",\
            \"fileName\":\"\",\
            \"moRef\":\"\",\
            \"preChangeID\":\"\",\
            \"replaceMode\":\"\",\
            \"snapType\":\"\",\
            \"storageType\":\"\",\
            \"volName\":\"\",\
            \"volNumber\":\"\",\
            \"volSizeInBytes\":10,\
            \"volType\":\"\",\
            \"volUuid\":\"123\"\
        },\
        {\
            \"busNumber\":\"\",\
            \"changeID\":\"\",\
            \"controllerType\":\"\",\
            \"dataCenter\":\"\",\
            \"datastoreMoRef\":\"\",\
            \"datastoreName\":\"\",\
            \"diskKey\":\"\",\
            \"fileName\":\"\",\
            \"moRef\":\"\",\
            \"preChangeID\":\"\",\
            \"replaceMode\":\"\",\
            \"snapType\":\"\",\
            \"storageType\":\"\",\
            \"volName\":\"\",\
            \"volNumber\":\"\",\
            \"volSizeInBytes\":10,\
            \"volType\":\"\",\
            \"volUuid\":\"456\"\
        }\
        ],\
    \"volName\": \"\",\
    \"volUuid\": \"\"\
}";

const std::string gPreContext = "{\
     \"copyId\":\"123456789\",\
     \"jobId\":\"987654321\",\
     \"snapshotId\":\"VIRTUAL_PLUGIN_5752D641-0A27-49B5-A0DA-FC2B74A535F1_1654402480\",\
     \"vmUuid\":\"bbbbbbbbbbbbbbbbbb\"\
}";

const std::string gCurContext = "{\
     \"copyId\":\"abcdefg\",\
     \"jobId\":\"gfedcba\",\
     \"snapshotId\":\"VIRTUAL_PLUGIN_5752D641-0A27-49B5-A0DA-FC2B74A535F1_1654402481\",\
     \"vmUuid\":\"bbbbbbbbbbbbbbbbbb\"\
}";

std::string gCreateCheckpointDirectoryFlag = "false";

static bool CreateCheckpointDirectory_Invoke(const std::string &content)
{
    gCreateCheckpointDirectoryFlag = "true";
    return true;
}

static bool Main_Task_Status_Exists_Invoke(const std::string &fileName)
{
    if (fileName.find("main_task_status.info") != std::string::npos) {
        return false;
    }
    return true;
}

static bool CreateCheckpointDirectoryInvokeFalse(const std::string &content)
{
    return false;
}

static int32_t FileSystemWriteSuccess_Invoke(const std::string &content)
{
    return content.length();
}

static int32_t FileSystemWriteFailed_Invoke(const std::string &content)
{
    return 0;
}

static int32_t FileSystemWriteBufSuccess_Invoke(std::shared_ptr<uint8_t[]> buf, size_t count)
{
    return count;
}
static void FileSystemGetFilesSuccess_Invoke(std::string pathName, std::vector<std::string> &files)
{
    files.push_back("ssss_sha256.info");
    return;
}
static void FileSystemGetFilesFailed_Invoke(std::string pathName, std::vector<std::string> &files)
{
    return;
}

static int32_t FileSystemCurVMInfoContextSuccess_Invoke(std::string &buf, size_t size)
{
    buf = gVmInfoContent;
    return gVmInfoContent.length();
}

static int32_t FileSystemCurVMInfoContextFailed_Invoke(std::string &buf, size_t size)
{
    return FAILED;
}

static void Stub_ReportJobDetails_OK(ActionResult& returnValue, const SubJobDetails& jobInfo)
{
    return;
}

static void Stub_AddNewJob_OK(ActionResult& returnValue, const std::vector<SubJob>& job)
{
    returnValue.code = 0;
    return;
}

static void Stub_AddNewJob_Failed(ActionResult& returnValue, const std::vector<SubJob>& job)
{
    returnValue.code = -1;
    return;
}

static uint64_t Stub_GetSegSizeFromConf()
{
    uint64_t size = 5 * DEFAULT_BLOCK_SIZE;
    return size; // 20m
}

static int Stub_ReturnSuccess()
{
    return SUCCESS;
}

static void StubNowTime(time_t &startTime)
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

/* Stub for ProtectEngine */
static std::shared_ptr<ProtectEngine> Stub_CreateEngine_OK(const JobType &jobType,
    std::shared_ptr<JobCommonInfo> jobInfo)
{
    std::shared_ptr<JobHandle> jobHandle = std::make_shared<JobHandle>(jobType, jobInfo);
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>(jobHandle);
    EXPECT_CALL(*protectEngine, CreateSnapshot(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, GetMachineMetadata(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, DeleteSnapshot(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, QuerySnapshotExists(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, PreHook(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, PostHook(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, GetSnapshotsOfVolume(_, _)).WillRepeatedly(Return(SUCCESS));
    return protectEngine;
}

static std::shared_ptr<RepositoryHandler> Stub_GenerateSubJob_CreateFSHandler_OK(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(Main_Task_Status_Exists_Invoke)));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(gVmInfoContent.length()));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(FileSystemCurVMInfoContextSuccess_Invoke));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    return fsHandler;
}

static std::shared_ptr<RepositoryHandler> Stub_CreateFSHandler_Failed(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(Main_Task_Status_Exists_Invoke)));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(gVmInfoContent.length()));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(FileSystemCurVMInfoContextFailed_Invoke));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    return fsHandler;
}

static std::shared_ptr<ProtectEngine> Stub_CreateEngine_CreateSnapshot_Failed(const JobType &jobType,
    std::shared_ptr<JobCommonInfo> jobInfo)
{
    std::shared_ptr<JobHandle> jobHandle = std::make_shared<JobHandle>(jobType, jobInfo);
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>(jobHandle);
    EXPECT_CALL(*protectEngine, CreateSnapshot(_,_)).WillRepeatedly(Return(FAILED));
    EXPECT_CALL(*protectEngine, GetMachineMetadata(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, PreHook(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, PostHook(_)).WillRepeatedly(Return(SUCCESS));
    return protectEngine;
}

static std::shared_ptr<ProtectEngine> Stub_CreateEngine_GetMachineMetadata_Failed(const JobType &jobType,
    std::shared_ptr<JobCommonInfo> jobInfo)
{
    std::shared_ptr<JobHandle> jobHandle = std::make_shared<JobHandle>(jobType, jobInfo);
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>(jobHandle);
    EXPECT_CALL(*protectEngine, CreateSnapshot(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, GetMachineMetadata(_)).WillRepeatedly(Return(FAILED));
    EXPECT_CALL(*protectEngine, PreHook(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, PostHook(_)).WillRepeatedly(Return(SUCCESS));
    return protectEngine;
}

static std::shared_ptr<ProtectEngine> Stub_CreateEngine_GetVolumeHandler_Failed(const JobType &jobType,
    std::shared_ptr<JobCommonInfo> jobInfo)
{
    std::shared_ptr<JobHandle> jobHandle = std::make_shared<JobHandle>(jobType, jobInfo);
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>(jobHandle);
    EXPECT_CALL(*protectEngine, GetVolumeHandler(_,_)).WillRepeatedly(Return(FAILED));
    EXPECT_CALL(*protectEngine, PreHook(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, PostHook(_)).WillRepeatedly(Return(SUCCESS));
    return protectEngine;
}

uint64_t VOL_SIZE = DEFAULT_BLOCK_SIZE;

static std::shared_ptr<ProtectEngine> Stub_CreateEngine_ReadVolume_OK(const JobType &jobType,
    std::shared_ptr<JobCommonInfo> jobInfo)
{
    VolInfo volInfo;
    volInfo.m_volSizeInBytes = DEFAULT_BLOCK_SIZE;
    std::shared_ptr<JobHandle> jobHandle = std::make_shared<JobHandle>(jobType, jobInfo);
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>(jobHandle);
    std::shared_ptr<VolumeHandlerMock> volHandler = std::make_shared<VolumeHandlerMock>(jobHandle, volInfo);
    DirtyRanges dirtyRanges;
    dirtyRanges.AddRange(DirtyRange(0, DEFAULT_BLOCK_SIZE));
    testing::Mock::AllowLeak(volHandler.get());
    testing::Mock::AllowLeak(protectEngine.get());
    EXPECT_CALL(*volHandler, GetVolumeSize()).WillRepeatedly(Return(strlen(g_volumeContent.c_str())));
    EXPECT_CALL(*volHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandler, CleanLeftovers()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandler, GetDirtyRanges(_,_,_,_,_)).WillRepeatedly(DoAll(SetArgReferee<2>(dirtyRanges),
        Return(SUCCESS)));
    EXPECT_CALL(*volHandler, ReadBlocks(_,_,_,_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, GetVolumeHandler(_,_)).WillRepeatedly(DoAll(SetArgReferee<1>(volHandler),
        Return(SUCCESS)));
    EXPECT_CALL(*protectEngine, PreHook(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, PostHook(_)).WillRepeatedly(Return(SUCCESS));
    return protectEngine;
}

static DirtyRange firsetDirty = DirtyRange(0, DEFAULT_BLOCK_SIZE * 5);
static DirtyRange sedondDirty = DirtyRange(DEFAULT_BLOCK_SIZE * 6, DEFAULT_BLOCK_SIZE * 2);
static DirtyRange thirdDirty = DirtyRange(DEFAULT_BLOCK_SIZE * 11, DEFAULT_BLOCK_SIZE * 2);

static std::shared_ptr<ProtectEngine> Stub_CreateEngine_GetDirtyRangeOK(const JobType &jobType,
    std::shared_ptr<JobCommonInfo> jobInfo)
{
    VolInfo volInfo;
    volInfo.m_volSizeInBytes = VOL_SIZE;
    std::shared_ptr<JobHandle> jobHandle = std::make_shared<JobHandle>(jobType, jobInfo);
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>(jobHandle);
    std::shared_ptr<VolumeHandlerMock> volHandler = std::make_shared<VolumeHandlerMock>(jobHandle, volInfo);
    testing::Mock::AllowLeak(volHandler.get());
    testing::Mock::AllowLeak(protectEngine.get());
    EXPECT_CALL(*volHandler, GetVolumeSize()).WillRepeatedly(Return(strlen(g_volumeContent.c_str())));
    EXPECT_CALL(*volHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandler, CleanLeftovers()).WillRepeatedly(Return(SUCCESS));
    int invokeTimes = 6;
    EXPECT_CALL(*volHandler, GetDirtyRanges(_,_,_,_,_)).Times(invokeTimes).WillOnce(::testing::Invoke([&](
                const VolSnapInfo &preVolSnapshot, const VolSnapInfo &curVolSnapshot, DirtyRanges &dirtyRanges,
                const uint64_t startOffset, uint64_t &endOffset) {
                dirtyRanges.AddRange(firsetDirty);
                return SUCCESS;
            })).WillOnce(::testing::Invoke([&](
                const VolSnapInfo &preVolSnapshot, const VolSnapInfo &curVolSnapshot, DirtyRanges &dirtyRanges,
                const uint64_t startOffset, uint64_t &endOffset) {
                dirtyRanges.AddRange(sedondDirty);
                return SUCCESS;
            })).WillOnce(::testing::Invoke([&](
                const VolSnapInfo &preVolSnapshot, const VolSnapInfo &curVolSnapshot, DirtyRanges &dirtyRanges,
                const uint64_t startOffset, uint64_t &endOffset) {
                dirtyRanges.AddRange(thirdDirty);
                return SUCCESS;
            })).WillOnce(::testing::Invoke([&](
                const VolSnapInfo &preVolSnapshot, const VolSnapInfo &curVolSnapshot, DirtyRanges &dirtyRanges,
                const uint64_t startOffset, uint64_t &endOffset) {
                dirtyRanges.AddRange(DirtyRange(0, DEFAULT_BLOCK_SIZE));
                return SUCCESS;
            })).WillOnce(::testing::Invoke([&](
                const VolSnapInfo &preVolSnapshot, const VolSnapInfo &curVolSnapshot, DirtyRanges &dirtyRanges,
                const uint64_t startOffset, uint64_t &endOffset) {
                int eight = 8;
                dirtyRanges.AddRange(DirtyRange(DEFAULT_BLOCK_SIZE * eight, DEFAULT_BLOCK_SIZE));
                return SUCCESS;
            })).WillOnce(::testing::Invoke([&](
                const VolSnapInfo &preVolSnapshot, const VolSnapInfo &curVolSnapshot, DirtyRanges &dirtyRanges,
                const uint64_t startOffset, uint64_t &endOffset) {
                int ten = 10;
                dirtyRanges.AddRange(DirtyRange(DEFAULT_BLOCK_SIZE * ten, DEFAULT_BLOCK_SIZE));
                return SUCCESS;
            }));
    EXPECT_CALL(*volHandler, ReadBlocks(_,_,_,_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, GetVolumeHandler(_,_)).WillRepeatedly(DoAll(SetArgReferee<1>(volHandler),
        Return(SUCCESS)));
    EXPECT_CALL(*protectEngine, PreHook(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, PostHook(_)).WillRepeatedly(Return(SUCCESS));
    return protectEngine;
}

static std::shared_ptr<ProtectEngine> Stub_CreateEngine_GetDirtyRanges_Failed(const JobType &jobType,
    std::shared_ptr<JobCommonInfo> jobInfo)
{
    VolInfo volInfo;
    std::shared_ptr<JobHandle> jobHandle = std::make_shared<JobHandle>(jobType, jobInfo);
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>(jobHandle);
    std::shared_ptr<VolumeHandlerMock> volHandler = std::make_shared<VolumeHandlerMock>(jobHandle, volInfo);
    EXPECT_CALL(*volHandler, GetVolumeSize()).WillRepeatedly(Return(strlen(g_volumeContent.c_str())));
    EXPECT_CALL(*volHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandler, CleanLeftovers()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandler, GetDirtyRanges(_,_,_,_,_)).WillRepeatedly(Return(FAILED));
    EXPECT_CALL(*protectEngine, GetVolumeHandler(_,_)).WillRepeatedly(DoAll(SetArgReferee<1>(volHandler),
        Return(SUCCESS)));
    EXPECT_CALL(*protectEngine, PreHook(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, PostHook(_)).WillRepeatedly(Return(SUCCESS));
    return protectEngine;
}

static std::shared_ptr<ProtectEngine> Stub_CreateEngine_ReadVolume_Failed(const JobType &jobType,
    std::shared_ptr<JobCommonInfo> jobInfo)
{
    VolInfo volInfo;
    std::shared_ptr<JobHandle> jobHandle = std::make_shared<JobHandle>(jobType, jobInfo);
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>(jobHandle);
    std::shared_ptr<VolumeHandlerMock> volHandler = std::make_shared<VolumeHandlerMock>(jobHandle, volInfo);
    DirtyRanges dirtyRanges;
    dirtyRanges.AddRange(DirtyRange(0, 1));
    testing::Mock::AllowLeak(volHandler.get());
    EXPECT_CALL(*volHandler, GetVolumeSize()).WillRepeatedly(Return(strlen(g_volumeContent.c_str())));
    EXPECT_CALL(*volHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandler, CleanLeftovers()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*volHandler, GetDirtyRanges(_,_,_,_,_)).WillRepeatedly(DoAll(SetArgReferee<2>(dirtyRanges),
        Return(SUCCESS)));
    EXPECT_CALL(*volHandler, ReadBlocks(_,_,_,_,_)).WillRepeatedly(Return(FAILED));
    EXPECT_CALL(*protectEngine, GetVolumeHandler(_,_)).WillRepeatedly(DoAll(SetArgReferee<1>(volHandler),
        Return(SUCCESS)));
    EXPECT_CALL(*protectEngine, PreHook(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, PostHook(_)).WillRepeatedly(Return(SUCCESS));
    return protectEngine;
}

static int32_t FileSystemReadSnapshotSuccess_Invoke(std::string &buf, size_t size)
{
    buf = gSnapshotInfo;
    return gSnapshotInfo.length();
}

static int32_t FileSystemReadCheckpointSuccess_Invoke(std::string &buf, size_t size)
{
    buf = gCheckpoint;
    return gCheckpoint.length();
}

/* Stub for FileSystemHandler */
static std::shared_ptr<RepositoryHandler> Stub_CreateFSHandler_OK(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    testing::Mock::AllowLeak(fsHandler.get());
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, CreateDirectory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, IsDirectory(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(gSnapshotInfo.length()));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(FileSystemReadSnapshotSuccess_Invoke));
    EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Write(_,_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteBufSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, GetFiles(_,_)).WillRepeatedly(DoAll(Invoke(FileSystemGetFilesSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(Main_Task_Status_Exists_Invoke)));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Truncate(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Flush(_)).WillRepeatedly(Return(true));
    return fsHandler;
}

static std::shared_ptr<RepositoryHandler> Stub_Checkpoint_CreateFSHandler_OK(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    testing::Mock::AllowLeak(fsHandler.get());
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, IsDirectory(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*fsHandler, CreateDirectory(_)).WillRepeatedly(DoAll(Invoke(CreateCheckpointDirectory_Invoke)));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(gSnapshotInfo.length()));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(FileSystemReadSnapshotSuccess_Invoke));
    EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Write(_,_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteBufSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    // EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(Main_Task_Status_Exists_Invoke)));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(Main_Task_Status_Exists_Invoke)));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Truncate(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Flush(_)).WillRepeatedly(Return(true));
    return fsHandler;
}
static std::shared_ptr<RepositoryHandler> Stub_GenInterruptFile_CreateFSHandler_OK(
        const AppProtect::StorageRepository &storageRepo)
        {
            std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
            testing::Mock::AllowLeak(fsHandler.get());
            EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
            EXPECT_CALL(*fsHandler, CreateDirectory(_)).WillRepeatedly(Return(true));
            EXPECT_CALL(*fsHandler, IsDirectory(_)).WillRepeatedly(Return(false));
            EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(gSnapshotInfo.length()));
            EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(FileSystemReadSnapshotSuccess_Invoke));
            EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
            EXPECT_CALL(*fsHandler, Write(_,_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteBufSuccess_Invoke)));
            EXPECT_CALL(*fsHandler, GetFiles(_,_)).WillRepeatedly(DoAll(Invoke(FileSystemGetFilesFailed_Invoke)));
            EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
            EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
            EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(Main_Task_Status_Exists_Invoke)));
            EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
            EXPECT_CALL(*fsHandler, Truncate(_)).WillRepeatedly(Return(SUCCESS));
            EXPECT_CALL(*fsHandler, Flush(_)).WillRepeatedly(Return(true));
            return fsHandler;
        }
static std::shared_ptr<RepositoryHandler> Stub_Checkpoint_CreateFSHandler_Write_Fail(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    testing::Mock::AllowLeak(fsHandler.get());
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, IsDirectory(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*fsHandler, CreateDirectory(_)).WillRepeatedly(DoAll(Invoke(CreateCheckpointDirectory_Invoke)));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(gSnapshotInfo.length()));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(FileSystemReadSnapshotSuccess_Invoke));
    EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteFailed_Invoke)));
    EXPECT_CALL(*fsHandler, Write(_,_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteBufSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(Main_Task_Status_Exists_Invoke)));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Truncate(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Flush(_)).WillRepeatedly(Return(true));
    return fsHandler;
}

static std::shared_ptr<RepositoryHandler> Stub_Checkpoint_CreateFSHandler_Write_Retry_Success(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    testing::Mock::AllowLeak(fsHandler.get());
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, IsDirectory(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*fsHandler, CreateDirectory(_)).WillRepeatedly(DoAll(Invoke(CreateCheckpointDirectory_Invoke)));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(gSnapshotInfo.length()));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(FileSystemReadSnapshotSuccess_Invoke));
    EXPECT_CALL(*fsHandler, Write(_)).Times(AtLeast(0)).WillOnce(Invoke(FileSystemWriteFailed_Invoke)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Write(_,_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteBufSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(Main_Task_Status_Exists_Invoke)));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Truncate(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Flush(_)).WillRepeatedly(Return(true));
    return fsHandler;
}

static std::shared_ptr<RepositoryHandler> Stub_Checkpoint_CreateFSHandler_Open_Retry_Success(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    testing::Mock::AllowLeak(fsHandler.get());
    // open第一次失败，重试成功
    EXPECT_CALL(*fsHandler, Open(_,_)).Times(AtLeast(0)).WillOnce(Return(FAILED)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, IsDirectory(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*fsHandler, CreateDirectory(_)).WillRepeatedly(DoAll(Invoke(CreateCheckpointDirectory_Invoke)));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(gCheckpoint.length()));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(FileSystemReadCheckpointSuccess_Invoke));
    EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Write(_,_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteBufSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(Main_Task_Status_Exists_Invoke)));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Truncate(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Flush(_)).WillRepeatedly(Return(true));
    return fsHandler;
}

static std::shared_ptr<RepositoryHandler> Stub_Checkpoint_CreateFSHandler_Open_Retry_Fail(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    testing::Mock::AllowLeak(fsHandler.get());
    // open一直失败
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(FAILED));
    EXPECT_CALL(*fsHandler, IsDirectory(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*fsHandler, CreateDirectory(_)).WillRepeatedly(DoAll(Invoke(CreateCheckpointDirectory_Invoke)));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(gCheckpoint.length()));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(FileSystemReadCheckpointSuccess_Invoke));
    EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Write(_,_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteBufSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(Main_Task_Status_Exists_Invoke)));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Truncate(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Flush(_)).WillRepeatedly(Return(true));
    return fsHandler;
}

static std::shared_ptr<RepositoryHandler> Stub_Checkpoint_CreateFSHandler_Retry(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    testing::Mock::AllowLeak(fsHandler.get());
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, IsDirectory(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*fsHandler, CreateDirectory(_)).Times(AtLeast(0)).WillOnce(Invoke(CreateCheckpointDirectoryInvokeFalse)).WillRepeatedly(DoAll(Invoke(CreateCheckpointDirectory_Invoke)));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(gSnapshotInfo.length()));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(FileSystemReadSnapshotSuccess_Invoke));
    EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Write(_,_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteBufSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(Main_Task_Status_Exists_Invoke)));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Truncate(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Flush(_)).WillRepeatedly(Return(true));
    return fsHandler;
}

static std::shared_ptr<RepositoryHandler> Stub_Checkpoint_CreateFSHandler_Retry_Fail(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    testing::Mock::AllowLeak(fsHandler.get());
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, IsDirectory(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*fsHandler, CreateDirectory(_)).WillRepeatedly(DoAll(Invoke(CreateCheckpointDirectoryInvokeFalse)));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(gSnapshotInfo.length()));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(FileSystemReadSnapshotSuccess_Invoke));
    EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Write(_,_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteBufSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(Main_Task_Status_Exists_Invoke)));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Truncate(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Flush(_)).WillRepeatedly(Return(true));
    return fsHandler;
}

/* Stub for FileSystemHandler */
static std::shared_ptr<RepositoryHandler> Stub_CreateFSHandler_GetCheckpoint_OK(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    testing::Mock::AllowLeak(fsHandler.get());
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, IsDirectory(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*fsHandler, CreateDirectory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(gCheckpoint.length()));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(FileSystemReadCheckpointSuccess_Invoke));
    EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Write(_,_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteBufSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(Main_Task_Status_Exists_Invoke)));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Flush(_)).WillRepeatedly(Return(true));
    return fsHandler;
}

static std::shared_ptr<RepositoryHandler> Stub_CreateFSHandler_Checkpoint_RemoveAll_Retry_OK(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    testing::Mock::AllowLeak(fsHandler.get());
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, IsDirectory(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*fsHandler, CreateDirectory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(gCheckpoint.length()));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(FileSystemReadCheckpointSuccess_Invoke));
    EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Write(_,_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteBufSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(Main_Task_Status_Exists_Invoke)));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
    // RemoveAll 第一次失败，后续成功
    EXPECT_CALL(*fsHandler, RemoveAll(_)).Times(AtLeast(0)).WillOnce(Return(false)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Flush(_)).WillRepeatedly(Return(true));
    return fsHandler;
}

static std::shared_ptr<RepositoryHandler> Stub_CreateFSHandler_Checkpoint_RemoveAll_Retry_Fail(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    testing::Mock::AllowLeak(fsHandler.get());
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, IsDirectory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, CreateDirectory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(gCheckpoint.length()));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(FileSystemReadCheckpointSuccess_Invoke));
    EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Write(_,_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteBufSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(Main_Task_Status_Exists_Invoke)));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
    // RemoveAll 一直失败
    EXPECT_CALL(*fsHandler, RemoveAll(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*fsHandler, Flush(_)).WillRepeatedly(Return(true));
    return fsHandler;
}

/* Stub for FileSystemHandler for block bitmap test */
static std::shared_ptr<RepositoryHandler> Stub_CreateFSHandler_BitMapTest(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemMock> fsHandler = std::make_shared<FileSystemMock>();
    testing::Mock::AllowLeak(fsHandler.get());
    return fsHandler;
}

static std::shared_ptr<RepositoryHandler> Stub_CreateFSHandler_Open_Failed(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(FAILED));
    EXPECT_CALL(*fsHandler, IsDirectory(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*fsHandler, CreateDirectory(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(Main_Task_Status_Exists_Invoke)));
    EXPECT_CALL(*fsHandler, Flush(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    return fsHandler;
}

static std::shared_ptr<RepositoryHandler> Stub_CreateFSHandler_WRITE_Failed(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    testing::Mock::AllowLeak(fsHandler.get());
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, IsDirectory(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*fsHandler, CreateDirectory(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(gSnapshotInfo.length()));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(FileSystemReadSnapshotSuccess_Invoke));
    EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(Return(FAILED));
    EXPECT_CALL(*fsHandler, Write(_,_)).WillRepeatedly(Return(FAILED));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(Main_Task_Status_Exists_Invoke)));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Truncate(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Flush(_)).WillRepeatedly(Return(true));
    return fsHandler;
}

static std::shared_ptr<RepositoryHandler> Stub_CreateFSHandler_Exists_Failed(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*fsHandler, Flush(_)).WillRepeatedly(Return(true));
    return fsHandler;
}

static void ReportJobDetailsSuc(void *obj, ActionResult& returnValue, const SubJobDetails& jobInfo)
{
    return;
}

static void AddNewJobSuc(void *obj, ActionResult& returnValue, const std::vector<SubJob>& job)
{
    return;
}

static void ReportCopyAdditionalInfoSuc(ActionResult& returnValue, const std::string& jobId, const Copy& copy)
{
    return;
}

static void ReportCopyAdditionalInfoFailed(ActionResult& returnValue, const std::string& jobId, const Copy& copy)
{
    returnValue.code = -1;
    return;
}

static bool Stub_FillUpBackupSubJob_Success(const VolInfo &vol, BackupSubJobInfo &subJobInfo)
{
    return true;
}

static int Stub_CreateSnapshot_Success(void* obj)
{
    VirtPlugin::BackupJob* o = (VirtPlugin::BackupJob*)obj;
    o->m_nextState = static_cast<int>(BackupJobSteps::STEP_PRE_GET_MACHINE_METADATA);
    INFOLOG("Create snapshot success, vmRef: %s", o->m_vmInfo.m_moRef.c_str());
    std::function<int32_t(void)> fun = [=]() -> int32_t { return o->m_protectEngine->DeleteSnapshot(o->m_snapshotInfo); };
    o->m_cleanHandlesForStop.push_back(fun);
    o->SetJobAborted();
    return SUCCESS;
}

static int Stub_CreateSnapshot_With_Delete_Fail(void* obj)
{
    VirtPlugin::BackupJob* o = (VirtPlugin::BackupJob*)obj;
    o->m_nextState = static_cast<int>(BackupJobSteps::STEP_PRE_GET_MACHINE_METADATA);
    INFOLOG("Create snapshot success, vmRef: %s", o->m_vmInfo.m_moRef.c_str());
    std::function<int32_t(void)> fun = [=]() -> int32_t { return -1; };
    o->m_cleanHandlesForStop.push_back(fun);
    o->SetJobAborted();
    return SUCCESS;
}

static int32_t StubGetSha256DataCalcRetFalse(std::shared_ptr<unsigned char[]>& pBuffer, uint64_t& startAddr,
    bool& calcRet)
{
    startAddr = 0;
    calcRet = false;
    return SUCCESS;
}


static int32_t StubSaveBlockSha256ValueSuccess(const std::shared_ptr<unsigned char[]>& shaBuf,
    const uint64_t& startAddr)
{
    return SUCCESS;
}

static int32_t StubCalculateSha256ValueFailed(const void* pData, const uint64_t dataSize,
    std::shared_ptr<uint8_t[]> outBuf)
{
    return FAILED;
}

static int32_t StubCalculateSha256ValueSuccess(const void* pData, const uint64_t dataSize,
    std::shared_ptr<uint8_t[]> outBuf)
{
    return SUCCESS;
}

static int32_t StubCalculateSha256DeviationFailed(const uint64_t& inDev, uint64_t& outDev)
{
    return FAILED;
}

static int32_t StubCalculateSha256DeviationSuccess(const uint64_t& inDev, uint64_t& outDev)
{
    return SUCCESS;
}

static bool Stub_Empty_False()
{
    return false;
}

static int32_t Stub_Open_Success(void* obj, const std::string &fileName, const std::string &mode)
{
    return SUCCESS;
}

static size_t Stub_SeekFailed(size_t offset, int origin = 0)
{
    return FAILED;
}

static size_t Stub_SeekSuccess(size_t offset, int origin = 0)
{
    return SUCCESS;
}

static size_t Stub_WriteSuccess(std::shared_ptr<uint8_t[]> buf, size_t count)
{
    return SUCCESS;
}

static int32_t StubGetSha256DataFailed(std::shared_ptr<unsigned char[]>& pBuffer, uint64_t& startAddr, bool& calcRet)
{
    return FAILED;
}

static int Stub_DoGenerateSubJob_Abort(void* obj)
{
    VirtPlugin::BackupJob* o = (VirtPlugin::BackupJob*)obj;
    o->SetJobAborted();
    return FAILED;
}

static int Stub_BackupDirtyRanges_Abort(void* obj)
{
    VirtPlugin::BackupJob* o = (VirtPlugin::BackupJob*)obj;
    o->SetJobAborted();
    return FAILED;
}

static int Stub_BlockBackupTaskInit(void* obj)
{
    return SUCCESS;
}

static int Stub_SaveBlockDataBitMap(uint64_t confSegSize)
{
    return SUCCESS;
}

static bool Stub_ExecBlockTaskStart(void* obj)
{
    return true;
}

static void Stub_ReportBackupSpeed(const uint64_t &dataSizeInByte) {}

static bool Stub_OpenRWHandler(void* obj)
{
    return SUCCESS;
}


static bool Stub_BackupDirtyRanges(void* obj)
{
    VirtPlugin::BackupJob* o = (VirtPlugin::BackupJob*)obj;
    o->m_nextState = static_cast<int>(BackupJobSteps::STEP_EXEC_POSTHOOK);
    return SUCCESS;
}

static bool Stub_UpdateSnapshotFile(void* obj)
{
    VirtPlugin::BackupJob* o = (VirtPlugin::BackupJob*)obj;
    o->m_nextState = static_cast<int>(BackupJobSteps::STEP_POST_CLEANUP_CHECKPOINT);
    return SUCCESS;
}

static int StubInitSha256FileFailed()
{
    return FAILED;
}

void BackupJobTest::SetUp()
{
    InitLogger();
    stub.set(sleep, Stub_Sleep);
    AppProtect::StorageRepository dataRepo;
    dataRepo.repositoryType = RepositoryDataType::DATA_REPOSITORY;
    dataRepo.path.push_back("/tmp/data_repo_0");
    dataRepo.path.push_back("/tmp/data_repo_1");
    m_backupJobInfo.repositories.push_back(dataRepo);

    AppProtect::StorageRepository metaRepo;
    metaRepo.repositoryType = RepositoryDataType::META_REPOSITORY;
    metaRepo.path.push_back("/tmp/meta_repo");
    m_backupJobInfo.repositories.push_back(metaRepo);

    AppProtect::StorageRepository cacheRepo;
    cacheRepo.repositoryType = RepositoryDataType::CACHE_REPOSITORY;
    cacheRepo.path.push_back("/tmp/cache_repo");
    m_backupJobInfo.repositories.push_back(cacheRepo);

    m_backupJobInfo.jobParam.advanceParams = "{\"copy_verify\":\"true\"}";
    m_backupJobInfo.jobParam.backupType = AppProtect::BackupJobType::FULL_BACKUP;
    // set main job id
    m_backupJobInfo.jobId = "123";
    m_backupJobInfo.__set_extendInfo("{\"projectId\":\"projectId\",\"esn\":\"esn\",\"copy_verify\":\"true\"}");
    m_subJobInfo.m_volInfo.m_moRef = "vm_moref";
    uint64_t fiftyM = 50 * 1024 * 1024;
    m_subJobInfo.m_volInfo.m_volSizeInBytes = fiftyM;  // 50 M
    m_subJob.jobId = "main_job_id";
    m_subJob.subJobId = "sub_job_id";
    std::string subJobInfoStr;
    Module::JsonHelper::StructToJsonString(m_subJobInfo, subJobInfoStr);
    m_subJob.jobInfo = subJobInfoStr;

    std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
    std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupJobInfo);
    std::shared_ptr<AppProtect::SubJob> subJobInfo = std::make_shared<AppProtect::SubJob>(m_subJob);
    jobInfo->SetJobInfo(data);
    m_job.SetJobInfo(jobInfo);
    m_job.SetSubJob(subJobInfo);
}

/*
 * 测试用例：执行备份前置任务
 * 前置条件：创建快照、获取虚拟机信息、获取卷信息、任务信息成功
 * CHECK点：执行备份前置任务成功
 */
TEST_F(BackupJobTest, PrerequisiteJob_OK)
{
    INFOLOG("Case: PrerequisiteJob_OK.");
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    stub.set(ADDR(VirtPlugin::BackupJob, PrerequisiteJobInner), Stub_ReturnSuccess);
    int ret = m_job.PrerequisiteJob();
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * 测试用例：执行备份前置任务,创建续作目录成功
 * 前置条件：无
 * CHECK点：创建续作目录成功
 */
TEST_F(BackupJobTest, PrerequisiteJob_Create_Checkpoint_Folder_OK)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_Checkpoint_CreateFSHandler_OK);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.PrerequisiteJob();
    // 检查目录路径是否正确
    std::string expect_dir = m_job.m_cacheRepoPath + VIRT_PLUGIN_CACHE_CHECKPOINT_ROOT + "main_job_123";
    EXPECT_EQ(m_job.m_checkpoint.m_dir, expect_dir);
    // 检查创建目录方法是否被调用
    EXPECT_EQ(gCreateCheckpointDirectoryFlag, "true");
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * 测试用例：执行备份前置任务,创建续作目录失败，重试成功
 * 前置条件：无
 * CHECK点：创建续作目录成功
 */
TEST_F(BackupJobTest, PrerequisiteJob_Create_Checkpoint_Folder_Retry_OK)
{
    INFOLOG("========== PrerequisiteJob_Create_Checkpoint_Folder_Retry_OK");
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_Checkpoint_CreateFSHandler_Retry);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.PrerequisiteJob();
    // 检查目录路径是否正确
    std::string expect_dir = m_job.m_cacheRepoPath + VIRT_PLUGIN_CACHE_CHECKPOINT_ROOT + "main_job_123";
    EXPECT_EQ(m_job.m_checkpoint.m_dir, expect_dir);
    // 检查创建目录方法是否被调用
    EXPECT_EQ(gCreateCheckpointDirectoryFlag, "true");
    EXPECT_EQ(SUCCESS, ret);
    INFOLOG("========== PrerequisiteJob_Create_Checkpoint_Folder_Retry_OK END");
}

/*
 * 测试用例：执行备份前置任务,创建续作目录3次失败
 * 前置条件：无
 * CHECK点：执行备份前置任务失败
 */
TEST_F(BackupJobTest, PrerequisiteJob_Create_Checkpoint_Folder_Retry_Fail)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_Checkpoint_CreateFSHandler_Retry_Fail);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.PrerequisiteJob();
    std::string expect_dir = m_job.m_cacheRepoPath + VIRT_PLUGIN_CACHE_CHECKPOINT_ROOT + "main_job_123";
    // 检查目录路径是否正确
    EXPECT_EQ(m_job.m_checkpoint.m_dir, expect_dir);
    EXPECT_EQ(FAILED, ret);
}

/*
 * 测试用例：执行备份前置任务,生成SHA中断文件成功
 * 前置条件：增量备份，未开启副本校验
 * CHECK点：生成中断文件成功
 */
 TEST_F(BackupJobTest, PrerequisiteJob_Gen_Sha_Interrupt_File_OK)
 {
        stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_OK);
        stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_Checkpoint_CreateFSHandler_OK);

        stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
        std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
        m_backupJobInfo.jobParam.backupType = AppProtect::BackupJobType::INCREMENT_BACKUP;
        std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupJobInfo);
        jobInfo->SetJobInfo(data);
        m_job.SetJobInfo(jobInfo);
        m_job.m_isCopyVerify = false;
        int ret = m_job.PrerequisiteJob();
        EXPECT_EQ(SUCCESS, ret);
 }

/*
 * 测试用例：执行备份前置任务,生成SHA中断文件成功
 * 前置条件：增量备份，未开启副本校验,前面未生成校验文件
 * CHECK点：生成中断文件成功
 */
 TEST_F(BackupJobTest, PrerequisiteJob_When_Not_Exit_Verify_File_Gen_Sha_Interrupt_File_OK)
 {
        stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_OK);
        stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_GenInterruptFile_CreateFSHandler_OK);
        stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);

        std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
        m_backupJobInfo.jobParam.backupType = AppProtect::BackupJobType::INCREMENT_BACKUP;
        std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupJobInfo);
        jobInfo->SetJobInfo(data);
        m_job.SetJobInfo(jobInfo);
        m_job.m_isCopyVerify = false;
        int ret = m_job.PrerequisiteJob();
        EXPECT_EQ(SUCCESS, ret);
 }


/*
 * 测试用例：执行备份前置任务
 * 前置条件：创建快照成功,然后中止任务
 * CHECK点：中止任务成功,状态置为STATE_NONE
 */
 TEST_F(BackupJobTest, AbortJobPrerequisiteCreateSnapshot)
 {
     INFOLOG("Case: AbortJobPrerequisiteSuccess");
     stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_OK);
     stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
     stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
     stub.set(ADDR(VirtPlugin::BackupJob, CreateSnapshot), Stub_CreateSnapshot_Success);
     int ret = m_job.PrerequisiteJob();
     EXPECT_EQ(FAILED, ret);
     EXPECT_EQ(m_job.m_nextState, -1);
 }

/*
 * 测试用例：执行备份前置任务
 * 前置条件：创建快照成功,然后中止任务,释放动作返回失败，释放时进行重试
 * CHECK点：中止任务成功,状态置为STATE_NONE
 */
 TEST_F(BackupJobTest, AbortJobPrerequisiteCreateSnapshotRetry)
 {
     INFOLOG("Case: AbortJobPrerequisiteSuccess");
     stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_OK);
     stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
     stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
     stub.set(ADDR(VirtPlugin::BackupJob, CreateSnapshot), Stub_CreateSnapshot_With_Delete_Fail);
     int ret = m_job.PrerequisiteJob();
     EXPECT_EQ(FAILED, ret);
     EXPECT_EQ(m_job.m_nextState, -1);
 }

/* Test ProtectEngine failure cases */
/*
 * 测试用例：执行备份前置任务
 * 前置条件：创建快照失败
 * CHECK点：执行备份前置任务失败
 */
TEST_F(BackupJobTest, PrerequisiteJob_CreateSnapshot_Failed)
{
    INFOLOG("Case: PrerequisiteJob_CreateSnapshot_Failed.");
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_CreateSnapshot_Failed);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.PrerequisiteJob();
    EXPECT_EQ(FAILED, ret);
}

/*
 * 测试用例：执行备份前置任务
 * 前置条件：获取虚拟机元数据失败
 * CHECK点：执行备份前置任务失败
 */
TEST_F(BackupJobTest, PrerequisiteJob_GetMachineMetadata_Failed)
{
    INFOLOG("Case: PrerequisiteJob_GetMachineMetadata_Failed.");

    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_GetMachineMetadata_Failed);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.PrerequisiteJob();
    EXPECT_EQ(FAILED, ret);
}

/* Test FileSystemHandler failure cases */
/*
 * 测试用例：执行备份前置任务
 * 前置条件：元数据持久化打开文件失败
 * CHECK点：执行备份前置任务失败
 */
TEST_F(BackupJobTest, PrerequisiteJob_OpenFailed)
{
    INFOLOG("Case: PrerequisiteJob_OpenFailed.");
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler),
        Stub_CreateFSHandler_Open_Failed);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.PrerequisiteJob();
    EXPECT_EQ(FAILED, ret);
}

/*
 * 测试用例：执行备份前置任务
 * 前置条件：元数据持久化写文件失败
 * CHECK点：执行备份前置任务失败
 */
TEST_F(BackupJobTest, PrerequisiteJob_WriteFailed)
{
    INFOLOG("Case: PrerequisiteJob_WriteFailed.");
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler),
        Stub_CreateFSHandler_WRITE_Failed);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.PrerequisiteJob();
    EXPECT_EQ(FAILED, ret);
}

/*
 * 测试用例：执行备份前置任务
 * 前置条件：元数据持久化检查元数据仓是否存在失败
 * CHECK点：执行备份前置任务失败
 */
TEST_F(BackupJobTest, PrerequisiteJob_ExistsFailed)
{
    INFOLOG("Case: PrerequisiteJob_ExistsFailed.");

    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler),
        Stub_CreateFSHandler_Exists_Failed);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.PrerequisiteJob();
    EXPECT_EQ(FAILED, ret);
}

/* Test execute sub job cases */
/*
 * 测试用例：执行备份子任务
 * 前置条件：创建Handler失败
 * CHECK点：执行备份子任务失败
 */
TEST_F(BackupJobTest, ExecuteSubJob_HandlerFailed)
{

    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_GetVolumeHandler_Failed);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(FAILED, ret);
}

/*
 * 测试用例：执行备份子任务
 * 前置条件：获取数据仓失败
 * CHECK点：执行备份子任务失败
 */
TEST_F(BackupJobTest, ExecuteSubJobFailed_RepoNotExist)
{

    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_Exists_Failed);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(FAILED, ret);
}

/*
 * 测试用例：执行备份子任务
 * 前置条件：打开卷失败
 * CHECK点：执行备份子任务失败
 */
TEST_F(BackupJobTest, ExecuteSubJob_OpenVolFailed)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_Open_Failed);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(FAILED, ret);
}

/*
 * 测试用例：执行备份子任务
 * 前置条件：获取DirtyRange失败
 * CHECK点：执行备份子任务失败
 */
TEST_F(BackupJobTest, ExecuteSubJob_GetDirtyRangesFailed)
{

    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_GetDirtyRanges_Failed);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(FAILED, ret);
}

/*
 * 测试用例：执行备份子任务
 * 前置条件：读取卷数据块失败
 * CHECK点：执行备份子任务失败
 */
TEST_F(BackupJobTest, ExecuteSubJob_ReadVolumeFailed)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_Failed);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(FAILED, ret);
}

/*
 * 测试用例：执行备份子任务
 * 前置条件：写入数据到文件系统失败
 * CHECK点：执行备份子任务失败
 */
TEST_F(BackupJobTest, ExecuteSubJob_WriteFsFailed)
{

    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_WRITE_Failed);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(FAILED, ret);
}

/*
 * 测试用例：执行备份子任务
 * 前置条件：打开sha256文件失败
 * CHECK点：执行备份子任务失败
 */
TEST_F(BackupJobTest, ExecuteSubJob_Sha256WriteFailed)
{

    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_WRITE_Failed);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(FAILED, ret);
}

/*
 * 测试用例：执行备份子任务
 * 前置条件：获取VolumeData失败
 * CHECK点：执行备份子任务成功---获取volumeData失败，不影响备份任务
 */
TEST_F(BackupJobTest, ExecuteSubJob_Sha256CalcFailed)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(ADDR(BackupIoTask, GetSha256Data), StubGetSha256DataFailed);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    m_job.m_sha256Result = true;
    m_job.m_isCopyVerify = true;
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * 测试用例：执行备份子任务
 * 前置条件：计算Sha256失败时，获取计算结果为false
 * CHECK点：执行备份子任务成功---长度为0保存失败不影响备份任务
 */
TEST_F(BackupJobTest, ExecuteSubJob_OK_GetDataLen0Failed)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(ADDR(BackupIoTask, GetSha256Data), StubGetSha256DataCalcRetFalse);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    m_job.m_sha256Result = true;
    m_job.m_isCopyVerify = true;
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * 测试用例：执行备份子任务
 * 前置条件：保存块Sha256值时计算SHA256值失败
 * CHECK点：执行备份子任务成功---块Sha256值保存失败不影响备份任务
 */
TEST_F(BackupJobTest, ExecuteSubJob_OK_CalcuValueFailed)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(ADDR(CalculateSha256, CalculateSha256Value), StubCalculateSha256ValueFailed);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    m_job.m_sha256Result = true;
    m_job.m_isCopyVerify = true;
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * 测试用例：执行备份子任务
 * 前置条件：保存块Sha256值时计算sha256文件偏移值失败
 * CHECK点：执行备份子任务成功---块Sha256值保存失败不影响备份任务
 */
TEST_F(BackupJobTest, ExecuteSubJob_OK_CalueSeekFailed)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(ADDR(CalculateSha256, CalculateSha256Value), StubCalculateSha256DeviationFailed);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    m_job.m_sha256Result = true;
    m_job.m_isCopyVerify = true;
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * 测试用例：执行备份子任务
 * 前置条件：保存块Sha256值成功
 * CHECK点：执行备份子任务成功
 */
TEST_F(BackupJobTest, ExecuteSubJob_OK_SaveValueSuccess)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    m_job.m_sha256Result = true;
    m_job.m_isCopyVerify = true;
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * 测试用例：执行备份子任务
 * 前置条件：分段写sha256buf至meta仓，metahandle为空时
 * CHECK点：执行备份子任务成功--写分段sha256值不影响备份任务
 */
TEST_F(BackupJobTest, ExecuteSubJob_OK_MetaHandlerNullSuccess)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    m_job.m_metaRepoHandler = nullptr;
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * 测试用例：执行备份子任务
 * 前置条件：分段写sha256buf至meta仓，设置偏移量失败
 * CHECK点：执行备份子任务成功--设置sha256文件偏移失败不影响备份任务
 */
TEST_F(BackupJobTest, ExecuteSubJob_OK_SeekFailed)
{
    typedef int32_t (*FptrSeek)(FileSystemHandler*,size_t, int);
    FptrSeek fptrSeek = (FptrSeek)(&FileSystemHandler::Seek);

    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    stub.set(fptrSeek, Stub_SeekFailed);
    m_job.m_sha256Result = true;
    m_job.m_isCopyVerify = true;
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * 测试用例：执行备份子任务
 * 前置条件：保存块的sha256值成功
 * CHECK点：执行备份子任务成功
 */
TEST_F(BackupJobTest, ExecuteSubJob_OK_SaveSuccess)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(ADDR(CalculateSha256, CalculateSha256Value), StubCalculateSha256ValueSuccess);
    stub.set(ADDR(VirtPlugin::BackupJob, SaveBlockSha256Value), StubSaveBlockSha256ValueSuccess);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    m_job.m_sha256Result = true;
    m_job.m_isCopyVerify = true;
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * 测试用例：执行备份子任务
 * 前置条件：生成检验状态文件成功（校验文件未生成）
 * CHECK点：执行备份子任务成功-检验文件不影响备份任务执行
 */
TEST_F(BackupJobTest, ExecuteSubJob_OK_StateFileSuccess)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(ADDR(VirtPlugin::BackupJob, SaveBlockSha256Value), StubSaveBlockSha256ValueSuccess);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    m_job.m_sha256Result = false;
    m_job.m_isCopyVerify = false;
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * 测试用例：执行备份子任务
 * 前置条件：初始化Sha256文件失败
 * CHECK点：执行备份子任务成功-检验文件生成失败不影响备份任务
 */
TEST_F(BackupJobTest, ExecuteSubJobSuccessButInitSha256FileFailed)
{

    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(ADDR(VirtPlugin::BackupJob, SaveBlockSha256Value), StubSaveBlockSha256ValueSuccess);
    stub.set(ADDR(VirtPlugin::BackupJob, InitSha256File), StubInitSha256FileFailed);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    m_job.m_sha256Result = true;
    m_job.m_isCopyVerify = true;
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * 测试用例：执行备份子任务
 * 前置条件：创建卷/文件句柄成功，获取、备份DirtyRange成功
 * CHECK点：执行备份子任务成功
 */
TEST_F(BackupJobTest, ExecuteSubJob_OK)
{
    INFOLOG("Case: ExecuteSubJob_OK.");
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(ADDR(VirtPlugin::BackupJob, SaveBlockSha256Value), StubSaveBlockSha256ValueSuccess);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(SUCCESS, ret);
    INFOLOG("Case: ExecuteSubJob_OK END.");
}

/*
 * 测试用例：执行备份子任务，根据子任务名包含索引数字，数据仓path返回对应挂载点
 * 前置条件：创建卷/文件句柄成功，获取、备份DirtyRange成功
 * CHECK点：子任务名解析正确，获取挂载点正确
 */
TEST_F(BackupJobTest, ExecuteSubJob_GetDataRepoPathWhenIndexExist)
{
    INFOLOG("Case: ExecuteSubJob_GetDataRepoPathWhenIndexExist.");
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(ADDR(VirtPlugin::BackupJob, SaveBlockSha256Value), StubSaveBlockSha256ValueSuccess);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    m_subJob.__set_jobName("VirtualizationBusinessSubJob_1");
    std::shared_ptr<AppProtect::SubJob> subJobInfo = std::make_shared<AppProtect::SubJob>(m_subJob);
    m_job.SetSubJob(subJobInfo);
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(SUCCESS, ret);
    INFOLOG("Case: ExecuteSubJob_GetDataRepoPathWhenIndexExist END.");
}

/*
 * 测试用例：执行备份子任务，根据子任务名无索引数字，数据仓path返回默认挂载点
 * 前置条件：创建卷/文件句柄成功，获取、备份DirtyRange成功
 * CHECK点：子任务名解析正确，获取挂载点正确
 */
TEST_F(BackupJobTest, ExecuteSubJob_GetDataRepoPathWhenIndexNotExist)
{
    INFOLOG("Case: ExecuteSubJob_GetDataRepoPathWhenIndexNotExist.");
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(ADDR(VirtPlugin::BackupJob, SaveBlockSha256Value), StubSaveBlockSha256ValueSuccess);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    m_subJob.__set_jobName("subjob");
    std::shared_ptr<AppProtect::SubJob> subJobInfo = std::make_shared<AppProtect::SubJob>(m_subJob);
    m_job.SetSubJob(subJobInfo);
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(SUCCESS, ret);
    INFOLOG("Case: ExecuteSubJob_GetDataRepoPathWhenIndexNotExist END.");
}

/*
 * 测试用例：执行备份子任务
 * 前置条件：创建sha256文件句柄失败
 * CHECK点：执行备份子任务失败
 */
TEST_F(BackupJobTest, ExecuteSubJob_CreateSha256Falied)
{

    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_WRITE_Failed);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(FAILED, ret);
}

/*
 * 测试用例：执行备份子任务
 * 前置条件：打开sha256文件失败
 * CHECK点：执行备份子任务失败
 */
TEST_F(BackupJobTest, ExecuteSubJob_Sha256OpenFailed)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_Open_Failed);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(FAILED, ret);
}

/*
 * 测试用例：初始化SHA256校验文件
 * 前置条件：元数据路径为空失败
 * CHECK点：初始化SHA256校验文件失败
 */
TEST_F(BackupJobTest, InitSha256File_InitFailed2)
{

    int32_t retValue = m_job.InitSha256File();
    EXPECT_EQ(FAILED, retValue);
}

/*
 * 测试用例：初始化SHA256校验文件
 * 前置条件：初始化成功
 * CHECK点：初始化SHA256校验文件成功
 */
TEST_F(BackupJobTest, InitSha256File_InitSuccess)
{
    typedef int32_t (*Fptr)(FileSystemHandler*,const std::string&, const std::string&);
    Fptr fptr = (Fptr)(&FileSystemHandler::Open);


    stub.set(ADDR(std::string, empty), Stub_Empty_False);
    stub.set(fptr, Stub_Open_Success);
    SetTestPara();
    int32_t retValue = m_job.InitSha256File();
    EXPECT_EQ(SUCCESS, retValue);
    stub.reset(ADDR(std::string, empty));
    stub.reset(fptr);
}

/*
 * 测试用例：执行备份子任务,中止任务
 * 前置条件：备份阶段中止任务
 * CHECK点：中止子任务成功
 */
TEST_F(BackupJobTest, ExecuteSubJob_Abort_OK)
{
    INFOLOG("Case: ExecuteSubJob_Abort_OK");

    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    // abort BackupDirtyRanges
    stub.set(ADDR(VirtPlugin::BackupJob, BackupDirtyRanges), Stub_BackupDirtyRanges_Abort);
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(FAILED, ret);
    INFOLOG("Case: ExecuteSubJob_Abort_OK END");
}

 /*
  * 用例名称：子任务运行过程中保存续作信息
  * 前置条件：无
  * check点：续作点初始化成功、续作信息保存成功
  */
 TEST_F(BackupJobTest, ExecuteSubJob_SaveCheckpoint_OK)
 {
     INFOLOG("Case: ExecuteSubJob_SaveCheckpoint_OK");

     stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
     stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
     stub.set(ADDR(VirtPlugin::BackupJob, ExecBlockTaskStart), Stub_ExecBlockTaskStart);
     stub.set(ADDR(VirtPlugin::BackupJob, ReportBackupSpeed), Stub_ReportBackupSpeed);
     stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
     int ret = m_job.ExecuteSubJob();
     std::string expect_dir = m_job.m_cacheRepoPath + VIRT_PLUGIN_CACHE_CHECKPOINT_ROOT + "main_job_main_job_id";
     EXPECT_EQ(m_job.m_checkpoint.m_dir, expect_dir);
     std::string expect_file =
             m_job.m_cacheRepoPath + VIRT_PLUGIN_CACHE_CHECKPOINT_ROOT + "main_job_main_job_id/sub_job_sub_job_id.json";
     EXPECT_EQ(m_job.m_checkpoint.m_file, expect_file);
     EXPECT_EQ(m_job.m_backupCheckpointInfo.m_completedSegmentSize, m_subJobInfo.m_volInfo.m_volSizeInBytes);
     EXPECT_EQ(SUCCESS, ret);
     INFOLOG("Case: ExecuteSubJob_SaveCheckpoint_OK END");
 }

/*
 * 用例名称：子任务运行过程中保存续作信息，创建文件，第一次open失败，重试成功
 * 前置条件：无
 * check点：续作信息保存成功
 */
TEST_F(BackupJobTest, ExecuteSubJob_SaveCheckpoint_Retry_Open_OK)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_Checkpoint_CreateFSHandler_Open_Retry_Success);
    stub.set(ADDR(VirtPlugin::BackupJob, ExecBlockTaskStart), Stub_ExecBlockTaskStart);
    stub.set(ADDR(VirtPlugin::BackupJob, OpenRWHandler), Stub_OpenRWHandler);
    stub.set(ADDR(VirtPlugin::BackupJob, ReportBackupSpeed), Stub_ReportBackupSpeed);
    stub.set(ADDR(VirtPlugin::BackupJob, SaveBlockDataBitMap), Stub_SaveBlockDataBitMap);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.ExecuteSubJob();
    std::string expect_dir = m_job.m_cacheRepoPath + VIRT_PLUGIN_CACHE_CHECKPOINT_ROOT + "main_job_main_job_id";
    EXPECT_EQ(m_job.m_checkpoint.m_dir, expect_dir);
    std::string expect_file =
            m_job.m_cacheRepoPath + VIRT_PLUGIN_CACHE_CHECKPOINT_ROOT + "main_job_main_job_id/sub_job_sub_job_id.json";
    EXPECT_EQ(m_job.m_checkpoint.m_file, expect_file);
    EXPECT_EQ(m_job.m_backupCheckpointInfo.m_completedSegmentSize, m_subJobInfo.m_volInfo.m_volSizeInBytes);
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * 用例名称：子任务运行过程中保存续作信息，创建文件，3次open失败，最后失败
 * 前置条件：无
 * check点：续作信息保存失败
 */
TEST_F(BackupJobTest, ExecuteSubJob_SaveCheckpoint_Retry_Open_Fail)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_Checkpoint_CreateFSHandler_Open_Retry_Fail);
    stub.set(ADDR(VirtPlugin::BackupJob, ExecBlockTaskStart), Stub_ExecBlockTaskStart);
    stub.set(ADDR(VirtPlugin::BackupJob, ReportBackupSpeed), Stub_ReportBackupSpeed);
    stub.set(ADDR(VirtPlugin::BackupJob, OpenRWHandler), Stub_OpenRWHandler);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.ExecuteSubJob();
    std::string expect_dir = m_job.m_cacheRepoPath + VIRT_PLUGIN_CACHE_CHECKPOINT_ROOT + "main_job_main_job_id";
    EXPECT_EQ(m_job.m_checkpoint.m_dir, expect_dir);
    std::string expect_file =
            m_job.m_cacheRepoPath + VIRT_PLUGIN_CACHE_CHECKPOINT_ROOT + "main_job_main_job_id/sub_job_sub_job_id.json";
    EXPECT_EQ(m_job.m_checkpoint.m_file, expect_file);
    EXPECT_EQ(m_job.m_backupCheckpointInfo.m_completedSegmentSize, 0);
    EXPECT_EQ(FAILED, ret);
}

/*
 * 用例名称：子任务运行过程中保存续作信息，第一次写磁盘失败，重试成功
 * 前置条件：无
 * check点：续作信息保存成功
 */
TEST_F(BackupJobTest, ExecuteSubJob_SaveCheckpoint_Write_Retry_OK)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_Checkpoint_CreateFSHandler_Write_Retry_Success);
    stub.set(ADDR(VirtPlugin::BackupJob, ExecBlockTaskStart), Stub_ExecBlockTaskStart);
    stub.set(ADDR(VirtPlugin::BackupJob, ReportBackupSpeed), Stub_ReportBackupSpeed);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.ExecuteSubJob();
    std::string expect_dir = m_job.m_cacheRepoPath + VIRT_PLUGIN_CACHE_CHECKPOINT_ROOT + "main_job_main_job_id";
    EXPECT_EQ(m_job.m_checkpoint.m_dir, expect_dir);
    std::string expect_file =
            m_job.m_cacheRepoPath + VIRT_PLUGIN_CACHE_CHECKPOINT_ROOT + "main_job_main_job_id/sub_job_sub_job_id.json";
    EXPECT_EQ(m_job.m_checkpoint.m_file, expect_file);
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * 用例名称：子任务运行过程中保存续作信息，3次写磁盘失败
 * 前置条件：无
 * check点：续作信息保存失败
 */
TEST_F(BackupJobTest, ExecuteSubJob_SaveCheckpoint_Write_Retry_Fail)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_Checkpoint_CreateFSHandler_Write_Fail);
    stub.set(ADDR(VirtPlugin::BackupJob, ExecBlockTaskStart), Stub_ExecBlockTaskStart);
    stub.set(ADDR(VirtPlugin::BackupJob, ReportBackupSpeed), Stub_ReportBackupSpeed);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.ExecuteSubJob();
    std::string expect_dir = m_job.m_cacheRepoPath + VIRT_PLUGIN_CACHE_CHECKPOINT_ROOT + "main_job_main_job_id";
    EXPECT_EQ(m_job.m_checkpoint.m_dir, expect_dir);
    std::string expect_file =
            m_job.m_cacheRepoPath + VIRT_PLUGIN_CACHE_CHECKPOINT_ROOT + "main_job_main_job_id/sub_job_sub_job_id.json";
    EXPECT_EQ(m_job.m_checkpoint.m_file, expect_file);
    EXPECT_EQ(m_job.m_backupCheckpointInfo.m_completedSegmentSize, m_subJobInfo.m_volInfo.m_volSizeInBytes);
    EXPECT_EQ(FAILED, ret);
}

/*
 * 用例名称：子任务运行过程中读取续作信息，继续执行任务
 * 前置条件：无
 * check点：续作信息读取成功
 */
TEST_F(BackupJobTest, ExecuteSubJob_GetCheckpoint_OK)
{

    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_GetCheckpoint_OK);
    stub.set(ADDR(VirtPlugin::BackupJob, ExecBlockTaskStart), Stub_ExecBlockTaskStart);
    stub.set(ADDR(VirtPlugin::BackupJob, OpenRWHandler), Stub_OpenRWHandler);
    stub.set(ADDR(VirtPlugin::BackupJob, BackupDirtyRanges), Stub_BackupDirtyRanges);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(m_job.m_backupCheckpointInfo.m_completedSegmentSize, 666);
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * 用例名称：子任务运行过程中读取续作信息，第一次获取失败，重试成功
 * 前置条件：无
 * check点：续作信息读取成功
 */
TEST_F(BackupJobTest, ExecuteSubJob_GetCheckpoint_Retry_OK)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_Checkpoint_CreateFSHandler_Open_Retry_Success);
    stub.set(ADDR(VirtPlugin::BackupJob, ExecBlockTaskStart), Stub_ExecBlockTaskStart);
    stub.set(ADDR(VirtPlugin::BackupJob, OpenRWHandler), Stub_OpenRWHandler);
    stub.set(ADDR(VirtPlugin::BackupJob, BackupDirtyRanges), Stub_BackupDirtyRanges);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(m_job.m_backupCheckpointInfo.m_completedSegmentSize, 666);
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * 用例名称：子任务运行过程中读取续作信息，3次获取失败，最后失败
 * 前置条件：无
 * check点：续作信息读取成功
 */
TEST_F(BackupJobTest, ExecuteSubJob_GetCheckpoint_Retry_Fail)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_Checkpoint_CreateFSHandler_Open_Retry_Fail);
    stub.set(ADDR(VirtPlugin::BackupJob, ExecBlockTaskStart), Stub_ExecBlockTaskStart);
    stub.set(ADDR(VirtPlugin::BackupJob, OpenRWHandler), Stub_OpenRWHandler);
    stub.set(ADDR(VirtPlugin::BackupJob, BackupDirtyRanges), Stub_BackupDirtyRanges);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(FAILED, ret);
}

/*
 * 用例名称：测试子任务卷空间分块以及块是否包含有效数据逻辑是否正常
 * 前置条件：第一次为全量恢复：卷大小为50M，分段大小为20M，每段获取的DirtyRange分别为[0-20M], [24-32M], [44-50M]
 *          第二次为增量恢复：卷大小为50M，分段大小为20M，每段获取的DirtyRange分别为:
 *             Segment1: [0-4M]
 *             Segment2: [32-36M]
 *             Segment3: [40-44M]
 * check点：全量备份该卷的有效数据位图为"1111101100011"，增量备份后位图为"1111101110111"
 */
// TEST_F(BackupJobTest, ExecuteSubJob_CheckSegBlockDataOK)
// {
//     int twelve = 12;
//     VOL_SIZE = DEFAULT_BLOCK_SIZE * twelve + DEFAULT_BLOCK_SIZE / 2;  // 50M
//     stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_GetDirtyRangeOK);
//     stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_BitMapTest);
//     stub.set(ADDR(VirtPlugin::BackupJob, ExecBlockTaskStart), Stub_ExecBlockTaskStart);
//     stub.set(ADDR(VirtPlugin::BackupJob, OpenRWHandler), Stub_OpenRWHandler); 
//     stub.set(ADDR(VirtPlugin::VirtualizationBasicJob, GetSegementSizeFromConf), Stub_GetSegSizeFromConf);
//     stub.set(ADDR(VirtPlugin::BackupJob, UpdateInfoByCheckpoint), Stub_ReturnSuccess);
//     stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
//     int ret = m_job.ExecuteSubJob();
//     EXPECT_EQ(SUCCESS, ret);
//     const std::string filename = "/tmp/volumes_block_bitmap.info";
//     std::string bitmap;
//     const std::shared_ptr<RepositoryHandler> fileHandler = std::make_shared<FileSystemHandler>();
//     ret = Utils::ReadFile(fileHandler, filename, bitmap);
//     EXPECT_EQ(SUCCESS, ret);
//     std::string expectStr = "1111101100011";
//     ret = expectStr.compare(bitmap);
//     EXPECT_EQ(0, ret);
//     std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
//     m_backupJobInfo.jobParam.backupType = AppProtect::BackupJobType::INCREMENT_BACKUP;
//     std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::BackupJob>(m_backupJobInfo);
//     jobInfo->SetJobInfo(data);
//     m_job.m_ifSaveValidDataBitMap = INITIAL_STATE;
//     m_job.m_backupCheckpointInfo.m_completedSegmentSize = 0;
//     m_job.SetJobInfo(jobInfo);

//     ret = m_job.ExecuteSubJob();
//     EXPECT_EQ(SUCCESS, ret);
//     ret = Utils::ReadFile(fileHandler, filename, bitmap);
//     EXPECT_EQ(SUCCESS, ret);
//     expectStr = "1111101110111";
//     ret = expectStr.compare(bitmap);
//     EXPECT_EQ(0, ret);
//     stub.reset(ADDR(EngineFactory, CreateProtectEngine));
//     stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
//     stub.reset(ADDR(VirtPlugin::BackupJob, ExecBlockTaskStart));
//     stub.reset(ADDR(VirtPlugin::BackupJob, OpenRWHandler));
//     stub.reset(ADDR(VirtPlugin::VirtualizationBasicJob, GetSegementSizeFromConf));
//     stub.reset(ADDR(VirtPlugin::BackupJob, UpdateInfoByCheckpoint));
//     stub.reset(JobService_ReportJobDetails);
// }

/*
 * 用例名称：子任务运行结束后清除续作信息
 * 前置条件：无
 * check点：清除续作信息成功
 */
TEST_F(BackupJobTest, ExecuteSubJob_CleanCheckpoint_OK)
{
    INFOLOG("Case: ExecuteSubJob_CleanCheckpoint_OK");

    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_GetCheckpoint_OK);
    stub.set(ADDR(JobService, AddNewJob), Stub_AddNewJob_OK);
    stub.set(ADDR(JobService, ReportJobDetails), Stub_ReportJobDetails_OK);
    stub.set(ADDR(VirtPlugin::BackupJob, UpdateSnapshotFile), Stub_UpdateSnapshotFile);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    stub.set(JobService_ReportCopyAdditionalInfo, ReportCopyAdditionalInfoSuc);
    int ret = m_job.PostJob();
    EXPECT_EQ(SUCCESS, ret);
    INFOLOG("Case: ExecuteSubJob_CleanCheckpoint_OK END");
}

/*
 * 用例名称：子任务运行结束后清除续作信息，第一次删除失败，重试成功
 * 前置条件：无
 * check点：清除续作信息成功
 */
TEST_F(BackupJobTest, ExecuteSubJob_CleanCheckpoint_Retry_OK)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_Checkpoint_RemoveAll_Retry_OK);
    stub.set(ADDR(JobService, AddNewJob), Stub_AddNewJob_OK);
    stub.set(ADDR(JobService, ReportJobDetails), Stub_ReportJobDetails_OK);
    stub.set(ADDR(VirtPlugin::BackupJob, UpdateSnapshotFile), Stub_UpdateSnapshotFile);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    stub.set(JobService_ReportCopyAdditionalInfo, ReportCopyAdditionalInfoSuc);
    int ret = m_job.PostJob();
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * 用例名称：子任务运行结束后清除续作信息，3次删除失败，最后失败
 * 前置条件：无
 * check点：清除续作信息失败
 */
TEST_F(BackupJobTest, ExecuteSubJob_CleanCheckpoint_Retry_Fail)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_Checkpoint_RemoveAll_Retry_Fail);
    stub.set(ADDR(JobService, AddNewJob), Stub_AddNewJob_OK);
    stub.set(ADDR(JobService, ReportJobDetails), Stub_ReportJobDetails_OK);
    stub.set(ADDR(VirtPlugin::BackupJob, UpdateSnapshotFile), Stub_UpdateSnapshotFile);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    stub.set(JobService_ReportCopyAdditionalInfo, ReportCopyAdditionalInfoSuc);
    int ret = m_job.PostJob();
    EXPECT_EQ(FAILED, ret);
}

/*
 * 测试用例：执行拆解子任务
 * 前置条件：加载任务context成功，加载虚拟机信息成功，添加子任务成功
 * CHECK点：执行拆解子任务成功
 */
TEST_F(BackupJobTest, GenerateSubJob_OK)
{
    INFOLOG("Case: GenerateSubJob_OK.");

    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_GenerateSubJob_CreateFSHandler_OK);
    stub.set(ADDR(JobService, AddNewJob), Stub_AddNewJob_OK);
    stub.set(ADDR(JobService, ReportJobDetails), Stub_ReportJobDetails_OK);
    stub.set(JobService_AddNewJob, AddNewJobSuc);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    stub.set(ADDR(VirtPlugin::BackupJob, FillUpBackupSubJob), Stub_FillUpBackupSubJob_Success);
    int ret = m_job.GenerateSubJob();
    EXPECT_EQ(SUCCESS, ret);
    INFOLOG("Case: GenerateSubJob_OK END.");
}

/*
 * 测试用例：执行拆解子任务
 * 前置条件：加载任务context成功，加载虚拟机信息失败，添加子任务成功
 * CHECK点：执行拆解子任务失败
 */
TEST_F(BackupJobTest, GenerateSubJob_CreateFSHandler_Failed)
{
    INFOLOG("Case: GenerateSubJob_CreateFSHandler_Failed.");
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_Failed);
    stub.set(ADDR(JobService, AddNewJob), Stub_AddNewJob_OK);
    stub.set(ADDR(JobService, ReportJobDetails), Stub_ReportJobDetails_OK);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.GenerateSubJob();
    EXPECT_EQ(FAILED, ret);
}

/*
 * 测试用例：执行拆解子任务
 * 前置条件：加载任务context成功，加载虚拟机信息成功，添加子任务失败
 * CHECK点：执行拆解子任务失败
 */
TEST_F(BackupJobTest, GenerateSubJob_AddNewJob_Failed)
{
    INFOLOG("Case: GenerateSubJob_AddNewJob_Failed.");

    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_GenerateSubJob_CreateFSHandler_OK);
    stub.set(ADDR(JobService, AddNewJob), Stub_AddNewJob_Failed);
    stub.set(ADDR(JobService, ReportJobDetails), Stub_ReportJobDetails_OK);
    stub.set(ADDR(Module::CTime, Now), StubNowTime);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    int ret = m_job.GenerateSubJob();
    EXPECT_EQ(FAILED, ret);
}

/*
 * 测试用例：执行拆解子任务,中止任务
 * 前置条件：生成子任务后中止任务
 * CHECK点：中止拆解子任务成功
 */
TEST_F(BackupJobTest, GenerateSubJob_Abort_OK)
{
    INFOLOG("Case: GenerateSubJob_Abort_OK");

    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_GenerateSubJob_CreateFSHandler_OK);
    stub.set(ADDR(JobService, AddNewJob), Stub_AddNewJob_OK);
    stub.set(ADDR(JobService, ReportJobDetails), Stub_ReportJobDetails_OK);
    stub.set(JobService_AddNewJob, AddNewJobSuc);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    stub.set(ADDR(VirtPlugin::BackupJob, FillUpBackupSubJob), Stub_FillUpBackupSubJob_Success);
    stub.set(ADDR(VirtPlugin::BackupJob, DoGenerateSubJob), Stub_DoGenerateSubJob_Abort);
    int ret = m_job.GenerateSubJob();
    EXPECT_EQ(FAILED, ret);
    INFOLOG("Case: GenerateSubJob_Abort_OK END");
}

static std::shared_ptr<ProtectEngine> Stub_CreateEngine_DeleteSnapshot_Failed(const JobType &jobType,
    std::shared_ptr<JobCommonInfo> jobInfo)
{
    std::shared_ptr<JobHandle> jobHandle = std::make_shared<JobHandle>(jobType, jobInfo);
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>(jobHandle);
    EXPECT_CALL(*protectEngine, CreateSnapshot(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, GetMachineMetadata(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, DeleteSnapshot(_)).WillRepeatedly(Return(FAILED));
    EXPECT_CALL(*protectEngine, QuerySnapshotExists(_)).WillRepeatedly(Return(FAILED));
    EXPECT_CALL(*protectEngine, PreHook(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, PostHook(_)).WillRepeatedly(Return(SUCCESS));
    return protectEngine;
}

static std::shared_ptr<RepositoryHandler> Stub_PostJob_CreateFSHandler_OK(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(Main_Task_Status_Exists_Invoke)));
    EXPECT_CALL(*fsHandler, Seek(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(gSnapshotInfo.length()));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(FileSystemReadSnapshotSuccess_Invoke));
    EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, CopyFile(_, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Rename(_,_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Truncate(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, IsDirectory(_)).WillRepeatedly(Return(false));
    return fsHandler;
}

static std::shared_ptr<RepositoryHandler> Stub_PostJob_CreateFSHandler_DeleteSnapshot_Failed(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(Return(false));
    if (storageRepo.repositoryType == RepositoryDataType::META_REPOSITORY) {
        EXPECT_CALL(*fsHandler, FileSize(_))
            .WillRepeatedly(Return(gSnapshotInfo.length()));
        EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _))
            .WillRepeatedly(Invoke(FileSystemReadSnapshotSuccess_Invoke));
        EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
    } else if (storageRepo.repositoryType == RepositoryDataType::CACHE_REPOSITORY) {
        EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
        EXPECT_CALL(*fsHandler, Rename(_,_)).WillRepeatedly(Return(true));
    }
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    return fsHandler;
}

const std::string g_vmInfoStrPost = "{\"moRef\":\"\",\"volUuid\":\"\",\"volName\":\"\",\"volList\":[{\"moRef\":\"vol1\",\"uuid\":\"vol1\"},{\"moRef\":\"vol2\",\"uuid\":\"vol2\"}]}";
const std::string g_curSnapStr = "{\"moRef\":\"\",\"vmName\":\"\",\"vmMoRef\":\"\",\"volSnapList\":[{\"volUuid\":\"vol1\",\"snapshotName\":\"curSnap1\",\"snapshotId\":\"curSnap1\",\"storageSnapId\":\"\",\"snapshotWwn\":\"\",\"createTime\":\"\",\"reserved\":true,\"deleted\":false,},{\"volUuid\":\"vol2\",\"snapshotName\":\"curSnap2\",\"snapshotId\":\"curSnap2\",\"storageSnapId\":\"\",\"snapshotWwn\":\"\",\"createTime\":\"\",\"reserved\":true,\"deleted\":false,},],\"reserved\":true,\"deleted\":false,\"extendInfo\":\"\",}";

static int32_t FileSystemReadVMSuccess_Invoke(std::string &buf, size_t size)
{
    buf = g_vmInfoStrPost;
    return g_vmInfoStrPost.length();
}

static int32_t FileSystemReadCurSnapSuccess_Invoke(std::string &buf, size_t size)
{
    buf = g_curSnapStr;
    return g_curSnapStr.length();
}

static std::shared_ptr<RepositoryHandler> Stub_PostJob_CreateFSHandler_FullBackup_OK(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(DoAll(Invoke(Main_Task_Status_Exists_Invoke)));
    EXPECT_CALL(*fsHandler, Seek(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*fsHandler, FileSize(_)).Times(AtLeast(0)).WillOnce(Return(gSnapshotInfo.length()))
                                        .WillOnce(Return(gSnapshotInfo.length()))
                                        .WillOnce(Return(g_vmInfoStrPost.length()))
                                        .WillOnce(Return(g_curSnapStr.length()))
                                        .WillOnce(Return(gSnapshotInfo.length()));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).Times(AtLeast(0)).WillOnce(Invoke(FileSystemReadSnapshotSuccess_Invoke))
                                                        .WillOnce(Invoke(FileSystemReadSnapshotSuccess_Invoke))
                                                        .WillOnce(Invoke(FileSystemReadVMSuccess_Invoke))
                                                        .WillOnce(Invoke(FileSystemReadCurSnapSuccess_Invoke))
                                                        .WillOnce(Invoke(FileSystemReadSnapshotSuccess_Invoke));
    EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, CopyFile(_, _)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Rename(_,_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Truncate(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, IsDirectory(_)).WillRepeatedly(Return(false));
    return fsHandler;
}

static int32_t GetSnapshotsOfVolume1Succcess_Invoke(const VirtPlugin::VolInfo &volInfo, std::vector<VirtPlugin::VolSnapInfo> &snapList)
{
    std::vector<std::pair<std::string, std::string>> volSnapPairs;
    volSnapPairs.push_back(std::make_pair("vol1", "preSnap1"));
    volSnapPairs.push_back(std::make_pair("vol1", "curSnap1"));
    for (const auto &it : volSnapPairs) {
        VirtPlugin::VolSnapInfo volSnap;
        volSnap.m_volUuid = it.first;
        volSnap.m_snapshotId = it.second;
        volSnap.m_snapshotName = it.second;
        snapList.push_back(volSnap);
    }
    return SUCCESS;
}

static int32_t GetSnapshotsOfVolume2Succcess_Invoke(const VirtPlugin::VolInfo &volInfo, std::vector<VirtPlugin::VolSnapInfo> &snapList)
{
    std::vector<std::pair<std::string, std::string>> volSnapPairs;
    volSnapPairs.push_back(std::make_pair("vol2", "preSnap2"));
    volSnapPairs.push_back(std::make_pair("vol2", "curSnap2"));
    for (const auto &it : volSnapPairs) {
        VirtPlugin::VolSnapInfo volSnap;
        volSnap.m_volUuid = it.first;
        volSnap.m_snapshotId = it.second;
        volSnap.m_snapshotName = it.second;
        snapList.push_back(volSnap);
    }
    return SUCCESS;
}

static std::shared_ptr<ProtectEngine> Stub_CreateEngine_GetSnapshotsOfVolume_OK(const JobType &jobType,
    std::shared_ptr<JobCommonInfo> jobInfo)
{
    std::shared_ptr<JobHandle> jobHandle = std::make_shared<JobHandle>(jobType, jobInfo);
    std::shared_ptr<ProtectEngineMock> protectEngine = std::make_shared<ProtectEngineMock>(jobHandle);
    EXPECT_CALL(*protectEngine, CreateSnapshot(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, GetMachineMetadata(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, DeleteSnapshot(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, QuerySnapshotExists(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, PreHook(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, PostHook(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*protectEngine, GetSnapshotsOfVolume(_, A<std::vector<VirtPlugin::VolSnapInfo> &>())).WillRepeatedly(Return(SUCCESS));
    return protectEngine;
}

/*
 * 测试用例：执行后置子任务
 * 前置条件：加载任务context成功，加载快照信息成功，删除快照信息成功，更新context成功
 * CHECK点：执行后置子任务成功
 */
TEST_F(BackupJobTest, PostJob_OK)
{

    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_PostJob_CreateFSHandler_OK);
    stub.set(ADDR(JobService, AddNewJob), Stub_AddNewJob_OK);
    stub.set(ADDR(JobService, ReportJobDetails), Stub_ReportJobDetails_OK);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    stub.set(JobService_ReportCopyAdditionalInfo, ReportCopyAdditionalInfoSuc);
    int ret = m_job.PostJob();
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * 测试用例：执行后置子任务
 * 前置条件：加载任务context成功，加载快照信息成功，查询卷快照成功，删除快照信息成功，更新context成功
 * CHECK点：执行后置子任务成功
 */
TEST_F(BackupJobTest, PostJob_GetSnapshotsOfVolume_OK)
{

    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_GetSnapshotsOfVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_PostJob_CreateFSHandler_FullBackup_OK);
    stub.set(ADDR(JobService, AddNewJob), Stub_AddNewJob_OK);
    stub.set(ADDR(JobService, ReportJobDetails), Stub_ReportJobDetails_OK);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    stub.set(JobService_ReportCopyAdditionalInfo, ReportCopyAdditionalInfoSuc);
    int ret = m_job.PostJob();
    EXPECT_EQ(SUCCESS, ret);
}

/*
 * 测试用例：执行后置子任务
 * 前置条件：加载任务context成功，加载快照信息成功，删除快照信息失败
 * CHECK点：执行后置子任务失败
 */
TEST_F(BackupJobTest, PostJob_DeleteSnapshot_Failed)
{

    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_DeleteSnapshot_Failed);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_PostJob_CreateFSHandler_DeleteSnapshot_Failed);
    stub.set(ADDR(JobService, AddNewJob), Stub_AddNewJob_OK);
    stub.set(ADDR(JobService, ReportJobDetails), Stub_ReportJobDetails_OK);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    stub.set(JobService_ReportCopyAdditionalInfo, ReportCopyAdditionalInfoSuc);
    int ret = m_job.PostJob();
    EXPECT_EQ(FAILED, ret);
}

/*
 * 测试用例：执行后置子任务
 * 前置条件：加载任务context成功，加载快照信息成功，任务执行失败
 * CHECK点：执行后置子任务失败
 */
TEST_F(BackupJobTest, PostJob_Failed)
{

    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_DeleteSnapshot_Failed);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_PostJob_CreateFSHandler_DeleteSnapshot_Failed);
    stub.set(ADDR(JobService, AddNewJob), Stub_AddNewJob_OK);
    stub.set(ADDR(JobService, ReportJobDetails), Stub_ReportJobDetails_OK);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    stub.set(JobService_ReportCopyAdditionalInfo, ReportCopyAdditionalInfoSuc);
    m_job.SetPostJobResultType(AppProtect::JobResult::type::FAILED);
    int ret = m_job.PostJob();
    EXPECT_EQ(FAILED, ret);
}

/*
 * 测试用例：执行卷过滤测试
 * 前置条件：未设置
 * CHECK点：卷过滤结果符合预期
 */
TEST_F(BackupJobTest, VolumeFileter)
{

    stub.set(ADDR(JobService, ReportJobDetails), Stub_ReportJobDetails_OK);
    AppProtect::ApplicationResource res1, res2;
    VolInfo volInfo0, volInfo1, volInfo2;
    volInfo1.m_uuid = "uuid1";
    volInfo2.m_uuid = "uuid2";
    m_job.m_vmInfo.m_volList.push_back(volInfo0);
    m_job.m_vmInfo.m_volList.push_back(volInfo1);
    m_job.m_vmInfo.m_volList.push_back(volInfo2);
    m_job.m_volToBeBackupMap["uuid1"] = res1;
    m_job.m_volToBeBackupMap["uuid2"] = res2;
    m_job.VolumeFilter();
    EXPECT_EQ(m_job.m_vmInfo.m_volList.size(), 2);
}

/*
 * 测试用例：执行卷信息日志组装
 * 前置条件：未设置
 * CHECK点：日志信息组装符合预期
 */
TEST_F(BackupJobTest, GetSnapshotsLogDetails)
{
    SnapToBeDeleted snap;
    VolSnapInfo volSnap0, volSnap1, volSnap2;
    volSnap0.m_volUuid = "uuid0";
    volSnap1.m_volUuid = "uuid1";
    volSnap2.m_volUuid = "uuid2";
    volSnap0.m_snapshotName = "snap0";
    volSnap1.m_snapshotName = "snap1";
    volSnap2.m_snapshotName = "snap2";
    snap.m_snapshotInfo.m_volSnapList.push_back(volSnap0);
    snap.m_snapshotInfo.m_volSnapList.push_back(volSnap1);
    snap.m_snapshotInfo.m_volSnapList.push_back(volSnap2);
    std::string expectResult = "[vol id: uuid0, snapshot name: snap0][vol id: uuid1, snapshot name: snap1][vol id: uuid2, snapshot name: snap2]";
    EXPECT_EQ(m_job.GetSnapshotsLogDetails(snap.m_snapshotInfo.m_volSnapList), expectResult);
}

/*
 * 测试用例：执行副本上报业务子任务，上报副本信息失败
 * 前置条件：上报副本信息接口失败
 * CHECK点：子任务执行失败
 */
TEST_F(BackupJobTest, ExecuteLastSubJob_Failed)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    stub.set(JobService_ReportCopyAdditionalInfo, ReportCopyAdditionalInfoFailed);
    m_subJob.__set_jobName("ReportCopySubJob");
    std::shared_ptr<AppProtect::SubJob> subJobInfo = std::make_shared<AppProtect::SubJob>(m_subJob);
    m_job.SetSubJob(subJobInfo);
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(FAILED, ret);
}

/*
 * 测试用例：执行副本上报业务子任务，上报副本信息成功
 * 前置条件：上报副本信息接口成功
 * CHECK点：子任务执行成功
 */
TEST_F(BackupJobTest, ExecuteLastSubJob_Ok)
{
    stub.set(ADDR(EngineFactory, CreateProtectEngine), Stub_CreateEngine_ReadVolume_OK);
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(JobService_ReportJobDetails, ReportJobDetailsSuc);
    stub.set(JobService_ReportCopyAdditionalInfo, ReportCopyAdditionalInfoSuc);
    m_subJob.__set_jobName("ReportCopySubJob");
    std::shared_ptr<AppProtect::SubJob> subJobInfo = std::make_shared<AppProtect::SubJob>(m_subJob);
    m_job.SetSubJob(subJobInfo);
    int ret = m_job.ExecuteSubJob();
    EXPECT_EQ(SUCCESS, ret);
}

static std::string Stub_RunShell_Get_And_Save_OK(bool isOpenstack)
{
    static int count_shell = 0;
    if (count_shell == 0)
    {
        count_shell ++;
        return "6be2e11e-b65e-46b4-a743-f6f2f5dc60a5";
    } else {
        return "";
    }
}

/*
 * 测试用例：运行命令获取HostId成功
 * 前置条件：
 * CHECK点：获取的HostId正确
 */
TEST_F(BackupJobTest, GetProxyHostId_Ok)
{
    stub.set(Utils::DoGetProxyHostId, Stub_RunShell_Get_And_Save_OK);
    std::string hostId = Utils::GetProxyHostId(true);
    EXPECT_EQ(hostId, "6be2e11e-b65e-46b4-a743-f6f2f5dc60a5");
}
}
