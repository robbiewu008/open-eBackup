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

import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.oceanbase.common.constants.OBConstants;
import openbackup.oceanbase.provider.OceanBaseAgentProvider;
import openbackup.oceanbase.service.OceanBaseService;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.BeanUtils;
import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;

/**
 * 功能描述
 *
 */
@Slf4j
@Component
public class OceanBaseClusterBackupInterceptor extends AbstractDbBackupInterceptor {
    private final OceanBaseService oceanBaseService;

    private final OceanBaseAgentProvider oceanBaseAgentProvider;

    private final DeployTypeService deployTypeService;

    /**
     * 有参构造
     *
     * @param oceanBaseService oceanBaseService
     * @param oceanBaseAgentProvider oceanBaseAgentProvider
     * @param deployTypeService deployTypeService
     */
    public OceanBaseClusterBackupInterceptor(OceanBaseService oceanBaseService,
        OceanBaseAgentProvider oceanBaseAgentProvider, DeployTypeService deployTypeService) {
        super();
        this.oceanBaseService = oceanBaseService;
        this.oceanBaseAgentProvider = oceanBaseAgentProvider;
        this.deployTypeService = deployTypeService;
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.OCEAN_BASE_CLUSTER.getType().equals(resourceSubType);
    }

    @Override
    protected void supplyAgent(BackupTask backupTask) {
        if (deployTypeService.isE1000()) {
            oceanBaseService.checkSupportNFSV41Dependent(backupTask.getRepositories());
        } else {
            oceanBaseService.checkSupportNFSV41();
        }
        // 检查连通性提前。
        checkConnection(backupTask);
        ProtectedResource resource = BeanTools.copy(backupTask.getProtectObject(), ProtectedResource::new);
        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .build();

        backupTask.setAgents(oceanBaseAgentProvider.getSelectedAgents(agentSelectParam));
    }

    @Override
    protected void checkConnention(BackupTask backupTask) {
        // 检查连通性提前。
    }

    /**
     * 检查连通性
     *
     * @param backupTask backupTask
     */
    protected void checkConnection(BackupTask backupTask) {
        // 租户集备份没有连通性检查方法，这里直接使用rootuuid，检查集群的连通性。
        Optional<ProtectedResource> resOptional = oceanBaseService.getResourceById(
            backupTask.getProtectObject().getRootUuid());
        if (!resOptional.isPresent()) {
            return;
        }

        ResourceConnectionCheckProvider provider = providerManager.findProvider(ResourceConnectionCheckProvider.class,
            resOptional.get());
        ResourceCheckContext resourceCheckContext = provider.checkConnection(resOptional.get());
        if (resourceCheckContext == null || resourceCheckContext.getActionResults() == null) {
            return;
        }
        for (ActionResult actionResult : resourceCheckContext.getActionResults()) {
            if (!Objects.equals(actionResult.getCode(), 0L)) {
                throw new LegoCheckedException("backup connect failed.");
            }
        }

        // 主要为了更新task中集群的linkStatus
        ProtectedEnvironment env = oceanBaseService.getEnvironmentById(backupTask.getProtectObject().getRootUuid());
        TaskEnvironment taskEnvironment = new TaskEnvironment();
        BeanUtils.copyProperties(env, taskEnvironment);
        backupTask.setProtectEnv(taskEnvironment);
    }

    @Override
    public BackupTask supplyBackupTask(BackupTask backupTask) {
        // 副本格式 日志备份用目录格式，全量、增量备份用快照格式
        backupTask.setCopyFormat(CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());
        String backupType = backupTask.getBackupType();
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupType)) {
            backupTask.setCopyFormat(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
        } else {
            backupTask.setCopyFormat(CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());
        }

        // 部署类型
        Map<String, String> extendInfo = backupTask.getProtectEnv().getExtendInfo();
        extendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.DISTRIBUTED.getType());
        List<StorageRepository> repositories = backupTask.getRepositories();

        // 如果备份方式为日志备份, 添加：3-LOG_REPOSITORY 如果是日志备份，则只能在上一次进行全量备份的节点进行备份
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupType)) {
            StorageRepository logRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
            logRepository.setType(RepositoryTypeEnum.LOG.getType());
            repositories.add(logRepository);
        } else {
            // 数据仓设置持续挂载参数: 1-DATA_REPOSITORY
            for (StorageRepository repo : repositories) {
                if (RepositoryTypeEnum.DATA.getType() != repo.getType()) {
                    continue;
                }
                Map<String, Object> repoExtendInfo = Optional.ofNullable(repo.getExtendInfo()).orElseGet(HashMap::new);
                repoExtendInfo.put(OBConstants.PERSISTENT_MOUNT, true);
                repoExtendInfo.put(OBConstants.MANUAL_MOUNT, true);
                repoExtendInfo.put(OBConstants.NEED_DELETE_DTREE, false);
                repo.setExtendInfo(repoExtendInfo);
            }
        }

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
