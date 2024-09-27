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
#include <list>
#include <cstdio>
#include <openssl/sha.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include <job/JobCommonInfo.h>
#include "ClientInvoke.h"
#include <log/Log.h>
#include <common/Macros.h>
#include <common/Structs.h>
#include "common/utils/Utils.h"
#include "common/sha256/Sha256.h"
#include "common/CommonMock.h"
#include "job_controller/jobs/verify/VerifyJob.h"
#include "protect_engines/engine_factory/EngineFactory.h"
#include "repository_handlers/RepositoryHandler.h"
#include "repository_handlers/factory/RepositoryFactory.h"
#include "protect_engines/ProtectEngineMock.h"
#include "repository_handlers/mock/FileSystemHandlerMock.h"
#include "volume_handlers/volumeHandlerMock.h"
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

namespace HDT_TEST {
const int BLOCK_NUM = 5; 
const std::string SHA256_FILE_PATH = "/sha256file";
const std::string volMetaData = "{\"uuid\":\"1\",\"moRef\":\"1-1\"}";
const std::string volumeExtendInfo = "{\
    \"copyVerifyFile\" : \"true\",\
    \"dataAfterReduction\" : 1639908,\
    \"dataBeforeReduction\" : 4274497,\
    \"multiFileSystem\" : \"false\",\
    \"volList\" : [{\
        \"attachments\" : null,\
        \"bootable\" : \"true\",\
        \"datastore\" :{\
            \"dcMoRef\" : \"\",\
            \"extendInfo\" : \"\",\
            \"ip\" : \"\",\
            \"moRef\" : \"2102351NPT10J3000001\", \
            \"name\" : \"\",\
            \"poolId\" : \"-168868144\",\
            \"port\" : \"\",\
            \"type\" : \"OceanStorV5\",\
            \"volumeName\" : \"\"\
        },\
        \"extendInfo\" : \"\", \
        \"metadata\" : \"\",\
        \"moRef\" : \"feb072d5-4b20-419a-ad28-183161cc75a6\",\
        \"name\" : \"ecs-4d45-0008-volume-0000\",\
        \"slotId\" : \"\",\
        \"type\" : \"business_type_01\",\
        \"uuid\" : \"feb072d5-4b20-419a-ad28-183161cc75a6\",\
        \"vmMoRef\" : \"66a5e75c-2728-4da3-8b79-57ea0d0fe037\",\
        \"volSizeInBytes\" : 10737418240\
    } ]}";

const std::string volumeExtendInfoNull = "{\
    \"copyVerifyFile\" : \"true\",\
    \"dataAfterReduction\" : 1639908,\
    \"dataBeforeReduction\" : 4274497,\
    \"multiFileSystem\" : \"false\"}";

static int32_t FileSystemReadSnapshotSuccess_Invoke(std::string &buf, size_t size)
{
    buf = volMetaData;
    return volMetaData.length();
}

static int32_t FileSystemWriteSuccess_Invoke(const std::string &content)
{
    return content.length();
}

static int32_t FileSystemWriteBufSuccess_Invoke(std::shared_ptr<uint8_t[]> buf, size_t count)
{
    return count;
}

static void Stub_JobServiceAddNewJob_OK(ActionResult& result, const std::vector<SubJob>& subs)
{
    result.__set_code(SUCCESS);
    return;
}

static void Stub_JobServiceAddNewJob_NO(ActionResult& result, const std::vector<SubJob>& subs)
{
    result.__set_code(FAILED);
    return;
}

static void Stub_JobServiceReportJobDetails(ActionResult& result, const SubJobDetails& subJobDetails)
{
    return;
}

void Stub_ReportLog2AgentSuccess(ReportLog2AgentParam &param)
{
    return;
}

/* Stub for FileSystemHandler */
static std::shared_ptr<RepositoryHandler> Stub_CreateFSHandler_OK(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    testing::Mock::AllowLeak(fsHandler.get());
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, CreateDirectory(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(volMetaData.length()));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(FileSystemReadSnapshotSuccess_Invoke));
    EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Write(_,_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteBufSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Truncate(_)).WillRepeatedly(Return(SUCCESS));
    return fsHandler;
}

