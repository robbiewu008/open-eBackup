/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
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
