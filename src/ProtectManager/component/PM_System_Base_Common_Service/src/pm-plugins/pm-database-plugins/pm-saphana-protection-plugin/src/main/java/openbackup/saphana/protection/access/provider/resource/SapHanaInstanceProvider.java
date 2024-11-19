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
package openbackup.saphana.protection.access.provider.resource;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.NodeType;
import openbackup.database.base.plugin.provider.DatabaseEnvironmentProvider;
import openbackup.database.base.plugin.service.ClusterEnvironmentService;
import openbackup.saphana.protection.access.constant.SapHanaConstants;
import openbackup.saphana.protection.access.service.SapHanaResourceService;
import openbackup.saphana.protection.access.util.SapHanaUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.util.stream.Collectors;

/**
 * SAP HANA实例Provider
 *
 */
@Component
@Slf4j
public class SapHanaInstanceProvider extends DatabaseEnvironmentProvider {
    private final ResourceConnectionCheckProvider connectionCheckProvider;

    private final ClusterEnvironmentService clusterEnvironmentService;

    private final SapHanaResourceService hanaResourceService;

    /**
     * SapHanaInstanceProvider构造方法
     *
     * @param providerManager Provider管理
     * @param pluginConfigManager 插件配置管理
     * @param connectionCheckProvider 连通检查Provider
     * @param clusterEnvironmentService 集群环境业务类
     * @param hanaResourceService SAP HANA资源业务类
     */
    public SapHanaInstanceProvider(ProviderManager providerManager, PluginConfigManager pluginConfigManager,
        ResourceConnectionCheckProvider connectionCheckProvider, ClusterEnvironmentService clusterEnvironmentService,
        SapHanaResourceService hanaResourceService) {
        super(providerManager, pluginConfigManager);
        this.connectionCheckProvider = connectionCheckProvider;
        this.clusterEnvironmentService = clusterEnvironmentService;
        this.hanaResourceService = hanaResourceService;
    }

    @Override
    public boolean applicable(String resourceSubType) {
        return ResourceSubTypeEnum.SAPHANA_INSTANCE.equalsSubType(resourceSubType);
    }