static std::shared_ptr<RepositoryHandler> Stub_CreateFSHandler_Read_Failed(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    testing::Mock::AllowLeak(fsHandler.get());
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, CreateDirectory(_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(volMetaData.length()));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Return(FAILED));
    EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Write(_,_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteBufSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Seek(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
    EXPECT_CALL(*fsHandler, Truncate(_)).WillRepeatedly(Return(SUCCESS));
    return fsHandler;
}

/* Stub for FileSystemHandler */
static std::shared_ptr<RepositoryHandler> Stub_CreateFSHandler_NoExist(
    const AppProtect::StorageRepository &storageRepo)
{
    std::shared_ptr<FileSystemHandlerMock> fsHandler = std::make_shared<FileSystemHandlerMock>();
    EXPECT_CALL(*fsHandler, Open(_,_)).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, Close()).WillRepeatedly(Return(SUCCESS));
    EXPECT_CALL(*fsHandler, FileSize(_)).WillRepeatedly(Return(volMetaData.length()));
    EXPECT_CALL(*fsHandler, Read(A<std::string &>(), _)).WillRepeatedly(Invoke(FileSystemReadSnapshotSuccess_Invoke));
    EXPECT_CALL(*fsHandler, Write(_)).WillRepeatedly(DoAll(Invoke(FileSystemWriteSuccess_Invoke)));
    EXPECT_CALL(*fsHandler, Exists(_)).WillRepeatedly(Return(false));
    EXPECT_CALL(*fsHandler, Remove(_)).WillRepeatedly(Return(true));
    return fsHandler;
}

enum class RepoObjectValue {
    REPO_NONE = 0,
    REPO_META_OBJ,
    REPO_DATA_OBJ,
    REPO_META_DATA_OBJ
};

class CopyVerifyJobTest : public testing::Test {
public:
    void SetUp() {
        stub.set(sleep, Stub_Sleep);
    }
    void TearDown() {}

    void SetRepoObjectInfo(const int32_t& repoObj)
    {
        if (repoObj == 1) {
            AppProtect::StorageRepository metaRepo;
            metaRepo.protocol = RepositoryProtocolType::NFS;
            metaRepo.repositoryType = RepositoryDataType::META_REPOSITORY;
            metaRepo.path.push_back(metaPath);
            AppProtect::Copy copyItem;
            copyItem.repositories.push_back(metaRepo);
            m_checkCopyJob.copies.push_back(copyItem);
        } else if (repoObj == 2) {
            AppProtect::StorageRepository dataRepo;
			dataRepo.protocol = RepositoryProtocolType::NFS;
            dataRepo.repositoryType = RepositoryDataType::DATA_REPOSITORY;
            dataRepo.path.push_back(dataPath);
            AppProtect::Copy copyItem;
            copyItem.repositories.push_back(dataRepo);
            m_checkCopyJob.copies.push_back(copyItem);
        } else {
            AppProtect::StorageRepository metaRepo;
			metaRepo.protocol = RepositoryProtocolType::NFS;
            metaRepo.repositoryType = RepositoryDataType::META_REPOSITORY;
            metaRepo.path.push_back(metaPath);

            AppProtect::StorageRepository dataRepo;
            dataRepo.protocol = RepositoryProtocolType::NFS;
            dataRepo.repositoryType = RepositoryDataType::DATA_REPOSITORY;
            dataRepo.path.push_back(dataPath);

            AppProtect::Copy copyItem;
            copyItem.repositories.push_back(metaRepo);
            copyItem.repositories.push_back(dataRepo);
            m_checkCopyJob.copies.push_back(copyItem);
        }
    }

    void SetExtendInfo(const bool addItem = true)
    {
        std::string tempStr = "";
        if (addItem) {
            tempStr = volumeExtendInfo;
        } else {
            tempStr = volumeExtendInfoNull;
        }
        m_checkCopyJob.copies[0].extendInfo = tempStr;
    }

