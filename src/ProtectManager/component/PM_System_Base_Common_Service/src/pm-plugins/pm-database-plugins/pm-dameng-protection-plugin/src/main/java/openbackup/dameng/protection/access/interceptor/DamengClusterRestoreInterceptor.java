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
package openbackup.dameng.protection.access.interceptor;

import com.google.common.collect.Maps;

import lombok.extern.slf4j.Slf4j;
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

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * dameng集群恢复任务下发provider
 *
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
        Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElse(Maps.newHashMap());
        // 恢复时，副本是否需要可写，除 DWS 之外，所有数据库应用都设置为 True
        advanceParams.put(DatabaseConstants.IS_COPY_RESTORE_NEED_WRITABLE, Boolean.TRUE.toString());
        task.setAdvanceParams(advanceParams);
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
