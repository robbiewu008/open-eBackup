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
package openbackup.mongodb.protection.access.provider.backup;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.NodeInfo;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.CopyFormatEnum;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.mongodb.protection.access.constants.MongoDBConstants;
import openbackup.mongodb.protection.access.service.MongoDBBaseService;
import openbackup.mongodb.protection.access.util.MongoDBConstructionUtils;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * MongoDB备份拦截器
 *
 * @author lwx1012372
 * @version [DataBackup 1.5.0]
 * @since 2023-04-07
 */
@Component
@Slf4j
public class MongoDBBackupInterceptor extends AbstractDbBackupInterceptor {
    private final MongoDBBaseService mongoDBBaseService;

    public MongoDBBackupInterceptor(MongoDBBaseService mongoDBBaseService) {
        this.mongoDBBaseService = mongoDBBaseService;
    }

    /**
     * 填充agent信息
     *
     * @param backupTask backupTask
     */
    @Override
    protected void supplyAgent(BackupTask backupTask) {
        super.supplyAgent(backupTask);
        List<Endpoint> agents = backupTask.getAgents();
        Set<String> uuid = new HashSet<>();
        agents = agents.stream().filter(endpoint -> uuid.add(endpoint.getId())).collect(Collectors.toList());
        backupTask.setAgents(agents);
    }

    /**
     * 重写设置节点参数
     *
     * @param backupTask backupTask
     */
    @Override
    protected void supplyNodes(BackupTask backupTask) {
        log.info("Start supply nodes MongoDB cluster data. taskId: {}", backupTask.getTaskId());
        if (ResourceSubTypeEnum.MONGODB_SINGLE.equalsSubType(backupTask.getProtectObject().getSubType())) {
            super.supplyNodes(backupTask);
            TaskEnvironment taskEnvironment = backupTask.getProtectEnv().getNodes().get(0);
            taskEnvironment.getExtendInfo().putAll(backupTask.getProtectEnv().getExtendInfo());
            taskEnvironment.getExtendInfo().put(MongoDBConstants.AGENT_UUID, taskEnvironment.getUuid());
            return;
        }

        List<TaskEnvironment> nodesList = mongoDBBaseService.buildBackupTaskNodes(
            backupTask.getProtectObject().getUuid());
        backupTask.getProtectEnv().setNodes(nodesList);

        // env 的auth对象
        backupTask.getProtectEnv().setAuth(nodesList.get(IsmNumberConstant.ZERO).getAuth());
        log.info("End supply nodes MongoDB cluster. taskId: {}", backupTask.getTaskId());
    }

    /**
     * 检查连通性
     *
     * @param backupTask backupTask
     */
    protected void checkConnention(BackupTask backupTask) {
        if (ResourceSubTypeEnum.MONGODB_SINGLE.equalsSubType(backupTask.getProtectObject().getSubType())) {
            super.checkConnention(backupTask);
            return;
        }
        ProtectedEnvironment environment = mongoDBBaseService.getEnvironmentById(backupTask.getProtectEnv().getUuid());
        List<ProtectedResource> protectedResources = environment.getDependencies().get(DatabaseConstants.CHILDREN);
        mongoDBBaseService.checkAgentIsOnline(environment);
        // 获取当前所有查询集群列表
        ProtectedResource protectedResource = MongoDBConstructionUtils.getProtectedResource(environment);
        List<String> urlList = mongoDBBaseService.getAllIpAndPortList(environment);
        List<AppEnvResponse> appEnvResponseList = mongoDBBaseService.getAppEnvResponses(protectedResource, urlList,
            false);

        // 副本集集群在线小于一半节点或者主节点健康状态为离线;
        mongoDBBaseService.checkReplicationCluster(protectedResources, appEnvResponseList);
        // 分片集群缺少 route config 或者 shard服务信息; 离线
        mongoDBBaseService.checkShardCLuster(appEnvResponseList);
        List<NodeInfo> clusterNodesCollect = MongoDBConstructionUtils.getNodeInfos(appEnvResponseList);
        mongoDBBaseService.checkPrimarySizeIsMeet(environment.getExtendInfo().get(DatabaseConstants.NODE_COUNT),
            clusterNodesCollect);
    }

    /**
     * 追加额外参数
     *
     * @param backupTask backupTask
     * @return 返回修改后对象
     */
    @Override
    public BackupTask supplyBackupTask(BackupTask backupTask) {
        log.info("Start supply backup task MongoDB  data. taskId: {}", backupTask.getTaskId());
        updateRepositories(backupTask);
        // 设置部署类型
        Map<String, String> envExtendInfo = Optional.ofNullable(backupTask.getProtectEnv().getExtendInfo())
            .orElse(new HashMap<>());
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        if (ResourceSubTypeEnum.MONGODB_CLUSTER.getType().equals(backupTask.getProtectObject().getSubType())) {
            envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.AP.getType());
        }
        backupTask.getProtectEnv().setExtendInfo(envExtendInfo);

        Map<String, String> advanceParams = Optional.ofNullable(backupTask.getAdvanceParams()).orElse(new HashMap<>());
        advanceParams.put(MongoDBConstants.MULTI_POST_JOB, "true");
        advanceParams.put(MongoDBConstants.MULTI_FILE_SYSTEM, "false");
        advanceParams.put(DatabaseConstants.AGENTS, JSONArray.fromObject(backupTask.getAgents()).toString());
        backupTask.setAdvanceParams(advanceParams);
        // protectObject对象
        TaskUtil.setBackupTaskSpeedStatisticsEnum(backupTask, SpeedStatisticsEnum.UBC);
        // 副本格式：0-快照格式（原生格式） 1-目录格式（非原生） 全量-副本-和日志副本格式都是目录格式？
        backupTask.setCopyFormat(CopyFormatEnum.INNER_DIRECTORY.getCopyFormat());
        log.info("End supply backup task MongoDB data. taskId: {}", backupTask.getTaskId());
        return backupTask;
    }

    private void checkIsInstanceLogBackup(String backupType, String subType) {
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupType) && ResourceSubTypeEnum.MONGODB_SINGLE.getType()
            .equals(subType)) {
            throw new LegoCheckedException(CommonErrorCode.OPERATION_FAILED, "instance not log backup");
        }
    }

    private void updateRepositories(BackupTask backupTask) {
        List<StorageRepository> repositories = backupTask.getRepositories();
        // 添加存储仓库类型：2-CACHE_REPOSITORY
        StorageRepository cacheRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(cacheRepository);
        backupTask.setRepositories(repositories);

        String backupType = backupTask.getBackupType();
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupType)) {
            // 添加存储仓库类型：2-LOG_REPOSITORY
            StorageRepository logRepository = BeanTools.copy(repositories.get(0), StorageRepository::new);
            logRepository.setType(RepositoryTypeEnum.LOG.getType());
            repositories.add(logRepository);
        }
        backupTask.setRepositories(repositories);
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return Arrays.asList(ResourceSubTypeEnum.MONGODB_CLUSTER.getType(),
            ResourceSubTypeEnum.MONGODB_SINGLE.getType()).contains(resourceSubType);
    }

    @Override
    public boolean isSupportDataAndLogParallelBackup(ProtectedResource resource) {
        return true;
    }
}
