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
package openbackup.clickhouse.plugin.service.impl;

import openbackup.clickhouse.plugin.constant.ClickHouseConstant;
import openbackup.clickhouse.plugin.provider.ClickHouseAgentProvider;
import openbackup.clickhouse.plugin.service.ClickHouseService;
import openbackup.clickhouse.plugin.util.ClickHouseValidator;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceBase;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.utils.AuthParamUtil;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;

import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import com.huawei.oceanprotect.system.base.kerberos.service.KerberosService;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;
import openbackup.system.base.util.BeanTools;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;
import com.google.common.collect.Maps;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.MapUtils;
import org.apache.commons.lang3.StringUtils;
import org.apache.logging.log4j.util.Strings;
import org.springframework.stereotype.Component;

import java.nio.charset.StandardCharsets;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.Set;
import java.util.UUID;
import java.util.stream.Collectors;

/**
 * ClickHouse服务实现类
 *
 */
@Slf4j
@Component
@AllArgsConstructor
public class ClickHouseServiceImpl implements ClickHouseService {
    private final ResourceService resourceService;

    private final AgentUnifiedService agentUnifiedService;

    private final ProtectedEnvironmentService protectedEnvironmentService;

    private final KerberosService kerberosService;

    private final EncryptorService encryptorService;

    private final ClickHouseAgentProvider clickHouseAgentProvider;

    @Override
    public void preCheck(ProtectedEnvironment environment) {
        List<ProtectedResource> children = environment.getDependencies().get(ResourceConstants.CHILDREN);
        List<String> childrenUuids = children.stream()
            .map(ProtectedResource::getUuid)
            .filter(StringUtils::isNotEmpty)
            .collect(Collectors.toList());
        // 从已注册的集群中找出一个在线的节点
        List<ProtectedResource> allRegisteredNodes = null;
        ProtectedResource firstOnlineChild = null;
        if (CollectionUtils.isNotEmpty(childrenUuids)) {
            allRegisteredNodes = resourceService.query(0, CollectionUtils.size(childrenUuids),
                ImmutableMap.of("uuid", childrenUuids)).getRecords();
            Optional<ProtectedResource> firstOnlineChildOptional = allRegisteredNodes.stream()
                .filter(item -> ClusterEnum.StatusEnum.ONLINE.getStatus() == MapUtils.getIntValue(item.getExtendInfo(),
                    DatabaseConstants.STATUS))
                .findFirst();
            if (firstOnlineChildOptional.isPresent()) {
                firstOnlineChild = firstOnlineChildOptional.get();
            }
        }

        String version = null;
        String userId = null;
        String authorizedUser = null;
        ProtectedEnvironment firstHost = null;
        for (ProtectedResource child : children) {
            ClickHouseValidator.checkNode(child);
            checkNodeExists(child);
            AuthParamUtil.convertKerberosAuth(child.getAuth(), kerberosService,
                child.getAuth().getExtendInfo().get(DatabaseConstants.EXTEND_INFO_KEY_KERBEROS_ID),
                encryptorService);

            if (CollectionUtils.isEmpty(childrenUuids) && children.indexOf(child) == 0
                || !Objects.isNull(firstOnlineChild) && StringUtils.equals(firstOnlineChild.getUuid(),
                child.getUuid())) {
                ProtectedEnvironment agent = protectedEnvironmentService.getEnvironmentById(selectAgent(child).getId());
                firstHost = agent;
                version = getVersion(child, agent);
                userId = agent.getUserId();
                authorizedUser = agent.getAuthorizedUser();
            }
            setChildExtendInfo(allRegisteredNodes, child);
        }
        setChildAttribute(children, version, userId, authorizedUser);
        setEnvironmentAttribute(environment, version, userId, authorizedUser, firstHost);
        if (shouldCheckClusterNodesConsistent(childrenUuids, environment.getUuid(), children)) {
            checkClusterNodesConsistentAndAddExtendInfo(firstOnlineChild, children, firstHost);
        }
        // 敏感信息最后一次使用结束后未在内存中移除
        AuthParamUtil.removeSensitiveInfo(children);
    }

