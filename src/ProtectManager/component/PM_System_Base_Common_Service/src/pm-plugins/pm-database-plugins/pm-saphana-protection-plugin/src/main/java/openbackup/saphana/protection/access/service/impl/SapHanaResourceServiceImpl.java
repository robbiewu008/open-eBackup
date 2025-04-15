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
package openbackup.saphana.protection.access.service.impl;

import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceBase;
import openbackup.data.protection.access.provider.sdk.resource.ResourceCheckContext;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConnectionCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.enums.NodeType;
import openbackup.saphana.protection.access.constant.SapHanaConstants;
import openbackup.saphana.protection.access.constant.SapHanaErrorCode;
import openbackup.saphana.protection.access.service.SapHanaResourceService;
import openbackup.saphana.protection.access.util.SapHanaUtil;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.constants.IsmNumberConstant;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.JSONArray;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;
import openbackup.system.base.util.StreamUtil;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;
import java.util.stream.Stream;

/**
 * SAP HANA资源Service接口实现类
 *
 */
@Service
@Slf4j
public class SapHanaResourceServiceImpl implements SapHanaResourceService {
    private final ResourceConnectionCheckProvider connectionCheckProvider;

    private final ProtectedEnvironmentService environmentService;

    private final ResourceService resourceService;

    public SapHanaResourceServiceImpl(ResourceConnectionCheckProvider connectionCheckProvider,
        ProtectedEnvironmentService environmentService, ResourceService resourceService) {
        this.connectionCheckProvider = connectionCheckProvider;
        this.environmentService = environmentService;
        this.resourceService = resourceService;
    }

    @Override
    public List<ProtectedEnvironment> queryEnvironments(List<ProtectedResource> agents) {
        return agents.stream()
            .map(host -> environmentService.getEnvironmentById(host.getUuid()))
            .collect(Collectors.toList());
    }

    @Override
    public List<ProtectedResource> listResourcesByConditions(Map<String, Object> conditions) {
        int pageNo = IsmNumberConstant.ZERO;
        PageListResponse<ProtectedResource> queryResult;
        List<ProtectedResource> protectedResources = new ArrayList<>();
        do {
            // 分页查询一次查200条
            queryResult = resourceService.query(pageNo, IsmNumberConstant.TWO_HUNDRED, conditions);
            protectedResources.addAll(queryResult.getRecords());
            pageNo++;
        } while (queryResult.getTotalCount() > pageNo * IsmNumberConstant.TWO_HUNDRED);
        return protectedResources;
    }

