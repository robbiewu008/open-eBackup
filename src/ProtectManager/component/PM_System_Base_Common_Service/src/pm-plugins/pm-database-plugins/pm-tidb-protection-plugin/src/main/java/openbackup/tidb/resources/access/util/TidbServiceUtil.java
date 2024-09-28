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
package openbackup.tidb.resources.access.util;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.tidb.resources.access.constants.TidbConstants;

/**
 * 功能描述
 *
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
