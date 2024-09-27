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
#include "stub.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "gmock/gmock-actions.h"
#include "host/HostCommonService.h"
#include "Backup.h"
#include "Module/src/common/Path.h"
#include "HostCommonService.h"

using namespace std;
using namespace PluginUtils;
using namespace FilePlugin;

class HostCommonServiceImp : public HostCommonService {

public:
    int PrerequisiteJob(){}
    int GenerateSubJob(){}
    int ExecuteSubJob(){}
    int PostJob(){}

    HostCommonServiceImp():HostCommonService(){}
};

class HostCommonServiceTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    HostCommonServiceImp hostCommonServiceImp;
};

void HostCommonServiceTest::SetUp() {}
void HostCommonServiceTest::TearDown() {}
void HostCommonServiceTest::SetUpTestCase() {}
void HostCommonServiceTest::TearDownTestCase() {}

void FunctionVoid() {}

static void Stub_AddNewJob_SUCCESS(ActionResult& returnValue, void *obj)
{
    returnValue.code = Module::SUCCESS;
}

static void Stub_AddNewJob_FAILED(ActionResult& returnValue, void *obj)
{
    returnValue.code = Module::FAILED;
}

static bool Funciont_Success()
{
    return true;
}

static bool Funciont_Failed()
{
    return false;
}

/*
 * 用例名称:SerializeBackupStats
 * 前置条件：
 * check点：类BackupStats 转为 结构体BackupStatistic
 */
TEST_F(HostCommonServiceTest, SerializeBackupStats)
{

    BackupStats backupStats;
    backupStats.noOfDirToBackup = 1;
    BackupStatistic backupStatistic;

    hostCommonServiceImp.SerializeBackupStats(backupStats, backupStatistic);

    EXPECT_EQ(backupStatistic.noOfDirToBackup, backupStats.noOfDirToBackup);
}


/*
 * 用例名称:CalcuSumStructBackupStatistic
 * 前置条件：
 * check点：结构体信息求和
 */
TEST_F(HostCommonServiceTest, CalcuSumStructBackupStatistic)
{

    BackupStatistic A_backupStatistic, B_backupStatistic;
    A_backupStatistic.noOfDirToBackup = 1;
    B_backupStatistic.noOfDirToBackup = 2;

    BackupStatistic ret = hostCommonServiceImp.CalcuSumStructBackupStatistic(A_backupStatistic, B_backupStatistic);

    EXPECT_EQ(ret.noOfDirToBackup, 3);
}


/*
 * 用例名称:CalcuAddedBackupStatistic
 * 前置条件：
 * check点：结构体数据相加
 */
TEST_F(HostCommonServiceTest, CalcuAddedBackupStatistic)
{

    BackupStatistic A_backupStatistic, B_backupStatistic;
    A_backupStatistic.noOfDirToBackup = 2;
    B_backupStatistic.noOfDirToBackup = 1;

    BackupStatistic ret = hostCommonServiceImp.CalcuAddedBackupStatistic(A_backupStatistic, B_backupStatistic);

    EXPECT_EQ(ret.noOfDirToBackup, 1);
}


/*
 * 用例名称:GetIncBackupStats
 * 前置条件：
 * check点：差值函数返回结果校验
 */
TEST_F(HostCommonServiceTest, GetIncBackupStats)
{

    BackupStats currStats;
    BackupStatistic prevStats;

    BackupStatistic ret = hostCommonServiceImp.GetIncBackupStats(currStats, prevStats);

    EXPECT_EQ(ret.noOfDirToBackup, 0);
}

/*
 * 用例名称:CheckFilePathAndGetSrcFileList
 * 前置条件：
 * check点：检查文件路径，并获取源文件列表
 */
TEST_F(HostCommonServiceTest, CheckFilePathAndGetSrcFileList_Success)
{
    string srcDir = Module::CPath::GetInstance().GetLogPath();
    string dstDir = Module::CPath::GetInstance().GetRootPath();
    vector<string> srcFileList;

    bool ret = hostCommonServiceImp.CheckFilePathAndGetSrcFileList(srcDir, dstDir, srcFileList);

    EXPECT_EQ(ret, true);
}


