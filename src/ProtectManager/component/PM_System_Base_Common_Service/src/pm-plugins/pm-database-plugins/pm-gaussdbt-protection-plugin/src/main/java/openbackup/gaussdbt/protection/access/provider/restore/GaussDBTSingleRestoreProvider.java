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
package openbackup.gaussdbt.protection.access.provider.restore;

import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.database.base.plugin.utils.ProtectionTaskUtils;
import openbackup.gaussdbt.protection.access.provider.service.GaussDBTSingleService;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * GaussDBT单机版恢复拦截器
 *
 * @author lWX776769
 * @version [DataBackup 1.5.0]
 * @since 2023/7/24
 */
@Component
@Slf4j
public class GaussDBTSingleRestoreProvider extends AbstractDbRestoreInterceptorProvider {
    private final CopyRestApi copyRestApi;

    private final GaussDBTSingleService gaussDBTSingleService;

    public GaussDBTSingleRestoreProvider(CopyRestApi copyRestApi, GaussDBTSingleService gaussDBTSingleService) {
        this.copyRestApi = copyRestApi;
        this.gaussDBTSingleService = gaussDBTSingleService;
    }

    @Override
    public void restoreTaskCreationPreCheck(RestoreTask task) {
        log.info("Pre check gaussdbt restore task. taskId: {}", task.getTaskId());
        gaussDBTSingleService.checkSupportRestore(task);
    }

    @Override
    public RestoreTask initialize(RestoreTask task) {
        log.info("Start gaussdbt single restore interceptor set parameters. uuid: {}", task.getTaskId());
        // 设置恢复的目标对象
        setTargetObject(task);

        // 设置速度统计方式为UBC
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);

        // 设置恢复模式
        ProtectionTaskUtils.setRestoreMode(task, copyRestApi);

        // 设置目标环境扩展参数
        setTargetEnvExtendInfo(task);

        ProtectedResource resource = gaussDBTSingleService.getResourceById(task.getTargetObject().getUuid());

        // 设置nodes
        task.getTargetEnv().setNodes(gaussDBTSingleService.getEnvNodes(resource));

        // 设置agents
        task.setAgents(gaussDBTSingleService.getAgents(resource));

        // 设置恢复目标对象扩展参数
        setTargetObjectExtendInfo(task, resource);

        // 设置高级参数
        setRestoreAdvanceParams(task);
        log.info("End gaussdbt single interceptor set parameters. uuid: {}", task.getTaskId());
        return task;
    }

    private void setTargetObject(RestoreTask task) {
        if (!RestoreLocationEnum.ORIGINAL.equals(task.getTargetLocation())) {
            return;
        }
        ProtectedResource protectedResource = gaussDBTSingleService.getResourceById(task.getTargetObject().getUuid());
        task.setTargetObject(BeanTools.copy(protectedResource, TaskResource::new));
    }

    private void setTargetEnvExtendInfo(RestoreTask task) {
        Map<String, String> envExtendInfo = Optional.ofNullable(task.getTargetEnv().getExtendInfo())
            .orElse(new HashMap<>());
        envExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        task.getTargetEnv().setExtendInfo(envExtendInfo);
    }

    private void setRestoreAdvanceParams(RestoreTask task) {
        Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElseGet(HashMap::new);
        advanceParams.put(DatabaseConstants.TARGET_LOCATION_KEY, task.getTargetLocation().getLocation());
        JSONObject copyResource = getCopyResource(task.getCopyId());
        advanceParams.put(DatabaseConstants.COPY_PROTECT_OBJECT_VERSION_KEY,
            copyResource.getString(DatabaseConstants.VERSION));
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
        return Collections.singletonList(new LockResourceBo(task.getTargetObject().getUuid(), LockType.WRITE));
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.GAUSSDBT_SINGLE.equalsSubType(subType);
    }
}
