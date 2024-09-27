/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.tidb.resources.access.backup;

import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.tidb.resources.access.constants.TidbConstants;
import openbackup.tidb.resources.access.provider.TidbAgentProvider;
import openbackup.tidb.resources.access.service.TidbService;
import openbackup.tidb.resources.access.util.TidbServiceUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

/**
 * tidb table 备份
 *
 * @author w00426202
 * @since 2023-07-21
 */
@Slf4j
@Component
public class TidbTableBackupInterceptor extends TidbClusterBackupInterceptor {
    /**
     * 构造函数
     *
     * @param tidbService tidbService
     * @param resourceService resourceService
     * @param tidbAgentProvider tidbAgentProvider
     * @param defaultSelector defaultSelector
     */
    public TidbTableBackupInterceptor(TidbService tidbService, ResourceService resourceService,
        TidbAgentProvider tidbAgentProvider, DefaultProtectAgentSelector defaultSelector) {
        super(tidbService, resourceService, tidbAgentProvider, defaultSelector);
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.TIDB_TABLE.getType().equals(object);
    }

    /**
     * 填充agent信息， 配置资源依赖的机器。
     *
     * @param backupTask 备份对象
     */
    @Override
    protected void supplyAgent(BackupTask backupTask) {
        log.info("begin tidb table backup.");
        super.supplyAgent(backupTask);

        String dbUuid = backupTask.getProtectObject().getParentUuid();
        ProtectedResource dbResource = tidbService.getResourceByCondition(dbUuid);
        backupTask.getProtectObject()
            .getExtendInfo()
            .put(TidbConstants.DATABASE_NAME, dbResource.getExtendInfo().get(TidbConstants.DATABASE_NAME));
        String clusterUuid = dbResource.getParentUuid();
        ProtectedResource clusterResource = tidbService.getResourceByCondition(clusterUuid);
        TidbServiceUtil.buildBaseBackUpTask(backupTask, clusterResource);
    }

    @Override
    public boolean isSupportDataAndLogParallelBackup(ProtectedResource resource) {
        return true;
    }
}
