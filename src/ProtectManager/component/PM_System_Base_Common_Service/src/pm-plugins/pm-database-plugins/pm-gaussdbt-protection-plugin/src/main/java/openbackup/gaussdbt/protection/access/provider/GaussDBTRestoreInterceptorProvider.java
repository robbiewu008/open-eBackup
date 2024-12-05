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
package openbackup.gaussdbt.protection.access.provider;

import com.huawei.oceanprotect.sla.sdk.api.SlaQueryService;
import com.huawei.oceanprotect.sla.sdk.dto.PolicyDto;
import com.huawei.oceanprotect.sla.sdk.dto.SlaDto;

import com.fasterxml.jackson.databind.JsonNode;

import lombok.extern.slf4j.Slf4j;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskEnvironment;
import openbackup.data.protection.access.provider.sdk.enums.MountTypeEnum;
import openbackup.data.protection.access.provider.sdk.enums.RestoreModeEnum;
import openbackup.data.protection.access.provider.sdk.enums.SpeedStatisticsEnum;
import openbackup.data.protection.access.provider.sdk.lock.LockResourceBo;
import openbackup.data.protection.access.provider.sdk.lock.LockType;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedObject;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.data.protection.access.provider.sdk.util.TaskUtil;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.interceptor.AbstractDbRestoreInterceptorProvider;
import openbackup.gaussdbt.protection.access.provider.constant.GaussDBTConstant;
import openbackup.gaussdbt.protection.access.provider.util.GaussDBTClusterUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.CopyRestApi;
import openbackup.system.base.sdk.copy.model.Copy;
import openbackup.system.base.sdk.copy.model.CopyGeneratedByEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.OptionalUtil;

import org.apache.commons.collections.CollectionUtils;
import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 实现恢复接口
 *
 */
@Component
@Slf4j
public class GaussDBTRestoreInterceptorProvider extends AbstractDbRestoreInterceptorProvider {
    private final ProviderManager providerManager;

    private final ResourceService resourceService;

    private final CopyRestApi copyRestApi;

    private final SlaQueryService slaQueryService;

    /**
     * Constructor
     *
     * @param providerManager 框架服务
     * @param resourceService 资源服务
     * @param copyRestApi 副本接口
     * @param slaQueryService slaQueryService
     */
    public GaussDBTRestoreInterceptorProvider(ProviderManager providerManager, ResourceService resourceService,
        CopyRestApi copyRestApi, SlaQueryService slaQueryService) {
        this.providerManager = providerManager;
        this.resourceService = resourceService;
        this.copyRestApi = copyRestApi;
        this.slaQueryService = slaQueryService;
    }

    @Override
    public boolean applicable(String object) {
        return ResourceSubTypeEnum.GAUSSDBT.getType().equals(object);
    }

    @Override
    public RestoreTask initialize(RestoreTask task) {
        // 回填GaussDBT环境的agents
        buildRestoreAgents(task);

        // 设置高级参数：targetLocation、
        buildRestoreAdvanceParams(task);

        // 目标恢复环境的nodes信息
        buildRestoreTargetEvn(task);

        // 设置恢复副本类型
        buildRestoreMode(task);

        // 设置速度统计方式为UBC
        TaskUtil.setRestoreTaskSpeedStatisticsEnum(task, SpeedStatisticsEnum.UBC);

        return task;
    }

    @Override
    public List<LockResourceBo> getLockResources(RestoreTask task) {
        return Collections.singletonList(new LockResourceBo(task.getTargetObject().getUuid(), LockType.WRITE));
    }

    /**
     * 设置恢复模式
     *
     * @param task 恢复任务
     */
    public void buildRestoreMode(RestoreTask task) {
        Copy copy = copyRestApi.queryCopyByID(task.getCopyId());
        String generatedBy = copy.getGeneratedBy();
        if (Arrays.asList(CopyGeneratedByEnum.BY_CLOUD_ARCHIVE.value(), CopyGeneratedByEnum.BY_TAPE_ARCHIVE.value())
            .contains(generatedBy)) {
            task.setRestoreMode(RestoreModeEnum.DOWNLOAD_RESTORE.getMode());
        } else {
            task.setRestoreMode(RestoreModeEnum.LOCAL_RESTORE.getMode());
        }
        log.info("build GaussDBT copy restore mode. copy id: {}, mode: {}", task.getCopyId(), task.getRestoreMode());
    }

