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
package openbackup.tdsql.resources.access.interceptor;

import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.tdsql.resources.access.constant.TdsqlConstant;
import openbackup.tdsql.resources.access.dto.cluster.OssNode;
import openbackup.tdsql.resources.access.dto.instance.TdsqlInstance;
import openbackup.tdsql.resources.access.provider.TdsqlAgentProvider;
import openbackup.tdsql.resources.access.service.TdsqlService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;
import java.util.Map;

/**
 * 功能描述 备份
 *
 */
@Slf4j
@Component
public class TdsqlBackupInterceptor extends AbstractDbBackupInterceptor {
    private final TdsqlService tdsqlService;

    private final TdsqlTaskCheck tdsqlTaskCheck;

    private final ResourceService resourceService;

    private final TdsqlAgentProvider tdsqlAgentProvider;

    /**
     * 构造器
     *
     * @param tdsqlService tdsqlService
     * @param tdsqlTaskCheck tdsqlTaskCheck
     * @param resourceService resourceService
     * @param tdsqlAgentProvider tdsqlAgentProvider
     */
    public TdsqlBackupInterceptor(TdsqlService tdsqlService, TdsqlTaskCheck tdsqlTaskCheck,
        ResourceService resourceService, TdsqlAgentProvider tdsqlAgentProvider) {
        this.tdsqlService = tdsqlService;
        this.tdsqlTaskCheck = tdsqlTaskCheck;
        this.resourceService = resourceService;
        this.tdsqlAgentProvider = tdsqlAgentProvider;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.TDSQL_CLUSTERINSTANCE.getType().equals(object);
    }

    /**
     * 填充agent信息
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
        List<Endpoint> endpointList = tdsqlAgentProvider.getSelectedAgents(agentSelectParam);
        backupTask.setAgents(endpointList);
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
        extendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SHARDING.getType());
        List<StorageRepository> repositories = backupTask.getRepositories();

        // 如果备份方式为日志备份, 添加：3-LOG_REPOSITORY 如果是日志备份，则只能在上一次进行全量备份的节点进行备份
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupType)) {
            StorageRepository logRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
            logRepository.setType(RepositoryTypeEnum.LOG.getType());
            repositories.add(logRepository);
        }

        // 添加存储仓库类型：0-META_REPOSITORY
        StorageRepository metaRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
        metaRepository.setType(RepositoryTypeEnum.META.getType());
        repositories.add(metaRepository);

        // 添加存储仓库类型：2-CACHE_REPOSITORY
        StorageRepository cacheRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(cacheRepository);

        // 防止实例主备切换，此时需要再查一遍实例的信息
        log.info("rescan node roles");
        TdsqlInstance instance = tdsqlTaskCheck.checkEnvChange(backupTask.getProtectEnv().getUuid(),
            backupTask.getProtectObject().getUuid());
        backupTask.getProtectObject().getExtendInfo().put(TdsqlConstant.CLUSTER_INSTANCE_INFO, JsonUtil.json(instance));

        // 获取oss信息下发给任务
        ProtectedEnvironment clusterEnv = tdsqlService.getEnvironmentById(instance.getCluster());
        OssNode ossNode = tdsqlService.getOssNode(clusterEnv).get(0);
        String ossUrl = "http://" + ossNode.getIp() + ":" + ossNode.getPort() + "/tdsql";
        backupTask.getProtectObject().getExtendInfo().put(TdsqlConstant.OSS_URL, ossUrl);

        // 更新数据库的信息
        ProtectedResource instanceResource = tdsqlService.getResourceById(backupTask.getProtectObject().getUuid());
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid(instanceResource.getUuid());
        resource.setExtendInfoByKey(TdsqlConstant.CLUSTER_INSTANCE_INFO, JsonUtil.json(instance));
        resourceService.updateSourceDirectly(Collections.singletonList(resource));
        return backupTask;
    }

    @Override
    public boolean isSupportDataAndLogParallelBackup(ProtectedResource resource) {
        return true;
    }
}
