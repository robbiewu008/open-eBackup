/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.sqlserver.protection.restore;

import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.exception.DataProtectionAccessException;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.database.base.plugin.utils.ProtectionTaskUtils;
import openbackup.sqlserver.common.SqlServerErrorCode;
import openbackup.sqlserver.protection.service.SqlServerBaseService;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.StreamUtil;

import com.google.common.collect.Maps;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * sqlserver集群实例或可用性组恢复任务下发provider。两者放在一起，本质是可用性组新位置恢复目标对象类型是集群实例
 *
 * @author xwx950025
 * @version [OceanProtect X8000 1.2.0]
 * @since 2022/7/11
 */
@Component
@Slf4j
public class SqlServerGroupRestoreProvider extends AbstractDbRestoreInterceptorProvider {
    private SqlServerBaseService sqlServerBaseService;

    private ResourceService resourceService;

    private CopyRestApi copyRestApi;

    @Autowired
    public void setSqlServerBaseService(final SqlServerBaseService sqlServerBaseService) {
        this.sqlServerBaseService = sqlServerBaseService;
    }

    @Autowired
    public void setResourceService(final ResourceService resourceService) {
        this.resourceService = resourceService;
    }

    @Autowired
    public void setCopyRestApi(final CopyRestApi copyRestApi) {
        this.copyRestApi = copyRestApi;
    }

    @Override
    public void restoreTaskCreationPreCheck(RestoreTask task) {
        log.info("Pre check sqlserver group restore task. taskId: {}", task.getTaskId());
        // 恢复任务参数校验
        sqlServerBaseService.checkRestoreTaskParam(task);
    }