    private void setChildAttribute(List<ProtectedResource> children, String version, String userId,
        String authorizedUser) {
        for (ProtectedResource child : children) {
            child.setUserId(userId);
            child.setVersion(version);
            child.setAuthorizedUser(authorizedUser);
        }
    }

    private void setEnvironmentAttribute(ProtectedEnvironment protectedEnvironment, String version, String userId,
        String authorizedUser, ProtectedEnvironment agent) {
        if (!Objects.isNull(agent)) {
            protectedEnvironment.setVersion(version);
            protectedEnvironment.setCluster(true);
            protectedEnvironment.setEndpoint(agent.getEndpoint());
            protectedEnvironment.setUserId(userId);
            protectedEnvironment.setAuthorizedUser(authorizedUser);
            protectedEnvironment.setPath(protectedEnvironment.getName());
        }
    }

    private String getVersion(ProtectedResource child, ProtectedEnvironment agent) {
        String version;
        List<ProtectedResource> versionInfos = queryClusterDetail(agent, child,
            ClickHouseConstant.QUERY_TYPE_VALUE_VERSION, null, null).getRecords();
        if (CollectionUtils.isEmpty(versionInfos)) {
            log.error("ClickHouse node connect failed, node: {}", child.getName());
            throw new LegoCheckedException(ClickHouseConstant.CLICK_HOUSE_CONNECT_FAILED,
                "ClickHouse node connect failed.");
        }
        version = versionInfos.get(0).getExtendInfo().get(ClickHouseConstant.VERSION);
        checkClusterVersion(version);
        return version;
    }

    /**
     * 校验用户输入节点与实际集群的节点的一致性，以及添加扩展信息
     *
     * @param firstOnlineChild 第一个在线的节点
     * @param nodeList 集群的节点
     * @param host 集群
     */
    private void checkClusterNodesConsistentAndAddExtendInfo(ProtectedResource firstOnlineChild,
        List<ProtectedResource> nodeList, ProtectedEnvironment host) {
        ProtectedResource chosenNode = Objects.isNull(firstOnlineChild) ? nodeList.get(0) : firstOnlineChild;
        AuthParamUtil.convertKerberosAuth(chosenNode.getAuth(), kerberosService,
            chosenNode.getAuth().getExtendInfo().get(DatabaseConstants.EXTEND_INFO_KEY_KERBEROS_ID),
            encryptorService);
        AppEnvResponse response = agentUnifiedService.getClusterInfoNoRetry(chosenNode, Objects.isNull(host)
            ? protectedEnvironmentService.getEnvironmentById(selectAgent(chosenNode).getId())
            : host);
        if (Objects.isNull(response) || CollectionUtils.isEmpty(response.getNodes())) {
            throw new LegoCheckedException(ClickHouseConstant.CLICK_HOUSE_CONNECT_FAILED,
                "ClickHouse node connect failed.");
        }
        List<ProtectedResource> cluster = response.getNodes()
            .stream()
            .map(item -> BeanTools.copy(item, ProtectedResource::new))
            .collect(Collectors.toList());

        // 校验集群节点是不是在一个集群
        if (!StringUtils.equals(
            getIp(cluster, ClickHouseConstant.HOST_ADDRESS).stream().sorted().collect(Collectors.joining()),
            getIp(nodeList, ClickHouseConstant.IP).stream().sorted().collect(Collectors.joining()))) {
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_INCONSISTENT,
                "Cluster nodes are inconsistent.");
        }

