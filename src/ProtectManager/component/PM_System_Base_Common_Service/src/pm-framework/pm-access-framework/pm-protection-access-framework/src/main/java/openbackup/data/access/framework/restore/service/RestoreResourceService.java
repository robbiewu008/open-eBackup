/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.data.access.framework.restore.service;

import openbackup.data.access.framework.agent.DefaultProtectAgentSelector;
import openbackup.data.access.framework.agent.ProtectAgentSelector;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.access.framework.protection.common.util.FibreUtil;
import openbackup.data.access.framework.restore.constant.RestoreTaskErrorCode;
import openbackup.data.access.framework.restore.controller.req.CreateRestoreTaskRequest;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.v2.TaskResource;
import openbackup.data.protection.access.provider.sdk.enums.ProviderJobStatusEnum;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreInterceptorProvider;
import openbackup.data.protection.access.provider.sdk.restore.v2.RestoreTask;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.sdk.copy.model.Copy;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.BeanUtils;
import org.springframework.scheduling.annotation.Async;
import org.springframework.stereotype.Service;
import org.springframework.util.Assert;

import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 恢复任务资源服务
 *
 * @author y00559272
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022/7/26
 **/
@Slf4j
@Service
public class RestoreResourceService {
    private final ResourceService resourceService;
    private final ProtectedEnvironmentService protectedEnvironmentService;
    private final ProviderManager providerManager;
    private final ProtectAgentSelector defaultSelector;


    public RestoreResourceService(ResourceService resourceService,
        ProtectedEnvironmentService protectedEnvironmentService, ProviderManager providerManager,
        DefaultProtectAgentSelector defaultSelector) {
        this.providerManager = providerManager;
        this.resourceService = resourceService;
        this.protectedEnvironmentService = protectedEnvironmentService;
        this.defaultSelector = defaultSelector;
    }

    /**
     * 根据agent的id列表查询Endpoint列表
     *
     * @param advanceParams 扩展属性
     * @param subType 恢复资源子类型
     * @param envType 恢复目标环境类型
     * @param targetEnv 恢复目标环境
     * @return Endpoint对象列表
     */
    public List<Endpoint> queryEndpoints(Map<String, String> advanceParams, String subType, String envType,
        ProtectedEnvironment targetEnv) {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setSubType(subType);
        protectedResource.setEnvironment(targetEnv);
        // 设置保护代理
        ProtectAgentSelector selector =
            providerManager.findProviderOrDefault(ProtectAgentSelector.class, envType, defaultSelector);
        return selector.select(protectedResource, advanceParams);
    }

    /**
     * 构造目标对象
     * <p>
     * 本函数有两种情况：1.如果targetObject为资源id，这个时候可以查询到资源，由框架统一处理，则构造任务资源信息返回</br>
     * 2. 如果targetObject不是资源id，查询不到资源，框架无法处理，避免插件中拿不到数据</br>
     * 构造默认对象将targetObject放到 {@code TaskResource}的name字段中，</br>
     * 同时设置uuid和资源子类型，由插件自行处理</br>
     * </p>
     *
     * @param targetObject 目标资源
     * @return TaskResource 任务资源对象
     */
    TaskResource buildTaskResourceByTargetObject(final String targetObject) {
        Assert.isTrue(StringUtils.isNotBlank(targetObject), "param targetObject is empty");
        final ProtectedResource protectedResource = resourceService.getResourceById(targetObject)
                .orElseGet(ProtectedResource::new);
        TaskResource taskResource = new TaskResource();
        BeanUtils.copyProperties(protectedResource, taskResource);
        return taskResource;
    }

    private RestoreInterceptorProvider getProvider(String resourceSubType) {
        return providerManager.findProvider(
                RestoreInterceptorProvider.class,
                resourceSubType,
                new LegoCheckedException("Restore task can not find provider."));
    }

    ProtectedEnvironment queryProtectedEnvironment(CreateRestoreTaskRequest taskRequest, Copy copy) {
        String targetEnv = taskRequest.getTargetEnv();
        try {
            RestoreInterceptorProvider provider = getProvider(copy.getResourceSubType());
            Optional<ProtectedEnvironment> protectedEnvironment = provider.queryEnvironment(targetEnv);
            if (!protectedEnvironment.isPresent()) {
                return protectedEnvironmentService.getEnvironmentById(targetEnv);
            }
            return protectedEnvironment.get();
        } catch (LegoCheckedException ex) {
            if (ex.getErrorCode() != CommonErrorCode.OBJ_NOT_EXIST) {
                throw ex;
            }
            // 修改原因：查询环境如果不存在会抛出OBJ_NOT_EXIST错误码，错误提示不明确。
            throw new LegoCheckedException(RestoreTaskErrorCode.RESTORE_TARGET_ENV_NOT_EXISTED,
                new String[] {targetEnv}, "Restore target env does not exist.");
        }
    }

    /**
     * 刷新恢复目标环境资源
     * <p>
     * 该请求为异步请求，并捕获全部异常，刷新环境失败不影响恢复任务结果。
     * 如果执行失败，后续通过系统定时刷新同步环境资源信息。
     * </p>
     *
     * @param requestId 请求id
     * @param targetEnvId 请求id
     * @param jobStatus 任务状态
     */
    @Async
    void refreshTargetEnv(String requestId, String targetEnvId, ProviderJobStatusEnum jobStatus) {
        log.info("Restore task refresh env, requestId={}, envId={}", requestId, targetEnvId);
        if (!jobStatus.checkSuccess()) {
            // 失败状态任务不刷新资源
            return;
        }
        try {
            protectedEnvironmentService.refreshEnvironment(targetEnvId);
        } catch (Exception ex) {
            log.error("Restore task refresh env error.", ex);
        }
    }

    /**
     * 通过agent id集合，获取其存在扩展表中的LanFree配置项
     * lanFree配置项 可能为0（未配置）,1（已配置）等
     *
     * @param subType 资源类型
     * @param task 恢复任务
     * @return Map结构：key: agentId, value: lanFree配置项
     */
    public Map<String, String> getLanFreeConfig(String subType, RestoreTask task) {
        return resourceService.getLanFreeConfig(subType, FibreUtil.getAgentIds(task.getAgents()));
    }
}