    void SetJobInfoData(const bool addJob = true)
    {
        std::shared_ptr<JobCommonInfo> jobInfo = std::make_shared<JobCommonInfo>();
        std::shared_ptr<ThriftDataBase> data = std::make_shared<AppProtect::CheckCopyJob>(m_checkCopyJob);
        if (addJob) {
            jobInfo->SetJobInfo(data);
        }
        m_copyVerifyJob.SetJobInfo(jobInfo);

        VerifySubJobInfo mockSubJob;
        mockSubJob.m_volUuid = volUuid;
        std::string subJobInfoStr;
        Module::JsonHelper::StructToJsonString(mockSubJob, subJobInfoStr);
        m_subJob.jobInfo = subJobInfoStr;
        std::shared_ptr<AppProtect::SubJob> subJob = std::make_shared<AppProtect::SubJob>(m_subJob);
        m_copyVerifyJob.SetSubJob(subJob);
    }

public:
    std::string metaPath = "/tmp/meta_repo";
    std::string dataPath = "/tmp/data_repo";
    std::string volUuid = "1111-111-11111-111";
    VirtPlugin::VerifyJob m_copyVerifyJob;
    AppProtect::CheckCopyJob m_checkCopyJob;
    AppProtect::SubJob m_subJob;
    Stub stub;
};

/**
 * 用例名称：校验任务前置任务
 * 前置条件：默认前置成功
 * check点：校验任务前置任务成功
 */
TEST_F(CopyVerifyJobTest, PrerequisiteJob_SUCCESS)
{
    stub.set(ADDR(JobService, ReportJobDetails), Stub_JobServiceReportJobDetails);
    int32_t retValue = m_copyVerifyJob.PrerequisiteJob();
    EXPECT_EQ(retValue, SUCCESS);
    stub.reset(ADDR(JobService, ReportJobDetails));
}

/**
 * 用例名称：校验任务分解子任务
 * 前置条件：创建成功
 * check点：校验任务分解子任务成功
 */
TEST_F(CopyVerifyJobTest, GenerateSubJob_SUCCESS)
{
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(ADDR(JobService, AddNewJob), Stub_JobServiceAddNewJob_OK);
    stub.set(ADDR(JobService, ReportJobDetails), Stub_JobServiceReportJobDetails);
    SetRepoObjectInfo(static_cast<int32_t>(RepoObjectValue::REPO_META_DATA_OBJ));
    SetExtendInfo();
    SetJobInfoData();
    int32_t retValue = m_copyVerifyJob.GenerateSubJob();
    EXPECT_EQ(retValue, SUCCESS);
    stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
    stub.reset(ADDR(JobService, AddNewJob));
    stub.reset(ADDR(JobService, ReportJobDetails));
}

/**
 * 用例名称：校验任务分解子任务
 * 前置条件：JobcommonInfo为空
 * check点：校验任务分解子任务失败
 */
TEST_F(CopyVerifyJobTest, GenerateSubJob_JobCommonInfoFailed)
{
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(ADDR(JobService, ReportJobDetails), Stub_JobServiceReportJobDetails);
    SetRepoObjectInfo(static_cast<int32_t>(RepoObjectValue::REPO_META_DATA_OBJ));
    SetExtendInfo();
    int32_t retValue = m_copyVerifyJob.GenerateSubJob();
    EXPECT_EQ(retValue, FAILED);
    stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
    stub.reset(ADDR(JobService, ReportJobDetails));
}

/**
 * 用例名称：校验任务分解子任务
 * 前置条件：初始化时jobInfo为空
 * check点：校验任务分解子任务失败
 */
TEST_F(CopyVerifyJobTest, GenerateSubJob_JonInfoFailed)
{
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(ADDR(JobService, ReportJobDetails), Stub_JobServiceReportJobDetails);
    SetRepoObjectInfo(static_cast<int32_t>(RepoObjectValue::REPO_META_DATA_OBJ));
    SetExtendInfo();
    SetJobInfoData(false);
    int32_t retValue = m_copyVerifyJob.GenerateSubJob();
    EXPECT_EQ(retValue, FAILED);
    stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
    stub.reset(ADDR(JobService, ReportJobDetails));
}

/**
 * 用例名称：校验任务分解子任务
 * 前置条件：dataRepo地址不存在
 * check点：校验任务分解子任务失败
 */