    @Override
    public ProtectedResource getResourceById(String resourceId) {
        return resourceService.getResourceById(resourceId)
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "Protected resource not exist."));
    }

    @Override
    public void checkInstanceNumber() {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.RESOURCE_TYPE, ResourceTypeEnum.DATABASE.getType());
        conditions.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.SAPHANA_INSTANCE.getType());
        List<ProtectedResource> protectedResources = listResourcesByConditions(conditions);
        if (protectedResources.size() >= SapHanaConstants.SAP_HANA_INSTANCE_MAX_COUNT) {
            log.error("The number of sap hana instances exceeds the limit ({}).",
                SapHanaConstants.SAP_HANA_INSTANCE_MAX_COUNT);
            throw new LegoCheckedException(CommonErrorCode.NUMBER_LIMIT,
                new String[] {String.valueOf(SapHanaConstants.SAP_HANA_INSTANCE_MAX_COUNT)},
                "The number of sap hana instances exceeds the limit");
        }
    }

    @Override
    public void checkInstanceIsRegistered(ProtectedEnvironment environment) {
        String systemId = environment.getExtendInfo().get(SapHanaConstants.SYSTEM_ID);
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.RESOURCE_TYPE, ResourceTypeEnum.DATABASE.getType());
        conditions.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.SAPHANA_INSTANCE.getType());
        conditions.put(SapHanaConstants.SYSTEM_ID, systemId);
        // 根据subType和systemId查询SAP HANA实例
        List<ProtectedResource> protectedResources = listResourcesByConditions(conditions);
        if (protectedResources.isEmpty()) {
            return;
        }
        List<String> agentIds = environment.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .map(ResourceBase::getUuid)
            .collect(Collectors.toList());
        for (ProtectedResource tmpResource : protectedResources) {
            String tmpNodesInfo = tmpResource.getExtendInfoByKey(SapHanaConstants.NODES);
            if (StringUtils.isEmpty(tmpNodesInfo)) {
                log.warn("The nodes parameter of sap hana instance is empty, instance uuid: {}.",
                    tmpResource.getUuid());
                continue;
            }
            JSONArray tmpNodesJsonArray = JSONArray.fromObject(tmpNodesInfo);
            List<ProtectedEnvironment> tmpNodes = JSONArray.toCollection(tmpNodesJsonArray, ProtectedEnvironment.class);
            List<String> tmpNodesIds = tmpNodes.stream()
                .map(ProtectedEnvironment::getUuid)
                .collect(Collectors.toList());
            // 存在重复使用的主机，则提示实例已注册
            if (!CollectionUtils.intersection(tmpNodesIds, agentIds).isEmpty()) {
                log.error("This sap hana instance is registered. name: {}", environment.getName());
                throw new LegoCheckedException(SapHanaErrorCode.RESOURCE_IS_REGISTERED,
                    "This sap hana instance is registered.");
            }
        }
    }

    @Override
    public void checkDbIsRegistered(ProtectedResource resource) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.RESOURCE_TYPE, ResourceTypeEnum.DATABASE.getType());
        conditions.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.SAPHANA_DATABASE.getType());
        conditions.put(DatabaseConstants.PARENT_UUID, resource.getParentUuid());
        conditions.put(DatabaseConstants.NAME, resource.getName());
        // 根据subType、所属实例资源的UUID、数据库名称查询SAP HANA数据库
        List<ProtectedResource> protectedResources = listResourcesByConditions(conditions);
        if (!protectedResources.isEmpty()) {
            log.error("This sap hana database is registered, name: {}, instance resource id: {}.", resource.getName(),
                resource.getParentUuid());
            throw new LegoCheckedException(SapHanaErrorCode.RESOURCE_IS_REGISTERED,
                "This sap hana database is registered.");
        }
    }

    @Override
    public void checkDbNameBeforeUpdate(ProtectedResource resource) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.RESOURCE_TYPE, ResourceTypeEnum.DATABASE.getType());
        conditions.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.SAPHANA_DATABASE.getType());
        conditions.put(DatabaseConstants.PARENT_UUID, resource.getParentUuid());
        conditions.put(DatabaseConstants.NAME, resource.getName());
        // 根据subType、所属实例资源的UUID、数据库名称查询SAP HANA数据库
        List<ProtectedResource> protectedResources = listResourcesByConditions(conditions);
        conditions.put(DatabaseConstants.UUID, resource.getUuid());
        List<ProtectedResource> protectedResourcesTwo = listResourcesByConditions(conditions);
        if (!protectedResources.isEmpty() && protectedResourcesTwo.isEmpty()) {
            log.error("This sap hana database name is duplicated, name: {}, instance resource id: {}.",
                resource.getName(),
                resource.getParentUuid());
            throw new LegoCheckedException(SapHanaErrorCode.RESOURCE_IS_REGISTERED,
                "This sap hana database is registered.");
        }
    }

    @Override
    public void checkDbIsRegisteredInGeneralDb(ProtectedResource resource) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.RESOURCE_TYPE, ResourceTypeEnum.DATABASE.getType());
        conditions.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.GENERAL_DB.getType());
        conditions.put(SapHanaConstants.GENERAL_DB_TYPE_DISPLAY_KEY, SapHanaConstants.GENERAL_DB_TYPE_SAP_HANA);
        List<ProtectedResource> protectedResources = listResourcesByConditions(conditions);
        if (protectedResources.isEmpty()) {
            return;
        }
        String dbName = resource.getName();
        String curSystemId = resource.getExtendInfoByKey(SapHanaConstants.SYSTEM_ID);
        List<String> agentIds = resource.getDependencies()
            .get(DatabaseConstants.AGENTS)
            .stream()
            .flatMap(StreamUtil.match(ProtectedResource.class))
            .map(ResourceBase::getUuid)
            .collect(Collectors.toList());
        for (ProtectedResource tmpResource : protectedResources) {
            if (!dbName.toUpperCase(Locale.ROOT).equals(tmpResource.getName().toUpperCase(Locale.ROOT))) {
                continue;
            }
            String customParams = tmpResource.getExtendInfoByKey(SapHanaConstants.GENERAL_DB_EXT_CUSTOM_PARAMS);
            String tmpSystemId = SapHanaUtil.getSystemIdFromCustomParams(customParams);
            // system id不一致则忽略
            if (!curSystemId.toLowerCase(Locale.ROOT).equals(tmpSystemId)) {
                log.debug("The system id: {} of sap hana database in general databases is different with current: {}, "
                    + "registered resource uuid: {}.", tmpSystemId, curSystemId, tmpResource.getUuid());
                continue;
            }
            String relatedHostIdStr = tmpResource.getExtendInfoByKey(SapHanaConstants.GENERAL_DB_EXT_RELATED_HOST_IDS);
            if (StringUtils.isEmpty(relatedHostIdStr)) {
                log.warn("The relatedHostIds of sap hana general database is empty, registered resource name: {}, "
                    + "uuid: {}.", tmpResource.getName(), tmpResource.getUuid());
                continue;
            }
            List<String> tmpResourceAgentIds = Arrays.asList(relatedHostIdStr.split(","));
            // 存在重复使用的主机，则提示实例已注册
            if (!CollectionUtils.intersection(tmpResourceAgentIds, agentIds).isEmpty()) {
                log.error("This sap hana database is registered in general databases, registered resource name: {}, "
                    + "uuid: {}.", tmpResource.getName(), tmpResource.getUuid());
                throw new LegoCheckedException(SapHanaErrorCode.RESOURCE_IS_REGISTERED,
                    "This sap hana database is registered in general databases.");
            }
        }
    }

    @Override
    public void updateInstAndDbLinkStatusByInst(ProtectedEnvironment environment, String linkStatus,
        boolean shouldUpdateSystemDb, boolean shouldUpdateTenantDb) {
        updateInstanceLinkStatus(environment, linkStatus);
        updateDbLinkStatusOfInstance(environment, linkStatus, shouldUpdateSystemDb, shouldUpdateTenantDb);
    }

    @Override
    public void updateInstanceLinkStatus(ProtectedEnvironment environment, String linkStatus) {
        ProtectedEnvironment newEnv = new ProtectedEnvironment();
        newEnv.setUuid(environment.getUuid());
        newEnv.setLinkStatus(linkStatus);
        resourceService.updateSourceDirectly(Stream.of(newEnv).collect(Collectors.toList()));
    }

    @Override
    public void updateDbLinkStatusOfInstance(ProtectedEnvironment environment, String linkStatus,
        boolean shouldUpdateSystemDb, boolean shouldUpdateTenantDb) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.PARENT_UUID, environment.getUuid());
        conditions.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.SAPHANA_DATABASE.getType());
        List<ProtectedResource> dbResources = listResourcesByConditions(conditions);
        if (dbResources.isEmpty()) {
            return;
        }
        for (ProtectedResource tmpDbResource : dbResources) {
            if (SapHanaUtil.isSystemDatabase(tmpDbResource)) {
                if (shouldUpdateSystemDb) {
                    updateDbLinkStatus(tmpDbResource, linkStatus);
                    continue;
                }
                continue;
            }
            if (shouldUpdateTenantDb) {
                updateDbLinkStatus(tmpDbResource, linkStatus);
            }
        }
    }

    @Override
    public void updateDbLinkStatus(ProtectedResource resource, String linkStatus) {
        ProtectedResource newResource = new ProtectedResource();
        newResource.setUuid(resource.getUuid());
        ProtectedResource updatedResource = SapHanaUtil.setDatabaseResourceLinkStatus(newResource, linkStatus);
        resourceService.updateSourceDirectly(Stream.of(updatedResource).collect(Collectors.toList()));
    }

    @Override
    public void checkAndUpdateTenantDbLinkStatusOfInstance(ProtectedEnvironment environment) {
        Map<String, Object> conditions = new HashMap<>();
        conditions.put(DatabaseConstants.PARENT_UUID, environment.getUuid());
        conditions.put(DatabaseConstants.SUB_TYPE, ResourceSubTypeEnum.SAPHANA_DATABASE.getType());
        List<ProtectedResource> dbResources = listResourcesByConditions(conditions);
        if (dbResources.isEmpty()) {
            return;
        }
        for (ProtectedResource tmpDbResource : dbResources) {
            if (SapHanaUtil.isSystemDatabase(tmpDbResource)) {
                continue;
            }
            // 租户数据库有一个离线则离线
            if (!checkAllNodeOnlineForTenantDb(tmpDbResource)) {
                updateDbLinkStatus(tmpDbResource, LinkStatusEnum.OFFLINE.getStatus().toString());
            }
        }
    }

    private boolean checkAllNodeOnlineForTenantDb(ProtectedResource resource) {
        List<ProtectedResource> dbNodeResList = SapHanaUtil.parseDbHostProtectedResourceList(resource);
        List<ProtectedEnvironment> currNodeEnvList = queryEnvironments(dbNodeResList);
        return currNodeEnvList.stream()
            .allMatch(env -> LinkStatusEnum.ONLINE.getStatus().toString().equals(env.getLinkStatus()));
    }

    @Override
    public String getInstStatusByActionResults(ProtectedEnvironment environment, List<ActionResult> actionResults) {
        if (VerifyUtil.isEmpty(actionResults)) {
            log.error("The sap hana instance check connection result is empty, uuid: {}", environment.getUuid());
            return LinkStatusEnum.OFFLINE.getStatus().toString();
        }
        List<ProtectedEnvironment> nodes = SapHanaUtil.parseInstanceHostEnvList(environment);
        if (nodes.size() == IsmNumberConstant.ONE) {
            // 单机
            ActionResult actionResult = actionResults.get(IsmNumberConstant.ZERO);
            if (actionResult.getCode() != DatabaseConstants.SUCCESS_CODE) {
                log.warn("The sap hana single instance check connection failed, uuid: {}, actionResults: {}.",
                    environment.getUuid(), JSONObject.stringify(actionResults));
                return LinkStatusEnum.OFFLINE.getStatus().toString();
            }
            SapHanaUtil.setNodeRole(nodes.get(0), NodeType.MASTER.getNodeType());
        } else {
            // 集群
            boolean isOnline = false;
            for (int i = 0; i < actionResults.size(); i++) {
                ActionResult actionResult = actionResults.get(i);
                if (actionResult.getCode() != DatabaseConstants.SUCCESS_CODE) {
                    log.warn("The sap hana cluster instance check connection failed, uuid: {}, actionResults: {}.",
                        environment.getUuid(), JSONObject.stringify(actionResults));
                    continue;
                }
                SapHanaUtil.setNodeRole(nodes.get(i),
                    SapHanaUtil.getValueFromActionResultByKey(actionResult, DatabaseConstants.ROLE));
                isOnline = true;
            }
            if (!isOnline) {
                log.warn("All sap hana cluster instance hosts check connection failed, uuid: {}, actionResults: {}.",
                    environment.getUuid(), JSONObject.stringify(actionResults));
                return LinkStatusEnum.OFFLINE.getStatus().toString();
            }
        }
        ProtectedEnvironment newEnv = new ProtectedEnvironment();
        newEnv.setUuid(environment.getUuid());
        ProtectedEnvironment updatedEnv = SapHanaUtil.setInstanceEnvExtendInfoNodes(newEnv, nodes);
        resourceService.updateSourceDirectly(Stream.of(updatedEnv).collect(Collectors.toList()));
        return LinkStatusEnum.ONLINE.getStatus().toString();
    }

    @Override
    public void checkDatabaseConnection(ProtectedResource resource) {
        ResourceCheckContext context = connectionCheckProvider.tryCheckConnection(resource);
        List<ActionResult> actionResults = context.getActionResults();
        if (VerifyUtil.isEmpty(actionResults)) {
            log.error("The sap hana database check connection result is empty. name: {}", resource.getName());
            throw new LegoCheckedException(CommonErrorCode.ERR_PARAM, "check database connection result is empty.");
        }
        List<ProtectedResource> agents = resource.getDependencies().get(DatabaseConstants.AGENTS);
        if (agents.size() == IsmNumberConstant.ONE) {
            // 单节点
            ActionResult actionResult = actionResults.get(IsmNumberConstant.ZERO);
            if (actionResult.getCode() != DatabaseConstants.SUCCESS_CODE) {
                log.error("The sap hana database check connection failed, name: {}, uuid: {}, all actionResults: {}.",
                    resource.getName(), resource.getUuid(), JSONObject.stringify(actionResults));
                String[] params = Optional.ofNullable(actionResult.getDetailParams())
                    .map(e -> e.toArray(new String[0]))
                    .orElse(new String[0]);
                throw new LegoCheckedException(Long.parseLong(actionResult.getBodyErr()), params,
                    actionResult.getMessage());
            }
            return;
        }
        checkDatabaseConnectionOfCluster(resource, actionResults);
    }

    private void checkDatabaseConnectionOfCluster(ProtectedResource resource, List<ActionResult> actionResults) {
        // 多节点
        if (SapHanaUtil.isSystemDatabase(resource)) {
            // 系统数据库有一个节点在线则在线
            boolean isOnline = false;
            ActionResult lastActionResult = new ActionResult(CommonErrorCode.AGENT_NETWORK_ERROR,
                "check cluster system database connection failed.");
            for (ActionResult actionResult : actionResults) {
                if (actionResult.getCode() == DatabaseConstants.SUCCESS_CODE) {
                    isOnline = true;
                    break;
                }
                lastActionResult = actionResult;
            }
            if (!isOnline) {
                log.error("The sap hana cluster system database check connection failed, name: {}, uuid: {}, "
                        + "all actionResults: {}.", resource.getName(), resource.getUuid(),
                    JSONObject.stringify(actionResults));
                String[] params = Optional.ofNullable(lastActionResult.getDetailParams())
                    .map(e -> e.toArray(new String[0]))
                    .orElse(new String[0]);
                throw new LegoCheckedException(Long.parseLong(lastActionResult.getBodyErr()), params,
                    lastActionResult.getMessage());
            }
        } else {
            // 租户数据库所有节点在线才在线
            for (ActionResult actionResult : actionResults) {
                if (actionResult.getCode() != DatabaseConstants.SUCCESS_CODE) {
                    log.error("The sap hana cluster tenant database check connection failed, name: {}, uuid: {}, "
                            + "all actionResults: {}.", resource.getName(), resource.getUuid(),
                        JSONObject.stringify(actionResults));
                    String[] params = Optional.ofNullable(actionResult.getDetailParams())
                        .map(e -> e.toArray(new String[0]))
                        .orElse(new String[0]);
                    throw new LegoCheckedException(Long.parseLong(actionResult.getBodyErr()), params,
                        actionResult.getMessage());
                }
            }
        }
    }

    @Override
    public boolean isModifyResource(ProtectedResource resource) {
        return SapHanaConstants.MODIFY_OPERATION_TYPE.equals(
            resource.getExtendInfoByKey(SapHanaConstants.OPERATION_TYPE));
    }

    @Override
    public void setDatabaseResourceInfo(ProtectedResource resource) {
        String instanceUuid = resource.getParentUuid();
        ProtectedEnvironment instanceEnv = environmentService.getEnvironmentById(instanceUuid);
        // 设置systemId
        resource.setExtendInfoByKey(SapHanaConstants.SYSTEM_ID,
            instanceEnv.getExtendInfoByKey(SapHanaConstants.SYSTEM_ID));
        List<ProtectedResource> oriAgentResList;
        if (SapHanaUtil.isSystemDatabase(resource)) {
            // 系统数据库需要设置认证信息为实例的认证信息
            resource.setAuth(instanceEnv.getAuth());
            resource.setExtendInfoByKey(SapHanaConstants.SYSTEM_DB_PORT,
                instanceEnv.getExtendInfoByKey(SapHanaConstants.SYSTEM_DB_PORT));
            // 系统数据库agents信息取实例的agents
            String tmpNodesInfo = instanceEnv.getExtendInfoByKey(SapHanaConstants.NODES);
            if (StringUtils.isEmpty(tmpNodesInfo)) {
                log.warn("The nodes parameter of sap hana instance is empty, instance uuid: {}.", instanceUuid);
                throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM,
                    "The nodes parameter of sap hana instance is empty.");
            }
            JSONArray tmpNodesJsonArray = JSONArray.fromObject(tmpNodesInfo);
            oriAgentResList = JSONArray.toCollection(tmpNodesJsonArray, ProtectedResource.class);
        } else {
            oriAgentResList = getAgentResourceListForTenantDb(resource);
        }
        List<ProtectedEnvironment> agentEnvList = queryEnvironments(oriAgentResList);
        List<ProtectedResource> agentResList = agentEnvList.stream()
            .map(env -> BeanTools.copy(env, ProtectedResource::new))
            .collect(Collectors.toList());
        Map<String, List<ProtectedResource>> dependencies = Optional.ofNullable(resource.getDependencies())
            .orElse(new HashMap<>());
        dependencies.put(DatabaseConstants.AGENTS, agentResList);
        // 设置节点信息
        resource.setExtendInfoByKey(SapHanaConstants.NODES, JSONObject.stringify(agentEnvList));
        resource.setExtendInfoByKey(SapHanaConstants.INSTANCE_SYSTEM_DB_PORT,
            instanceEnv.getExtendInfoByKey(SapHanaConstants.SYSTEM_DB_PORT));
        resource.setEnvironment(instanceEnv);
        Authentication dbAuth = buildInstanceAuth(resource, instanceEnv);
        resource.setAuth(dbAuth);
        String hanaDbType = Optional.ofNullable(resource.getExtendInfoByKey(SapHanaConstants.SAP_HANA_DB_TYPE))
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "The sapHanaDbType parameter is empty."));
        resource.setExtendInfoByKey(SapHanaConstants.SAP_HANA_DB_DEPLOY_TYPE,
                SapHanaUtil.getDeployType(oriAgentResList.size(), hanaDbType));
    }

    private List<ProtectedResource> getAgentResourceListForTenantDb(ProtectedResource resource) {
        List<ProtectedResource> oriAgentResList;
        Map<String, List<ProtectedResource>> dependencies = resource.getDependencies();
        // 定时扫描时，没有dependencies信息
        if (VerifyUtil.isEmpty(dependencies) || !dependencies.containsKey(DatabaseConstants.AGENTS)) {
            log.info("The sap hana tenant database(uuid={}) has no agents dependencies.", resource.getUuid());
            oriAgentResList = SapHanaUtil.parseDbHostProtectedResourceList(resource);
        } else {
            oriAgentResList = resource.getDependencies().get(DatabaseConstants.AGENTS);
        }
        if (VerifyUtil.isEmpty(oriAgentResList)) {
            throw new LegoCheckedException(CommonErrorCode.ILLEGAL_PARAM, "The depend agents is null");
        }
        return oriAgentResList;
    }

    private Authentication buildInstanceAuth(ProtectedResource resource, ProtectedEnvironment instanceEnv) {
        Authentication dbAuth = resource.getAuth();
        Map<String, String> dbAuthExtInfo = Optional.ofNullable(dbAuth.getExtendInfo()).orElseGet(HashMap::new);
        Authentication instAuth = instanceEnv.getAuth();
        dbAuthExtInfo.put(SapHanaConstants.INSTANCE_AUTH, JSONObject.stringify(instAuth));
        dbAuth.setExtendInfo(dbAuthExtInfo);
        return dbAuth;
    }
}
