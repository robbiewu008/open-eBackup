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
package openbackup.database.base.plugin.provider;

import com.google.common.collect.Maps;

import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.backup.BackupTypeConstants;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.restore.RestoreFeature;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.AppConf;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.common.GeneralDbConstant;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.database.base.plugin.util.GeneralDbUtil;
import openbackup.database.base.plugin.utils.AgentDtoUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.EnumUtil;

import org.springframework.stereotype.Component;
import org.springframework.util.CollectionUtils;

import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 通用数据库恢复拦截器
 *
 */
@Component
public class GeneralDbRestoreInterceptorProvider extends AbstractDbRestoreInterceptorProvider {
    private final GeneralDbProtectAgentService generalDbProtectAgentService;

    private final AgentUnifiedService agentUnifiedService;

    private final CopyRestApi copyRestApi;

    public GeneralDbRestoreInterceptorProvider(GeneralDbProtectAgentService generalDbProtectAgentService,
        AgentUnifiedService agentUnifiedService, CopyRestApi copyRestApi) {
        this.generalDbProtectAgentService = generalDbProtectAgentService;
        this.agentUnifiedService = agentUnifiedService;
        this.copyRestApi = copyRestApi;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.GENERAL_DB.equalsSubType(object);
    }

    @Override
    public RestoreTask initialize(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        // 校验conf信息
        checkAppConf(task, copy);
        // multiPostJob
        setConfInfo(task);
        // restoreMode
        setRestoreMode(task, copy);
        // 设置配置信息
        setConfInfo(task);
        // agents
        supplyAgent(task);
        // nodes
        supplyNodes(task);
        return task;
    }

    @Override
    public RestoreFeature getRestoreFeature() {
        RestoreFeature restoreFeature = super.getRestoreFeature();
        restoreFeature.setShouldCheckEnvironmentIsOnline(false);
        return restoreFeature;
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        return Collections.singletonList(new LockResourceBo(task.getTargetObject().getUuid(), LockType.WRITE));
    }

    private void setRestoreMode(RestoreTask task, Copy copy) {
        String generatedBy = copy.getGeneratedBy();
        if (CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value().equals(generatedBy)
            || CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value().equals(generatedBy)) {
            task.setRestoreMode(RestoreModeEnum.DOWNLOAD_RESTORE.getMode());
        } else {
            task.setRestoreMode(RestoreModeEnum.LOCAL_RESTORE.getMode());
        }
    }

    private void supplyAgent(RestoreTask task) {
        String targetEnvId = task.getTargetEnv().getUuid();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid(targetEnvId);
        List<Endpoint> endpoints = generalDbProtectAgentService.select(protectedResource);
        task.setAgents(endpoints);
    }

    private void checkAppConf(RestoreTask task, Copy copy) {
        Optional<AppConf> appConf = getAppConf(task);
        if (!appConf.isPresent()) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "script is null");
        }
        List<AppConf.Restore.Support> supports = appConf.map(AppConf::getRestore)
            .map(AppConf.Restore::getSupports)
            .orElse(Collections.emptyList());
        if (VerifyUtil.isEmpty(supports)) {
            return;
        }
        String confRestoreType = switchConfRestoreType(task);
        boolean hasCheckSupport = false;
        for (AppConf.Restore.Support support : supports) {
            if (confRestoreType.equalsIgnoreCase(support.getRestoreType())) {
                hasCheckSupport = checkBackupSupport(support, task, copy);
                if (hasCheckSupport) {
                    break;
                }
            }
        }
        if (!hasCheckSupport) {
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "check conf support error.");
        }
    }

    private boolean checkBackupSupport(AppConf.Restore.Support support, RestoreTask task, Copy copy) {
        if (VerifyUtil.isEmpty(support.getIncludeBackupTypes())) {
            return true;
        }
        String backupTypeName = getBackupNameFromCopy(copy);
        if (VerifyUtil.isEmpty(backupTypeName)) {
            return true;
        }
        String version = getResourceVersionFromCopy(copy);
        for (String includeBackupType : support.getIncludeBackupTypes()) {
            if (backupTypeName.equalsIgnoreCase(includeBackupType) && GeneralDbUtil.checkVersion(version,
                support.getMinVersion(), support.getMaxVersion())) {
                return true;
            }
        }
        return false;
    }

    private String getBackupNameFromCopy(Copy copy) {
        return BackupTypeConstants.convert2BackupType(copy.getBackupType());
    }

    private String getResourceVersionFromCopy(Copy copy) {
        String resourceProperties = copy.getResourceProperties();
        ProtectedResource copyResource = JsonUtil.read(resourceProperties, ProtectedResource.class);
        return copyResource.getVersion();
    }

    private String switchConfRestoreType(RestoreTask task) {
        String timeStamp = Optional.ofNullable(task.getAdvanceParams())
            .map(e -> e.get(GeneralDbConstant.RESTORE_TIME_STAMP_KEY))
            .orElse(null);
        if (!VerifyUtil.isEmpty(timeStamp)) {
            return GeneralDbConstant.RESTORE_ANY_TIME_POINT;
        }
        return task.getRestoreType();
    }

    private void supplyNodes(RestoreTask task) {
        if (CollectionUtils.isEmpty(task.getAgents())) {
            return;
        }
        List<TaskEnvironment> hostList = task.getAgents()
            .stream()
            .map(agent -> agentUnifiedService.getHost(agent.getIp(), agent.getPort()))
            .map(AgentDtoUtil::toTaskEnvironment)
            .collect(Collectors.toList());
        if (task.getTargetEnv() == null) {
            return;
        }
        task.getTargetEnv().setNodes(hostList);
    }

    private void setConfInfo(RestoreTask task) {
        Optional<AppConf> appConf = getAppConf(task);

        // 统计速率
        String speedStatistics = appConf.map(AppConf::getRestore)
            .map(AppConf.Restore::getSpeedStatistics)
            .orElse(SpeedStatisticsEnum.UBC.getType());

        SpeedStatisticsEnum speedStatisticsEnum = EnumUtil.get(SpeedStatisticsEnum.class, SpeedStatisticsEnum::getType,
            speedStatistics, true, true);
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task,
            speedStatisticsEnum == null ? SpeedStatisticsEnum.UBC : speedStatisticsEnum);

        // 是否多节点执行
        boolean isMultiPostJob = appConf.map(AppConf::getRestore).map(AppConf.Restore::getIsMultiPostJob).orElse(false);
        Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElse(Maps.newHashMap());
        advanceParams.put(DatabaseConstants.MULTI_POST_JOB, String.valueOf(isMultiPostJob));
        task.setAdvanceParams(advanceParams);
    }

    private Optional<AppConf> getAppConf(RestoreTask task) {
        Map<String, String> extendInfo = Optional.ofNullable(task.getTargetEnv().getExtendInfo())
            .orElse(new HashMap<>());
        String confStr = extendInfo.get(GeneralDbConstant.EXTEND_SCRIPT_CONF);
        if (VerifyUtil.isEmpty(confStr)) {
            return Optional.empty();
        }
        return GeneralDbUtil.getAppConf(confStr);
    }
}
