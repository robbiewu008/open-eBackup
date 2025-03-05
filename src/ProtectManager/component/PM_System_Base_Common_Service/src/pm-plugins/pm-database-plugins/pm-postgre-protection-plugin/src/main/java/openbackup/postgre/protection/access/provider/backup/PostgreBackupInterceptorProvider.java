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
package openbackup.postgre.protection.access.provider.backup;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.backup.v2.BackupTask;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbBackupInterceptor;
import openbackup.database.base.plugin.utils.ProtectionTaskUtils;
import openbackup.postgre.protection.access.common.PostgreConstants;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.StreamUtil;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * postgre备份拦截器
 *
 */
@Component
@Slf4j
public class PostgreBackupInterceptorProvider extends AbstractDbBackupInterceptor {
    private final ResourceService resourceService;

    public PostgreBackupInterceptorProvider(ResourceService resourceService) {
        this.resourceService = resourceService;
    }

    @Override
    public BackupTask supplyBackupTask(BackupTask backupTask) {
        // 设置部署类型
        setDeployType(backupTask);

        // 设置存储仓
        setRepositories(backupTask);

        // 设置子实例
        setSubInstances(backupTask);

        // 设置副本格式
        ProtectionTaskUtils.setCopyFormat(backupTask);

        // 设置速度统计方式为UBC
        TaskUtil.setBackupTaskSpeedStatisticsEnum(backupTask, SpeedStatisticsEnum.UBC);

        // 日志备份加上检查任务类型校验
        checkIsLogBackup(backupTask);
        return backupTask;
    }

    private void setDeployType(BackupTask backupTask) {
        String deployType;
        if (ResourceSubTypeEnum.POSTGRE_INSTANCE.equalsSubType(backupTask.getProtectObject().getSubType())) {
            deployType = DatabaseDeployTypeEnum.SINGLE.getType();
        } else {
            deployType = DatabaseDeployTypeEnum.AP.getType();
        }
        Map<String, String> extendInfo = Optional.ofNullable(backupTask.getProtectEnv().getExtendInfo())
            .orElseGet(HashMap::new);
        extendInfo.put(DatabaseConstants.DEPLOY_TYPE, deployType);
        backupTask.getProtectEnv().setExtendInfo(extendInfo);
    }

    /**
     * 设置postgre备份存储仓
     * 全量备份需要设置data,cache 2种repository
     * 日志备份需要设置log,cache 2种repository
     *
     * @param backupTask BackupTask
     */
    private void setRepositories(BackupTask backupTask) {
        List<StorageRepository> repositories = backupTask.getRepositories();
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            StorageRepository logRepository = BeanTools.copy(repositories.get(IsmNumberConstant.ZERO),
                StorageRepository::new);
            logRepository.setType(RepositoryTypeEnum.LOG.getType());
            repositories.add(logRepository);
            repositories.remove(0);
        }
        StorageRepository cacheRepository = BeanTools.copy(repositories.get(IsmNumberConstant.ZERO),
            StorageRepository::new);
        cacheRepository.setType(RepositoryTypeEnum.CACHE.getType());
        repositories.add(cacheRepository);
    }

    private void setSubInstances(BackupTask backupTask) {
        if (ResourceSubTypeEnum.POSTGRE_INSTANCE.equalsSubType(backupTask.getProtectObject().getSubType())) {
            log.info("This backup is a single instance task. taskId: {}", backupTask.getTaskId());
            return;
        }
        ProtectedResource resource = queryResourceById(backupTask.getProtectObject().getUuid());
        List<TaskResource> subInstances = resource.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .stream()
            .map(childNode -> BeanTools.copy(childNode, TaskResource::new))
            .collect(Collectors.toList());
        backupTask.setProtectSubObjects(subInstances);
    }

    @Override
    protected void supplyNodes(BackupTask backupTask) {
        List<TaskEnvironment> nodeList = queryNodeList(backupTask);
        List<TaskEnvironment> nodes = Optional.ofNullable(backupTask.getProtectEnv().getNodes())
            .orElseGet(ArrayList::new);
        nodes.addAll(nodeList);
        backupTask.getProtectEnv().setNodes(nodes);
    }

    private List<TaskEnvironment> queryNodeList(BackupTask backupTask) {
        ProtectedResource resource = queryResourceById(backupTask.getProtectObject().getUuid());
        if (ResourceSubTypeEnum.POSTGRE_INSTANCE.equalsSubType(backupTask.getProtectObject().getSubType())) {
            return applyTaskEnvironment(resource);
        }
        return resource.getDependencies()
            .get(DatabaseConstants.CHILDREN)
            .stream()
            .map(this::applyTaskEnvironment)
            .flatMap(List::stream)
            .collect(Collectors.toList());
    }

    private ProtectedResource queryResourceById(String resourceId) {
        return resourceService.getResourceById(resourceId)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                "Protected Resource is not exists. uuid: " + resourceId));
    }

    private List<TaskEnvironment> applyTaskEnvironment(ProtectedResource protectedResource) {
        return protectedResource.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .map(childNode -> buildTaskEnvironment(childNode, protectedResource))
            .collect(Collectors.toList());
    }

    private TaskEnvironment buildTaskEnvironment(ProtectedEnvironment environment, ProtectedResource resource) {
        environment.getExtendInfo().putAll(resource.getExtendInfo());
        environment.setAuth(resource.getAuth());
        return BeanTools.copy(environment, TaskEnvironment::new);
    }

    @Override
    protected void supplyAgent(BackupTask backupTask) {
        List<Endpoint> endpointList;
        List<TaskEnvironment> nodeList = queryNodeList(backupTask);
        endpointList = nodeList.stream()
            .map(node -> new Endpoint(node.getUuid(), node.getEndpoint(), node.getPort()))
            .collect(Collectors.toList());
        backupTask.setAgents(endpointList);
    }

    @Override
    protected void checkConnention(BackupTask backupTask) {
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return Arrays.asList(ResourceSubTypeEnum.POSTGRE_INSTANCE.getType(),
            ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.getType()).contains(resourceSubType);
    }

    private void checkIsLogBackup(BackupTask backupTask) {
        if (DatabaseConstants.LOG_BACKUP_TYPE.equals(backupTask.getBackupType())) {
            Map<String, String> advanceParams = backupTask.getAdvanceParams();
            advanceParams.put(PostgreConstants.IS_CHECK_BACKUP_JOB_TYPE, "true");
        }
    }
}
