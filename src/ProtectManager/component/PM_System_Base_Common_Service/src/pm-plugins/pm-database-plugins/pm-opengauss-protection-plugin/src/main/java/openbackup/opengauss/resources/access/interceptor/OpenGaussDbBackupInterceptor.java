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
package openbackup.opengauss.resources.access.interceptor;

import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.opengauss.resources.access.provider.OpenGaussAgentProvider;
import openbackup.opengauss.resources.access.service.OpenGaussAgentService;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * openGauss环境注册提供者
 *
 */
@Component
@Slf4j
public class OpenGaussDbBackupInterceptor extends AbstractDbBackupInterceptor {
    private final OpenGaussAgentService openGaussAgentService;

    private final OpenGaussAgentProvider agentProvider;

    public OpenGaussDbBackupInterceptor(OpenGaussAgentService openGaussAgentService,
        OpenGaussAgentProvider agentProvider) {
        this.openGaussAgentService = openGaussAgentService;
        this.agentProvider = agentProvider;
    }

    @Override
    public BackupTask supplyBackupTask(BackupTask backupTask) {
        // 高级参数设置speedStatistics等于true，表示使用UBC统计速度
        TaskUtil.setBackupTaskSpeedStatisticsEnum(backupTask, SpeedStatisticsEnum.UBC);

        // 设置存储仓
        updateRepositories(backupTask);
        // 副本格式：0-快照格式（原生格式） 1-目录格式（非原生） openGauss为非原生
        backupTask.setCopyFormat(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());

        // 更新环境中的node信息
        backupTask.setProtectEnv(openGaussAgentService.buildEnvironmentNodes(backupTask.getProtectEnv()));
        return backupTask;
    }

    /**
     * 更新仓库
     *
     * @param backupTask 通用备份框架备份参数对象
     */
    private void updateRepositories(BackupTask backupTask) {
        List<StorageRepository> repositories = backupTask.getRepositories();
        // 备份类型为日志备份时 添加log仓
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            StorageRepository logRepository = BeanTools.copy(repositories.get(IsmNumberConstant.ZERO),
                StorageRepository::new);
            logRepository.setType(RepositoryTypeEnum.LOG.getType());
            repositories.add(logRepository);
        }
        StorageRepository cacheRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(cacheRepository);
        backupTask.setRepositories(repositories);
        log.info("backupTask repositories size: {}", repositories.size());
    }

    /**
     * 设置注册的agent信息到备份任务
     *
     * @param backupTask backupTask
     */
    @Override
    protected void supplyAgent(BackupTask backupTask) {
        // 获取环境ID
        String envId = backupTask.getProtectEnv().getUuid();
        List<Endpoint> agentEndpoint = openGaussAgentService.getAgentEndpoint(envId);
        backupTask.setAgents(agentEndpoint);
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return Arrays.asList(ResourceSubTypeEnum.OPENGAUSS_DATABASE.getType(),
            ResourceSubTypeEnum.OPENGAUSS_INSTANCE.getType()).contains(resourceSubType);
    }
}
