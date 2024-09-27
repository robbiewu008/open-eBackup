#include "taskmanager/externaljob/ClearMountPointsJobTest.h"
#include "taskmanager/externaljob/ClearMountPointsJob.h"
#include "servicecenter/services/device/PrepareFileSystem.h"
#include "common/File.h"
#include "common/Ip.h"
#include "common/ConfigXmlParse.h"
#include "securecom/RootCaller.h"

#include <vector>
#include <set>
#include <list>
#include <iostream>
#define private public

using namespace std;
using namespace AppProtect;

static mp_void StubCLoggerLog(mp_void){
    return;
}

/*
* 用例名称：执行挂载点清除，定时服务，所有情况都返回true
* 前置条件：无
* check点：1、清除成功，返回true
           2、获取任务目录路径失败，返回true
*/
TEST_F(ClearMountPointsJobTest, ExecClearMountPoints_stub)
{
    ClearMountPointsJob om;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        StubClearMountPointsGetValueInt32ReturnSuccess);
    stub.set(ADDR(ClearMountPointsJob, GetJobIDFloderPath), StubGetJobIDFloderPathTestSuccess);
    stub.set(ADDR(ClearMountPointsJob, DeleteRunningJobIDPath), StubDeleteRunningJobIDPathTest);
    stub.set(ADDR(ClearMountPointsJob, GetMountPointsPath), StubGetMountPointsPathTestSuccess);
    stub.set(ADDR(ClearMountPointsJob, GetFolderChangeElapsedTime), StubGetFolderAccessElapsedTimeTest);
    stub.set(ADDR(ClearMountPointsJob, UmountAndDeleteJobFloder), StubUmountAndDeleteJobFloderTest);
    bool ret = om.ExecClearMountPoints();
    ASSERT_EQ(ret, true);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),
        StubClearMountPointsGetValueInt32ReturnFail);
    stub.set(ADDR(ClearMountPointsJob, GetJobIDFloderPath), StubGetJobIDFloderPathTestFail);
    ret = om.ExecClearMountPoints();
    ASSERT_EQ(ret, true);
}

/*
* 用例名称：获取任务ID层级目录
* 前置条件：无
* check点：1、获取成功常，返回MP_SUCCESS
*/
TEST_F(ClearMountPointsJobTest, GetJobIDFloderPath_stub)
{
    ClearMountPointsJob om;
    list<mp_string> vecFolderPath;

    stub.set(ADDR(ClearMountPointsJob, GetFolderNameAndPath), StubGetFolderNameAndPathTestSuccess);
    mp_int32 ret = om.GetJobIDFloderPath(vecFolderPath);
    ASSERT_EQ(ret, MP_SUCCESS);

    stub.set(ADDR(ClearMountPointsJob, GetFolderNameAndPath), StubGetFolderNameAndPathTestFail);
    ret = om.GetJobIDFloderPath(vecFolderPath);
    ASSERT_EQ(ret, MP_FAILED); 
}

/*
* 用例名称：任务ID目录列表中删除正在运行的任务
* 前置条件：无
* check点：1、删除成功，检查删除后的列表个数
*/
TEST_F(ClearMountPointsJobTest, DeleteRunningJobIDPath_stub)
{
    ClearMountPointsJob om;
    list<mp_string> jobIDFolderPathList;
    jobIDFolderPathList.push_back("/mnt/databackup/mainID");
    
    stub.set(ADDR(AppProtectJobHandler, GetRunJobs), StubGetAllJobsTest);
    om.DeleteRunningJobIDPath(jobIDFolderPathList);
    ASSERT_EQ(1, jobIDFolderPathList.size());
}

/*
* 用例名称：获取指定文件夹下子文件夹名称和路径
* 前置条件：无
* check点：1、获取成功常，返回MP_SUCCESS
*/
TEST_F(ClearMountPointsJobTest, GetFolderNameAndPath_stub)
{
    ClearMountPointsJob om;
    mp_string strFolderPath = "/home";
    vector<mp_string> folderNameList;
    list<mp_string> vecFolderPath;

    mp_int32 ret = om.GetFolderNameAndPath(strFolderPath, folderNameList, vecFolderPath);
    ASSERT_EQ(ret, MP_SUCCESS);
}

