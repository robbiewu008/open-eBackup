#ifndef __CLEAR_MOUNT_POINTS_TEST_H__
#define __CLEAR_MOUNT_POINTS_TEST_H__

#define private public
#include "stub.h"
#include "gtest/gtest.h"
#include "common/Types.h"
#include "taskmanager/externaljob/Job.h"
#include "taskmanager/externaljob/PluginJobFactory.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"
#include <set>
#include <list>
#include <vector>
#include <memory>
#include <iostream>

class ClearMountPointsJobTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void ClearMountPointsJobTest::SetUp() {}

void ClearMountPointsJobTest::TearDown() {}

void ClearMountPointsJobTest::SetUpTestCase() {}

void ClearMountPointsJobTest::TearDownTestCase() {}

mp_bool StubDirNotExistTest(const mp_char* pszDirPath)
{
    return MP_FALSE;
}

mp_bool StubDirExistTest(const mp_char* pszDirPath)
{
    return MP_TRUE;
}

mp_int32 StubUmountNasFileSystemTestSuccess(const std::vector<mp_string> &successMountPath)
{
    return MP_SUCCESS;
}

mp_int32 StubUmountNasFileSystemTestFail(const std::vector<mp_string> &successMountPath)
{
    return MP_FAILED;
}

mp_int32 StubGetFolderNameAndPathTestSuccess(void *obj, const mp_string &strFolder, std::vector<mp_string> &folderNameList, 
    std::list<mp_string> &vecFolderPath)
{
    vecFolderPath.push_back("/tmp/databackup");
    return MP_SUCCESS;
}

mp_int32 StubGetFolderNameAndPathEmptyTestSuccess(void *obj, const mp_string &strFolder, std::vector<mp_string> &folderNameList, 
    std::list<mp_string> &vecFolderPath)
{
    return MP_SUCCESS;
}

mp_int32 StubGetFolderNameAndPathTestFail(const mp_string &strFolder, std::vector<mp_string> &folderNameList, 
    std::list<mp_string> &vecFolderPath)
{
    return MP_FAILED;
}

std::list<std::shared_ptr<AppProtect::Job>> StubGetAllJobsTest(void *obj)
{
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();

    std::list<std::shared_ptr<AppProtect::Job>> m_runJobs;
    AppProtect::PluginJobData jobData = {"pluginName", "mainID", "subID", Json::Value(), AppProtect::MainJobType::BACKUP_JOB};
    std::shared_ptr<AppProtect::Job> pJob = AppProtect::PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    if (pJob) {
        m_runJobs.push_back(pJob);
    }
    return m_runJobs;
}

mp_bool StubIsIPV4(const mp_string& strIpAddr)
{
    return MP_FALSE;
}

mp_bool StubIsNotIPV4(const mp_string& strIpAddr)
{
    return MP_TRUE;
}

mp_bool StubIsIPV6(const mp_string& strIpAddr)
{
    return MP_FALSE;
}

mp_bool StubIsNotIPV6(const mp_string& strIpAddr)
{
    return MP_TRUE;
}

mp_int32 StubCreateTestSuccess(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
    return MP_SUCCESS;
}

mp_int32 StubCreateTestFail(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
    return MP_FAILED;
}

mp_int32 StubRemoveDirSuccess(const mp_string& floderPath)
{
    return MP_SUCCESS;
}

mp_int32 StubRemoveDirFail(const mp_string& floderPath)
{
    return MP_FAILED;
}

mp_int32 StubClearMountPointsGetValueInt32ReturnSuccess(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

mp_int32 StubClearMountPointsGetValueInt32ReturnFail(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_FAILED;
}

mp_void StubDoSleepTest()
{
    return;   
}

mp_int32 StubGetJobIDFloderPathTestSuccess(std::list<mp_string> &vecJobIDFolderPath)
{
    return MP_SUCCESS;
}

mp_int32 StubGetJobIDFloderPathTestFail(std::list<mp_string> &vecJobIDFolderPath)
{
    return MP_FAILED;
}

mp_void StubDeleteRunningJobIDPathTest(void *obj, std::list<mp_string> &jobIDFolderPathList)
{
    jobIDFolderPathList.push_back("1111");
    jobIDFolderPathList.push_back("2222");
}

mp_int32 StubGetMountPointsPathTestSuccess(void *obj, const mp_string &jobIDFolderPath, 
    std::vector<mp_string> &vecMountPoints)
{
    vecMountPoints.push_back("1111");
    vecMountPoints.push_back("1111");
    return MP_SUCCESS;
}

mp_int32 StubGetFolderAccessElapsedTimeTest(void *obj, const mp_string &strFolder, mp_int32 &accessElapsedTime)
{
    accessElapsedTime = 1441;
    return MP_SUCCESS;
}

mp_int32 StubUmountAndDeleteJobFloderTest(const mp_string &jobFloderPath, const std::vector<mp_string> &vecMountPoints)
{
    return MP_SUCCESS;
}

mp_bool StubDirNotEmpty(const mp_string& dirPath)
{
    return MP_FALSE;
}

mp_int32 StubExecTestSuccess(mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&) = NULL, void* pTaskStep = NULL)
{
    return MP_SUCCESS;
}

mp_int32 StubExecTestFail(mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&) = NULL, void* pTaskStep = NULL)
{
    return MP_FAILED;
}
#endif