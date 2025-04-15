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
#include "LinuxVolumeCancelLivemount.h"

using namespace FilePlugin;

bool LinuxVolumeCancelLivemount::LoadBasicDirectory()
{
    INFOLOG("jobId %s, last clone copyId %s", m_cancelLivemountPara->jobId.c_str(), m_cloneCopyId.c_str());
    m_nasShareMountTarget = PluginUtils::PathJoin(VOLUME_LIVEMOUNT_PATH_ROOT, m_cloneCopyId, "share");
    m_volumesMountTargetRoot = PluginUtils::PathJoin(VOLUME_LIVEMOUNT_PATH_ROOT, m_cloneCopyId, "volumes");
    m_volumesMountRecordRoot = PluginUtils::PathJoin(
        PluginUtils::GetPathName(m_cacheRepo->path[0]), "volumelivemount", m_cloneCopyId, "records");
    INFOLOG("share path %s, volume mount target root %s, volume mount record %s, clone copyId %s",
        m_nasShareMountTarget.c_str(), m_volumesMountTargetRoot.c_str(),
        m_volumesMountRecordRoot.c_str(), m_cancelLivemountPara->jobId.c_str());
    if (!PluginUtils::IsDirExist(m_nasShareMountTarget) ||
        !PluginUtils::IsDirExist(m_volumesMountTargetRoot) ||
        !PluginUtils::IsDirExist(m_volumesMountRecordRoot)) {
        ERRLOG("volume mount record path not exists! %s", m_volumesMountRecordRoot.c_str());
        return false;
    }
    return true;
}