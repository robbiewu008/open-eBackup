#ifndef __WAL_BACKUP_TEST_H__
#define __WAL_BACKUP_TEST_H__

#include "stub.h"
#include "gtest/gtest.h"
#include "common/Types.h"
#include "taskmanager/externaljob/Job.h"
#include "common/JsonUtils.h"
#include "taskmanager/externaljob/PluginJobFactory.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"
#include <vector>
#include <iostream>
#define private public

class LogBackupTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void LogBackupTest::SetUp() {}

void LogBackupTest::TearDown() {}

void LogBackupTest::SetUpTestCase() {}

void LogBackupTest::TearDownTestCase() {}

mp_int32 StubPreWalBackupTestSuccess(const AppProtect::PluginJobData &data)
{
    return MP_SUCCESS;
}

mp_int32 StubPreWalBackupTestFail(const AppProtect::PluginJobData &data)
{
    return MP_FAILED;
}

mp_int32 StubAssembleRepositoryTestSuccess(AppProtect::PluginJobData &data)
{
    return MP_SUCCESS;
}

mp_int32 StubAssembleRepositoryTestFail(AppProtect::PluginJobData &data)
{
    return MP_FAILED;
}

mp_bool StubTmpLogDirIsExistTestSuccess(const AppProtect::PluginJobData &data)
{
    return MP_TRUE;
}

mp_bool StubTmpLogDirIsExistTestFail(const AppProtect::PluginJobData &data)
{
    return MP_FALSE;
}

mp_bool StubIsLogBackupJobTestTrue(const AppProtect::PluginJobData &data)
{
    return MP_TRUE;
}

mp_bool StubIsLogBackupJobTestFalse(const AppProtect::PluginJobData &data)
{
    return MP_FALSE;
}

mp_int32 StubReadMetaDataTestSuccess(mp_string &lastLogBackupPath, const mp_string &mainID,
    std::vector<mp_string> &vecOutput)
{
    return MP_SUCCESS;
}

mp_int32 StubReadMetaDataTestFail(mp_string &lastLogBackupPath, const mp_string &mainID,
    std::vector<mp_string> &vecOutput)
{
    return MP_FAILED;
}


mp_int32 StubGetExtendInfoTestSuccess(void* obj, AppProtect::PluginJobData &data, mp_string &latestDataCopyId,
    mp_string &latestLogCopyName)
{
    latestDataCopyId = "123456";
    latestLogCopyName = "11111-22222_1111-2222";
    return MP_SUCCESS;
}

mp_int32 StubGetExtendInfoTestFail(AppProtect::PluginJobData &data, mp_string &latestDataCopyId, 
    mp_string &latestLogCopyName)
{
    return MP_FAILED;
}

mp_bool StubDirNotExist(const mp_char* pszDirPath)
{
    return MP_FALSE;
}

mp_bool StubDirExist(const mp_char* pszDirPath)
{
    return MP_TRUE;
}

mp_int32 StubCreateDirSuccess(const mp_char* pszDirPath)
{
    return MP_SUCCESS;
}

mp_int32 StubCreateDirFail(const mp_char* pszDirPath)
{
    return MP_FAILED;
}

mp_int32 StubWriteFileTestSuccess(mp_string& strFilePath, const std::vector<mp_string>& vecInput)
{
    return MP_SUCCESS;
}

mp_int32 StubWriteFileTestFail(mp_string& strFilePath, const std::vector<mp_string>& vecInput)
{
    return MP_FAILED;
}

mp_int32 StubReadFileTestSuccess(const mp_string& strFilePath, std::vector<mp_string>& vecInput)
{
    return MP_SUCCESS;
}

mp_int32 StubReadFileTestFail(const mp_string& strFilePath, std::vector<mp_string>& vecInput)
{
    return MP_FAILED;
}

mp_bool StubFileNotExist(const mp_string& pszFilePath)
{
    return MP_FALSE;
}

mp_int32 StubCreateFileTestSuccess(const mp_string& pszFilePath)
{
    return MP_SUCCESS;
}

mp_int32 StubCreateFileTestFail(const mp_string& pszFilePath)
{
    return MP_FAILED;
}

mp_bool StubIsLogBackupJobTest(const AppProtect::PluginJobData &data)
{
    return MP_TRUE;
}

mp_bool StubIsNotLogBackupJobTest(void* obj, const AppProtect::PluginJobData &data)
{
    return MP_FALSE;
}

mp_int32 StubQueryJobTestSuccess(void *obj, const mp_string& mainId, const mp_string& subId, 
    AppProtect::PluginJobData& jobData)
{
    mp_string strValue = R"({"taskId" : "5ec7b13d-9aac-4d03-adb6-f1e0f387081e","appInfo" :{"type" : "HDFS",
    "subType" : "HDFSFileset",},"taskParams" :{"backupType" : "logBackup","copyFormat" : 0, "dataLayout":
    {"compression" : true,"deduption" : true,"encryption" : false,"extendInfo" : null},"filters" : [],"qos" : null,
    "scripts":{"failPostScript" : null,"postScript" : null,"preScript" : null}},"taskType" : 1,"repositories" :[
    {"path" : ["/mnt/databackup/test"],"repositoryType" : 3}]})";
    Json::Value jsValue;
    CJsonUtils::ConvertStringtoJson(strValue, jsValue);
    jobData.param = jsValue;
    return MP_SUCCESS;
}

mp_int32 StubQueryJobTestFail(const mp_string& mainId, const mp_string& subId, AppProtect::PluginJobData& jobData)
{
    return MP_FAILED;
}

mp_int32 StubDirRenameTestSuccess(const mp_string &oldDirPath, const mp_string &newDirPath, const mp_string &mainID)
{
    return MP_SUCCESS;
}

mp_int32 StubDirRenameTestFail(const mp_string &oldDirPath, const mp_string &newDirPath, const mp_string &mainID)
{
    return MP_FAILED;
}

mp_int32 StubCreateLogDirTestSuccess(const std::vector<mp_string> &vecOutput, const mp_string &latestDataCopyId,
    const mp_string &latestLogCopyName, const AppProtect::PluginJobData &data)
{
    return MP_SUCCESS;
}

mp_int32 StubCreateLogDirTestFail(const std::vector<mp_string> &vecOutput, const mp_string &latestDataCopyId,
    const mp_string &latestLogCopyName, const AppProtect::PluginJobData &data)
{
    return MP_FAILED;
}

mp_int32 StubWriteDMELastBackupMetaTestSuccess(mp_string &lastLogBackupPath, const mp_string &latestDataCopyId,
    const mp_string &latestLogCopyName)
{
    return MP_SUCCESS;
}

mp_int32 StubWriteDMELastBackupMetaTestFail(mp_string &lastLogBackupPath, const mp_string &latestDataCopyId,
    const mp_string &latestLogCopyName)
{
    return MP_FAILED;
}

mp_int32 StubGetMountPathTestSuccess(void *obj, const AppProtect::PluginJobData &data, mp_string &mountPath)
{
    mountPath = "/mnt/databackup/test";
    return MP_SUCCESS;
}

mp_int32 StubGetMountPathTestFail(const AppProtect::PluginJobData &data, mp_string &mountPath)
{
    return MP_FAILED;
}

mp_int32 StubCreateLogDir_ExTestSuccess(const mp_string &tmpLogBackupDir, mp_string &agentLastLogBackupMetaPath)
{
    return MP_SUCCESS;
}

mp_int32 StubCreateLogDir_ExTestFail(const mp_string &tmpLogBackupDir, mp_string &agentLastLogBackupMetaPath)
{
    return MP_FAILED;
}

#endif