TEST_F(CopyVerifyJobTest, GenerateSubJob_DataRepoPathFailed)
{
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_NoExist);
    stub.set(ADDR(JobService, ReportJobDetails), Stub_JobServiceReportJobDetails);
    SetRepoObjectInfo(static_cast<int32_t>(RepoObjectValue::REPO_DATA_OBJ));
    SetExtendInfo();
    SetJobInfoData();
    int32_t retValue = m_copyVerifyJob.GenerateSubJob();
    EXPECT_EQ(retValue, FAILED);
    stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
    stub.reset(ADDR(JobService, ReportJobDetails));
}

/**
 * 用例名称：校验任务分解子任务
 * 前置条件：metaRepo地址不存在
 * check点：校验任务分解子任务失败
 */
TEST_F(CopyVerifyJobTest, GenerateSubJob_MataRepoPathFailed)
{
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_NoExist);
    stub.set(ADDR(JobService, ReportJobDetails), Stub_JobServiceReportJobDetails);
    SetRepoObjectInfo(static_cast<int32_t>(RepoObjectValue::REPO_META_OBJ));
    SetExtendInfo();
    SetJobInfoData();
    int32_t retValue = m_copyVerifyJob.GenerateSubJob();
    EXPECT_EQ(retValue, FAILED);
    stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
    stub.reset(ADDR(JobService, ReportJobDetails));
}

/**
 * 用例名称：校验任务分解子任务
 * 前置条件：创建分解时jobInfo为空
 * check点：校验任务分解子任务失败
 */
TEST_F(CopyVerifyJobTest, GenerateSubJob_DataRepoPathFailed2)
{
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(ADDR(JobService, ReportJobDetails), Stub_JobServiceReportJobDetails);
    SetRepoObjectInfo(static_cast<int32_t>(RepoObjectValue::REPO_META_DATA_OBJ));
    SetExtendInfo();
    SetJobInfoData(false);
    int32_t retValue = m_copyVerifyJob.GenerateSubJob();
    EXPECT_EQ(retValue, FAILED);
    stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
    stub.reset(ADDR(JobService, ReportJobDetails));
}

/**
 * 用例名称：校验任务分解子任务
 * 前置条件：副本列表为空
 * check点：校验任务分解子任务失败
 */
TEST_F(CopyVerifyJobTest, GenerateSubJob_CopysNullFailed)
{
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(ADDR(JobService, ReportJobDetails), Stub_JobServiceReportJobDetails);
    SetRepoObjectInfo(static_cast<int32_t>(RepoObjectValue::REPO_META_DATA_OBJ));
    SetJobInfoData();
    int32_t retValue = m_copyVerifyJob.GenerateSubJob();
    EXPECT_EQ(retValue, FAILED);
    stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
    stub.reset(ADDR(JobService, ReportJobDetails));
}

/**
 * 用例名称：校验任务分解子任务
 * 前置条件：校验卷列表为空
 * check点：校验任务分解子任务失败
 */
TEST_F(CopyVerifyJobTest, GenerateSubJob_SubObjectsNullFailed)
{
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(ADDR(JobService, ReportJobDetails), Stub_JobServiceReportJobDetails);
    SetRepoObjectInfo(static_cast<int32_t>(RepoObjectValue::REPO_META_DATA_OBJ));
    SetExtendInfo(false);
    SetJobInfoData();
    int32_t retValue = m_copyVerifyJob.GenerateSubJob();
    EXPECT_EQ(retValue, FAILED);
    stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
    stub.reset(ADDR(JobService, ReportJobDetails));
}

/**
 * 用例名称：校验任务分解子任务
 * 前置条件：添加子任务失败
 * check点：校验任务分解子任务成功
 */
TEST_F(CopyVerifyJobTest, GenerateSubJob_AddSubFailed)
{
    stub.set(ADDR(RepositoryFactory, CreateRepositoryHandler), Stub_CreateFSHandler_OK);
    stub.set(ADDR(JobService, AddNewJob), Stub_JobServiceAddNewJob_NO);
    stub.set(ADDR(JobService, ReportJobDetails), Stub_JobServiceReportJobDetails);
    SetRepoObjectInfo(static_cast<int32_t>(RepoObjectValue::REPO_META_DATA_OBJ));
    SetExtendInfo();
    SetJobInfoData();
    int32_t retValue = m_copyVerifyJob.GenerateSubJob();
    EXPECT_EQ(retValue, FAILED);
    stub.reset(ADDR(RepositoryFactory, CreateRepositoryHandler));
    stub.reset(ADDR(JobService, AddNewJob));
    stub.reset(ADDR(JobService, ReportJobDetails));
}

