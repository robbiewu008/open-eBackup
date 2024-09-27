/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.redis.plugin.provider;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.redis.plugin.constant.RedisConstant;
import openbackup.redis.plugin.service.RedisService;
import openbackup.redis.plugin.util.RedisValidator;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Objects;
import java.util.Optional;

/**
 * 集群环境健康检查
 *
 * @author x30028756
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-25
 */
@Component
@Slf4j
public class RedisEnvironmentProvider extends DatabaseEnvironmentProvider {
    private final RedisService redisService;

    private final ResourceService resourceService;

    private final RedisResourceScanProvider redisResourceScanProvider;

    /**
     * RedisEnvironmentProvider 构造器
     *
     * @param providerManager provider管理器，获取bean和过滤bean
     * @param pluginConfigManager 插件配置管理器
     * @param resourceService 资源服务
     * @param redisService RedisService
     * @param redisResourceScanProvider redisResourceScanProvider
     */
    public RedisEnvironmentProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        ResourceService resourceService, RedisService redisService,
        RedisResourceScanProvider redisResourceScanProvider) {
        super(providerManager, pluginConfigManager);
        this.resourceService = resourceService;
        this.redisService = redisService;
        this.redisResourceScanProvider = redisResourceScanProvider;
    }

    /**
     * Redis集群环境注册provider过滤接口
     *
     * @param resourceSubType 受保护资源subType
     * @return boolean
     */
    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.REDIS.getType().equals(resourceSubType);
    }

    /**
     * 创建集群时，对参数的校验，以及添加从agent获取的信息
     *
     * @param environment 集群
     */
    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("Redis cluster,check,environment.name:{}", environment.getName());
        RedisValidator.checkCluster(environment);
        redisService.preCheck(environment);
        Optional<String> status = checkConnect(environment, false);
        environment.setLinkStatus(status.orElseGet(() -> LinkStatusEnum.ONLINE.getStatus().toString()));
        environment.setUuid(UUIDGenerator.getUUID());
    }

    private Optional<String> checkConnect(ProtectedEnvironment environment, boolean isUpdateDB) {
        log.info("Redis healthCheckWithResultStatus,environment.uuid:{}", environment.getUuid());
        ActionResult[] results = resourceService.check(environment);
        log.info("Redis healthCheckWithResultStatus,results.length:{}", results.length);
        int status = ClusterEnum.StatusEnum.NORMAL.getStatus();
        List<ProtectedResource> children = environment.getDependencies().get(ResourceConstants.CHILDREN);
        List<ProtectedResource> updatedChildren = new ArrayList<>();
        int normalCount = children.size();
        for (int i = 0; i < children.size(); i++) {
            ActionResult actionResult = results[i];
            String targetStatus;
            if (Objects.isNull(actionResult) || actionResult.getCode() != DatabaseConstants.SUCCESS_CODE) {
                normalCount--;
                targetStatus = String.valueOf(ClusterEnum.StatusEnum.OFFLINE.getStatus());
                status = RedisConstant.ABNORMAL_CODE;
            } else {
                targetStatus = String.valueOf(ClusterEnum.StatusEnum.ONLINE.getStatus());
            }
            ProtectedResource child = children.get(i);
            if (isUpdateDB) {
                if (!StringUtils.equals(child.getExtendInfo().get(DatabaseConstants.STATUS), targetStatus)) {
                    ProtectedResource updateChild = new ProtectedResource();
                    updateChild.setUuid(child.getUuid());
                    updateChild.setExtendInfo(Collections.singletonMap(DatabaseConstants.STATUS, targetStatus));
                    updatedChildren.add(updateChild);
                    log.info("Redis healthCheckWithResultStatus,update child:{}", child.getUuid());
                }
            } else {
                child.setExtendInfoByKey(DatabaseConstants.STATUS, targetStatus);
            }
        }
        if (isUpdateDB && CollectionUtils.isNotEmpty(updatedChildren)) {
            resourceService.updateSourceDirectly(updatedChildren);
        }
        if (normalCount == 0) {
            throw new LegoCheckedException(CommonErrorCode.HOST_OFFLINE, "Protected environment is offLine!");
        }
        return Optional.of(String.valueOf(status));
    }

    /**
     * 健康检查后，返回状态信息
     *
     * @param environment 集群
     * @return 状态信息
     */
    @Override
    public Optional<String> healthCheckWithResultStatus(ProtectedEnvironment environment) {
        return checkConnect(environment, true);
    }

    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        return redisResourceScanProvider.scan(environment);
    }
}