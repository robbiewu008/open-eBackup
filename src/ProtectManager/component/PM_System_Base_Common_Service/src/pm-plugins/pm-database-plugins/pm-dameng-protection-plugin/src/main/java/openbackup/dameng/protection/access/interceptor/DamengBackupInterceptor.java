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
package openbackup.dameng.protection.access.interceptor;

import openbackup.dameng.protection.access.constant.DamengConstant;
import openbackup.dameng.protection.access.provider.DamengAgentProvider;
import openbackup.dameng.protection.access.service.DamengService;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * dameng备份任务拦截器
 *
 * @author lWX1100347
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-17
 */
@Slf4j
@Component
public class DamengBackupInterceptor extends AbstractDbBackupInterceptor {
    private static final String MULTI_FILE_SYSTEM = "multiFileSystem";

    private static final String FALSE = "false";

    private static final String TRUE = "true";

    private final DamengService damengService;

    private final DamengAgentProvider agentProvider;

    public DamengBackupInterceptor(DamengService damengService, DamengAgentProvider agentProvider) {
        this.damengService = damengService;
        this.agentProvider = agentProvider;
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType().equals(subType)
            || ResourceSubTypeEnum.DAMENG_CLUSTER.getType().equals(subType);
    }

    /**
     * 填充agent信息
     *
     * @param backupTask 备份对象
     */
    @Override
    protected void supplyAgent(BackupTask backupTask) {
        AgentSelectParam agentSelectParam = super.buildAgentSelectParam(backupTask);
        backupTask.setAgents(agentProvider.getSelectedAgents(agentSelectParam));
        log.info("dameng backup object:{}, agents size:{}.", backupTask.getProtectObject().getUuid(),
            backupTask.getAgents().size());
    }

    /**
     * 不去检查实例连通性
     *
     * @param backupTask 备份对象
     */
    @Override
    protected void checkConnention(BackupTask backupTask) {
        return;
    }

    @Override
    protected void supplyNodes(BackupTask backupTask) {
        String subType = backupTask.getProtectObject().getSubType();
        if (ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType().equals(subType)) {
            super.supplyNodes(backupTask);
            return;
        }
        List<TaskEnvironment> nodesList = damengService.buildTaskNodes(backupTask.getProtectObject().getUuid());
        backupTask.getProtectEnv().setNodes(nodesList);
    }

    @Override
    public BackupTask supplyBackupTask(BackupTask backupTask) {
        // 设置速度统计方式为UBC
        TaskUtil.setBackupTaskSpeedStatisticsEnum(backupTask, SpeedStatisticsEnum.UBC);

        updateRepositories(backupTask);
        // 设置多文件系统为false
        Map<String, String> advanceParams = Optional.ofNullable(backupTask.getAdvanceParams()).orElse(new HashMap<>());
        advanceParams.put(MULTI_FILE_SYSTEM, FALSE);
        advanceParams.put(DamengConstant.MULTI_POST_JOB, TRUE);
        backupTask.setAdvanceParams(advanceParams);

        // 设置部署类型
        Map<String, String> envExtendInfo = Optional.ofNullable(backupTask.getProtectEnv().getExtendInfo())
            .orElse(new HashMap<>());
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        String subType = backupTask.getProtectObject().getSubType();
        if (ResourceSubTypeEnum.DAMENG_CLUSTER.getType().equals(subType)) {
            envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.AP.getType());
        }
        // 副本格式：0-快照格式（原生格式） 1-目录格式（非原生）
        backupTask.setCopyFormat(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
        return backupTask;
    }

    private void updateRepositories(BackupTask backupTask) {
        List<StorageRepository> repositories = backupTask.getRepositories();

        // 添加存储仓库类型：2-CACHE_REPOSITORY
        StorageRepository cacheRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(cacheRepository);

        // 如果备份方式为日志备份, 添加：3-LOG_REPOSITORY, 移除：1-DATA_REPOSITORY
        String backupType = backupTask.getBackupType();
        if (DamengConstant.LOG_BACKUP_TYPE.equals(backupType)) {
            StorageRepository logRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
            logRepository.setType(RepositoryTypeEnum.LOG.getType());
            backupTask.addRepository(logRepository);
            repositories.remove(0);
        }
        backupTask.setRepositories(repositories);
    }

    @Override
    public boolean isSupportDataAndLogParallelBackup(ProtectedResource resource) {
        return true;
    }
}
