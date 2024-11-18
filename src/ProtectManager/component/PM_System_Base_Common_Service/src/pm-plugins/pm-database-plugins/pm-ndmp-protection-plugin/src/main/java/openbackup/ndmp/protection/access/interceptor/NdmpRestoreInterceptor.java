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
package openbackup.ndmp.protection.access.interceptor;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.DatabaseDeployTypeEnum;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.ndmp.protection.access.constant.NdmpConstant;
import openbackup.ndmp.protection.access.service.NdmpService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.enums.ProtectionStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.MapUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Random;

/**
 * HCS-GaussDb 恢复任务基础数据Provider
 *
 */
@Slf4j
@Component
public class NdmpRestoreInterceptor extends AbstractDbRestoreInterceptorProvider {
    private final List<String> copyGeneratedByEnumList = Arrays.asList(CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value(),
        CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value());

    private final NdmpService ndmpService;

    private final CopyRestApi copyRestApi;

    private final ResourceService resourceService;

    /**
     * 构造器
     *
     * @param ndmpService ndmpService
     * @param copyRestApi copyRestApi
     * @param resourceService resourceService
     */
    public NdmpRestoreInterceptor(NdmpService ndmpService, CopyRestApi copyRestApi, ResourceService resourceService) {
        this.ndmpService = ndmpService;
        this.copyRestApi = copyRestApi;
        this.resourceService = resourceService;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.NDMP_BACKUPSET.getType().equals(object);
    }

    @Override
    public RestoreTask supplyRestoreTask(RestoreTask task) {
        TaskResource taskResource = task.getTargetObject();
        Map<String, String> taskExtendInfo = taskResource.getExtendInfo();
        String fullName = MapUtils.getString(taskExtendInfo, "fullName");
        taskResource.setPath(fullName);
        taskResource.setName(fullName);
        ProtectedEnvironment environment = ndmpService.getEnvironmentById(task.getTargetEnv().getUuid());
        if (!LinkStatusEnum.ONLINE.getStatus().toString().equals(environment.getLinkStatus())) {
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_LINK_STATUS_ERROR,
                "Ndmp  " + environment.getName() + " not online");
        }
        supplyAgent(task);
        supplyNodes(task);
        supplyRestoreMode(task);
        supplyAdvancedParams(task);
        return task;
    }

    private void supplyAdvancedParams(RestoreTask task) {
        Map<String, String> advancedParams = Optional.ofNullable(task.getAdvanceParams()).orElse(new HashMap<>());
        advancedParams.put(NdmpConstant.SPEED_STATISTICS, SpeedStatisticsEnum.APPLICATION.getType());
        task.setAdvanceParams(advancedParams);
        task.getTargetEnv().getExtendInfo().put(DatabaseConstants.DEPLOY_TYPE, DatabaseDeployTypeEnum.SINGLE.getType());
        task.getTargetEnv().getExtendInfo().put(DatabaseConstants.SERVICE_IP, task.getAgents().get(0).getIp());
    }

    /**
     * 设置恢复模式
     *
     * @param task 恢复任务
     */
    public void supplyRestoreMode(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        String generatedBy = copy.getGeneratedBy();
        if (copyGeneratedByEnumList.contains(generatedBy)) {
            task.setRestoreMode(RestoreModeEnum.DOWNLOAD_RESTORE.getMode());
        } else {
            task.setRestoreMode(RestoreModeEnum.LOCAL_RESTORE.getMode());
        }
        log.info("build NDMP copy restore mode. copy id: {}, mode: {}", task.getCopyId(), task.getRestoreMode());
    }

    private void supplyAgent(RestoreTask task) {
        String resourceId = task.getTargetObject().getUuid();
        ProtectedResource resource = resourceService.getResourceById(resourceId)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST,
                "Protected Resource is not exists. uuid: " + resourceId));
        if (!Objects.equals(resource.getProtectionStatus(), ProtectionStatusEnum.PROTECTED.getType())) {
            // 恢复到新位置未保护，下发内置代理
            log.info("Target resource not protected, request id: {}", task.getRequestId());
            List<ProtectedResource> interAgents = this.ndmpService.getInterAgents();
            if (CollectionUtils.isEmpty(interAgents)) {
                throw new LegoCheckedException(CommonErrorCode.AGENT_NOT_EXIST, "No agent found");
            }
            Endpoint endpoint = interAgents.stream()
                .skip(new Random().nextInt(interAgents.size()))
                .findFirst()
                .map((env) -> new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort()))
                .orElse(null);
            task.setAgents(Collections.singletonList(endpoint));
            return;
        }
        String agents = MapUtils.getString(resource.getProtectedObject().getExtParameters(), DatabaseConstants.AGENTS);
        if (StringUtils.isEmpty(agents)) {
            throw new LegoCheckedException(CommonErrorCode.AGENT_NOT_EXIST, "No agent found");
        }
        log.info("NdmpRestoreInterceptor supplyAgent, agents:{}", agents);
        String parentUuid = task.getTargetObject().getParentUuid();
        log.info("NdmpRestoreInterceptor supplyAgent, parentUuid:{}", parentUuid);
        task.setAgents(ndmpService.getAgents(parentUuid, agents));
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        return Collections.singletonList(new LockResourceBo(task.getTargetObject().getUuid(), LockType.WRITE));
    }

    private void supplyNodes(RestoreTask task) {
        task.getTargetEnv().setNodes(ndmpService.supplyNodes());
    }
}