        // 更新集群信息，以实际为准
        updateNodeExtendInfo(nodeList, response);
    }

    private boolean shouldCheckClusterNodesConsistent(List<String> childrenUuids, String environmentUuid,
        List<ProtectedResource> children) {
        if (CollectionUtils.isEmpty(childrenUuids)) {
            return true;
        }
        // 根据parentUuid去查
        ProtectedResource databaseCluster = resourceService.getResourceById(false, environmentUuid)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "resource is not exist"));
        List<ProtectedResource> databaseNodes = databaseCluster.getDependencies().get(ResourceConstants.CHILDREN);
        if (CollectionUtils.size(databaseNodes) != CollectionUtils.size(childrenUuids)
            || CollectionUtils.size(databaseNodes) != CollectionUtils.size(children)) {
            return true;
        }
        // 校验主机、客户端安装路径、业务IP、端口是否改变
        for (ProtectedResource databaseNode : databaseNodes) {
            Optional<ProtectedResource> childOptional = children.stream()
                .filter(item -> StringUtils.equals(databaseNode.getUuid(), item.getUuid()))
                .findFirst();
            if (!childOptional.isPresent()) {
                return true;
            } else {
                ProtectedResource child = childOptional.get();
                if (!StringUtils.equals(child.getDependencies().get(DatabaseConstants.AGENTS).get(0).getUuid(),
                    databaseNode.getDependencies().get(DatabaseConstants.AGENTS).get(0).getUuid())) {
                    return true;
                }
                Map<String, String> childExtendInfo = child.getExtendInfo();
                Map<String, String> resourceExtendInfo = databaseNode.getExtendInfo();
                if (!StringUtils.equals(MapUtils.getString(childExtendInfo, ClickHouseConstant.CLIENT_PATH),
                    MapUtils.getString(resourceExtendInfo, ClickHouseConstant.CLIENT_PATH)) || !StringUtils.equals(
                    MapUtils.getString(childExtendInfo, ClickHouseConstant.IP),
                    MapUtils.getString(resourceExtendInfo, ClickHouseConstant.IP)) || !StringUtils.equals(
                    MapUtils.getString(childExtendInfo, ClickHouseConstant.PORT),
                    MapUtils.getString(resourceExtendInfo, ClickHouseConstant.PORT))) {
                    return true;
                }
            }
        }
        return false;
    }

    private void setChildExtendInfo(List<ProtectedResource> allRegisteredNodes, ProtectedResource child) {
        // 设置集群扩展信息
        if (StringUtils.isNotEmpty(child.getUuid())) {
            Optional<ProtectedResource> childOptional = allRegisteredNodes.stream()
                .filter(item -> StringUtils.equals(child.getUuid(), item.getUuid()))
                .findFirst();
            if (childOptional.isPresent()) {
                Map<String, String> databaseChildExtendInfo = childOptional.get().getExtendInfo();
                Map<String, String> currentChildExtendInfo = child.getExtendInfo();
                if (StringUtils.equals(MapUtils.getString(databaseChildExtendInfo, ClickHouseConstant.IP),
                    MapUtils.getString(currentChildExtendInfo, ClickHouseConstant.IP)) && StringUtils.equals(
                    MapUtils.getString(databaseChildExtendInfo, ClickHouseConstant.PORT),
                    MapUtils.getString(currentChildExtendInfo, ClickHouseConstant.PORT)) && StringUtils.equals(
                    MapUtils.getString(databaseChildExtendInfo, DatabaseConstants.EXTEND_INFO_KEY_KERBEROS_ID),
                    MapUtils.getString(currentChildExtendInfo, DatabaseConstants.EXTEND_INFO_KEY_KERBEROS_ID))) {
                    currentChildExtendInfo.putAll(databaseChildExtendInfo);
                }
            }
        } else {
            child.getExtendInfo()
                .put(DatabaseConstants.STATUS, String.valueOf(ClusterEnum.StatusEnum.ONLINE.getStatus()));
        }
    }

    private void checkClusterVersion(String version) {
        // 仅支持ClickHouse 20.1.X.XX及以上版本
        String[] versionSplit = version.split("\\.");
        if (Integer.parseInt(versionSplit[0]) < ClickHouseConstant.CLICK_HOUSE_VERSION_FIRST) {
            // 第一个.前的数小于20，不通过
            throw new LegoCheckedException(CommonErrorCode.VERSION_ERROR, "Version id is error.");
        } else if (Integer.parseInt(versionSplit[0]) == ClickHouseConstant.CLICK_HOUSE_VERSION_FIRST) {
            // 第一个.前的数等于20，需要看第二个.之前的数，是否大于等于1，如果小于1，则不通过
            if (versionSplit.length >= 2 && Integer.getInteger(versionSplit[1]) < 1) {
                throw new LegoCheckedException(CommonErrorCode.VERSION_ERROR, "Version id is error.");
            }
        } else {
            log.debug("version is ok");
        }
    }

    private AppEnvResponse queryClusterInfo(ProtectedEnvironment environment,
        ProtectedResource checkProtectedResource) {
        AppEnvResponse response = agentUnifiedService.getClusterInfoNoRetry(checkProtectedResource, environment);
        if (Objects.isNull(response) || CollectionUtils.isEmpty(response.getNodes())) {
            throw new LegoCheckedException(ClickHouseConstant.CLICK_HOUSE_CONNECT_FAILED,
                "ClickHouse node connect failed.");
        }
        return response;
    }

    private void updateNodeExtendInfo(List<ProtectedResource> nodeList, AppEnvResponse response) {
        // 更新集群信息，以实际为准
        nodeList.forEach(node -> response.getNodes().forEach(nodeInfo -> {
            if (StringUtils.equals(MapUtils.getString(node.getExtendInfo(), ClickHouseConstant.IP),
                MapUtils.getString(nodeInfo.getExtendInfo(), ClickHouseConstant.HOST_ADDRESS))) {
                String port = MapUtils.getString(node.getExtendInfo(), DatabaseConstants.PORT);
                node.getExtendInfo().putAll(nodeInfo.getExtendInfo());
                node.getExtendInfo().put(DatabaseConstants.PORT, port);
            }
        }));
    }

    private List<String> getIp(List<ProtectedResource> nodeList, String ipKey) {
        return nodeList.stream()
            .map(ProtectedResource::getExtendInfo)
            .map(item -> MapUtils.getString(item, ipKey))
            .collect(Collectors.toList());
    }

    @Override
    public void checkNodeExists(ProtectedResource protectedResource) {
        // Uuid不为空并且ip和端口号相同说明是已添加过的节点，不用检查
        if (!StringUtils.isEmpty(protectedResource.getUuid())) {
            // 先根据id查
            PageListResponse<ProtectedResource> queryResponseById = resourceService.query(0, 1,
                ImmutableMap.of(ClickHouseConstant.UUID, protectedResource.getUuid()));
            Map<String, String> databaseChildExtendInfo = queryResponseById.getRecords().get(0).getExtendInfo();
            Map<String, String> currentChildExtendInfo = protectedResource.getExtendInfo();
            if (sameIpAndPort(databaseChildExtendInfo, currentChildExtendInfo)) {
                return;
            }
        }
        // Uuid为空并且根据ip和端口号能查询到节点信息，表面节点信息已注册， 不需要重新注册
        // uuid不为空，且和数据库已存在其他uuid的相同ip和port的节点，不需要重新注册。
        Map<String, String> extendInfo = protectedResource.getExtendInfo();
        PageListResponse<ProtectedResource> response = resourceService.query(0, 1,
            ImmutableMap.of(ClickHouseConstant.IP, MapUtils.getString(extendInfo, ClickHouseConstant.IP),
                ClickHouseConstant.PORT, MapUtils.getString(extendInfo, ClickHouseConstant.PORT)));

        if (!response.getRecords().isEmpty()) {
            throw new LegoCheckedException(CommonErrorCode.RESOURCE_REPEAT, "The node already exists.");
        }
    }

    private static boolean sameIpAndPort(Map<String, String> databaseChildExtendInfo,
        Map<String, String> currentChildExtendInfo) {
        return StringUtils.equals(MapUtils.getString(databaseChildExtendInfo, ClickHouseConstant.IP),
            MapUtils.getString(currentChildExtendInfo, ClickHouseConstant.IP)) && StringUtils.equals(
            MapUtils.getString(databaseChildExtendInfo, ClickHouseConstant.PORT),
            MapUtils.getString(currentChildExtendInfo, ClickHouseConstant.PORT));
    }

    @Override
    public List<ProtectedResource> scanDataBases(ProtectedEnvironment cluster) {
        List<ProtectedResource> nodes = cluster.getDependencies().get(ResourceConstants.CHILDREN);
        if (CollectionUtils.isEmpty(nodes)) {
            return Lists.newArrayList();
        }

        List<Set<String>> databaseResourceNames = Lists.newArrayList();
        nodes.forEach(node -> {
            ProtectedEnvironment agent = protectedEnvironmentService.getEnvironmentById(selectAgent(node).getId());
            databaseResourceNames.add(
                queryClusterDetail(agent, node, ClickHouseConstant.QUERY_TYPE_VALUE_DATABASE, null, null).getRecords()
                    .stream()
                    .map(item -> item.getExtendInfoByKey(ClickHouseConstant.DB_NAME))
                    .collect(Collectors.toSet()));
        });
        Set<String> commonDatabaseResourceNames = databaseResourceNames.stream().reduce((list1, list2) -> {
            list1.retainAll(list2);
            return list1;
        }).orElse(Collections.emptySet());

        return commonDatabaseResourceNames.stream()
            .map(item -> convertToDatabase(cluster, item))
            .collect(Collectors.toList());
    }

    @Override
    public Endpoint selectAgent(ProtectedResource child) {
        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(child)
            .jobType(JobTypeEnum.RESOURCE_SCAN.getValue())
            .build();

        return clickHouseAgentProvider.getSelectedAgents(agentSelectParam)
            .stream()
            .findFirst()
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "not find any agent can connect"));
    }

    @Override
    public PageListResponse<ProtectedResource> browseTables(ProtectedEnvironment cluster,
        BrowseEnvironmentResourceConditions environmentConditions) {
        List<ProtectedResource> nodes = cluster.getDependencies().get(ResourceConstants.CHILDREN);
        if (CollectionUtils.isEmpty(nodes)) {
            return new PageListResponse<>(0, Lists.newArrayList());
        }

        ProtectedResource node = nodes.get(0);
        ProtectedEnvironment agent = protectedEnvironmentService.getEnvironmentById(selectAgent(node).getId());
        Optional<ProtectedResource> databaseResource = resourceService.getResourceById(
            environmentConditions.getParentId());
        if (!databaseResource.isPresent()) {
            return new PageListResponse<>(0, Lists.newArrayList());
        }
        PageListResponse<ProtectedResource> allTableResources = queryClusterDetail(agent, node,
            ClickHouseConstant.QUERY_TYPE_VALUE_TABLE, databaseResource.get().getName(), environmentConditions);
        List<ProtectedResource> tableResources = allTableResources.getRecords();
        // 查出当前库下，已经被添加到表集的所有表，如果已经被添加过了，则透传信息。
        Map<String, ProtectedResource> savedTableResources = querySavedTableResource(databaseResource.get().getUuid());
        List<ProtectedResource> tables = tableResources.stream()
            .map(item -> convertToTable(savedTableResources, cluster, item))
            .collect(Collectors.toList());
        return new PageListResponse<>(allTableResources.getTotalCount(), tables);
    }

    @Override
    public Optional<String> healthCheckWithResultStatus(ProtectedEnvironment environment, boolean isUpdateDB) {
        log.info("start clickhouse health check, cluster uuid: {}", environment.getUuid());
        ActionResult[] results = resourceService.check(environment);
        int status = LinkStatusEnum.ONLINE.getStatus();
        List<ProtectedResource> children = environment.getDependencies().get(ResourceConstants.CHILDREN);
        List<ProtectedResource> updateChildren = Lists.newArrayList();
        if (results.length != children.size()) {
            log.warn("clickhouse cluster: {} healtch check response node not enough", environment.getName());
            return Optional.of(String.valueOf(status));
        }
        int normalCount = children.size();
        for (int i = 0; i < children.size(); i++) {
            ActionResult actionResult = results[i];
            String targetStatus;
            if (Objects.isNull(actionResult) || actionResult.getCode() != DatabaseConstants.SUCCESS_CODE) {
                normalCount--;
                targetStatus = String.valueOf(ClusterEnum.StatusEnum.OFFLINE.getStatus());
                status = LinkStatusEnum.PARTLY_ONLING.getStatus();
            } else {
                targetStatus = String.valueOf(ClusterEnum.StatusEnum.ONLINE.getStatus());
            }
            ProtectedResource child = children.get(i);
            if (isUpdateDB) {
                prepareUpdateData(child, targetStatus, updateChildren);
            } else {
                child.setExtendInfoByKey(DatabaseConstants.STATUS, targetStatus);
            }
        }
        if (isUpdateDB) {
            // 取第一个在线节点，给所有节点更新extendInfo
            Optional<ProtectedResource> firstOnlineNodeOptional = children.stream()
                .filter(node -> StringUtils.equals(String.valueOf(ClusterEnum.StatusEnum.ONLINE.getStatus()),
                    node.getExtendInfo().get(DatabaseConstants.STATUS)))
                .findFirst();
            if (firstOnlineNodeOptional.isPresent()) {
                updateNodeExtendInfo(updateChildren, firstOnlineNodeOptional);
            }
            resourceService.updateSourceDirectly(updateChildren);
        }

        if (normalCount == 0) {
            throw new LegoCheckedException(CommonErrorCode.HOST_OFFLINE, "Protected environment is offLine!");
        }
        return Optional.of(String.valueOf(status));
    }

    private static void prepareUpdateData(ProtectedResource child, String targetStatus,
        List<ProtectedResource> updateChildren) {
        ProtectedResource updateChild = new ProtectedResource();
        updateChild.setUuid(child.getUuid());
        Map<String, String> currentExtendInfo = child.getExtendInfo();
        Map<String, String> childExtendInfo = new HashMap<>();
        childExtendInfo.put(ClickHouseConstant.IP, MapUtils.getString(currentExtendInfo, ClickHouseConstant.IP));
        childExtendInfo.put(DatabaseConstants.PORT, MapUtils.getString(currentExtendInfo, DatabaseConstants.PORT));
        childExtendInfo.put(DatabaseConstants.STATUS, targetStatus);
        updateChild.setExtendInfo(childExtendInfo);
        updateChildren.add(updateChild);
    }

    private void updateNodeExtendInfo(List<ProtectedResource> updateChildren,
        Optional<ProtectedResource> firstOnlineNodeOptional) {
        ProtectedEnvironment agent = protectedEnvironmentService.getEnvironmentById(
            selectAgent(firstOnlineNodeOptional.get()).getId());
        ProtectedResource firstOnlineNode = firstOnlineNodeOptional.get();
        AuthParamUtil.convertKerberosAuth(firstOnlineNode.getAuth(), kerberosService,
            Optional.ofNullable(firstOnlineNode.getAuth())
                .map(Authentication::getExtendInfo)
                .map(item -> item.get(DatabaseConstants.EXTEND_INFO_KEY_KERBEROS_ID))
                .orElse(Strings.EMPTY), encryptorService);
        AppEnvResponse response = queryClusterInfo(agent, firstOnlineNode);
        AuthParamUtil.removeSensitiveInfo(firstOnlineNode);
        updateNodeExtendInfo(updateChildren, response);
    }

    /**
     * 查出当前库下，已经被添加到表集的所有表
     *
     * @param databaseUuid 数据库的UUID
     * @return 该库下所有被添加过的表，key为表名
     */
    private Map<String, ProtectedResource> querySavedTableResource(String databaseUuid) {
        // 数据库和表没有直接关系，需要先通过库查到表集，再通过表集查表
        return queryChildResources(databaseUuid).stream()
            .map(ResourceBase::getUuid)
            .map(this::queryChildResources)
            .flatMap(Collection::stream)
            .collect(Collectors.toMap(ResourceBase::getName, tableResource -> tableResource));
    }

    /**
     * 循环查出父资源下面的所有子资源
     *
     * @param parentUuid 父资源UUID
     * @return 子资源集合
     */
    private List<ProtectedResource> queryChildResources(String parentUuid) {
        Map<String, Object> conditions = Maps.newHashMap();
        conditions.put(ClickHouseConstant.SUB_TYPE, ResourceSubTypeEnum.CLICK_HOUSE.getType());
        conditions.put(ClickHouseConstant.PARENT_UUID, parentUuid);
        int pageNo = 0;
        PageListResponse<ProtectedResource> queryResult;
        List<ProtectedResource> childResources = Lists.newArrayList();
        do {
            queryResult = resourceService.query(pageNo, ClickHouseConstant.QUERY_RESOURCE_PAGE_SIZE, conditions);
            childResources.addAll(queryResult.getRecords());
            pageNo++;
        } while (queryResult.getTotalCount() > pageNo * ClickHouseConstant.QUERY_RESOURCE_PAGE_SIZE);
        return childResources;
    }

    private ProtectedResource convertToTable(Map<String, ProtectedResource> savedTableResources,
        ProtectedEnvironment cluster, ProtectedResource item) {
        String tableName = item.getName();
        if (savedTableResources.containsKey(tableName)) {
            return savedTableResources.get(tableName);
        }

        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setEnvironment(cluster);
        protectedResource.setName(tableName);
        protectedResource.setType(ClickHouseConstant.TABLE_TYPE);
        protectedResource.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        protectedResource.setVersion(cluster.getVersion());
        protectedResource.setRootUuid(cluster.getUuid());
        protectedResource.setPath(StringUtils.defaultIfEmpty(cluster.getName(), tableName));
        protectedResource.setAuthorizedUser(cluster.getAuthorizedUser());
        protectedResource.setUserId(cluster.getUserId());
        protectedResource.setExtendInfo(item.getExtendInfo());
        return protectedResource;
    }

    @Override
    public PageListResponse<ProtectedResource> queryClusterDetail(ProtectedEnvironment environment,
        ProtectedResource resource, String queryType, String queryDatabase,
        BrowseEnvironmentResourceConditions environmentConditions) {
        AuthParamUtil.convertKerberosAuth(resource.getAuth(), kerberosService,
            resource.getExtendInfo().get(DatabaseConstants.EXTEND_INFO_KEY_KERBEROS_ID), encryptorService);
        ListResourceV2Req request = new ListResourceV2Req();
        request.setPageSize(Objects.isNull(environmentConditions) ? 0 : environmentConditions.getPageSize());
        request.setPageNo(Objects.isNull(environmentConditions) ? 0 : environmentConditions.getPageNo());
        request.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        Application application = BeanTools.copy(resource, Application::new);
        application.getExtendInfo().put(ClickHouseConstant.QUERY_DETAIL_QUERY_TYPE, queryType);
        if (StringUtils.isNotBlank(queryDatabase)) {
            application.getExtendInfo().put(ClickHouseConstant.QUERY_DETAIL_QUERY_DATABASE, queryDatabase);
        }
        request.setApplications(Lists.newArrayList(application));
        PageListResponse<ProtectedResource> response = agentUnifiedService.getDetailPageListNoRetry(
            ResourceSubTypeEnum.CLICK_HOUSE.getType(), environment.getEndpoint(), environment.getPort(), request,
            false);
        if (Objects.isNull(response) || response.getTotalCount() < 0) {
            log.error("ClickHouse node connect failed.");
            throw new LegoCheckedException(ClickHouseConstant.CLICK_HOUSE_CONNECT_FAILED,
                "ClickHouse node connect failed.");
        }
        List<ProtectedResource> resources = response.getRecords();
        log.info("Query detail finish, env name: {}, details num: {}, totalCount:{}.", environment.getName(),
            resources.size(), response.getTotalCount());
        return new PageListResponse<>(response.getTotalCount(), resources);
    }

    private ProtectedResource convertToDatabase(ProtectedEnvironment cluster, String databaseName) {
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setEnvironment(cluster);
        protectedResource.setName(databaseName);
        protectedResource.setType(ResourceTypeEnum.DATABASE.getType());
        protectedResource.setSubType(ResourceSubTypeEnum.CLICK_HOUSE.getType());
        protectedResource.setUuid(
            UUID.nameUUIDFromBytes((cluster.getUuid() + databaseName).getBytes(StandardCharsets.UTF_8)).toString());
        protectedResource.setVersion(cluster.getVersion());
        protectedResource.setParentName(cluster.getName());
        protectedResource.setParentUuid(cluster.getUuid());
        protectedResource.setRootUuid(cluster.getUuid());
        protectedResource.setPath(StringUtils.defaultIfEmpty(cluster.getName(), databaseName));
        protectedResource.setAuthorizedUser(cluster.getAuthorizedUser());
        protectedResource.setUserId(cluster.getUserId());
        return protectedResource;
    }
}
