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
package openbackup.tidb.resources.access.backup;

import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tidb.resources.access.provider.TidbAgentProvider;
import openbackup.tidb.resources.access.service.TidbService;
import openbackup.tidb.resources.access.util.TidbServiceUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * 数据库备份
 *
 */
@Slf4j
@Component
public class TidbDatabaseBackupInterceptor extends TidbClusterBackupInterceptor {
    /**
     * 构造器
     *
     * @param tidbService tdsqlService
     * @param resourceService resourceService
     * @param tidbAgentProvider tidbAgentProvider
     * @param defaultSelector defaultSelector
     */
    public TidbDatabaseBackupInterceptor(TidbService tidbService, ResourceService resourceService,
        TidbAgentProvider tidbAgentProvider, DefaultProtectAgentSelector defaultSelector) {
        super(tidbService, resourceService, tidbAgentProvider, defaultSelector);
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.TIDB_DATABASE.getType().equals(object);
    }

    /**
     * 填充agent信息， 配置资源依赖的机器。
     *
     * @param backupTask 备份对象
     */
    @Override
    protected void supplyAgent(BackupTask backupTask) {
        log.info("begin tidb database backup.");
        super.supplyAgent(backupTask);
        String clusterUuid = backupTask.getProtectObject().getParentUuid();
        ProtectedResource clusterResource = tidbService.getResourceByCondition(clusterUuid);
        TidbServiceUtil.buildBaseBackUpTask(backupTask, clusterResource);
    }

    @Override
    public boolean isSupportDataAndLogParallelBackup(ProtectedResource resource) {
        return true;
    }
}
