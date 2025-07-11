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
package openbackup.oceanbase.interceptor;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.oceanbase.provider.OceanBaseAgentProvider;
import openbackup.oceanbase.service.OceanBaseService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;

/**
 * 租户集备份
 *
 */
@Slf4j
@Component
public class OceanBaseTenantBackupInterceptor extends OceanBaseClusterBackupInterceptor {
    /**
     * 有参构造
     *
     * @param oceanBaseService oceanBaseService
     * @param oceanBaseAgentProvider oceanBaseAgentProvider
     * @param deployTypeService deployTypeService
     */
    public OceanBaseTenantBackupInterceptor(OceanBaseService oceanBaseService,
        OceanBaseAgentProvider oceanBaseAgentProvider, DeployTypeService deployTypeService) {
        super(oceanBaseService, oceanBaseAgentProvider, deployTypeService);
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.OCEAN_BASE_TENANT.getType().equals(resourceSubType);
    }

    @Override
    public BackupTask supplyBackupTask(BackupTask backupTask) {
        // 租户集备份用快照格式
        backupTask.setCopyFormat(CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());

        // 部署类型
        Map<String, String> extendInfo = backupTask.getProtectEnv().getExtendInfo();
        extendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.DISTRIBUTED.getType());
        List<StorageRepository> repositories = backupTask.getRepositories();

        // 添加存储仓库类型：2-CACHE_REPOSITORY
        StorageRepository cacheRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(cacheRepository);
        return backupTask;
    }

    @Override
    public boolean isSupportDataAndLogParallelBackup(ProtectedResource resource) {
        return true;
    }
}
