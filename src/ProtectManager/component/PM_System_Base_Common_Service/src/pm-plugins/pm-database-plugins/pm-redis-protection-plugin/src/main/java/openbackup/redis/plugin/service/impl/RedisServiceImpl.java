/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.redis.plugin.service.impl;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironmentService;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceConstants;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.database.base.plugin.common.DatabaseConstants;
import openbackup.database.base.plugin.utils.AuthParamUtil;
import com.huawei.oceanprotect.kms.sdk.EncryptorService;
import openbackup.redis.plugin.constant.RedisConstant;
import openbackup.redis.plugin.provider.RedisAgentProvider;
import openbackup.redis.plugin.service.RedisService;
import openbackup.redis.plugin.util.RedisValidator;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import com.huawei.oceanprotect.system.base.kerberos.service.KerberosService;
import openbackup.system.base.sdk.cluster.enums.ClusterEnum;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.BeanTools;

import com.google.common.collect.ImmutableMap;
import com.google.common.collect.Lists;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.MapUtils;
import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Service;

import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * The RedisServiceImpl
 *
 * @author x30028756
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-14
 */
@Slf4j
@Service
@AllArgsConstructor
public class RedisServiceImpl implements RedisService {
    private final ResourceService resourceService;

    private final AgentUnifiedService agentUnifiedService;

    private final KerberosService kerberosService;

    private final EncryptorService encryptorService;

    private final ProtectedEnvironmentService environmentService;

    private final RedisAgentProvider redisAgentProvider;

    @Override
    public void preCheck(ProtectedEnvironment protectedEnvironment) {
        List<ProtectedResource> children = protectedEnvironment.getDependencies().get(ResourceConstants.CHILDREN);
        List<String> childrenUuidList = children.stream()
            .map(ProtectedResource::getUuid)
            .filter(StringUtils::isNotEmpty)
            .collect(Collectors.toList());
        // 从已注册的集群中找出一个在线的节点
        List<ProtectedResource> allRegisteredNodes = null;
        ProtectedResource firstOnlineChild = null;
        if (CollectionUtils.isNotEmpty(childrenUuidList)) {
            allRegisteredNodes = resourceService.query(0, CollectionUtils.size(childrenUuidList),
                ImmutableMap.of("uuid", childrenUuidList)).getRecords();
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
        ProtectedEnvironment firstHost = null;
        for (int i = 0; i < children.size(); i++) {
            ProtectedResource child = children.get(i);
            RedisValidator.checkNode(child);
            // 调公共框架的查询接口校验节点是否已添加过
            checkNodeExists(child);
            AuthParamUtil.convertKerberosAuth(child.getAuth(), kerberosService,
                child.getAuth().getExtendInfo().get(DatabaseConstants.EXTEND_INFO_KEY_KERBEROS_ID),
                encryptorService);
            ProtectedEnvironment host = environmentService.getEnvironmentById(selectAgent(child).getId());
            // 用一个在线的节点获取版本号信息
            if ((CollectionUtils.isEmpty(childrenUuidList) && i == 0)
                || !Objects.isNull(firstOnlineChild) && StringUtils.equals(firstOnlineChild.getUuid(),
                child.getUuid())) {
                firstHost = host;
                version = getVersion(child, host);
                userId = firstHost.getUserId();
                // 版本号配置文件也能校验，报的统一的错误码，但是版本号有专门的错误码；
                if (Integer.parseInt(version.split("\\.")[0]) < RedisConstant.REDIS_SUPPORT_VERSION) {
                    throw new LegoCheckedException(CommonErrorCode.VERSION_ERROR, "Version id is error.");
                }
            }
            setChildExtendInfo(allRegisteredNodes, child);
        }
        setEnvironmentAttribute(protectedEnvironment, children, version, userId, firstHost);
        if (shouldCheckClusterNodesConsistent(childrenUuidList, protectedEnvironment.getUuid(), children)) {
            checkClusterNodesConsistentAndAddExtendInfo(firstOnlineChild, children, firstHost);
        }
        // 敏感信息最后一次使用结束后未在内存中移除
        AuthParamUtil.removeSensitiveInfo(children);
    }

    @Override
    public Endpoint selectAgent(ProtectedResource child) {
        AgentSelectParam agentSelectParam = AgentSelectParam.builder()
            .resource(child)
            .jobType(JobTypeEnum.RESOURCE_SCAN.getValue())
            .build();

        return redisAgentProvider.getSelectedAgents(agentSelectParam)
            .stream()
            .findFirst()
            .orElseThrow(
                () -> new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "not find any agent can connect"));
    }