    /**
     * 拦截恢复请求，对恢复请求进行拦截
     *
     * @param task 恢复参数对象
     * @return 返回恢复任务
     */
    @Override
    public RestoreTask initialize(RestoreTask task) {
        // 高级参数设置speedStatistics等于true，表示使用UBC统计速度
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);
        // 设置恢复模式
        ProtectionTaskUtils.setRestoreMode(task, copyRestApi);
        TaskResource targetObject = task.getTargetObject();
        List<TaskEnvironment> nodeList = getNodeList(task);
        List<Endpoint> endpointList = nodeList.stream()
            .map(node -> new Endpoint(node.getUuid(), node.getEndpoint(), node.getPort()))
            .collect(Collectors.toList());
        // 设置 可用性组agents
        task.setAgents(endpointList);
        // 设置node
        task.getTargetEnv().setNodes(nodeList);
        sqlServerBaseService.logRestoreAddData(task, copyRestApi);
        log.info("[SQL Server] restore subType: {}, object: {}, agents size: {}, nodes size: {}, requestId: {}",
            targetObject.getSubType(), targetObject.getUuid(), task.getAgents().size(),
            task.getTargetEnv().getNodes().size(), task.getRequestId());
        if (ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON.getType().equals(targetObject.getSubType())
            && RestoreLocationEnum.ORIGINAL.equals(task.getTargetLocation())) {
            task.getAdvanceParams().put("multiPostJob", "true");
        }
        return task;
    }

    private List<TaskEnvironment> getNodeList(RestoreTask task) {
        task.getTargetEnv().getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.AP.getType());
        TaskResource targetObject = task.getTargetObject();
        String uuid = targetObject.getUuid();
        String subType = targetObject.getSubType();
        List<TaskEnvironment> nodeList = new ArrayList<>();
        // 细粒度新位置恢复，均是将数据库恢复到某一个单实例下面
        // 该场景下targetObject是单实例
        if (RestoreTypeEnum.FLR.getType().equals(task.getRestoreType()) && RestoreLocationEnum.NEW.equals(
            task.getTargetLocation())) {
            task.getTargetEnv().getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE,
                    DatabaseDeployTypeEnum.SINGLE.getType());
            ProtectedResource singleInstance = sqlServerBaseService.getResourceByUuid(uuid);
            nodeList.addAll(applyTaskEnvironment(singleInstance));
            return nodeList;
        }
        // alwaysOn 原位置恢复（包括细粒度）
        // 该场景下targetObject是可用性组
        if (ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON.getType().equals(subType) && RestoreLocationEnum.ORIGINAL.equals(
            task.getTargetLocation())) {
            setAlwaysOnOriginalRestoreNodes(uuid, nodeList);
            return nodeList;
        }
        // alwaysOn 新位置恢复或集群实例原位置恢复
        // 该场景下targetObject是集群实例
        setOtherRestoreTypeNodes(task, targetObject, uuid, nodeList);
        return nodeList;
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        ProtectedResource resource = sqlServerBaseService.getResourceByUuid(task.getTargetObject().getUuid());
        // 可用性组副本数据库级别恢复到新位置，targetObj是新单实例-->锁：新位置单实例资源
        // 集群实例数据库级别恢复到新位置，targetObj是新单实例-->锁：新位置单实例资源
        // 集群实例数据库级别恢复到新位置，targetObj是新集群实例-->锁：新位置集群实例资源
        // 可用性组副本数据库级别恢复到新位置，targetObj是新集群实例-->锁：新位置集群实例资源
        Set<String> relatedLockResources = new HashSet<>(Collections.singleton(resource.getUuid()));
        handleNormalRestoreAGroupRelatedResources(relatedLockResources, task, resource);
        if (RestoreLocationEnum.ORIGINAL.equals(task.getTargetLocation())) {
            if (ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType().equals(resource.getSubType())) {
                // 集群实例恢复到原位置，targetObj是原集群实例-->锁：原位置的集群实例资源、实例下所有数据库
                // 集群实例数据库级别恢复到原位置，targetObj是原集群实例-->锁：原位置集群实例资源，实例下所有数据库
                relatedLockResources.addAll(
                    new HashSet<>(resourceService.queryRelatedResourceUuids(resource.getUuid(), new String[] {})));
                log.info("[SQL Server] cluster instance get lock resources: {}, size: {}, requestId: {}.",
                    JSONArray.fromObject(relatedLockResources).toString(), relatedLockResources.size(),
                    task.getRequestId());
            } else {
                // 可用性组副本恢复到原位置，targetObj是原可用性组-->锁：原位置的可用性组资源、关联的实例加锁
                // 可用性组副本数据库级别恢复到原位置，targetObj是原可用性组-->锁：原位置的可用性组资源、关联的实例加锁
                relatedLockResources.addAll(
                    new HashSet<>(sqlServerBaseService.getInstanceListDependentOnAGroup(resource)));
                log.info("[SQL Server] origin always on get lock resources: {}, size: {}, requestId: {}.",
                    JSONArray.fromObject(relatedLockResources).toString(), relatedLockResources.size(),
                    task.getRequestId());
            }
        }
        log.info("[SQL Server] instance get lock resources: {}, size: {}, requestId: {}",
            JSONArray.fromObject(relatedLockResources).toString(), relatedLockResources.size(), task.getRequestId());
        return sqlServerBaseService.buildLockResourceList(relatedLockResources);
    }

    private void handleNormalRestoreAGroupRelatedResources(Set<String> relatedLockResources, RestoreTask task,
        ProtectedResource resource) {
        if (ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType().equals(resource.getSubType())
            && RestoreLocationEnum.NEW.equals(task.getTargetLocation()) && RestoreTypeEnum.CR.getType()
            .equals(task.getRestoreType())) {
            // 可用性组副本恢复到新位置，targetObj是新集群实例-->锁：集群下所有的实例
            relatedLockResources.addAll(sqlServerBaseService.getResourceOfClusterByType(resource.getParentUuid(),
                ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType(), true)
                .stream()
                .map(ProtectedResource::getUuid)
                .collect(Collectors.toSet()));
            log.info("[SQL Server] new normal cluster instance get lock resources: {}, size: {}, requestId: {}.",
                JSONArray.fromObject(relatedLockResources).toString(), relatedLockResources.size(),
                task.getRequestId());
        }
    }

    private void setOtherRestoreTypeNodes(RestoreTask task, TaskResource targetObject, String uuid,
        List<TaskEnvironment> nodeList) {
        RestoreLocationEnum targetLocation = task.getTargetLocation();
        if (RestoreLocationEnum.NEW.equals(targetLocation)) {
            // 可用性组新位置恢复
            // 注意：可用性组新位置恢复，目标对象是集群实例，所以会在该逻辑内部进行判断
            Map<String, Object> condition = new HashMap<>();
            condition.put(DatabaseConstants.PARENT_UUID, task.getTargetEnv().getUuid());
            condition.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType());
            PageListResponse<ProtectedResource> pageListResponse = resourceService.query(0,
                DatabaseConstants.PAGE_SIZE, condition);
            if (pageListResponse.getTotalCount() < 2) {
                throw new DataProtectionAccessException(SqlServerErrorCode.SQLSERVER_RESTORE_INSTANCE_INSUFFICIENT,
                    new String[] {task.getTargetEnv().getName()});
            }
            if (pageListResponse.getTotalCount() > pageListResponse.getRecords().size()) {
                pageListResponse = resourceService.query(0, pageListResponse.getTotalCount(), condition);
            }
            List<TaskEnvironment> nodes = pageListResponse.getRecords()
                .stream()
                .map(clusterInstance -> applyTaskEnvironment(
                    sqlServerBaseService.getResourceByUuid(clusterInstance.getUuid())))
                .flatMap(List::stream)
                .collect(Collectors.toList());
            nodeList.addAll(nodes);
            targetObject.setSubType(ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON.getType());
            Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElse(Maps.newHashMap());
            advanceParams.put(DatabaseConstants.MULTI_POST_JOB, Boolean.TRUE.toString());
            task.setAdvanceParams(advanceParams);
        } else {
            task.getTargetEnv().getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE,
                    DatabaseDeployTypeEnum.SINGLE.getType());
            ProtectedResource clusterInstance = sqlServerBaseService.getResourceByUuid(uuid);
            nodeList.addAll(applyTaskEnvironment(clusterInstance));
        }
    }

    private void setAlwaysOnOriginalRestoreNodes(String uuid, List<TaskEnvironment> nodeList) {
        ProtectedResource alwaysOn = sqlServerBaseService.getResourceByUuid(uuid);
        List<ProtectedResource> clusterInstances = alwaysOn.getDependencies().get(DatabaseConstants.INSTANCE);
        clusterInstances.forEach(instance -> {
            // 重新查询是为了获取认证信息
            ProtectedResource clusterInstance = sqlServerBaseService.getResourceByUuid(instance.getUuid());
            nodeList.addAll(applyTaskEnvironment(clusterInstance));
        });
    }

    private List<TaskEnvironment> applyTaskEnvironment(ProtectedResource protectedResource) {
        return protectedResource.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .flatMap(StreamUtil.match(ProtectedEnvironment.class))
            .map(childNode -> {
                childNode.setAuth(protectedResource.getAuth());
                childNode.getExtendInfo().put(DatabaseConstants.INSTANCE, protectedResource.getName());
                return BeanTools.copy(childNode, TaskEnvironment::new);
            })
            .collect(Collectors.toList());
    }

    /**
     * 判断目标资源是否可以应用此拦截器
     *
     * @param subType 目标资源subType
     * @return 是否可以应用此拦截器
     */
    @Override
    public boolean applicable(String subType) {
        return Arrays.asList(ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType(),
            ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON.getType()).contains(subType);
    }

    /**
     * 获取与此相关联的资源，用于恢复成功后下次转全量
     *
     * @param task RestoreTask
     * @return 关联资源，若包含自身，也需要返回
     */
    @Override
    protected List<String> findAssociatedResourcesToSetNextFull(RestoreTask task) {
        return sqlServerBaseService.findAssociatedResourcesToSetNextFull(task);
    }
}