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
#include "apps/dws/XBSAServer/BsaMountManager.h"
#include "common/Log.h"

BsaMountManager BsaMountManager::m_instance;

mp_void BsaMountManager::AddIntoMountedList(const mp_string &taskId, const mp_string &deviceSN, const DwsFsInfo &fsInfo,
                                            std::vector<FsMountInfo> &output)
{
    FsMountInfo newFs;
    newFs.deviceSN = deviceSN;
    newFs.fsId = fsInfo.id;
    newFs.fsName = fsInfo.name;
    newFs.mountPathList.insert(newFs.mountPathList.end(), fsInfo.mountPath.begin(), fsInfo.mountPath.end());
    output.push_back(newFs);

    INFOLOG("Add deviceSN(%s),fsId(%s),fsName(%s) into task(%s).", deviceSN.c_str(), fsInfo.id.c_str(),
            fsInfo.name.c_str(), taskId.c_str());
    for (const auto &iter : newFs.mountPathList) {
        INFOLOG("Add mountPath(%s).", iter.c_str()); // 重要日志，需要在循环中INFO打印
    }
}

mp_void BsaMountManager::SetRepository(const mp_string &taskId, const std::vector<DwsRepository> &repos)
{
    std::lock_guard<std::mutex> lock(m_mutexMountFsList);
    auto target_mount = m_mountedFsList.find(taskId);
    if (target_mount != m_mountedFsList.end()) {
        INFOLOG("Task %s already set repository.", taskId.c_str());
        return;
    }

    std::vector<FsMountInfo> tempVec;
    for (const auto &repo : repos) {
        for (const auto &fs : repo.filesystems) {
            AddIntoMountedList(taskId, repo.deviceSN, fs, tempVec);
        }
    }
    m_mountedFsList[taskId] = std::move(tempVec);
}

mp_void BsaMountManager::AllocFilesystem(const mp_string &taskId, mp_string &deviceSN, mp_string &fsId,
                                         mp_string &fsName)
{
    std::lock_guard<std::mutex> lock(m_mutexMountFsList);
    auto iter = m_allocFsIndex.find(taskId);
    if (iter == m_allocFsIndex.end()) {
        m_allocFsIndex[taskId] = 0;
    }

    auto fsIter = m_mountedFsList.find(taskId);
    if (fsIter == m_mountedFsList.end()) {
        ERRLOG("Task id %s did not set repositories.", taskId.c_str());
        return;
    }

    if (++m_allocFsIndex[taskId] >= m_mountedFsList[taskId].size()) {
        m_allocFsIndex[taskId] = 0;
    }

    deviceSN = m_mountedFsList[taskId][m_allocFsIndex[taskId]].deviceSN;
    fsId = m_mountedFsList[taskId][m_allocFsIndex[taskId]].fsId;
    fsName = m_mountedFsList[taskId][m_allocFsIndex[taskId]].fsName;
}

mp_bool BsaMountManager::IsFsMounted(const mp_string &taskId, const mp_string &deviceSN, const mp_string &fsId,
                                     const mp_string &fsName)
{
    std::lock_guard<std::mutex> lock(m_mutexMountFsList);
    auto fsIter = m_mountedFsList.find(taskId);
    if (fsIter == m_mountedFsList.end()) {
        ERRLOG("Task id %s did not set repositories.", taskId.c_str());
        return MP_FALSE;
    }
    for (const auto &fs : fsIter->second) {
        if (fs.deviceSN == deviceSN && fs.fsName == fsName && fs.fsId == fsId) {
            return MP_TRUE;
        }
    }
    return MP_FALSE;
}

mp_string BsaMountManager::GetMountPath(const mp_string &taskId, const mp_string &deviceSN, const mp_string &fsName)
{
    std::lock_guard<std::mutex> lock(m_mutexMountFsList);
    auto fsIter = m_mountedFsList.find(taskId);
    if (fsIter == m_mountedFsList.end()) {
        ERRLOG("Task id %s did not set repositories.", taskId.c_str());
        return "";
    }

    for (auto &fs : fsIter->second) {
        if (fs.deviceSN == deviceSN && fs.fsName == fsName) {
            if (++fs.allocIndex >= fs.mountPathList.size()) {
                fs.allocIndex = 0;
            }
            return fs.mountPathList[fs.allocIndex];
        }
    }
    return "";
}

mp_void BsaMountManager::ClearMountInfoByTaskId(const mp_string &taskId)
{
    std::lock_guard<std::mutex> lock(m_mutexMountFsList);
    auto iter = m_mountedFsList.find(taskId);
    if (iter != m_mountedFsList.end()) {
        m_mountedFsList.erase(iter);
        INFOLOG("Clear mount info for taskId(%s).", taskId.c_str());
    }
}