/**
 * 用例名称：文件都存在时执行卷的校验子任务成功
 * 前置条件：数据镜像文件、校验文件均存在，且匹配
 * check点：校验子任务成功
 */
TEST_F(CopyVerifyJobTest, ExecSubJobSuccess)
{
    std::string imgFile = dataPath + "/" + volUuid + ".raw";
    std::string checkFile = metaPath + SHA256_FILE_PATH + "/" + volUuid + "_sha256.info";
    FileSystemHandler fsHandler;
    fsHandler.CreateDirectory(dataPath);
    fsHandler.CreateDirectory(metaPath);

    int blockNum = 10;
    std::shared_ptr<uint8_t[]> blockBuf = std::make_unique<uint8_t[]>(DIRTY_RANGE_BLOCK_SIZE);
    memset_s(blockBuf.get(), DIRTY_RANGE_BLOCK_SIZE, 1, DIRTY_RANGE_BLOCK_SIZE);
    fsHandler.Open(imgFile, "w");
    for(int i = 0; i < blockNum; ++i) {
        fsHandler.Write(blockBuf, DIRTY_RANGE_BLOCK_SIZE);
    }
    fsHandler.Close();

    std::shared_ptr<uint8_t[]> shaBuf = std::make_unique<uint8_t[]>(SHA256_DIGEST_LENGTH);
    CalculateSha256::CalculateSha256Value(blockBuf, DIRTY_RANGE_BLOCK_SIZE, shaBuf);
    fsHandler.Open(checkFile, "w");
    for(int i = 0; i < blockNum; ++i) {
        fsHandler.Write(shaBuf, SHA256_DIGEST_LENGTH);
    }
    fsHandler.Close();

    Stub stub;
    stub.set(ADDR(VirtualizationBasicJob, ReportLog2Agent), Stub_ReportLog2AgentSuccess);

    SetRepoObjectInfo(static_cast<int32_t>(RepoObjectValue::REPO_META_DATA_OBJ));
    SetExtendInfo();
    SetJobInfoData();

    int32_t retValue = m_copyVerifyJob.ExecuteSubJob();
    EXPECT_EQ(retValue, SUCCESS);
}

/**
 * 用例名称：副本数据被损坏时执行卷的校验子任务失败
 * 前置条件：数据镜像文件被损坏--校验数据损坏
 * check点：校验不通过--返回文件被损坏
 */
TEST_F(CopyVerifyJobTest, ExecSubJobFailedWhenDataBreak)
{
    std::string imgFile = dataPath + "/" + volUuid + ".raw";
    std::string checkFile = metaPath + SHA256_FILE_PATH + "/" + volUuid + "_sha256.info";
    FileSystemHandler fsHandler;
    fsHandler.CreateDirectory(dataPath);
    fsHandler.CreateDirectory(metaPath);

    std::shared_ptr<uint8_t[]> blockBuf = std::make_unique<uint8_t[]>(DIRTY_RANGE_BLOCK_SIZE);
    memset_s(blockBuf.get(), DIRTY_RANGE_BLOCK_SIZE, 1, DIRTY_RANGE_BLOCK_SIZE);
    fsHandler.Open(imgFile, "w");
    for(int i = 0; i < BLOCK_NUM; ++i) {
        fsHandler.Write(blockBuf, DIRTY_RANGE_BLOCK_SIZE);
    }
    // 构造数据损坏的场景
    fsHandler.Seek(DIRTY_RANGE_BLOCK_SIZE);
    fsHandler.Write("00112233");
    fsHandler.Close();

    std::shared_ptr<uint8_t[]> shaBuf = std::make_unique<uint8_t[]>(SHA256_DIGEST_LENGTH);
    CalculateSha256::CalculateSha256Value(blockBuf, DIRTY_RANGE_BLOCK_SIZE, shaBuf);
    fsHandler.Open(checkFile, "w");
    for(int i = 0; i < BLOCK_NUM; ++i) {
        fsHandler.Write(shaBuf, SHA256_DIGEST_LENGTH);
    }
    fsHandler.Close();

    stub.set(ADDR(VirtualizationBasicJob, ReportLog2Agent), Stub_ReportLog2AgentSuccess);

    SetRepoObjectInfo(static_cast<int32_t>(RepoObjectValue::REPO_META_DATA_OBJ));
    SetExtendInfo();
    SetJobInfoData();

    int32_t retValue = m_copyVerifyJob.ExecuteSubJob();
    EXPECT_EQ(retValue, DAMAGED);
}

