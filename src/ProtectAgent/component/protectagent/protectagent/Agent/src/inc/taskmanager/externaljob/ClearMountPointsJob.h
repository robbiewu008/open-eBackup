/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file ClearMountPointsJob.h
 * @brief  Contains function declarations about Log Backup
 * @version 1.1.0
 * @date 2022-01-13
 * @author lixilong lwx1101878
 */
#ifndef CLEAR_MOUNT_POINTS_H
#define CLEAR_MOUNT_POINTS_H

#include <set>
#include <list>
#include <vector>
#include <memory>
#include <regex>
#include "common/Types.h"
#include "servicecenter/timerservice/include/ITimer.h"

namespace AppProtect {
class ClearMountPointsJob {
public:
    ClearMountPointsJob();
    ~ClearMountPointsJob();
    mp_void ClearMountPointsTimer();
#ifdef WIN32
    mp_void ClearMountPoints();
#endif

private:
#ifdef WIN32
    bool ExecClearMountPoints();
    // 查询目录下（仅一级）文件和目录的最后修改时间
    mp_time FindLastModifyTime(mp_string strDir);
#else
    bool ExecClearMountPoints();
    mp_int32 GetJobIDFloderPath(std::list<mp_string> &vecJobIDFolderPath);
    mp_void DeleteRunningJobIDPath(std::list<mp_string> &jobIDFolderPathList);
    mp_int32 GetFolderChangeElapsedTime(const mp_string &strFolder, mp_int32 &accessElapsedTime);
    mp_int32 GetFolderNameAndPath(const mp_string &strFolder, std::vector<mp_string> &folderNameList,
        std::list<mp_string> &vecFolderPath);
    mp_int32 GetFolderNameAndPathUmountPoints(const mp_string &strFolder, std::vector<mp_string> &folderNameList,
        std::list<mp_string> &vecFolderPath);
    mp_int32 GetMountPointsPath(const mp_string &vecJobIDFolderPath, std::vector<mp_string> &vecMountPoints);
    mp_int32 UmountAndDeleteJobFloder(mp_string &jobFloderPath, const std::vector<mp_string> &vecMountPoints);
    mp_void GetTaskIdFrompath(std::list<mp_string> &jobIDFolderPathList);
#endif
};
}
#endif