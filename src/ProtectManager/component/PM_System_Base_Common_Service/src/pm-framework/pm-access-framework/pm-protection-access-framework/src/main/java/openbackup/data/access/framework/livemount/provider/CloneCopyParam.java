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
package openbackup.data.access.framework.livemount.provider;

import openbackup.data.access.framework.livemount.common.model.LiveMountFileSystemShareInfo;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.livemount.LiveMountPerformance;

import java.util.List;

/**
 * Clone Copy Param
 *
 * @author l00272247
 * @since 2022-01-06
 */
public class CloneCopyParam {
    private final String backupId;
    private final String cloneBackupId;
    private final String resourceSubType;
    private final List<LiveMountFileSystemShareInfo> fileSystemShareInfo;
    private final LiveMountPerformance performance;

    private List<StorageRepository> repositories;

    /**
     * constructor
     *
     * @param backupId backup id
     * @param cloneBackupId clone backup id
     * @param resourceSubType resource sub type
     * @param fileSystemShareInfo file system share info
     * @param performance performance
     */
    public CloneCopyParam(
            String backupId,
            String cloneBackupId,
            String resourceSubType,
            List<LiveMountFileSystemShareInfo> fileSystemShareInfo,
            LiveMountPerformance performance) {
        this.backupId = backupId;
        this.cloneBackupId = cloneBackupId;
        this.resourceSubType = resourceSubType;
        this.fileSystemShareInfo = fileSystemShareInfo;
        this.performance = performance;
    }

    public List<StorageRepository> getRepositories() {
        return repositories;
    }

    public void setRepositories(List<StorageRepository> repositories) {
        this.repositories = repositories;
    }

    public String getBackupId() {
        return backupId;
    }

    public String getCloneBackupId() {
        return cloneBackupId;
    }

    public String getResourceSubType() {
        return resourceSubType;
    }

    public List<LiveMountFileSystemShareInfo> getFileSystemShareInfo() {
        return fileSystemShareInfo;
    }

    public LiveMountPerformance getPerformance() {
        return performance;
    }
}