TEST_F(HostCommonServiceTest, CheckFilePathAndGetSrcFileList_Fail)
{

    Stub stub;
    stub.set(Module::CFile::DirExist, Funciont_Failed);
    stub.set(GetFileListInDirectory, Funciont_Failed);

    string srcDir;
    string dstDir;
    vector<string> srcFileList;

    bool ret = hostCommonServiceImp.CheckFilePathAndGetSrcFileList(srcDir, dstDir, srcFileList);

    EXPECT_EQ(ret, false);

    stub.reset(Module::CFile::DirExist);
    stub.reset(GetFileListInDirectory);
}

/*
 * 用例名称:IsValidCtrlFile
 * 前置条件：
 * check点：判断控制文件路径的有效性
 */
TEST_F(HostCommonServiceTest, IsValidCtrlFile_Invalid)
{
    string ctrlFileFullPath = "txt.tmp";

    bool ret = hostCommonServiceImp.IsValidCtrlFile(ctrlFileFullPath);

    EXPECT_EQ(ret, false);
}

TEST_F(HostCommonServiceTest, IsValidCtrlFile_Hardlink)
{
    string ctrlFileFullPath = "/hardlink_control_";

    bool ret = hostCommonServiceImp.IsValidCtrlFile(ctrlFileFullPath);

    EXPECT_EQ(ret, true);
}

TEST_F(HostCommonServiceTest, IsValidCtrlFile_Mtime)
{
    string ctrlFileFullPath = "/mtime_";

    bool ret = hostCommonServiceImp.IsValidCtrlFile(ctrlFileFullPath);

    EXPECT_EQ(ret, true);
}

TEST_F(HostCommonServiceTest, IsValidCtrlFile_Control)
{
    string ctrlFileFullPath = "/control_";

    bool ret = hostCommonServiceImp.IsValidCtrlFile(ctrlFileFullPath);

    EXPECT_EQ(ret, true);
}

TEST_F(HostCommonServiceTest, IsValidCtrlFile_DotFile)
{
    string ctrlFileFullPath = "...";

    bool ret = hostCommonServiceImp.IsValidCtrlFile(ctrlFileFullPath);

    EXPECT_EQ(ret, false);
}

/*
 * 用例名称:InitSubTask
 * 前置条件：
 * check点：初始化子任务
 */
TEST_F(HostCommonServiceTest, InitSubTask)
{
    SubJob subjob;
    string ctrlFile = "/file.cpp";

    bool ret = hostCommonServiceImp.InitSubTask(subjob, ctrlFile);

    EXPECT_EQ(ret, true);
}

/*
 * 用例名称:CreateSubTask
 * 前置条件：
 * check点：创建子任务
 */
TEST_F(HostCommonServiceTest, CreateSubTask_TaskList)
{
    Stub stub;
    stub.set(ADDR(JobService, AddNewJob), Stub_AddNewJob_SUCCESS);

    vector<SubJob> subJobList;
    SubJob subJob;
    subJob.__set_jobId("");
    subJob.__set_jobName("subJobName");
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_policy(ExecutePolicy::LOCAL_NODE);
    subJob.__set_jobInfo("restoreSubJobInfoStr");
    subJob.__set_jobPriority(0);
    subJob.__set_ignoreFailed(false);
    subJobList.push_back(subJob);

    vector<string> ctrlFileList;
    ctrlFileList.push_back("nan jing");

    bool ret = hostCommonServiceImp.CreateSubTask(subJobList, ctrlFileList);

    EXPECT_EQ(ret, true);

    stub.reset(ADDR(JobService, AddNewJob));

}

/*
 * 用例名称:CreateSubTask
 * 前置条件：
 * check点：创建子任务，ret.code = 0
 */
