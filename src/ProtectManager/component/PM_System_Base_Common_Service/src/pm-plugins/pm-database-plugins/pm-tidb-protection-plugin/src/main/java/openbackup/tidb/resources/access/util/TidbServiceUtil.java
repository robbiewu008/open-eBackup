/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tidb.resources.access.util;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.tidb.resources.access.constants.TidbConstants;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-08-18
 */
public class TidbServiceUtil {
    /**
     * buildBaseBackUpTask
     *
     * @param backupTask backupTask
     * @param clusterResource clusterResource
     */
    public static void buildBaseBackUpTask(BackupTask backupTask, ProtectedResource clusterResource) {
        backupTask.getProtectObject()
            .getExtendInfo()
            .put(TidbConstants.CLUSTER_INFO_LIST, clusterResource.getExtendInfo().get(TidbConstants.CLUSTER_INFO_LIST));

        backupTask.getProtectObject()
            .getExtendInfo()
            .put(TidbConstants.TIUP_UUID, clusterResource.getExtendInfo().get(TidbConstants.TIUP_UUID));

        backupTask.getProtectObject()
            .getExtendInfo()
            .put(TidbConstants.TIUP_PATH, clusterResource.getExtendInfo().get(TidbConstants.TIUP_PATH));

        backupTask.getProtectObject().setAuth(clusterResource.getAuth());
    }
}
