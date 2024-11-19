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

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.RestoreFeature;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.opengauss.resources.access.constants.OpenGaussConstants;
import openbackup.opengauss.resources.access.service.OpenGaussAgentService;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 实现恢复接口
 *
 */
@Component
@Slf4j
public class OpenGaussRestoreInterceptorProvider extends AbstractDbRestoreInterceptorProvider {
    private final OpenGaussAgentService openGaussAgentService;

    private final CopyRestApi copyRestApi;

    private final ResourceService resourceService;

    public OpenGaussRestoreInterceptorProvider(OpenGaussAgentService openGaussAgentService, CopyRestApi copyRestApi,
        ResourceService resourceService) {
        this.openGaussAgentService = openGaussAgentService;
        this.copyRestApi = copyRestApi;
        this.resourceService = resourceService;
    }

    @Override
    public RestoreTask initialize(RestoreTask task) {
        log.info("openGauss restore task taskId :{}", task.getRequestId());
        // 高级参数设置speedStatistics等于true，表示使用UBC统计速度
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);

        // 设置agent信息
        task.setAgents(openGaussAgentService.getAgentEndpoint(task.getTargetEnv().getUuid()));

        // 设置环境的node信息
        task.setTargetEnv(openGaussAgentService.buildEnvironmentNodes(task.getTargetEnv()));

        // 设置恢复副本类型
        buildRestoreMode(task);

        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());

        // 设置恢复目标位置
        setTaskResourceTargetLocation(task, copy);

        // 通过副本中的resourceType下发恢复到agent插件
        task.getTargetObject().setSubType(copy.getResourceSubType());
        return task;
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        // 数据库恢复和实例恢复互斥  实例恢复加锁：锁定目标实例和目标实例相应的数据库， 数据库只锁定目标数据库
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        if (ResourceSubTypeEnum.OPENGAUSS_DATABASE.getType().equals(copy.getResourceSubType())) {
            return getDatabaseLockResource(task, copy);
        } else {
            return getInstanceLockResource(task);
        }
    }

    private List<LockResourceBo> getInstanceLockResource(RestoreTask task) {
        PageListResponse<ProtectedResource> data;
        List<ProtectedResource> records = new ArrayList<>();
        String instanceId = task.getTargetObject().getUuid();
        int page = 0;
        do {
            Map<String, Object> conditions = new HashMap<>();
            conditions.put(DatabaseConstants.PARENT_UUID, instanceId);
            conditions.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.OPENGAUSS_DATABASE.getType());
            data = resourceService.query(page, IsmNumberConstant.HUNDRED, conditions);
            records.addAll(data.getRecords());
            page++;
        } while (data.getRecords().size() >= IsmNumberConstant.HUNDRED);
        List<String> resourceIds = records.stream()
            .map(ProtectedResource::getUuid)
            .distinct()
            .collect(Collectors.toList());
        resourceIds.add(instanceId);
        log.info("instance restore lock resourceIds :{}", JSONArray.fromObject(resourceIds));
        return resourceIds.stream()
            .map(uuid -> new LockResourceBo(uuid, LockType.WRITE))
            .collect(Collectors.toList());
    }

    private List<LockResourceBo> getDatabaseLockResource(RestoreTask task, Copy copy) {
        Map<String, Object> conditions = new HashMap<>();
        String databaseName = Optional.ofNullable(task.getAdvanceParams())
            .orElse(new HashMap<>())
            .getOrDefault(DatabaseConstants.DATABASE_NEW_NAME, copy.getResourceName());
        log.info("The name of the databaseName :{}", databaseName);
        conditions.put(DatabaseConstants.PARENT_UUID, task.getTargetObject().getUuid());
        conditions.put(DatabaseConstants.NAME, databaseName);
        // 实例下的数据库名字唯一，只会存在一条
        PageListResponse<ProtectedResource> data = resourceService.query(0, IsmNumberConstant.ONE, conditions);
        if (data.getRecords().size() > 0) {
            String databaseId = data.getRecords().get(0).getUuid();
            log.info("database restore lock databaseId :{}", databaseId);
            return Collections.singletonList(
                new LockResourceBo(databaseId, LockType.WRITE));
        }
        return Collections.emptyList();
    }

    private void setTaskResourceTargetLocation(RestoreTask task, Copy copy) {
        TaskEnvironment targetEnv = task.getTargetEnv();
        String clusterName = targetEnv.getName();
        TaskResource targetObject = task.getTargetObject();
        String instanceName = targetObject.getName();
        // 实例恢复格式:/集群名字/实例名
        if (ResourceSubTypeEnum.OPENGAUSS_INSTANCE.getType().equals(targetObject.getSubType())) {
            String instanceTargetLocation = OpenGaussConstants.PATH_DELIMITER + clusterName
                + OpenGaussConstants.PATH_DELIMITER + instanceName;
            targetObject.setTargetLocation(instanceTargetLocation);
        } else {
            // 数据库恢复格式:/集群名字/实例名/数据库名字
            // 如果数据库重命名，取重命名的名字，不重命名就取副本里面的资源名字
            String databaseName = Optional.ofNullable(task.getAdvanceParams())
                .orElse(new HashMap<>())
                .getOrDefault(DatabaseConstants.DATABASE_NEW_NAME, copy.getResourceName());
            String databaseTargetLocation = OpenGaussConstants.PATH_DELIMITER + clusterName
                + OpenGaussConstants.PATH_DELIMITER + instanceName + OpenGaussConstants.PATH_DELIMITER + databaseName;
            targetObject.setTargetLocation(databaseTargetLocation);
        }
    }

    /**
     * 设置恢复模式
     *
     * @param task 恢复任务
     */
    public void buildRestoreMode(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        String generatedBy = copy.getGeneratedBy();
        if (Arrays.asList(CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value(), CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value())
            .contains(generatedBy)) {
            task.setRestoreMode(RestoreModeEnum.DOWNLOAD_RESTORE.getMode());
        } else {
            task.setRestoreMode(RestoreModeEnum.LOCAL_RESTORE.getMode());
        }
        log.info("build OpenGauss copy restore mode. copy id: {}, mode: {}", task.getCopyId(), task.getRestoreMode());
    }

    @Override
    public boolean applicable(String resourceSubType) {
        log.info("open gauss restore resource subType: {}", resourceSubType);
        return Arrays.asList(ResourceSubTypeEnum.OPENGAUSS_INSTANCE.getType(),
            ResourceSubTypeEnum.OPENGAUSS_DATABASE.getType()).contains(resourceSubType);
    }

    /**
     * 检查目标环境在线
     *
     * @return RestoreFeature
     */
    @Override
    public RestoreFeature getRestoreFeature() {
        RestoreFeature feature = new RestoreFeature();
        feature.setShouldCheckEnvironmentIsOnline(false);
        return feature;
    }
}
