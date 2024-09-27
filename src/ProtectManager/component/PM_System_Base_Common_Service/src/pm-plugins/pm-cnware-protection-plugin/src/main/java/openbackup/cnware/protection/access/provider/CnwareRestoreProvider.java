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
package openbackup.cnware.protection.access.provider;

import openbackup.cnware.protection.access.constant.CnwareConstant;
import openbackup.data.access.framework.core.common.util.RestoreUtil;
import openbackup.data.protection.access.provider.sdk.anti.ransomware.CopyRansomwareService;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 功能描述: Cnware恢复拦截器
 *
 * @author q30048244
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2024-02-01 9:42
 */
@Slf4j
@Component
public class CnwareRestoreProvider implements RestoreInterceptorProvider {
    private final CopyRestApi copyRestApi;

    private CopyRansomwareService copyRansomwareService;

    /**
     * 构造器注入
     *
     * @param copyRestApi copyRestApi
     */
    public CnwareRestoreProvider(CopyRestApi copyRestApi) {
        this.copyRestApi = copyRestApi;
    }

    @Autowired
    public void setCopyRansomwareService(CopyRansomwareService copyRansomwareService) {
        this.copyRansomwareService = copyRansomwareService;
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.CNWARE_VM.equalsSubType(subType);
    }

    @Override
    public RestoreTask initialize(RestoreTask task) {
        String taskId = task.getTaskId();
        String targetId = task.getTargetObject().getUuid();
        log.info("Start cnware restore interception.TaskId:{}, targetId:{}", taskId, targetId);

        // 填充环境扩展信息
        fillEnvironmentExtendInfo(task);

        // 填充恢复卷信息
        fillSubObjects(task);

        // 填充恢复目标扩展信息
        fillTargetExtendInfo(task);

        // 填充恢复模式
        fillRestoreMode(task);

        // 填充恢复目标位置
        fillRestoreTargetLocation(task);

        log.info("End cnware restore interception.TaskId:{}, targetId:{}", taskId, targetId);
        return task;
    }

    private void fillRestoreMode(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        switch (CopyGeneratedByEnum.getByGenerateType(copy.getGeneratedBy())) {
            case BY_CLOUD_ARCHIVE:
                task.setRestoreMode(RestoreModeEnum.REMOTE_RESTORE.getMode());
                break;
            case BY_TAPE_ARCHIVE:
                task.setRestoreMode(RestoreModeEnum.DOWNLOAD_RESTORE.getMode());
                break;
            default:
                task.setRestoreMode(RestoreModeEnum.LOCAL_RESTORE.getMode());
        }
        log.info("Build CnWare copy restore mode. TaskId:{}, mode: {}", task.getTaskId(), task.getRestoreMode());
    }

    private void fillTargetExtendInfo(RestoreTask task) {
        String powerState = Optional.ofNullable(task.getAdvanceParams())
            .map(advanceParams -> advanceParams.get(CnwareConstant.POWER_STATE))
            .orElse(CnwareConstant.POWER_ON);
        TaskResource protectObject = task.getTargetObject();
        Map<String, String> proExtendInfo = Optional.ofNullable(protectObject.getExtendInfo()).orElse(new HashMap<>());
        proExtendInfo.put(CnwareConstant.POWER_STATE, powerState);
    }

    private void fillRestoreTargetLocation(RestoreTask task) {
        TaskResource targetObject = task.getTargetObject();
        String targetLocation = Optional.ofNullable(task.getAdvanceParams())
            .map(advanceParams -> advanceParams.get(CnwareConstant.RESTORE_LOCATION))
            .orElse(targetObject.getPath());
        targetObject.setTargetLocation(targetLocation);
        String targetLocationType = task.getTargetLocation().getLocation();
        task.getAdvanceParams().put(CnwareConstant.TARGET_LOCATION, targetLocationType);
    }

    private void fillEnvironmentExtendInfo(RestoreTask task) {
        TaskEnvironment protectEnv = task.getTargetEnv();
        Map<String, String> envExtendInfo = Optional.ofNullable(protectEnv.getExtendInfo()).orElse(new HashMap<>());
        envExtendInfo.put(CnwareConstant.IS_DELETE_ORIGINAL_VM, "false");

        protectEnv.setExtendInfo(envExtendInfo);
    }

    private void fillSubObjects(RestoreTask task) {
        Optional.ofNullable(task.getSubObjects())
            .ifPresent(taskResources -> {
                List<TaskResource> subObjects = taskResources.stream()
                    .map(item -> JSONObject.toBean(item.getName(), TaskResource.class))
                    .collect(Collectors.toList());
                task.setSubObjects(subObjects);
            });
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        if (CnwareConstant.RESTORE_LEVEL_ONE.equals(task.getAdvanceParams().get(CnwareConstant.RESTORE_LEVEL))) {
            return Collections.singletonList(
                new LockResourceBo(task.getAdvanceParams().get(CnwareConstant.RESOURCE_LOCK_ID), LockType.WRITE));
        } else if (CnwareConstant.RESTORE_LEVEL_TWO.equals(task.getAdvanceParams().get(CnwareConstant.RESTORE_LEVEL))
            && CnwareConstant.CLEAN_ORIGIN_VM_ONE.equals(task.getAdvanceParams().get(CnwareConstant.CLEAN_ORIGIN_VM))) {
            return Collections.singletonList(
                new LockResourceBo(task.getAdvanceParams().get(CnwareConstant.RESOURCE_LOCK_ID), LockType.WRITE));
        } else {
            return Collections.emptyList();
        }
    }

    @Override
    public void restoreTaskCreationPreCheck(RestoreTask task) {
        copyRansomwareService.checkCopyOperationValid(task.getCopyId(),
                RestoreUtil.getRestoreOperation(task.getRestoreType(), task.getTargetLocation()));
    }
}