/**
 * 用例名称：数据镜像为空洞文件时执行卷的校验子任务成功
 * 前置条件：数据镜像为空洞文件
 * check点：校验通过
 */
TEST_F(CopyVerifyJobTest, ExecSubJobSuccessWhenDataIsHole)
{
    std::string imgFile = dataPath + "/" + volUuid + ".raw";
    std::string checkFile = metaPath + SHA256_FILE_PATH + "/" + volUuid + "_sha256.info";
    FileSystemHandler fsHandler;
    fsHandler.CreateDirectory(dataPath);
    fsHandler.CreateDirectory(metaPath);

    int blockNum = 10; 
    std::shared_ptr<uint8_t[]> blockBuf = std::make_unique<uint8_t[]>(DIRTY_RANGE_BLOCK_SIZE);
    memset_s(blockBuf.get(), DIRTY_RANGE_BLOCK_SIZE, 1, DIRTY_RANGE_BLOCK_SIZE);
    fsHandler.Open(imgFile, "w");
    for(int i = 0; i < blockNum; ++i) {
        fsHandler.Write(blockBuf, DIRTY_RANGE_BLOCK_SIZE);
        fsHandler.Seek(DIRTY_RANGE_BLOCK_SIZE, SEEK_CUR); // 跳过1个4M数据块，制造空洞
    }
    fsHandler.Close();

    std::shared_ptr<uint8_t[]> shaBuf = std::make_unique<uint8_t[]>(SHA256_DIGEST_LENGTH);
    CalculateSha256::CalculateSha256Value(blockBuf, DIRTY_RANGE_BLOCK_SIZE, shaBuf);
    fsHandler.Open(checkFile, "w");
    for(int i = 0; i < blockNum; ++i) {
        fsHandler.Write(shaBuf, SHA256_DIGEST_LENGTH);
        fsHandler.Seek(SHA256_DIGEST_LENGTH, SEEK_CUR); // 跳过1个SHA256值（32字节），制造空洞
    }
    fsHandler.Close();

    Stub stub;
    stub.set(ADDR(VirtualizationBasicJob, ReportLog2Agent), Stub_ReportLog2AgentSuccess);

    SetRepoObjectInfo(static_cast<int32_t>(RepoObjectValue::REPO_META_DATA_OBJ));
    SetExtendInfo();
    SetJobInfoData();

    int32_t retValue = m_copyVerifyJob.ExecuteSubJob();
    EXPECT_EQ(retValue, SUCCESS);
}

/**
 * 用例名称：校验副本文件被损坏时执行卷的校验子任务失败
 * 前置条件：Sha256文件被损坏,文件长度不是32倍数
 * check点：校验不通过--返回文件被损坏
 */