TEST_F(HostCommonServiceTest, CreateSubTask_OneTask)
{

    Stub stub;
    stub.set(ADDR(JobService, AddNewJob), Stub_AddNewJob_SUCCESS);

    SubJob subJob;
    subJob.__set_jobId("");
    subJob.__set_jobName("subJobName");
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_policy(ExecutePolicy::LOCAL_NODE);
    subJob.__set_jobInfo("restoreSubJobInfoStr");
    subJob.__set_jobPriority(0);
    subJob.__set_ignoreFailed(false);

    bool ret = hostCommonServiceImp.CreateSubTask(subJob);

    EXPECT_EQ(ret, true);

    stub.reset(ADDR(JobService, AddNewJob));
}

/*
 * 用例名称:GetSubTaskInfoByFileName
 * 前置条件：
 * check点：检查多个分支中子任务类型
 */
TEST_F(HostCommonServiceTest, GetSubTaskInfoByFileName_Hardlink)
{

    string fileName = ".../hardlink_control_";
    string subTaskName = "";
    uint32_t subTaskType = 0;
    uint32_t subTaskPrio = 0;
    hostCommonServiceImp.GetSubTaskInfoByFileName(fileName, subTaskName, subTaskType, subTaskPrio);

    EXPECT_EQ(subTaskType, 4);
}

TEST_F(HostCommonServiceTest, GetSubTaskInfoByFileName_Mtime)
{

    string fileName = ".../mtime_";
    string subTaskName = "";
    uint32_t subTaskType = 0;
    uint32_t subTaskPrio = 0;
    hostCommonServiceImp.GetSubTaskInfoByFileName(fileName, subTaskName, subTaskType, subTaskPrio);

    EXPECT_EQ(subTaskType, 5);
}

TEST_F(HostCommonServiceTest, GetSubTaskInfoByFileName_Delete)
{

    string fileName = ".../delete_control_";
    string subTaskName = "";
    uint32_t subTaskType = 0;
    uint32_t subTaskPrio = 0;
    hostCommonServiceImp.GetSubTaskInfoByFileName(fileName, subTaskName, subTaskType, subTaskPrio);

    EXPECT_EQ(subTaskType, 3);
}

TEST_F(HostCommonServiceTest, GetSubTaskInfoByFileName_Control)
{

    string fileName = ".../control_";
    string subTaskName = "";
    uint32_t subTaskType = 0;
    uint32_t subTaskPrio = 0;
    hostCommonServiceImp.GetSubTaskInfoByFileName(fileName, subTaskName, subTaskType, subTaskPrio);

    EXPECT_EQ(subTaskType, 2);
}

/*
 * 用例名称:PrintSubJobInfo
 * 前置条件：
 * check点：打印信息输出到日志
 */
TEST_F(HostCommonServiceTest, PrintSubJobInfo)
{
    shared_ptr<SubJob> subJob = make_shared<AppProtect::SubJob>();
    EXPECT_NO_THROW(hostCommonServiceImp.PrintSubJobInfo(subJob));
}


/*
 * 用例名称:SetJobCtrlPhase
 * 前置条件：
 * check点：变量赋值
 */
TEST_F(HostCommonServiceTest, SetJobCtrlPhase)
{
    string jobCtrlPhase;
    EXPECT_NO_THROW(hostCommonServiceImp.SetJobCtrlPhase(jobCtrlPhase));
}

/*
 * 用例名称: GetSubJobTypeByFileName
 * 前置条件：
 * check点：获取子任务类型
 */
TEST_F(HostCommonServiceTest, GetSubJobTypeByFileName)
{
    string filename = "/hardlink_control_";
    EXPECT_NO_THROW(hostCommonServiceImp.GetSubJobTypeByFileName(filename));
    filename = "/mtime_";
    EXPECT_NO_THROW(hostCommonServiceImp.GetSubJobTypeByFileName(filename));
    filename = "/delete_control_";
    EXPECT_NO_THROW(hostCommonServiceImp.GetSubJobTypeByFileName(filename));
}

/*
 * 用例名称: 初始化IdGenerator
 * 前置条件：
 * check点：获取子任务类型
 */
TEST_F(HostCommonServiceTest, InitIdGenerator)
{
    bool ret = hostCommonServiceImp.InitIdGenerator();
    EXPECT_EQ(ret, true);
}
