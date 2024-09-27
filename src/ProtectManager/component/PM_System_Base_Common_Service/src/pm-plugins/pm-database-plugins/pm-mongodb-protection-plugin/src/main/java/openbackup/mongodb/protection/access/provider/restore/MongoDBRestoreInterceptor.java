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
package openbackup.mongodb.protection.access.provider.restore;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.RestoreFeature;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.database.base.plugin.utils.AgentDtoUtil;
import openbackup.database.base.plugin.utils.ProtectionTaskUtils;
import openbackup.mongodb.protection.access.constants.MongoDBConstants;
import openbackup.mongodb.protection.access.service.MongoDBBaseService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.BeanUtils;
import org.springframework.stereotype.Component;
import org.springframework.util.CollectionUtils;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * mongo集群恢复任务下发provider
 *
 * @author lWX1012372
 * @version [DataBackup 1.5.0]
 * @since 2023-04-24
 */
@Slf4j
@Component
public class MongoDBRestoreInterceptor extends AbstractDbRestoreInterceptorProvider {
    private final CopyRestApi copyRestApi;

    private final MongoDBBaseService mongoDBBaseService;

    private final ProtectedEnvironmentRetrievalsService envRetrievalsService;

    private final AgentUnifiedService agentUnifiedService;

    /**
     * 构造器
     *
     * @param copyRestApi copyRestApi
     * @param envRetrievalsService envRetrievalsService
     * @param agentUnifiedService agentUnifiedService
     * @param mongoDBBaseService mongoDBBaseService
     */
    public MongoDBRestoreInterceptor(CopyRestApi copyRestApi,
        ProtectedEnvironmentRetrievalsService envRetrievalsService, AgentUnifiedService agentUnifiedService,
        MongoDBBaseService mongoDBBaseService) {
        this.copyRestApi = copyRestApi;
        this.envRetrievalsService = envRetrievalsService;
        this.agentUnifiedService = agentUnifiedService;
        this.mongoDBBaseService = mongoDBBaseService;
    }

    @Override
    public void restoreTaskCreationPreCheck(RestoreTask task) {
        log.info("Pre check mongoDB restore task. taskId: {}", task.getTaskId());
        // 校验数据库版本
        checkDbVersion(task);
    }

    /**
     * 恢复参数添加
     *
     * @param task task
     * @return 返回修改后对象
     */
    @Override
    public RestoreTask initialize(RestoreTask task) {
        log.info("Start MongoDB restore interceptor set parameters. taskId: {}", task.getTaskId());
        // 设置恢复的目标对象
        setTargetObject(task);
        // 设置速度统计方式为UBC
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);

