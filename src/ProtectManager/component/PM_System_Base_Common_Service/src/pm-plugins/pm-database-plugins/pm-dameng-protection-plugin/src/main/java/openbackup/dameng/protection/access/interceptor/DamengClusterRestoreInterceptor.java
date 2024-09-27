/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.dameng.protection.access.interceptor;

import openbackup.dameng.protection.access.service.DamengService;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.restore.RestoreFeature;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;

/**
 * dameng集群恢复任务下发provider
 *
 * @author lWX1100347
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-09
 */
@Slf4j
@Component
public class DamengClusterRestoreInterceptor extends AbstractDbRestoreInterceptorProvider {
    private final DamengService damengService;

    public DamengClusterRestoreInterceptor(DamengService damengService) {
        this.damengService = damengService;
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.DAMENG_CLUSTER.getType().equals(subType);
    }

    @Override
    public void restoreTaskCreationPreCheck(RestoreTask task) {
        log.info("Pre check dameng restore task. taskId: {}", task.getTaskId());
    }

    @Override
    public RestoreTask initialize(RestoreTask task) {
        // 设置部署类型
        task.getTargetEnv().getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.AP.getType());
        // 设置子实例对应的agent到恢复对象中
        setRestoreAgents(task);
        // 设置nodes信息
        List<TaskEnvironment> nodesList = damengService.buildTaskNodes(task.getTargetEnv().getUuid());
        task.getTargetEnv().setNodes(nodesList);
        // 设置速度统计方式为UBC
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);
        // 设置高级参数targetLocation
        damengService.setRestoreAdvanceParams(task);
        // 设置恢复模式
        damengService.setRestoreMode(task);
        return task;
    }

    @Override
    public RestoreFeature getRestoreFeature() {
        RestoreFeature feature = new RestoreFeature();
        feature.setShouldCheckEnvironmentIsOnline(false);
        return feature;
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        return Collections.singletonList(new LockResourceBo(task.getTargetObject().getUuid(), LockType.WRITE));
    }

    private void setRestoreAgents(RestoreTask task) {
        List<Endpoint> endpointList = damengService.getEndpointList(task.getTargetEnv().getUuid());
        task.setAgents(endpointList);
    }
}
