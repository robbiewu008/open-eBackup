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
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.tidb.resources.access.constants.TidbConstants;
import openbackup.tidb.resources.access.provider.TidbAgentProvider;
import openbackup.tidb.resources.access.service.TidbService;
import openbackup.tidb.resources.access.util.TidbUtil;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Map;

/**
 * 集群备份
 *
 * @author w00426202
 * @since 2023-07-21
 */
@Slf4j
@Component
public class TidbClusterBackupInterceptor extends AbstractDbBackupInterceptor {
    final TidbService tidbService;

    /**
     * 资源类
     */
    final ResourceService resourceService;

    final TidbAgentProvider tidbAgentProvider;

    private final DefaultProtectAgentSelector defaultSelector;

    /**
     * 构造器
     *
     * @param tidbService tdsqlService
     * @param resourceService resourceService
     * @param tidbAgentProvider tidbAgentProvider
     * @param defaultSelector defaultSelector
     */
    public TidbClusterBackupInterceptor(TidbService tidbService, ResourceService resourceService,
        TidbAgentProvider tidbAgentProvider, DefaultProtectAgentSelector defaultSelector) {
        this.tidbService = tidbService;
        this.resourceService = resourceService;
        this.tidbAgentProvider = tidbAgentProvider;
        this.defaultSelector = defaultSelector;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.TIDB_CLUSTER.getType().equals(object);
    }

    /**
     * 填充agent信息， 配置资源依赖的机器。
     *
     * @param backupTask 备份对象
     */
    @Override
    protected void supplyAgent(BackupTask backupTask) {
        ProtectedResource resource = BeanTools.copy(backupTask.getProtectObject(), ProtectedResource::new);
        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(resource)
            .jobType(JobTypeEnum.BACKUP.getValue())
            .build();
        List<Endpoint> endpointList = tidbAgentProvider.getSelectedAgents(agentSelectParam);
        // 目标处理节点
        backupTask.setAgents(endpointList);
    }

    /**
     * 不去检查实例连通性
     *
     * @param backupTask 备份对象
     */
    @Override
    protected void checkConnention(BackupTask backupTask) {
        // 日志备份版本要不小于6.2.0
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            log.info("Tidb logBackup check version.");
            String version = backupTask.getProtectObject().getVersion();
            version = version.replace("v", "");
            if (!checkVersion(version, TidbConstants.LOG_BACK_UP_VERSION)) {
                log.error("Tidb logBackup check version error. Current Version: {}", version);
                throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "Invalid version.");
            }
        }
    }

    private boolean checkVersion(String version, String targetVersion) {
        String[] versions = version.split("\\.");
        String[] targetVersions = targetVersion.split("\\.");
        if (Integer.parseInt(versions[0]) > Integer.parseInt(targetVersions[0])) {
            return true;
        } else if (Integer.parseInt(versions[0]) < Integer.parseInt(targetVersions[0])) {
            return false;
        } else {
            if (Integer.parseInt(versions[1]) > Integer.parseInt(targetVersions[1])) {
                return true;
            } else if (Integer.parseInt(versions[1]) < Integer.parseInt(targetVersions[1])) {
                return false;
            } else {
                return Integer.parseInt(versions[2]) >= Integer.parseInt(targetVersions[2]);
            }
        }
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
        }

        // 添加存储仓库类型：2-CACHE_REPOSITORY
        StorageRepository cacheRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(cacheRepository);
        TidbUtil.setTiupUuid(backupTask.getProtectObject().getExtendInfo(), backupTask.getProtectObject().getRootUuid(),
            resourceService, defaultSelector, tidbService);
        log.info("begin tidb cluster backup. ");
        return backupTask;
    }

    @Override
    public boolean isSupportDataAndLogParallelBackup(ProtectedResource resource) {
        return true;
    }
}