    private boolean shouldCheckClusterNodesConsistent(List<String> childrenUuidList, String environmentUuid,
        List<ProtectedResource> children) {
        if (CollectionUtils.isEmpty(childrenUuidList)) {
            return true;
        }
        // 根据parentUuid去查
        ProtectedResource databaseCluster = resourceService.getResourceById(false, environmentUuid)
            .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.OBJ_NOT_EXIST, "resource is not exist"));
        List<ProtectedResource> databaseNodeList = databaseCluster.getDependencies().get(ResourceConstants.CHILDREN);
        if (CollectionUtils.size(databaseNodeList) != CollectionUtils.size(childrenUuidList)
            || CollectionUtils.size(databaseNodeList) != CollectionUtils.size(children)) {
            return true;
        }
        // 校验主机、客户端安装路径、业务IP、端口是否改变
        for (ProtectedResource databaseNode : databaseNodeList) {
            Optional<ProtectedResource> optionalChild = children.stream()
                .filter(item -> StringUtils.equals(databaseNode.getUuid(), item.getUuid()))
                .findFirst();
            if (!optionalChild.isPresent()) {
                return true;
            } else {
                ProtectedResource child = optionalChild.get();
                if (!StringUtils.equals(child.getDependencies().get(DatabaseConstants.AGENTS).get(0).getUuid(),
                    databaseNode.getDependencies().get(DatabaseConstants.AGENTS).get(0).getUuid())) {
                    return true;
                }
                Map<String, String> childExtendInfo = child.getExtendInfo();
                Map<String, String> resourceExtendInfo = databaseNode.getExtendInfo();
                if (!StringUtils.equals(MapUtils.getString(childExtendInfo, RedisConstant.CLIENT_PATH),
                    MapUtils.getString(resourceExtendInfo, RedisConstant.CLIENT_PATH)) || !StringUtils.equals(
                    MapUtils.getString(childExtendInfo, RedisConstant.IP),
                    MapUtils.getString(resourceExtendInfo, RedisConstant.IP)) || !StringUtils.equals(
                    MapUtils.getString(childExtendInfo, RedisConstant.PORT),
                    MapUtils.getString(resourceExtendInfo, RedisConstant.PORT))) {
                    return true;
                }
            }
        }
        return false;
    }

    private String getVersion(ProtectedResource child, ProtectedEnvironment host) {
        PageListResponse<ProtectedResource> response = agentUnifiedService.getDetailPageListNoRetry(
            ResourceSubTypeEnum.REDIS.getType(), host.getEndpoint(), host.getPort(),
            generateListResourceV2Req(host, child), false);
        if (Objects.isNull(response) || response.getTotalCount() < 1) {
            throw new LegoCheckedException(CommonErrorCode.REDIS_CONNECT_FAILED, "Redis node connect failed.");
        }
        return MapUtils.getString(response.getRecords().get(0).getExtendInfo(), RedisConstant.EXTEND_INFO_KEY_VERSION);
    }

    private void setEnvironmentAttribute(ProtectedEnvironment protectedEnvironment, List<ProtectedResource> children,
        String version, String userId, ProtectedEnvironment firstHost) {
        if (!Objects.isNull(firstHost)) {
            // 设置集群的endpoint
            protectedEnvironment.setEndpoint(firstHost.getEndpoint());
            // 检查完成后，版本信息放到集群信息中
            protectedEnvironment.setVersion(version);
            protectedEnvironment.setCluster(true);
            protectedEnvironment.setPath(protectedEnvironment.getName());
            protectedEnvironment.setAuthorizedUser(firstHost.getAuthorizedUser());
            protectedEnvironment.setUserId(userId);
            for (ProtectedResource child : children) {
                child.setAuthorizedUser(firstHost.getAuthorizedUser());
                child.setUserId(userId);
                child.setVersion(version);
            }
        }
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
                if (StringUtils.equals(MapUtils.getString(databaseChildExtendInfo, RedisConstant.IP),
                    MapUtils.getString(currentChildExtendInfo, RedisConstant.IP)) && StringUtils.equals(
                    MapUtils.getString(databaseChildExtendInfo, RedisConstant.PORT),
                    MapUtils.getString(currentChildExtendInfo, RedisConstant.PORT)) && StringUtils.equals(
                    MapUtils.getString(databaseChildExtendInfo, DatabaseConstants.EXTEND_INFO_KEY_KERBEROS_ID),
                    MapUtils.getString(currentChildExtendInfo, DatabaseConstants.EXTEND_INFO_KEY_KERBEROS_ID))
                    && StringUtils.equals(
                    MapUtils.getString(databaseChildExtendInfo, DatabaseConstants.EXTEND_INFO_KEY_SSL_ENABLE),
                    MapUtils.getString(currentChildExtendInfo, DatabaseConstants.EXTEND_INFO_KEY_SSL_ENABLE))) {
                    currentChildExtendInfo.putAll(databaseChildExtendInfo);
                }
            }
        } else {
            child.getExtendInfo()
                .put(DatabaseConstants.STATUS, String.valueOf(ClusterEnum.StatusEnum.ONLINE.getStatus()));
        }
    }

    /**
     * 生成ListResourceV2Req
     *
     * @param environment environment
     * @param child child
     * @return ListResourceV2Req ListResourceV2Req
     */
    public static ListResourceV2Req generateListResourceV2Req(ProtectedEnvironment environment,
        ProtectedResource child) {
        ListResourceV2Req listResourceV2Req = new ListResourceV2Req();
        listResourceV2Req.setAppEnv(BeanTools.copy(environment, AppEnv::new));
        listResourceV2Req.setPageSize(1);
        listResourceV2Req.setPageNo(0);
        listResourceV2Req.setApplications(Lists.newArrayList(BeanTools.copy(child, Application::new)));
        return listResourceV2Req;
    }

    @Override
    public void checkNodeExists(ProtectedResource protectedResource) {
        // Uuid不为空并且ip和端口号相同说明是已添加过的节点，不用检查
        if (!StringUtils.isEmpty(protectedResource.getUuid())) {
            // 先根据id查
            PageListResponse<ProtectedResource> queryResponseById = resourceService.query(0, 1,
                ImmutableMap.of(RedisConstant.UUID, protectedResource.getUuid()));
            Map<String, String> databaseChildExtendInfo = queryResponseById.getRecords().get(0).getExtendInfo();
            Map<String, String> currentChildExtendInfo = protectedResource.getExtendInfo();
            if (StringUtils.equals(MapUtils.getString(databaseChildExtendInfo, RedisConstant.IP),
                MapUtils.getString(currentChildExtendInfo, RedisConstant.IP)) && StringUtils.equals(
                MapUtils.getString(databaseChildExtendInfo, RedisConstant.PORT),
                MapUtils.getString(currentChildExtendInfo, RedisConstant.PORT))) {
                return;
            }
        }
        Map<String, String> extendInfo = protectedResource.getExtendInfo();
        PageListResponse<ProtectedResource> response = resourceService.query(0, 1,
            ImmutableMap.of(RedisConstant.IP, MapUtils.getString(extendInfo, RedisConstant.IP), RedisConstant.PORT,
                MapUtils.getString(extendInfo, RedisConstant.PORT)));

        if (!response.getRecords().isEmpty()) {
            throw new LegoCheckedException(CommonErrorCode.NODE_REPEAT, "The node already exists.");
        }
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
        AppEnvResponse response = agentUnifiedService.getClusterInfo(chosenNode,
            Objects.isNull(host) ? environmentService.getEnvironmentById(selectAgent(chosenNode).getId()) : host);
        if (Objects.isNull(response) || CollectionUtils.isEmpty(response.getNodes())) {
            throw new LegoCheckedException(CommonErrorCode.REDIS_CONNECT_FAILED, "Redis node connect failed.");
        }
        List<ProtectedResource> cluster = response.getNodes()
            .stream()
            .map(item -> BeanTools.copy(item, ProtectedResource::new))
            .collect(Collectors.toList());

        // 校验集群节点是不是在一个集群
        if (!StringUtils.equals(getIpAndPort(cluster).stream().sorted().collect(Collectors.joining()),
            getIpAndPort(nodeList).stream().sorted().collect(Collectors.joining()))) {
            throw new LegoCheckedException(CommonErrorCode.CLUSTER_NODES_INCONSISTENT,
                "Cluster nodes are inconsistent.");
        }

        // 更新集群信息，以实际为准
        nodeList.forEach(node -> response.getNodes().forEach(nodeInfo -> {
            if (StringUtils.equals(MapUtils.getString(node.getExtendInfo(), RedisConstant.IP),
                MapUtils.getString(nodeInfo.getExtendInfo(), RedisConstant.IP)) && StringUtils.equals(
                MapUtils.getString(node.getExtendInfo(), RedisConstant.PORT),
                MapUtils.getString(nodeInfo.getExtendInfo(), RedisConstant.PORT))) {
                node.getExtendInfo().putAll(nodeInfo.getExtendInfo());
            }
        }));
    }

    private List<String> getIpAndPort(List<ProtectedResource> nodeList) {
        return nodeList.stream()
            .map(ProtectedResource::getExtendInfo)
            .map(
                item -> MapUtils.getString(item, RedisConstant.IP) + "_" + MapUtils.getString(item, RedisConstant.PORT))
            .collect(Collectors.toList());
    }
}