        // 设置恢复模式
        ProtectionTaskUtils.setRestoreMode(task, copyRestApi);
        // 设置目标环境扩展参数
        ProtectedResource resource = mongoDBBaseService.getResource(task.getTargetObject().getUuid());
        // 设置agents
        supplyAgent(task);
        // 设置nodes
        supplyNodes(task);
        // 设置目标实例扩展参数
        setTargetObjectExtendInfo(task, resource);
        // 设置高级参数
        setRestoreAdvanceParams(task);
        task.getTargetEnv().getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        if (ResourceSubTypeEnum.MONGODB_CLUSTER.getType().equals(task.getTargetObject().getSubType())) {
            task.getTargetEnv().getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.AP.getType());
        }
        log.info("End MongoDB restore interceptor set parameters. taskId: {}", task.getTaskId());
        return task;
    }

    /**
     * 检查目标环境在线
     *
     * @return RestoreFeature
     */
    public RestoreFeature getRestoreFeature() {
        RestoreFeature restoreFeature = super.getRestoreFeature();
        restoreFeature.setShouldCheckEnvironmentIsOnline(false);
        return restoreFeature;
    }

    /**
     * 填充node信息
     *
     * @param task RestoreTask
     */
    private void supplyNodes(RestoreTask task) {
        List<Endpoint> agents = task.getAgents();
        if (CollectionUtils.isEmpty(agents)) {
            return;
        }
        List<TaskEnvironment> hostList = new ArrayList<>();
        if (ResourceSubTypeEnum.MONGODB_SINGLE.equalsSubType(task.getTargetObject().getSubType())) {
            hostList = agents.stream()
                .map(agent -> agentUnifiedService.getHost(agent.getIp(), agent.getPort()))
                .map(AgentDtoUtil::toTaskEnvironment)
                .collect(Collectors.toList());
            hostList.get(0).getExtendInfo().putAll(task.getTargetEnv().getExtendInfo());
            hostList.forEach(hostNode -> hostNode.getExtendInfo().put(MongoDBConstants.AGENT_UUID, hostNode.getUuid()));
        } else {
            hostList = mongoDBBaseService.buildBackupTaskNodes(
                task.getTargetEnv().getUuid());
        }
        if (task.getTargetEnv() == null) {
            return;
        }
        task.getTargetEnv().setNodes(hostList);
    }

    private void supplyAgent(RestoreTask task) {
        Map<ProtectedResource, List<ProtectedEnvironment>> protectedResourceMap =
            envRetrievalsService.collectConnectableResources(task.getTargetEnv().getUuid());
        List<Endpoint> endpointList = protectedResourceMap.values()
            .stream()
            .flatMap(List::stream)
            .map(this::toEndpoint)
            .collect(Collectors.toList());
        Set<String> uuid = new HashSet<>();
        endpointList = endpointList.stream()
            .filter(endpoint -> uuid.add(endpoint.getId()))
            .collect(Collectors.toList());
        task.setAgents(endpointList);
    }

    private Endpoint toEndpoint(ProtectedEnvironment protectedEnvironment) {
        Endpoint endpoint = new Endpoint();
        endpoint.setId(protectedEnvironment.getUuid());
        endpoint.setIp(protectedEnvironment.getEndpoint());
        endpoint.setPort(protectedEnvironment.getPort());
        return endpoint;
    }

    private void checkDbVersion(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        JSONObject resourceJson = JSONObject.fromObject(copy.getResourceProperties());
        Map<String, String> extendInfo = resourceJson.getJSONObject(DatabaseConstants.EXTEND_INFO).toMap(String.class);
        String targetVersion = extendInfo.get(DatabaseConstants.VERSION);
        ProtectedEnvironment environment = mongoDBBaseService.getEnvironmentById(task.getTargetEnv().getUuid());
        String currentVersion = environment.getExtendInfo().get(DatabaseConstants.VERSION);
        if (!Objects.equals(targetVersion, currentVersion)) {
            throw new LegoCheckedException(CommonErrorCode.VERSION_NOT_MATCH_BEFORE_RESTORE,
                "The MongoDB target version " + targetVersion + " is different from the current version "
                    + currentVersion);
        }
    }

    private void setTargetObject(RestoreTask task) {
        if (!RestoreLocationEnum.ORIGINAL.equals(task.getTargetLocation())) {
            return;
        }
        ProtectedResource protectedResource = mongoDBBaseService.getResource(task.getTargetObject().getUuid());
        TaskResource taskResource = new TaskResource();
        BeanUtils.copyProperties(protectedResource, taskResource);
        task.setTargetObject(taskResource);
    }

    private void setRestoreAdvanceParams(RestoreTask task) {
        Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElseGet(HashMap::new);
        advanceParams.put(DatabaseConstants.TARGET_LOCATION_KEY, task.getTargetLocation().getLocation());
        JSONObject copyResource = getCopyResource(task.getCopyId());
        advanceParams.put(DatabaseConstants.COPY_PROTECT_OBJECT_VERSION_KEY,
            copyResource.getString(DatabaseConstants.VERSION));
        advanceParams.put(MongoDBConstants.MULTI_POST_JOB, "true");
        task.setAdvanceParams(advanceParams);
    }

    private JSONObject getCopyResource(String copyId) {
        Copy copy = copyRestApi.queryCopyByID(copyId);
        return JSONObject.fromObject(copy.getResourceProperties());
    }

    private void setTargetObjectExtendInfo(RestoreTask task, ProtectedResource resource) {
        Map<String, String> targetObjectExtendInfo = Optional.ofNullable(task.getTargetObject().getExtendInfo())
            .orElseGet(HashMap::new);
        targetObjectExtendInfo.put(DatabaseConstants.VERSION, resource.getVersion());
        task.getTargetObject().setExtendInfo(targetObjectExtendInfo);
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        return mongoDBBaseService.getRestoreLockResource(task.getTargetObject().getUuid());
    }

    /**
     * 获取与此相关联的资源，用于恢复成功后下次转全量
     *
     * @param task 恢复任务
     * @return 关联资源，包含自身
     */
    @Override
    protected List<String> findAssociatedResourcesToSetNextFull(RestoreTask task) {
        return getLockResources(task).stream().map(LockResourceBo::getId).collect(Collectors.toList());
    }

    @Override
    public boolean applicable(String subType) {
        return Arrays.asList(ResourceSubTypeEnum.MONGODB_SINGLE.getType(),
            ResourceSubTypeEnum.MONGODB_CLUSTER.getType()).contains(subType);
    }
}
