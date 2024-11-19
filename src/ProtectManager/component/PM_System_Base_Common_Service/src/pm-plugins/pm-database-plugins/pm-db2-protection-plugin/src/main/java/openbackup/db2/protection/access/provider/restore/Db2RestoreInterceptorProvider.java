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
package openbackup.db2.protection.access.provider.restore;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.database.base.plugin.service.InstanceResourceService;
import openbackup.database.base.plugin.utils.ProtectionTaskUtils;
import openbackup.db2.protection.access.constant.Db2Constants;
import openbackup.db2.protection.access.enums.Db2ClusterTypeEnum;
import openbackup.db2.protection.access.enums.Db2ResourceStatusEnum;
import openbackup.db2.protection.access.service.Db2Service;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import org.springframework.beans.BeanUtils;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * db2恢复拦截器
 *
 */
@Component
@Slf4j
public class Db2RestoreInterceptorProvider extends AbstractDbRestoreInterceptorProvider {
    private final CopyRestApi copyRestApi;

    private final InstanceResourceService instanceResourceService;

    private final Db2Service db2Service;

    private final ResourceService resourceService;

    public Db2RestoreInterceptorProvider(CopyRestApi copyRestApi, InstanceResourceService instanceResourceService,
        Db2Service db2Service, ResourceService resourceService) {
        this.copyRestApi = copyRestApi;
        this.instanceResourceService = instanceResourceService;
        this.db2Service = db2Service;
        this.resourceService = resourceService;
    }

    @Override
    public void restoreTaskCreationPreCheck(RestoreTask task) {
        log.info("Pre check db2 restore task. taskId: {}", task.getTaskId());
        db2Service.checkSupportRestore(task);
    }

    @Override
    public RestoreTask initialize(RestoreTask task) {
        log.info("Start db2 restore interceptor set parameters. uuid: {}", task.getTaskId());
        // 设置恢复的目标对象
        setTargetObject(task);

        // 设置速度统计方式为UBC
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);

        // 设置恢复模式
        ProtectionTaskUtils.setRestoreMode(task, copyRestApi);

        // 设置目标环境扩展参数
        setTargetEnvExtendInfo(task);

        ProtectedResource resource = instanceResourceService.getResourceById(task.getTargetObject().getUuid());

        // 设置nodes
        task.getTargetEnv().setNodes(db2Service.getEnvNodesByInstanceResource(resource));

        // 设置agents
        task.setAgents(db2Service.getAgentsByInstanceResource(resource));

        // 设置目标实例扩展参数
        setTargetObjectExtendInfo(task, resource);

        // 设置高级参数
        setRestoreAdvanceParams(task);

        // 设置hadr数据库状态
        db2Service.updateHadrDatabaseStatus(task.getTargetObject(), Db2ResourceStatusEnum.RESTORING.getStatus());
        log.info("End db2 restore interceptor set parameters. uuid: {}", task.getTaskId());
        return task;
    }

    private void setTargetObject(RestoreTask task) {
        if (!RestoreLocationEnum.ORIGINAL.equals(task.getTargetLocation())) {
            return;
        }
        ProtectedResource protectedResource = instanceResourceService.getResourceById(task.getTargetObject().getUuid());
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
        String clusterType = copyResource.getJSONObject(DatabaseConstants.EXTEND_INFO)
            .getString(DatabaseConstants.CLUSTER_TYPE);
        if (Db2ClusterTypeEnum.HADR.getType().equals(clusterType)) {
            advanceParams.put(DatabaseConstants.MULTI_POST_JOB, Boolean.TRUE.toString());
        }
        task.setAdvanceParams(advanceParams);
    }

    private JSONObject getCopyResource(String copyId) {
        Copy copy = copyRestApi.queryCopyByID(copyId);
        return JSONObject.fromObject(copy.getResourceProperties());
    }

    private void setTargetEnvExtendInfo(RestoreTask task) {
        Map<String, String> envExtendInfo = Optional.ofNullable(task.getTargetEnv().getExtendInfo())
            .orElseGet(HashMap::new);
        String clusterType = task.getTargetEnv()
            .getExtendInfo()
            .getOrDefault(DatabaseConstants.CLUSTER_TYPE, Db2ClusterTypeEnum.SINGLE.getType());
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, Db2ClusterTypeEnum.getDeployType(clusterType));
        if (Db2ClusterTypeEnum.HADR.getType().equals(clusterType)) {
            envExtendInfo.put(Db2Constants.NODE_DATABASE_KEY, getNodeDatabase(task));
        }
        task.getTargetEnv().setExtendInfo(envExtendInfo);
    }

    private String getNodeDatabase(RestoreTask task) {
        if (ResourceSubTypeEnum.DB2_DATABASE.equalsSubType(task.getTargetObject().getSubType())) {
            return task.getTargetObject().getExtendInfo().get(Db2Constants.NODE_DATABASE_KEY);
        }
        ProtectedResource database = instanceResourceService.getResourceById(task.getTargetObject().getParentUuid());
        return database.getExtendInfoByKey(Db2Constants.NODE_DATABASE_KEY);
    }

    private void setTargetObjectExtendInfo(RestoreTask task, ProtectedResource resource) {
        Map<String, String> targetObjectExtendInfo = Optional.ofNullable(task.getTargetObject().getExtendInfo())
            .orElseGet(HashMap::new);
        targetObjectExtendInfo.put(DatabaseConstants.VERSION, resource.getVersion());
        task.getTargetObject().setExtendInfo(targetObjectExtendInfo);
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        if (ResourceSubTypeEnum.DB2_DATABASE.getType().equals(copy.getResourceSubType())) {
            return getRestoreLockResource(task.getTargetObject().getUuid());
        }
        return getRestoreLockResource(task.getTargetObject().getParentUuid());
    }

    private List<LockResourceBo> getRestoreLockResource(String databaseUuid) {
        Set<String> lockResourceIds = resourceService.queryRelatedResourceUuids(databaseUuid, new String[0]);
        lockResourceIds.add(databaseUuid);
        return lockResourceIds.stream()
            .map(uuid -> new LockResourceBo(uuid, LockType.WRITE))
            .collect(Collectors.toList());
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
    public void postProcess(RestoreTask task, ProviderJobStatusEnum jobStatus) {
        ProtectedResource targetResource = instanceResourceService.getResourceById(task.getTargetObject().getUuid());
        db2Service.updateHadrDatabaseStatus(BeanTools.copy(targetResource, TaskResource::new),
            Db2ResourceStatusEnum.NORMAL.getStatus());
        super.postProcess(task, jobStatus);
    }

    @Override
    public boolean applicable(String subType) {
        return Arrays.asList(ResourceSubTypeEnum.DB2_DATABASE.getType(), ResourceSubTypeEnum.DB2_TABLESPACE.getType())
            .contains(subType);
    }
}
