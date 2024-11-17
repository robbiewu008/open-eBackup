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
#include "taskmanager/externaljob/ClearMountPointsJob.h"

#ifdef WIN32
#else
#include <dirent.h>
#include <unistd.h>
#endif
#include <chrono>
#include <stack>
#include "common/Ip.h"
#include "common/File.h"
#include "common/Utils.h"
#include "common/CMpTime.h"
#include "securecom/RootCaller.h"
#include "common/ConfigXmlParse.h"
#include "common/CSystemExec.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"
#include "servicecenter/services/device/PrepareFileSystem.h"
#include "servicecenter/timerservice/detail/TimerService.h"
#include "servicecenter/servicefactory/include/ServiceFactory.h"

using namespace std;
using namespace std::chrono;
namespace {
    const mp_int32 CLEAR_LOOP_INTERVAL = 120;
    const mp_int32 MOUNT_POINT_CHANGE_TIMEOUT = 24 * 60;
    const mp_int32 MIN_TO_SECOND = 60;
    const mp_int32 SECOND_TO_MS = 1000;
    const mp_string MOUNT_POINT_PUBLIC_PATH = "/mnt/databackup";
    const mp_string JOBID_FILE_NAME = "rdagent_jobid.txt";
    std::vector<mp_string> vecDriverLetter = { "Z", "Y", "X",
        "W", "V", "U", "T", "S", "R", "Q", "P", "O", "N", "M", "L", "K", "J", "I", "H", "G", "F", "E" };
}
namespace AppProtect {
ClearMountPointsJob::ClearMountPointsJob()
{
}

ClearMountPointsJob::~ClearMountPointsJob()
{
}

mp_void ClearMountPointsJob::ClearMountPointsTimer()
{
    LOGGUARD("");
    mp_int32 jobIntervalTime;
    mp_int32 ret = CConfigXmlParser::GetInstance().GetValueInt32(CFG_CLEAR_MOUNT_POINTS_SECTION,
        CFG_CLEAR_LOOP_INTERVAL, jobIntervalTime);
    if (ret != MP_SUCCESS) {
        WARNLOG("Get interval time failed, will use default value.");
        jobIntervalTime = CLEAR_LOOP_INTERVAL;
    }
    auto service =
        servicecenter::ServiceFactory::GetInstance()->GetService<timerservice::ITimerService>("ITimerService");
    if (service) {
        auto clearMountPointTimer = service->CreateTimer();
        if (clearMountPointTimer == nullptr) {
            ERRLOG("Init clear mount points timer failed.");
            return;
        }
        clearMountPointTimer->AddTimeoutExecutor(
            [this]() -> bool { return ExecClearMountPoints(); }, (jobIntervalTime * MIN_TO_SECOND * SECOND_TO_MS));
        clearMountPointTimer->Start();
    } else {
        ERRLOG("Get ITimerService failed.");
    }
}

#ifdef WIN32
mp_void ClearMountPointsJob::ClearMountPoints()
{
    std::map<mp_string, std::vector<mp_string>> mapMountPoint; // jobid--driverLetter
    for (mp_string strDriverLetter : vecDriverLetter) {
        mp_string strJobIdPath = strDriverLetter + ":" + PATH_SEPARATOR + JOBID_FILE_NAME;
        vector<mp_string> vecOutput;
        CMpFile::ReadFile(strJobIdPath, vecOutput);
        if (!vecOutput.empty()) {
            mapMountPoint[vecOutput.front()].push_back(strDriverLetter + ":");
        }
    }

    std::vector<PluginJobData> vecJobData;
    if (JobStateDB::GetInstance().QueryAllJob(vecJobData) != MP_SUCCESS) {
        ERRLOG("QueryAllJob failed.");
        return;
    }
    for (const PluginJobData& jobData : vecJobData) {
        mapMountPoint.erase(jobData.mainID);
    }

    std::vector<mp_string> vecUmountDriver;
    for (auto iter = mapMountPoint.begin(); iter != mapMountPoint.end(); ++iter) {
        vecUmountDriver.insert(vecUmountDriver.end(), iter->second.begin(), iter->second.end());
    }
    if (!vecUmountDriver.empty()) {
        INFOLOG("(%s) will be unmount.", CMpString::StrJoin(vecUmountDriver, ",").c_str());
        PrepareFileSystem umountHandler;
        umountHandler.UmountNasFileSystem(vecUmountDriver);
    }
}

bool ClearMountPointsJob::ExecClearMountPoints()
{
    std::map<mp_string, mp_string> mapMountPoint; // jobid--driverLetter
    for (mp_string strDriverLetter : vecDriverLetter) {
        mp_string strJobIdPath = strDriverLetter + ":" + PATH_SEPARATOR + JOBID_FILE_NAME;
        vector<mp_string> vecOutput;
        CMpFile::ReadFile(strJobIdPath, vecOutput);
        if (!vecOutput.empty()) {
            mapMountPoint.insert(std::make_pair(vecOutput.front(), strDriverLetter + ":"));
        }
    }

    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    if (appProtectJobHandler == nullptr) {
        return false;
    }
    auto runJobs = appProtectJobHandler->GetRunJobs();
    for (const auto& pJob : runJobs) {
        mapMountPoint.erase(pJob->GetData().mainID);
    }

    mp_int32 mountPointChangeTimeout = MOUNT_POINT_CHANGE_TIMEOUT;
    CConfigXmlParser::GetInstance().GetValueInt32(CFG_CLEAR_MOUNT_POINTS_SECTION,
        CFG_MOUNT_POINT_CHANGE_TIMEOUT, mountPointChangeTimeout);
    std::vector<mp_string> vecUmountDriver;
    mp_time nowTime;
    CMpTime::Now(nowTime);
    for (auto iter = mapMountPoint.begin(); iter != mapMountPoint.end(); ++iter) {
        if ((nowTime - FindLastModifyTime(iter->second)) / MIN_TO_SECOND > mountPointChangeTimeout) {
            vecUmountDriver.push_back(iter->second);
            INFOLOG("%s not used for a long time, will be unmount.", iter->second.c_str());
        }
    }

    PrepareFileSystem umountHandler;
    umountHandler.UmountNasFileSystem(vecUmountDriver);
    return true;
}

mp_time ClearMountPointsJob::FindLastModifyTime(mp_string strDir)
{
    std::vector<mp_time> vecLastModifyTime;

    std::vector<mp_string> vecFileList;
    CMpFile::GetFolderFile(strDir, vecFileList);
    for (mp_string strFile : vecFileList) {
        mp_time lastTime;
        mp_string strPath = strDir + PATH_SEPARATOR + strFile;
        CMpFile::GetlLastModifyTime(strPath.c_str(), lastTime);
        vecLastModifyTime.push_back(lastTime);
    }

    std::vector<mp_string> vecDirList;
    CMpFile::GetFolderDir(strDir, vecDirList);
    for (mp_string strFile : vecDirList) {
        mp_time lastTime;
        mp_string strPath = strDir + PATH_SEPARATOR + strFile;
        CMpFile::GetlLastModifyTime(strPath.c_str(), lastTime);
        vecLastModifyTime.push_back(lastTime);
    }

    mp_time lastTime;
    CMpTime::Now(lastTime);
    if (!vecLastModifyTime.empty()) {
        lastTime = *max_element(vecLastModifyTime.begin(), vecLastModifyTime.end());
    }
    return lastTime;
}
#else
bool ClearMountPointsJob::ExecClearMountPoints()
{
    mp_int32 mountPointChangeTimeout;
    mp_int32 ret = CConfigXmlParser::GetInstance().GetValueInt32(CFG_CLEAR_MOUNT_POINTS_SECTION,
        CFG_MOUNT_POINT_CHANGE_TIMEOUT, mountPointChangeTimeout);
    if (ret != MP_SUCCESS) {
        WARNLOG("Get mount point change timeout failed, will use default value.");
        mountPointChangeTimeout = MOUNT_POINT_CHANGE_TIMEOUT;
    }
    list<mp_string> vecJobIDFolderPath;
    ret = GetJobIDFloderPath(vecJobIDFolderPath);
    if (ret != MP_SUCCESS) {
        WARNLOG("Failed to get job ID path.");
        return true;
    }

    DeleteRunningJobIDPath(vecJobIDFolderPath);
    for (auto iter : vecJobIDFolderPath) {
        vector<mp_string> vecMountPoints;
        mp_int32 jobIDDirChangeElapsedTime = 0;
        ret = GetMountPointsPath(iter, vecMountPoints);
        if (ret != MP_SUCCESS) {
            continue;
        }
        if (GetFolderChangeElapsedTime(iter, jobIDDirChangeElapsedTime) != MP_SUCCESS) {
            continue;
        }
        if (jobIDDirChangeElapsedTime > mountPointChangeTimeout) {
            (mp_void)UmountAndDeleteJobFloder(iter, vecMountPoints);
        }
    }
    return true;
}

mp_int32 ClearMountPointsJob::GetJobIDFloderPath(list<mp_string> &vecJobIDFolderPath)
{
    vector<mp_string> folderNameList;
    list<mp_string> vecFolderPath;
    vector<mp_string> jobIDList;
    mp_int32 ret = GetFolderNameAndPath(MOUNT_POINT_PUBLIC_PATH, folderNameList, vecFolderPath);
    if (ret != MP_SUCCESS) {
        ERRLOG("Get apptype folder failed.");
        return MP_FAILED;
    }
    for (auto iter : vecFolderPath) {
        ret = GetFolderNameAndPath(iter, jobIDList, vecJobIDFolderPath);
        if (ret != MP_SUCCESS) {
            WARNLOG("Get jobID folder failed.");
            continue;
        }
    }
    return MP_SUCCESS;
}

mp_void ClearMountPointsJob::DeleteRunningJobIDPath(list<mp_string> &jobIDFolderPathList)
{
    set<mp_string> runJobIDList;
    auto appProtectJobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    if (appProtectJobHandler == nullptr) {
        return;
    }
    auto runJobs = appProtectJobHandler->GetRunJobs();
    for (const auto& tempJob : runJobs) {
        runJobIDList.insert(tempJob->GetData().mainID);
    }
    for (auto iter = jobIDFolderPathList.begin(); iter != jobIDFolderPathList.end();) {
        if (runJobIDList.find((*iter).substr((*iter).find_last_of(PATH_SEPARATOR) + 1)) != runJobIDList.end()) {
            jobIDFolderPathList.erase(iter++);
        } else {
            ++iter;
        }
    }
}

mp_int32 ClearMountPointsJob::GetFolderChangeElapsedTime(const mp_string &strFolder, mp_int32 &changeElapsedTime)
{
    struct stat buf;
    if (stat(strFolder.c_str(), &buf) != 0) {
        ERRLOG("Get floder(%s) stat failed.", strFolder.c_str());
        return MP_FAILED;
    }
    mp_int32 changeTime = static_cast<mp_int32>(buf.st_ctime);
    chrono::seconds now = chrono::duration_cast<chrono::seconds>(system_clock::now().time_since_epoch());
    mp_int32 nowTime = static_cast<mp_int32>(now.count());
    changeElapsedTime = ((nowTime - changeTime) / MIN_TO_SECOND); // 单位 : min
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: GetFolderNameAndPath
Description  : 获取指定文件夹中所有子文件夹名称,返回完整路径
Others       :-------------------------------------------------------- */
mp_int32 ClearMountPointsJob::GetFolderNameAndPath(const mp_string &strFolder, vector<mp_string> &folderNameList,
    list<mp_string> &vecFolderPath)
{
    folderNameList.clear();
    mp_int32 iRet = CMpFile::GetFolderDir(strFolder, folderNameList);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    for (mp_string strName : folderNameList) {
        vecFolderPath.push_back(strFolder + PATH_SEPARATOR + strName);
    }
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Function Name: GetFolderNameAndPathUmountPoints
Description  : 获取指定文件夹中所有子文件夹名称,返回完整路径
Others       :-------------------------------------------------------- */
mp_int32 ClearMountPointsJob::GetFolderNameAndPathUmountPoints(
    const mp_string &strFolder, vector<mp_string> &folderNameList, list<mp_string> &vecFolderPath)
{
    folderNameList.clear();
#ifdef SOLARIS
    mp_int32 iRet = CMpFile::GetFolderDir(strFolder, folderNameList);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
#else
    mp_int32 iRet = CMpFile::GetFolderDirUmountPoints(strFolder, folderNameList);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
#endif
    for (mp_string strName : folderNameList) {
        vecFolderPath.push_back(strFolder + PATH_SEPARATOR + strName);
    }
    return MP_SUCCESS;
}

mp_int32 ClearMountPointsJob::GetMountPointsPath(const mp_string &jobIDFolderPath,
    vector<mp_string> &vecMountPoints)
{
    vector<mp_string> folderNameList;
    list<mp_string> vecFolderPath;
    stack<mp_string> stkPath;
    stkPath.push(jobIDFolderPath);
    while (!stkPath.empty()) {
        mp_int32 ret = GetFolderNameAndPathUmountPoints(stkPath.top(), folderNameList, vecFolderPath);
        if (ret != MP_SUCCESS) {
            ERRLOG("Get path [%s] sub floder name and path failed.", jobIDFolderPath.c_str());
        }
        stkPath.pop();
        for (auto iter : vecFolderPath) {
            stkPath.push(iter);
            mp_string floderName = iter.substr(iter.find_last_of(PATH_SEPARATOR) + 1);
            if (CIP::IsIPV4(floderName) || CIP::IsIPv6(floderName) || floderName == "Dataturbo") {
                INFOLOG("The mountpoints path will be clean:%s.", iter.c_str());
                vecMountPoints.push_back(iter);
                stkPath.pop();
            }
        }
        vecFolderPath.clear();
    }
    return MP_SUCCESS;
}

mp_int32 ClearMountPointsJob::UmountAndDeleteJobFloder(mp_string &jobFloderPath,
    const std::vector<mp_string> &vecMountPoints)
{
    PrepareFileSystem umountHandler;
    mp_int32 ret = umountHandler.UmountNasFileSystem(vecMountPoints);
    if (ret != MP_SUCCESS) {
        ERRLOG("Umount failed.");
        return MP_FAILED;
    }

    CHECK_FAIL_EX(CheckParamValid(jobFloderPath));
    ostringstream scriptParam;
    scriptParam << "jobIDPath=" << jobFloderPath << "\n";
    CRootCaller rootCaller;
    ret = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_CLEAR_MOUNT_POINT, scriptParam.str(), NULL);
    return MP_SUCCESS;
}
#endif
}