    @Override
    public void register(ProtectedEnvironment environment) {
        log.info("Start check sap hana instance, name: {}, uuid: {}.", environment.getName(), environment.getUuid());
        SapHanaUtil.checkEnvironmentExtendInfoParam(environment);
        List<ProtectedResource> agents = Optional.ofNullable(
            environment.getDependencies().get(DatabaseConstants.AGENTS))
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "The depend agents is null"));
        List<ProtectedEnvironment> agentList = hanaResourceService.queryEnvironments(agents);
        clusterEnvironmentService.checkClusterNodeStatus(agentList);
        clusterEnvironmentService.checkClusterNodeOsType(agentList);
        setInstanceEndpoint(environment, agentList);
        SapHanaUtil.setSystemId(environment);
        boolean isCreate = VerifyUtil.isEmpty(environment.getUuid());
        String checkResult;
        // 注册场景
        if (isCreate) {
            hanaResourceService.checkInstanceNumber();
            hanaResourceService.checkInstanceIsRegistered(environment);
            environment.setUuid(UUIDGenerator.getUUID());
            checkResult = checkInstanceConnection(environment);
        } else {
            // 修改场景
            SapHanaUtil.setOperationTypeExtendInfo(environment, SapHanaConstants.MODIFY_OPERATION_TYPE);
            try {
                checkResult = checkInstanceConnection(environment);
            } catch (LegoCheckedException e) {
                log.error("Modify instance environment failed");
                ProtectedResource oriResource = hanaResourceService.getResourceById(environment.getUuid());
                if (oriResource instanceof ProtectedEnvironment) {
                    log.info("Will re-modify to original instance environment.");
                    ProtectedEnvironment oriEnvironment = (ProtectedEnvironment) oriResource;
                    checkInstanceConnection(oriEnvironment);
                }
                throw e;
            }
        }
        setSapHanaInstance(environment, checkResult);
        SapHanaUtil.removeExtendInfoByKey(environment, SapHanaConstants.OPERATION_TYPE);
        environment.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
        log.info("End check SAP HANA instance, name: {}, uuid: {}.", environment.getName(), environment.getUuid());
    }

    private void setInstanceEndpoint(ProtectedEnvironment environment, List<ProtectedEnvironment> agentList) {
        List<String> endpoints = agentList.stream().map(ProtectedEnvironment::getEndpoint).collect(Collectors.toList());
        String endpointStr = String.join(",", endpoints);
        environment.setEndpoint(String.join(",", endpointStr));
        environment.setPath(endpointStr);
    }

    private String checkInstanceConnection(ProtectedEnvironment environment) {
        // 注册时设置nodes扩展参数
        List<ProtectedResource> agents = environment.getDependencies().get(DatabaseConstants.AGENTS);
        List<ProtectedEnvironment> nodes = hanaResourceService.queryEnvironments(agents);
        environment.setExtendInfoByKey(SapHanaConstants.NODES, JSONObject.stringify(nodes));
        ResourceCheckContext context = connectionCheckProvider.tryCheckConnection(environment);
        List<ActionResult> actionResults = context.getActionResults();
        if (VerifyUtil.isEmpty(actionResults)) {
            log.error("The sap hana instance check connection result is empty. name: {}", environment.getName());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "check instance connection result is empty.");
        }
        log.info("The sap hana instance check connection result: {}, nodes num: {}, name: {}.",
            JSONObject.stringify(actionResults), nodes.size(), environment.getName());
        ActionResult lastActionResult = new ActionResult(CommonErrorCode.AGENT_NETWORK_ERROR,
            "check connection failed.");
        if (nodes.size() == IsmNumberConstant.ONE) {
            // 单机
            ActionResult actionResult = actionResults.get(IsmNumberConstant.ZERO);
            if (actionResult.getCode() != DatabaseConstants.SUCCESS_CODE) {
                log.error("The sap hana single instance check connection failed, name: {}.", environment.getName());
                String[] params = Optional.ofNullable(actionResult.getDetailParams())
                    .map(e -> e.toArray(new String[0]))
                    .orElse(new String[0]);
                throw new LegoCheckedException(Long.parseLong(actionResult.getBodyErr()), params,
                    actionResult.getMessage());
            }
            SapHanaUtil.setNodeRole(nodes.get(0), NodeType.MASTER.getNodeType());
            lastActionResult = actionResult;
        } else {
            // 集群
            for (int i = 0; i < actionResults.size(); i++) {
                ActionResult actionResult = actionResults.get(i);
                if (actionResult.getCode() != DatabaseConstants.SUCCESS_CODE) {
                    log.error("The sap hana cluster instance check connection failed, ip: {}, name: {}.",
                        nodes.get(i).getEndpoint(), environment.getName());
                    String[] params = Optional.ofNullable(actionResult.getDetailParams())
                        .map(e -> e.toArray(new String[0]))
                        .orElse(new String[0]);
                    throw new LegoCheckedException(Long.parseLong(actionResult.getBodyErr()), params,
                        actionResult.getMessage());
                }
                SapHanaUtil.setNodeRole(nodes.get(i),
                    SapHanaUtil.getValueFromActionResultByKey(actionResult, DatabaseConstants.ROLE));
                lastActionResult = actionResult;
            }
        }
        // 设置节点的主备信息
        environment.setExtendInfoByKey(SapHanaConstants.NODES, JSONObject.stringify(nodes));
        return lastActionResult.getMessage();
    }

    private void setSapHanaInstance(ProtectedResource resource, String checkResult) {
        Map<String, String> messageMap = JSONObject.fromObject(checkResult).toMap(String.class);
        resource.setVersion(messageMap.get(DatabaseConstants.VERSION));
        resource.setExtendInfoByKey(SapHanaConstants.LANDSCAPE_ID, messageMap.get(SapHanaConstants.LANDSCAPE_ID));
    }

    private boolean checkHasOnlineNode(ProtectedEnvironment environment) {
        List<ProtectedResource> instanceNodeList = SapHanaUtil.parseInstanceHostResourceList(environment);
        List<ProtectedEnvironment> currentNodeList = hanaResourceService.queryEnvironments(instanceNodeList);
        return currentNodeList.stream()
            .anyMatch(env -> LinkStatusEnum.ONLINE.getStatus().toString().equals(env.getLinkStatus()));
    }

    /**
     * 健康检查后，返回状态信息：检查节点状态
     *
     * @param environment 集群
     * @return 状态信息
     */
    @Override
    public Optional<String> healthCheckWithResultStatus(ProtectedEnvironment environment) {
        String oriStatus = environment.getLinkStatus();
        log.info("Start health check sap hana instance, instance uuid: {}, origin status: {}.", environment.getUuid(),
            oriStatus);
        // 检查所有主机状态，主机全部离线则实例离线
        boolean isOnline = checkHasOnlineNode(environment);
        if (!isOnline) {
            log.warn("Health check sap hana instance hosts are offline, instance uuid: {}.", environment.getUuid());
            hanaResourceService.updateDbLinkStatusOfInstance(environment, LinkStatusEnum.OFFLINE.getStatus().toString(),
                true, true);
            return Optional.of(LinkStatusEnum.OFFLINE.getStatus().toString());
        }
        // 如果实例原状态是离线，当前有在线节点，需检查连通性
        if (LinkStatusEnum.OFFLINE.getStatus().toString().equals(oriStatus)) {
            String instStatus = healthCheckInstanceConnection(environment);
            if (LinkStatusEnum.OFFLINE.getStatus().toString().equals(instStatus)) {
                log.warn("Health check sap hana instance is offline, instance uuid: {}.", environment.getUuid());
                hanaResourceService.updateDbLinkStatusOfInstance(environment,
                    LinkStatusEnum.OFFLINE.getStatus().toString(), true, true);
                return Optional.of(LinkStatusEnum.OFFLINE.getStatus().toString());
            }
            log.info("Health check sap hana instance is online, instance uuid: {}.", environment.getUuid());
            hanaResourceService.updateDbLinkStatusOfInstance(environment, LinkStatusEnum.ONLINE.getStatus().toString(),
                true, false);
            healthCheckTenantDatabaseConnection(environment);
            return Optional.of(LinkStatusEnum.ONLINE.getStatus().toString());
        }
        // 实例原状态是在线，且存在在线的节点时，单独检查租户数据库主机状态
        hanaResourceService.checkAndUpdateTenantDbLinkStatusOfInstance(environment);
        log.info("End health check sap hana instance, instance uuid: {}.", environment.getUuid());
        return Optional.of(oriStatus);
    }

    /**
     * 扫描受保护环境：不扫描所属数据库资源，只定时检查实例和包含数据库的连通性并更新状态
     *
     * @param environment 受保护环境
     * @return 受保护环境中的资源列表
     */
    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        log.info("Start scan sap hana instance, uuid: {}.", environment.getUuid());
        List<ProtectedResource> resourceList = new ArrayList<>();
        SapHanaUtil.checkResourceExtendInfoParam(environment);
        // 检查所有主机状态，有一个在线则实例在线
        boolean isOnline = checkHasOnlineNode(environment);
        if (!isOnline) {
            log.warn("Scan sap hana instance hosts are offline, uuid: {}.", environment.getUuid());
            hanaResourceService.updateInstAndDbLinkStatusByInst(environment,
                LinkStatusEnum.OFFLINE.getStatus().toString(), true, true);
            return resourceList;
        }
        String instStatus = healthCheckInstanceConnection(environment);
        if (LinkStatusEnum.OFFLINE.getStatus().toString().equals(instStatus)) {
            // 实例离线，系统数据库离线，租户数据库连通检查
            log.warn("Scan sap hana instance is offline, uuid: {}.", environment.getUuid());
            hanaResourceService.updateInstAndDbLinkStatusByInst(environment,
                LinkStatusEnum.OFFLINE.getStatus().toString(), true, true);
            return resourceList;
        }
        // 实例在线，更新实例、系统数据库状态为在线，租户数据库做连通检查
        hanaResourceService.updateInstanceLinkStatus(environment, LinkStatusEnum.ONLINE.getStatus().toString());
        hanaResourceService.updateDbLinkStatusOfInstance(environment, LinkStatusEnum.ONLINE.getStatus().toString(),
            true, false);
        healthCheckTenantDatabaseConnection(environment);
        log.info("End scan sap hana instance, uuid: {}.", environment.getUuid());
        return resourceList;
    }

    private String healthCheckInstanceConnection(ProtectedEnvironment environment) {
        // 设置健康检查的标记
        SapHanaUtil.setOperationTypeExtendInfo(environment, SapHanaConstants.TEST_CONNECT_OPERATION_TYPE);
        ResourceCheckContext context = connectionCheckProvider.tryCheckConnection(environment);
        List<ActionResult> actionResults = context.getActionResults();
        return hanaResourceService.getInstStatusByActionResults(environment, actionResults);
    }

    private void healthCheckTenantDatabaseConnection(ProtectedEnvironment environment) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.PARENT_UUID, environment.getUuid());
        conditions.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.SAPHANA_DATABASE.getType());
        List<ProtectedResource> dbResources = hanaResourceService.listResourcesByConditions(conditions);
        if (dbResources.isEmpty()) {
            return;
        }
        // 对实例所包含的数据库进行健康检查
        List<ProtectedResource> tenantDbResources = new ArrayList<>();
        for (ProtectedResource tmpDbResource : dbResources) {
            // 系统数据库不处理
            if (SapHanaUtil.isSystemDatabase(tmpDbResource)) {
                continue;
            }
            // 设置健康检查的标记
            SapHanaUtil.setOperationTypeExtendInfo(tmpDbResource, SapHanaConstants.TEST_CONNECT_OPERATION_TYPE);
            tenantDbResources.add(tmpDbResource);
        }
        List<String> tenantDbIdList = tenantDbResources.stream()
            .map(ProtectedResource::getUuid)
            .collect(Collectors.toList());
        log.info("Scan sap hana tenant databases: {}, instance uuid: {}.", JSONObject.stringify(tenantDbIdList),
            environment.getUuid());
        if (tenantDbResources.isEmpty()) {
            return;
        }
        int taskNum = Math.min(tenantDbResources.size(), SapHanaConstants.THREAD_POOL_NUM);
        ExecutorService executorService = new ThreadPoolExecutor(taskNum, SapHanaConstants.THREAD_POOL_NUM,
            SapHanaConstants.CHECK_DB_TIMEOUT, TimeUnit.SECONDS,
            new LinkedBlockingQueue<>(SapHanaConstants.WORK_QUEUE_SIZE));
        for (ProtectedResource tmpTenantDbResource : tenantDbResources) {
            executorService.submit(() -> hanaResourceService.checkDatabaseConnection(tmpTenantDbResource));
        }
    }
}