TEST_F(CopyVerifyJobTest, ExecSubJobFailedWhenSha256FileLenFailed)
{
    std::string imgFile = dataPath + "/" + volUuid + ".raw";
    std::string checkFile = metaPath + SHA256_FILE_PATH + "/" + volUuid + "_sha256.info";
    FileSystemHandler fsHandler;
    fsHandler.CreateDirectory(dataPath);
    fsHandler.CreateDirectory(metaPath);

    std::shared_ptr<uint8_t[]> blockBuf = std::make_unique<uint8_t[]>(DIRTY_RANGE_BLOCK_SIZE);
    memset_s(blockBuf.get(), DIRTY_RANGE_BLOCK_SIZE, 1, DIRTY_RANGE_BLOCK_SIZE);
    fsHandler.Open(imgFile, "w");
    for(int i = 0; i < BLOCK_NUM; ++i) {
        fsHandler.Write(blockBuf, DIRTY_RANGE_BLOCK_SIZE);
    }
    fsHandler.Close();

    std::shared_ptr<uint8_t[]> shaBuf = std::make_unique<uint8_t[]>(SHA256_DIGEST_LENGTH);
    CalculateSha256::CalculateSha256Value(blockBuf, DIRTY_RANGE_BLOCK_SIZE, shaBuf);
    fsHandler.Open(checkFile, "w");
    for(int i = 0; i < BLOCK_NUM; ++i) {
        fsHandler.Write(shaBuf, SHA256_DIGEST_LENGTH);
    }
    // SHA256文件被损坏
    fsHandler.Write("00112233");
    fsHandler.Close();

    Stub stub;
    stub.set(ADDR(VirtualizationBasicJob, ReportLog2Agent), Stub_ReportLog2AgentSuccess);

    SetRepoObjectInfo(static_cast<int32_t>(RepoObjectValue::REPO_META_DATA_OBJ));
    SetExtendInfo();
    SetJobInfoData();

    int32_t retValue = m_copyVerifyJob.ExecuteSubJob();
    EXPECT_EQ(retValue, DAMAGED);
}

/**
 * 用例名称：校验副本文件被损坏时执行卷的校验子任务失败
 * 前置条件：Sha256文件被损坏,文件内容被修改
 * check点：校验不通过--返回文件被损坏
 */
TEST_F(CopyVerifyJobTest, ExecSubJobFailedWhenSha256FileDataChange)
{
    std::string imgFile = dataPath + "/" + volUuid + ".raw";
    std::string checkFile = metaPath + SHA256_FILE_PATH + "/" + volUuid + "_sha256.info";
    FileSystemHandler fsHandler;
    fsHandler.CreateDirectory(dataPath);
    fsHandler.CreateDirectory(metaPath);

    std::shared_ptr<uint8_t[]> blockBuf = std::make_unique<uint8_t[]>(DIRTY_RANGE_BLOCK_SIZE);
    memset_s(blockBuf.get(), DIRTY_RANGE_BLOCK_SIZE, 1, DIRTY_RANGE_BLOCK_SIZE);
    fsHandler.Open(imgFile, "w");
    for(int i = 0; i < BLOCK_NUM; ++i) {
        fsHandler.Write(blockBuf, DIRTY_RANGE_BLOCK_SIZE);
    }
    fsHandler.Close();

    std::shared_ptr<uint8_t[]> shaBuf = std::make_unique<uint8_t[]>(SHA256_DIGEST_LENGTH);
    CalculateSha256::CalculateSha256Value(blockBuf, DIRTY_RANGE_BLOCK_SIZE, shaBuf);
    fsHandler.Open(checkFile, "w");
    for(int i = 0; i < BLOCK_NUM; ++i) {
        fsHandler.Write(shaBuf, SHA256_DIGEST_LENGTH);
    }
    // SHA256文件被损坏
    fsHandler.Seek(SHA256_DIGEST_LENGTH);
    fsHandler.Write("00112233");
    fsHandler.Close();

    stub.set(ADDR(VirtualizationBasicJob, ReportLog2Agent), Stub_ReportLog2AgentSuccess);

    SetRepoObjectInfo(static_cast<int32_t>(RepoObjectValue::REPO_META_DATA_OBJ));
    SetExtendInfo();
    SetJobInfoData();

    int32_t retValue = m_copyVerifyJob.ExecuteSubJob();
    EXPECT_EQ(retValue, DAMAGED);
}

/**
 * 用例名称：校验副本文件被损坏时执行卷的校验子任务失败
 * 前置条件：校验文件被删除时
 * check点：校验不通过--返回文件被损坏
 */
