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
package openbackup.gaussdbt.protection.access.provider.backup;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.MountTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTConstant;
import openbackup.gaussdbt.protection.access.provider.service.GaussDBTSingleService;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.BeanTools;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * GaussDBT单机版备份拦截器
 *
 */
@Slf4j
@Component
public class GaussDBTSingleBackupProvider extends AbstractDbBackupInterceptor {
    private final GaussDBTSingleService gaussDBTSingleService;

    @Autowired
    private DeployTypeService deployTypeService;

    public GaussDBTSingleBackupProvider(GaussDBTSingleService gaussDBTSingleService) {
        this.gaussDBTSingleService = gaussDBTSingleService;
    }

    @Override
    public BackupTask initialize(BackupTask backupTask) {
        log.info("Start gaussdbt single interceptor set parameters. uuid: {}", backupTask.getTaskId());
        // 设置保护环境扩展参数
        setProtectEnvExtendInfo(backupTask);

        // 设置副本格式
        backupTask.setCopyFormat(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());

        // 设置速度统计方式为UBC
        TaskUtil.setBackupTaskSpeedStatisticsEnum(backupTask, SpeedStatisticsEnum.UBC);

        // 设置存储仓
        backupTask.setRepositories(buildRepositories(backupTask));

        ProtectedResource resource = gaussDBTSingleService.getResourceById(backupTask.getProtectObject().getUuid());

        // 设置环境nodes
        backupTask.getProtectEnv().setNodes(gaussDBTSingleService.getEnvNodes(resource));

        // 设置agents
        backupTask.setAgents(gaussDBTSingleService.getAgents(resource));

        // 设置高级参数
        setAdvanceParams(backupTask);
        log.info("End gaussdbt single interceptor set parameters. uuid: {}", backupTask.getTaskId());
        return backupTask;
    }

    private void setProtectEnvExtendInfo(BackupTask backupTask) {
        Map<String, String> envExtendInfo = Optional.ofNullable(backupTask.getProtectEnv().getExtendInfo())
                .orElseGet(HashMap::new);
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        backupTask.getProtectEnv().setExtendInfo(envExtendInfo);
    }

    private List<StorageRepository> buildRepositories(BackupTask backupTask) {
        List<StorageRepository> repositories = backupTask.getRepositories();
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            StorageRepository logRepository = BeanTools.copy(repositories.get(IsmNumberConstant.ZERO),
                    StorageRepository::new);
            logRepository.setType(RepositoryTypeEnum.LOG.getType());
            repositories.add(logRepository);
        }
        StorageRepository cacheRepository = BeanTools.copy(repositories.get(IsmNumberConstant.ZERO),
                StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(cacheRepository);
        return repositories;
    }

    private void setAdvanceParams(BackupTask backupTask) {
        if (!DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            log.debug("This gaussdbt single job type is not log backup. uuid: {}.", backupTask.getTaskId());
            return;
        }
        Map<String, String> advanceParams = Optional.ofNullable(backupTask.getAdvanceParams()).orElse(new HashMap<>());
        advanceParams.put(GaussDBTConstant.MOUNT_TYPE_KEY, MountTypeEnum.NON_FULL_PATH_MOUNT.getMountType());
        log.info("gaussDBT single backup isPacific:{}", deployTypeService.isPacific());
        if (deployTypeService.isPacific()) {
            // 恢复时，副本是否需要可写，除 DWS 之外，所有数据库应用都设置为 True
            advanceParams.put(DatabaseConstants.IS_COPY_RESTORE_NEED_WRITABLE, Boolean.TRUE.toString());
        }
        backupTask.setAdvanceParams(advanceParams);
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.GAUSSDBT_SINGLE.equalsSubType(resourceSubType);
    }
}
