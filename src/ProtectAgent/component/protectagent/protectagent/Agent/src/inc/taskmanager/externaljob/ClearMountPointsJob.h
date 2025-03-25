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