TEST_F(CopyVerifyJobTest, ExecSubJobFailedWhenSha256FileRemove)
{
    std::string imgFile = dataPath + "/" + volUuid + ".raw";
    std::string checkFile = metaPath + SHA256_FILE_PATH + "/" + volUuid + "_sha256.info";
    FileSystemHandler fsHandler;
    fsHandler.CreateDirectory(dataPath);
    fsHandler.CreateDirectory(metaPath);

    int BLOCK_NUM = 5; 
    std::shared_ptr<uint8_t[]> blockBuf = std::make_unique<uint8_t[]>(DIRTY_RANGE_BLOCK_SIZE);
    memset_s(blockBuf.get(), DIRTY_RANGE_BLOCK_SIZE, 1, DIRTY_RANGE_BLOCK_SIZE);
    fsHandler.Open(imgFile, "w");
    for(int i = 0; i < BLOCK_NUM; ++i) {
        fsHandler.Write(blockBuf, DIRTY_RANGE_BLOCK_SIZE);
    }
    fsHandler.Close();

    std::shared_ptr<uint8_t[]> shaBuf = std::make_unique<uint8_t[]>(SHA256_DIGEST_LENGTH);
    CalculateSha256::CalculateSha256Value(blockBuf, DIRTY_RANGE_BLOCK_SIZE, shaBuf);
    fsHandler.Open(checkFile, "w");
    for(int i = 0; i < BLOCK_NUM; ++i) {
        fsHandler.Write(shaBuf, SHA256_DIGEST_LENGTH);
    }
    fsHandler.Close();
    // SHA256文件被删除
    fsHandler.Remove(checkFile);

    Stub stub;
    stub.set(ADDR(VirtualizationBasicJob, ReportLog2Agent), Stub_ReportLog2AgentSuccess);

    SetRepoObjectInfo(static_cast<int32_t>(RepoObjectValue::REPO_META_DATA_OBJ));
    SetExtendInfo();
    SetJobInfoData();

    int32_t retValue = m_copyVerifyJob.ExecuteSubJob();
    EXPECT_EQ(retValue, DAMAGED);
}

static size_t Stub_SeekFailed(size_t offset, int origin = 0)
{
    return FAILED;
}

static size_t Stub_FileSize_SUCCESS(const std::string &fileName)
{
    return BLOCK_NUM * SHA256_DIGEST_LENGTH;
}

/**
 * 用例名称：执行校验子任务
 * 前置条件：读取副本数据，设置偏移量失败
 * check点：执行校验子任务失败--校验文件未损坏
 */
TEST_F(CopyVerifyJobTest, ExecuteSubJob_SeekFailed)
{
    std::string imgFile = dataPath + "/" + volUuid + ".raw";
    std::string checkFile = metaPath + SHA256_FILE_PATH + "/" + volUuid + "_sha256.info";
    FileSystemHandler fsHandler;
    fsHandler.CreateDirectory(dataPath);
    fsHandler.CreateDirectory(metaPath);

    std::shared_ptr<uint8_t[]> blockBuf = std::make_unique<uint8_t[]>(DIRTY_RANGE_BLOCK_SIZE);
    memset_s(blockBuf.get(), DIRTY_RANGE_BLOCK_SIZE, 1, DIRTY_RANGE_BLOCK_SIZE);
    fsHandler.Open(imgFile, "w");
    for(int i = 0; i < BLOCK_NUM; ++i) {
        fsHandler.Write(blockBuf, DIRTY_RANGE_BLOCK_SIZE);
    }
    fsHandler.Close();

    std::shared_ptr<uint8_t[]> shaBuf = std::make_unique<uint8_t[]>(SHA256_DIGEST_LENGTH);
    CalculateSha256::CalculateSha256Value(blockBuf, DIRTY_RANGE_BLOCK_SIZE, shaBuf);
    fsHandler.Open(checkFile, "w");
    for(int i = 0; i < BLOCK_NUM; ++i) {
        fsHandler.Write(shaBuf, SHA256_DIGEST_LENGTH);
    }
    fsHandler.Close();

    typedef int32_t (*FptrSeek)(FileSystemHandler*,size_t, int);
    FptrSeek fptrSeek = (FptrSeek)(&FileSystemHandler::Seek);
    Stub stub;
    stub.set(ADDR(VirtualizationBasicJob, ReportLog2Agent), Stub_ReportLog2AgentSuccess);
    stub.set(fptrSeek, Stub_SeekFailed);
    SetRepoObjectInfo(static_cast<int32_t>(RepoObjectValue::REPO_META_DATA_OBJ));
    SetExtendInfo();
    SetJobInfoData();

    int32_t retValue = m_copyVerifyJob.ExecuteSubJob();
    EXPECT_EQ(retValue, FAILED);
}
}