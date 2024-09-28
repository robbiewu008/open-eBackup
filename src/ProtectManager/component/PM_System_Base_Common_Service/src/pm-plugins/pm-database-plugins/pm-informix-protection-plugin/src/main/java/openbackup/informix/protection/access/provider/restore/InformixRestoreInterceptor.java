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
package openbackup.informix.protection.access.provider.restore;

import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.base.v2.StorageRepository;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RepositoryTypeEnum;
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
import openbackup.informix.protection.access.constant.InformixConstant;
import openbackup.informix.protection.access.service.InformixService;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.BeanUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * informix恢复拦截器
 *
 */
@Component
@Slf4j
public class InformixRestoreInterceptor extends AbstractDbRestoreInterceptorProvider {
    private final CopyRestApi copyRestApi;

    private final InformixService informixService;

    /**
     * InformixRestoreInterceptor
     *
     * @param copyRestApi copyRestApi
     * @param informixService informixService
     */
    public InformixRestoreInterceptor(CopyRestApi copyRestApi, InformixService informixService) {
        this.copyRestApi = copyRestApi;
        this.informixService = informixService;
    }

    /**
     * provider过滤器，过滤条件接口
     *
     * @param resourceSubType 资源类型
     * @return boolean true 获取该bean false过滤掉该bean
     */
    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.INFORMIX_SINGLE_INSTANCE.getType().equals(resourceSubType)
                || ResourceSubTypeEnum.INFORMIX_CLUSTER_INSTANCE.getType().equals(resourceSubType);
    }

    /**
     * intercept
     *
     * @param restoreTask 恢复任务参数对象{@link RestoreTask}
     * @return RestoreTask
     */
    @Override
    public RestoreTask initialize(RestoreTask restoreTask) {
        log.info("Start to set parameters for Informix restore interceptor. uuid: {}", restoreTask.getTaskId());
        // 设置恢复的目标对象
        setTargetObject(restoreTask);
        // 设置速度统计方式为UBC
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(restoreTask, SpeedStatisticsEnum.UBC);
        // 设置恢复模式
        ProtectionTaskUtils.setRestoreMode(restoreTask, copyRestApi);
        // 获取资源信息
        ProtectedResource resource = informixService.getResourceById(restoreTask.getTargetObject().getUuid());
        // 设置nodes
        restoreTask.getTargetEnv().setNodes(informixService.getEnvNodesByInstanceResource(resource));
        setTargetEnvExtendInfo(restoreTask);
        // 设置agents
        restoreTask.setAgents(informixService.getAgentsByInstanceResource(resource));
        // 设置目标实例扩展参数
        setTargetObjectExtendInfo(restoreTask, resource);
        // 设置恢复存储仓
        setRestoreRepositories(restoreTask);
        // 设置高级参数
        setRestoreAdvanceParams(restoreTask);
        log.info("End to set parameters for Informix restore interceptor. uuid: {}", restoreTask.getTaskId());
        return restoreTask;
    }

    private void setTargetObject(RestoreTask task) {
        if (!RestoreLocationEnum.ORIGINAL.equals(task.getTargetLocation())) {
            return;
        }
        ProtectedResource protectedResource = informixService.getResourceById(task.getTargetObject().getUuid());
        TaskResource taskResource = new TaskResource();
        BeanUtils.copyProperties(protectedResource, taskResource);
        task.setTargetObject(taskResource);
    }

    private void setTargetEnvExtendInfo(RestoreTask task) {
        Map<String, String> targetEnvExtendInfo = Optional.ofNullable(task.getTargetEnv().getExtendInfo())
                .orElseGet(HashMap::new);
        String subType = task.getTargetObject().getSubType();
        String deployType = ResourceSubTypeEnum.INFORMIX_SINGLE_INSTANCE.equalsSubType(subType)
                ? DatabaseDeployTypeEnum.SINGLE.getType()
                : DatabaseDeployTypeEnum.AP.getType();
        targetEnvExtendInfo.put(DatabaseConstants.DEPLOY_TYPE, deployType);
        task.getTargetEnv().setExtendInfo(targetEnvExtendInfo);
    }

    private void setTargetObjectExtendInfo(RestoreTask task, ProtectedResource resource) {
        Map<String, String> targetObjectExtendInfo = Optional.ofNullable(task.getTargetObject().getExtendInfo())
                .orElseGet(HashMap::new);
        targetObjectExtendInfo.put(DatabaseConstants.VERSION, resource.getVersion());
        task.getTargetObject().setExtendInfo(targetObjectExtendInfo);
    }

    private void setRestoreAdvanceParams(RestoreTask task) {
        Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElseGet(HashMap::new);
        advanceParams.put(DatabaseConstants.TARGET_LOCATION_KEY, task.getTargetLocation().getLocation());
        JSONObject copyResource = getCopyResource(task.getCopyId());
        advanceParams.put(DatabaseConstants.COPY_PROTECT_OBJECT_VERSION_KEY,
                copyResource.getString(DatabaseConstants.VERSION));
        String subType = copyResource.getString(InformixConstant.SUB_TYPE);
        String multiPostJobValue = ResourceSubTypeEnum.INFORMIX_SINGLE_INSTANCE.getType().equals(subType)
                ? Boolean.FALSE.toString()
                : Boolean.TRUE.toString();
        advanceParams.put(DatabaseConstants.MULTI_POST_JOB, multiPostJobValue);
        task.setAdvanceParams(advanceParams);
    }

    private JSONObject getCopyResource(String copyId) {
        Copy copy = copyRestApi.queryCopyByID(copyId);
        return JSONObject.fromObject(copy.getResourceProperties());
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        return Collections.singletonList(
                new LockResourceBo(task.getTargetObject().getUuid(), LockType.WRITE));
    }

    private void setRestoreRepositories(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        String generatedBy = copy.getGeneratedBy();
        int backupType = copy.getBackupType();
        // 归档副本（全量）、差异备份、增量备份，恢复时删除日志仓
        if (CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value().equals(generatedBy)
                || CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value().equals(generatedBy)
                || backupType == BackupTypeConstants.DIFFERENCE_INCREMENT.getAbBackupType()
                || backupType == BackupTypeConstants.CUMULATIVE_INCREMENT.getAbBackupType()) {
            log.info("The current restore task is an archive copy restore or difference restore"
                    + " or increment restore, copy id: {}, copy generated by: {}, "
                    + "task id: {}.", task.getCopyId(), generatedBy, task.getTaskId());
            List<StorageRepository> oriRepositories = Optional.ofNullable(task.getRepositories())
                    .orElse(new ArrayList<>());
            List<StorageRepository> newRepositories = new ArrayList<>();
            for (StorageRepository tmpRepo : oriRepositories) {
                if (tmpRepo.getType() != RepositoryTypeEnum.LOG.getType()) {
                    newRepositories.add(tmpRepo);
                }
            }
            task.setRepositories(newRepositories);
        }
    }
}
