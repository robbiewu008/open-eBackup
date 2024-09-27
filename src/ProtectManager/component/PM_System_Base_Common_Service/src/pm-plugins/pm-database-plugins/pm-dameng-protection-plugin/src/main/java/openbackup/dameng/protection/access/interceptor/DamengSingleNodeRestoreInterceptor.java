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

import openbackup.dameng.protection.access.constant.DamengConstant;
import openbackup.dameng.protection.access.service.DamengService;
import openbackup.dameng.protection.access.util.DamengParamCheckUtil;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.enums.v2.RestoreTypeEnum;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreLocationEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.restore.RestoreFeature;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.database.base.plugin.utils.AgentDtoUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * dameng单机恢复任务下发provider
 *
 * @author lWX1100347
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-07-09
 */
@Slf4j
@Component
public class DamengSingleNodeRestoreInterceptor extends AbstractDbRestoreInterceptorProvider {
    private final DamengService damengService;

    private final AgentUnifiedService agentUnifiedService;

    private final CopyRestApi copyRestApi;

    public DamengSingleNodeRestoreInterceptor(DamengService damengService, AgentUnifiedService agentUnifiedService,
        CopyRestApi copyRestApi) {
        this.damengService = damengService;
        this.agentUnifiedService = agentUnifiedService;
        this.copyRestApi = copyRestApi;
    }

    @Override
    public boolean applicable(String subType) {
        return ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType().equals(subType);
    }

    @Override
    public RestoreTask initialize(RestoreTask task) {
        // 新位置恢复高级参数校验
        checkAdvanceParams(task);
        // 归档副本恢复校验
        checkArchiveCopyRestore(task);
        // 表空间恢复校验
        checkTablespaceRestore(task);
        // 设置部署类型
        task.getTargetEnv().getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        // 设置agents
        setAgents(task);
        // 设置nodes
        setNodes(task);
        // 设置速度统计方式为UBC
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);
        // 设置高级参数targetLocation
        damengService.setRestoreAdvanceParams(task);
        // 设置恢复模式
        damengService.setRestoreMode(task);
        // 细粒度恢复设置subObjects
        updateSubObjects(task);
        // 更新目标路径到dbPath
        if (RestoreLocationEnum.NEW.equals(task.getTargetLocation())) {
            updateDbPath(task);
            updateDbPort(task);
        }
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

    private void updateSubObjects(RestoreTask task) {
        if (!RestoreTypeEnum.FLR.getType().equals(task.getRestoreType())) {
            return;
        }
        List<TaskResource> subObjects = Optional.ofNullable(task.getSubObjects())
            .orElse(new ArrayList<>())
            .stream()
            .map(item -> {
                TaskResource taskResource = new TaskResource();
                Map<String, String> extendInfo = new HashMap<>();
                extendInfo.put(DatabaseConstants.NAME, item.getName());
                taskResource.setExtendInfo(extendInfo);
                return taskResource;
            })
            .collect(Collectors.toList());
        log.info("build restore task subObjects success, request id:{}", task.getRequestId());
        task.setSubObjects(subObjects);
    }

    private void setAgents(RestoreTask task) {
        ProtectedEnvironment environment = damengService.getEnvironmentById(task.getTargetObject().getParentUuid());
        List<Endpoint> agents = Collections.singletonList(damengService.getAgentEndpoint(environment));
        task.setAgents(agents);
    }

    private void setNodes(RestoreTask task) {
        List<TaskEnvironment> hostList = task.getAgents()
            .stream()
            .map(agent -> agentUnifiedService.getHost(agent.getIp(), agent.getPort()))
            .map(AgentDtoUtil::toTaskEnvironment)
            .collect(Collectors.toList());
        task.getTargetEnv().setNodes(hostList);
    }

    private void updateDbPath(RestoreTask task) {
        Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElse(new HashMap<>());
        String dbPath = advanceParams.get(DamengConstant.DB_PATH);
        if (!VerifyUtil.isEmpty(dbPath)) {
            Map<String, String> extendInfo = Optional.ofNullable(task.getTargetEnv().getExtendInfo())
                .orElse(new HashMap<>());
            extendInfo.put(DamengConstant.DB_PATH, dbPath);
            task.getTargetEnv().setExtendInfo(extendInfo);
        }
    }

    private void updateDbPort(RestoreTask task) {
        Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElse(new HashMap<>());
        String dbPort = advanceParams.get(DamengConstant.DB_PORT);
        if (!VerifyUtil.isEmpty(dbPort)) {
            Map<String, String> extendInfo = Optional.ofNullable(task.getTargetEnv().getExtendInfo())
                .orElse(new HashMap<>());
            extendInfo.put(DamengConstant.DB_PORT, dbPort);
            task.getTargetEnv().setExtendInfo(extendInfo);
        }
    }

    private void checkTablespaceRestore(RestoreTask task) {
        if (!RestoreTypeEnum.FLR.getType().equals(task.getRestoreType())) {
            return;
        }
        // 表空间恢复只能原机原位置
        if (!RestoreLocationEnum.ORIGINAL.equals(task.getTargetLocation())) {
            log.error("Dameng tablespace restore task:{} supports only the original location of the original host.",
                task.getTaskId());
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                "Dameng tablespace restoration supports only the original location of the original host.");
        }
        // 归档副本不支持表空间恢复
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        if (CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value().equals(copy.getGeneratedBy())
            || CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value().equals(copy.getGeneratedBy())) {
            log.error("Dameng archive copy restore task:{} do not support tablespace restoration.", task.getTaskId());
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                "Dameng archive copies do not support tablespace restoration.");
        }
    }

    private void checkAdvanceParams(RestoreTask task) {
        if (RestoreLocationEnum.ORIGINAL.equals(task.getTargetLocation())) {
            return;
        }
        Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElse(new HashMap<>());
        String dbPath = advanceParams.get(DamengConstant.DB_PATH);
        String dbName = advanceParams.get(DamengConstant.DB_NAME);
        String dbPort = advanceParams.get(DamengConstant.DB_PORT);
        if (VerifyUtil.isEmpty(dbPath) || VerifyUtil.isEmpty(dbName)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                "Dameng dbPath or dbName restore from the new location is empty.");
        }
        if (StringUtils.isNotBlank(dbPort)) {
            DamengParamCheckUtil.checkRestoreDbPort(dbPort);
        }
    }

    private void checkArchiveCopyRestore(RestoreTask task) {
        if (!RestoreLocationEnum.ORIGINAL.equals(task.getTargetLocation())) {
            return;
        }
        // 复制再归档的副本不支持原位置恢复
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        if ((CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value().equals(copy.getGeneratedBy())
            || CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value().equals(copy.getGeneratedBy())) && copy.getIsReplicated()) {
            throw new LegoCheckedException(CommonErrorCode.SYSTEM_ERROR,
                "Dameng archive copies generated by replicas cannot be restored from the original location.");
        }
    }
}
