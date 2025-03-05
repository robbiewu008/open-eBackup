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
package openbackup.redis.plugin.provider;

import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import com.huawei.oceanprotect.system.base.kerberos.service.KerberosService;

import com.google.common.collect.Lists;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.access.framework.resource.service.provider.UnifiedResourceConnectionChecker;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.CheckReport;
import openbackup.data.protection.access.provider.sdk.resource.CheckResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.utils.AuthParamUtil;
import openbackup.redis.plugin.service.RedisService;
import openbackup.redis.plugin.util.RedisValidator;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * 集群环境和单节点连通性校验
 *
 */
@Component
@Slf4j
public class RedisConnectionChecker extends UnifiedResourceConnectionChecker {
    private final ResourceService resourceService;

    private final RedisService redisService;

    private final KerberosService kerberosService;

    private final EncryptorService encryptorService;

    @Autowired
    private ProtectedEnvironmentService environmentService;

    /**
     * 有参构造
     *
     * @param environmentRetrievalsService 环境服务
     * @param agentUnifiedService agent接口实现
     * @param redisService redisService
     * @param kerberosService kerberosService
     * @param encryptorService encryptorService
     * @param resourceService resourceService
     */
    public RedisConnectionChecker(final ProtectedEnvironmentRetrievalsService environmentRetrievalsService,
        final AgentUnifiedService agentUnifiedService, RedisService redisService, KerberosService kerberosService,
        EncryptorService encryptorService, ResourceService resourceService) {
        super(environmentRetrievalsService, agentUnifiedService);
        this.redisService = redisService;
        this.kerberosService = kerberosService;
        this.encryptorService = encryptorService;
        this.resourceService = resourceService;
    }

    /**
     * 连通性子类过滤接口
     *
     * @param object 受保护资源
     * @return boolean 是否使用该类进行连通性校验
     */
    @Override
    public boolean applicable(ProtectedResource object) {
        return Objects.nonNull(object) && ResourceSubTypeEnum.REDIS.getType().equals(object.getSubType());
    }

    /**
     * 增加Redis应用的校验逻辑
     *
     * @param protectedResource 受保护资源
     * @return boolean 连通性校验的结果
     */
    @Override
    public CheckResult<Object> generateCheckResult(ProtectedResource protectedResource) {
        RedisValidator.checkNode(protectedResource);
        redisService.checkNodeExists(protectedResource);
        AuthParamUtil.convertKerberosAuth(protectedResource.getAuth(), kerberosService,
            protectedResource.getAuth().getExtendInfo().get(DatabaseConstants.EXTEND_INFO_KEY_KERBEROS_ID),
            encryptorService);
        CheckResult<Object> checkResult = super.generateCheckResult(protectedResource);
        // 根据结果更新资源状态
        updateChildRes(protectedResource, checkResult);
        // 敏感信息最后一次使用结束后未在内存中移除
        AuthParamUtil.removeSensitiveInfo(protectedResource);
        return checkResult;
    }

    private void updateChildRes(ProtectedResource protectedResource, CheckResult<Object> checkResult) {
        if (checkResult.getResults().getCode() == ActionResult.SUCCESS_CODE) {
            log.info("Start to update Redis Child Online after connection check, resource id: {}",
                protectedResource.getUuid());
            updateEnvStatus(protectedResource.getUuid(), LinkStatusEnum.ONLINE.getStatus().toString());
        } else {
            log.info("Start to update Redis Child OFFLINE after connection check, resource id: {}",
                protectedResource.getUuid());
            updateEnvStatus(protectedResource.getUuid(), LinkStatusEnum.OFFLINE.getStatus().toString());
        }
    }

    /**
     * 获取集群节点及其对应的主机
     *
     * @param environment 集群
     * @return key为集群节点，value为集群节点对应的主机列表
     */
    @Override
    public Map<ProtectedResource, List<ProtectedEnvironment>> collectConnectableResources(
        ProtectedResource environment) {
        log.info("RedisConnectionChecker,collectConnectableResources,environment.uuid: {}", environment.getUuid());
        Map<ProtectedResource, List<ProtectedEnvironment>> nodeHostMap = new LinkedHashMap<>();
        List<ProtectedResource> children = environment.getDependencies().get(ResourceConstants.CHILDREN);
        for (ProtectedResource child : children) {
            String uuid = child.getDependencies().get(DatabaseConstants.AGENTS).get(0).getUuid();
            log.info("agent_id: {}", uuid);
            ProtectedEnvironment host = environmentService.getEnvironmentById(uuid);
            nodeHostMap.put(child, Lists.newArrayList(host));
        }
        return nodeHostMap;
    }


    @Override
    public List<ActionResult> collectActionResults(List<CheckReport<Object>> checkReport, Map<String, Object> context) {
        return super.collectActionResults(updateResourceLinkStatus(checkReport), context);
    }

    private List<CheckReport<Object>> updateResourceLinkStatus(List<CheckReport<Object>> checkReport) {
        String resourceId = checkReport.get(0).getResource().getUuid();
        // 注册时，检查连通性之后不做任何操作，直接返回
        Optional<ProtectedResource> resourceById = resourceService.getResourceById(resourceId);
        if (!resourceById.isPresent()) {
            return checkReport;
        }
        String envId = resourceById.get().getRootUuid();
        log.info(
            "Start update Redis ENV resourceENV1 link status after check connection, "
                + "resource id: {}, rootResourceId is:{}", resourceId, envId);
        // 对于集群数据库来说，只要有一个实例在线，集群数据库就在线
        List<CheckReport<Object>> online = checkReport.stream()
            .filter(report -> isOnline(report.getResults()))
            .collect(Collectors.toList());

        log.info("Start to update Redis ENV resourceENV1 onlineNum: {}, children.checkReport.size:{}", online.size(),
            checkReport.size());
        if (online.size() == 0) {
            // 没有在线子节点，根据结果更新资源状态为离线
            log.info("Start to update Redis ENV resourceENV1 OFFLINE after connection check, resource id: {}", envId);
            updateEnvStatus(envId, LinkStatusEnum.OFFLINE.getStatus().toString());
        } else if (online.size() == checkReport.size()) {
            // 所有子节点在线，根据结果更新资源状态为在线
            log.info("Start to update Redis ENV resourceENV1 ONLINE after connection check, resource id: {}", envId);
            updateEnvStatus(envId, LinkStatusEnum.ONLINE.getStatus().toString());
        } else {
            // 部分子节点离线，根据结果更新资源状态为异常
            log.info("Start to update Redis ENV resourceENV1 PARTLY_ONLING after connection check, resource id: {}",
                envId);
            updateEnvStatus(envId, LinkStatusEnum.PARTLY_ONLING.getStatus().toString());
        }
        return checkReport;
    }

    private boolean isOnline(List<CheckResult<Object>> results) {
        return results.stream()
            .anyMatch(checkResult -> checkResult.getResults().getCode() == DatabaseConstants.SUCCESS_CODE);
    }

    private void updateEnvStatus(String resourceId, String status) {
        ProtectedEnvironment resource = new ProtectedEnvironment();
        resource.setUuid(resourceId);
        resource.setLinkStatus(status);
        // 直接将环境的状态同步到数据库，不走update的重接口，避免两次检查连通性而导致同步数据库失败
        resourceService.updateSourceDirectly(Stream.of(resource).collect(Collectors.toList()));
        log.info("End to update Redis ENV status after connection check, resource id: {}", resourceId);
    }
}