/*
* 用例名称：获取文件夹距上次修改经过多长时间（单位：min）
* 前置条件：无
* check点：1、获取修改时间成功，返回MP_SUCCESS
           2、文件夹不存在，返回失败
*/
TEST_F(ClearMountPointsJobTest, GetFolderChangeElapsedTime_stub)
{
    ClearMountPointsJob om;
    mp_string strFolderPath = "/home";
    mp_int32 accessElapsedTime;

    mp_int32 ret = om.GetFolderChangeElapsedTime(strFolderPath, accessElapsedTime);
    ASSERT_EQ(ret, MP_SUCCESS);
    
    strFolderPath = "/tmp/test12345678";
    ret = om.GetFolderChangeElapsedTime(strFolderPath, accessElapsedTime);
    ASSERT_EQ(ret, MP_FAILED);
}

/*
* 用例名称：获取任务ID目录层级下所有挂载点的完整路径
* 前置条件：无
* check点：1、获取成功，返回MP_SUCCESS，检查个数
*/
TEST_F(ClearMountPointsJobTest, GetMountPointsPath_stub)
{
    ClearMountPointsJob om;
    mp_string jobIDFolderPath = "/mnt/databackup/NasShare/b3b34528-bf50-4818-9026-18262babea18";
    
    vector<mp_string> vecMountPoints;
    stub.set(ADDR(ClearMountPointsJob, GetFolderNameAndPath), StubGetFolderNameAndPathEmptyTestSuccess);
    stub.set(ADDR(CIP, IsIPV4), StubIsIPV4);
    stub.set(ADDR(CIP, IsIPv6), StubIsIPV6);

    mp_int32 ret = om.GetMountPointsPath(jobIDFolderPath, vecMountPoints);
    ASSERT_EQ(ret, MP_SUCCESS);

    stub.set(ADDR(ClearMountPointsJob, GetFolderNameAndPath), StubGetFolderNameAndPathTestFail);
    ret = om.GetMountPointsPath(jobIDFolderPath, vecMountPoints);
    ASSERT_EQ(ret, MP_FAILED);
}

/*
* 用例名称：卸载文件系统，删除任务ID层级目录
* 前置条件：无
* check点：1、卸载删除成功，返回MP_SUCCESS
           2、卸载失败，返回MP_FAILED
           2、删除失败，返回MP_FAILED
*/
TEST_F(ClearMountPointsJobTest, UmountAndDeleteJobFloder_stub)
{
    ClearMountPointsJob om;
    mp_string jobIDFolderPath = "/mnt/databackup/NasShare/fef92166-f2cf-44e7-9112-4bcc8c7855c2";
    vector<mp_string> vecMountPoints;

    stub.set(ADDR(PrepareFileSystem, UmountNasFileSystem), StubUmountNasFileSystemTestSuccess);
    stub.set(ADDR(CRootCaller, Exec), StubExecTestSuccess);
    mp_int32 ret = om.UmountAndDeleteJobFloder(jobIDFolderPath, vecMountPoints);
    ASSERT_EQ(ret, MP_SUCCESS);

    stub.set(ADDR(PrepareFileSystem, UmountNasFileSystem), StubUmountNasFileSystemTestFail);
    ret = om.UmountAndDeleteJobFloder(jobIDFolderPath, vecMountPoints);
    ASSERT_EQ(ret, MP_FAILED);

    stub.set(ADDR(PrepareFileSystem, UmountNasFileSystem), StubUmountNasFileSystemTestSuccess);
    stub.set(ADDR(CRootCaller, Exec), StubExecTestFail);
    ret = om.UmountAndDeleteJobFloder(jobIDFolderPath, vecMountPoints);
    ASSERT_EQ(MP_SUCCESS, ret);
}