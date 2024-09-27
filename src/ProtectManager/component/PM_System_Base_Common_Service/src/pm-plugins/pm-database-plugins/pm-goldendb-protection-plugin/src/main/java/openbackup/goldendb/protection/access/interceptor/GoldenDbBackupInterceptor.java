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
package openbackup.goldendb.protection.access.interceptor;

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
import openbackup.goldendb.protection.access.constant.GoldenDbConstant;
import openbackup.goldendb.protection.access.dto.instance.GoldenInstance;
import openbackup.goldendb.protection.access.provider.GoldenDBAgentProvider;
import openbackup.goldendb.protection.access.service.GoldenDbService;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import com.google.common.collect.Maps;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 功能描述
 *
 * @author s30036254
 * @since 2023-02-16
 */
@Slf4j
@Component
public class GoldenDbBackupInterceptor extends AbstractDbBackupInterceptor {
    private final GoldenDbService goldenDbService;

    private final GoldenDbTaskCheck goldenDbTaskCheck;

    private final ResourceService resourceService;

    private final GoldenDBAgentProvider goldenDBAgentProvider;

    /**
     * 构造器
     *
     * @param goldenDbService goldenDbService
     * @param goldenDbTaskCheck goldenDbTaskCheck
     * @param resourceService resourceService
     * @param goldenDBAgentProvider goldenDBAgentProvider
     */
    public GoldenDbBackupInterceptor(GoldenDbService goldenDbService, GoldenDbTaskCheck goldenDbTaskCheck,
        ResourceService resourceService, GoldenDBAgentProvider goldenDBAgentProvider) {
        this.goldenDbService = goldenDbService;
        this.goldenDbTaskCheck = goldenDbTaskCheck;
        this.resourceService = resourceService;
        this.goldenDBAgentProvider = goldenDBAgentProvider;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.GOLDENDB_CLUSETER_INSTANCE.getType().equals(object);
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
        List<Endpoint> endpointList = goldenDBAgentProvider.getSelectedAgents(agentSelectParam);
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
        String backupType = backupTask.getBackupType();
        // 副本格式
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupType)) {
            backupTask.setCopyFormat(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
        } else {
            backupTask.setCopyFormat(CopyFormatEnum.INNER_SNAPSHOT.getCopyFormat());
        }

        // 后置任务所有节点都执行
        Map<String, String> advanceParams = Optional.ofNullable(backupTask.getAdvanceParams())
            .orElse(Maps.newHashMap());
        advanceParams.put(DatabaseConstants.MULTI_POST_JOB, Boolean.TRUE.toString());
        backupTask.setAdvanceParams(advanceParams);

        // 部署类型
        Map<String, String> extendInfo = backupTask.getProtectEnv().getExtendInfo();
        extendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SHARDING.getType());
        List<StorageRepository> repositories = backupTask.getRepositories();

        // 添加存储仓库类型：2-CACHE_REPOSITORY
        StorageRepository cacheRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(cacheRepository);

        // 添加存储仓库类型：3-LOG_REPOSITORY
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupType)) {
            StorageRepository logRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
            logRepository.setType(RepositoryTypeEnum.LOG.getType());
            repositories.add(logRepository);
        }

        // 防止实例主备切换，此时需要再查一遍实例的信息
        log.info("rescan node roles");
        GoldenInstance instance = goldenDbTaskCheck.checkEnvChange(backupTask.getProtectEnv().getUuid(),
            backupTask.getProtectObject().getUuid());
        backupTask.getProtectObject().getExtendInfo().put(GoldenDbConstant.CLUSTER_INFO, JsonUtil.json(instance));

        // 更新数据库的信息
        ProtectedResource instanceResource = goldenDbService.getResourceById(backupTask.getProtectObject().getUuid());
        ProtectedResource resource = new ProtectedResource();
        resource.setUuid(instanceResource.getUuid());
        resource.setExtendInfoByKey(GoldenDbConstant.CLUSTER_INFO, JsonUtil.json(instance));
        resourceService.updateSourceDirectly(Collections.singletonList(resource));
        return backupTask;
    }
}