    /**
     * 添加高级参数
     *
     * @param task 恢复任务
     */
    private void buildRestoreAdvanceParams(RestoreTask task) {
        Map<String, String> advanceParams = Optional.ofNullable(task.getAdvanceParams()).orElse(new HashMap<>());

        // 框架不会带targetLocation参数到dme,回填targetLocation高级参数中下发到插件
        advanceParams.put(GaussDBTConstant.TARGET_LOCATION_KEY, task.getTargetLocation().getLocation());

        // 挂载类型：非全路径挂载
        advanceParams.put(GaussDBTConstant.MOUNT_TYPE_KEY, MountTypeEnum.FULL_PATH_MOUNT.getMountType());
        // 支持多任务
        advanceParams.put(DatabaseConstants.MULTI_POST_JOB, Boolean.TRUE.toString());
        Optional<ProtectedResource> protectedResourceOptional = resourceService.getResourceById(
            task.getTargetObject().getUuid());
        if (protectedResourceOptional.isPresent()) {
            ProtectedResource protectedResource = protectedResourceOptional.get();
            ProtectedObject protectedObject = protectedResource.getProtectedObject();
            setParallelProcess(advanceParams, protectedResource, protectedObject);
        }
        task.setAdvanceParams(advanceParams);
    }

    private void setParallelProcess(Map<String, String> advanceParams, ProtectedResource protectedResource,
        ProtectedObject protectedObject) {
        if (!Objects.isNull(protectedObject)) {
            SlaDto slaDto = slaQueryService.querySlaById(protectedResource.getProtectedObject().getSlaId());
            List<PolicyDto> policyDtoList = slaDto.getPolicyList();
            if (CollectionUtils.isNotEmpty(policyDtoList)) {
                PolicyDto policyDto = policyDtoList.get(0);
                JsonNode extParameters = policyDto.getExtParameters();
                if (extParameters.has(GaussDBTConstant.PARALLEL_PROCESS)) {
                    int parallelProcess = extParameters.get(GaussDBTConstant.PARALLEL_PROCESS).asInt();
                    advanceParams.put(GaussDBTConstant.PARALLEL_PROCESS, String.valueOf(parallelProcess));
                }
            }
        }
    }

    /**
     * 添加恢复的agents到任务中
     *
     * @param task 恢复任务
     */
    private void buildRestoreAgents(RestoreTask task) {
        ProtectedResource protectedResource = resourceService.getResourceById(false, task.getTargetEnv().getUuid())
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "protectedResource is not exist"));
        ResourceCheckContext resourceCheckContext = providerManager.findProvider(ResourceConnectionCheckProvider.class,
            protectedResource).checkConnection(protectedResource);
        List<Endpoint> endpointList = resourceCheckContext.getResourceConnectableMap()
            .values()
            .stream()
            .findFirst()
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Endpoint is not exist"))
            .stream()
            .map(env -> new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort()))
            .collect(Collectors.toList());
        task.setAgents(endpointList);
    }

    /**
     * 增加targetEnv中nodes
     *
     * @param task 恢复任务
     */
    private void buildRestoreTargetEvn(RestoreTask task) {
        TaskEnvironment targetEnv = Optional.ofNullable(task.getTargetEnv())
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "targetEnv is empty"));
        String uuid = targetEnv.getUuid();
        ProtectedEnvironment environment = resourceService.getResourceById(uuid)
            .flatMap(OptionalUtil.match(ProtectedEnvironment.class))
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "targetEnv not exist. uuid:" + uuid));

        // 获取目标环境的节点信息并设置到TargetEnv的nodes中
        List<TaskEnvironment> nodes = GaussDBTClusterUtil.getNodesFromEnv(environment);
        targetEnv.setNodes(nodes);
        task.setTargetEnv(targetEnv);
    }
}
