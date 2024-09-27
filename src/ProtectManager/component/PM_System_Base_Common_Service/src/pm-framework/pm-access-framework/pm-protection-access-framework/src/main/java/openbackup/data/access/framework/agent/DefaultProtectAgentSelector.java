/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 */

package openbackup.data.access.framework.agent;

import com.huawei.oceanprotect.base.cluster.sdk.service.MemberClusterService;
import com.huawei.oceanprotect.system.base.user.bo.UserDomainRelationBo;
import com.huawei.oceanprotect.system.base.user.service.DomainResourceSetService;
import com.huawei.oceanprotect.system.base.user.service.UserDomainRelationService;

import lombok.extern.slf4j.Slf4j;
import openbackup.access.framework.resource.persistence.dao.ProtectedResourceAgentMapper;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.protection.common.constants.AgentKeyConstant;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceBase;
import openbackup.data.protection.access.provider.sdk.resource.ResourceQueryParams;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.resource.model.AgentTypeEnum;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.auth.UserInnerResponse;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.user.enums.ResourceSetTypeEnum;
import openbackup.system.base.service.DeployTypeService;
import openbackup.system.base.util.OpServiceUtil;

import org.apache.commons.lang3.StringUtils;
import org.springframework.beans.factory.annotation.Autowired;
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
 * 默认的保护代理选择器，根据设置选择代理，或自动选择
 *
 * @author j00364432
 * @version [OceanProtect A8000 1.1.0]
 * @since 2021-11-30
 */
@Slf4j
@Component
public class DefaultProtectAgentSelector implements ProtectAgentSelector {
    // 内置agent的key
    private static final String INTERNAL_AGENT_KEY = "scenario";

    private static final String INTERNAL_AGENT_ESN = "internal_agent_esn";

    private static final int SIZE = 10000;

    private static final String PLUGIN = "Plugin";

    private static final List<String> HCS_SUB_TYPE_LIST = Arrays.asList(ResourceSubTypeEnum.HCS_CONTAINER.getType(),
        ResourceSubTypeEnum.HCS_TENANT.getType(), ResourceSubTypeEnum.HCS_REGION.getType(),
        ResourceSubTypeEnum.HCS_PROJECT.getType(), ResourceSubTypeEnum.HCS_CLOUD_HOST.getType());

    private final ResourceService resourceService;

    private final MemberClusterService memberClusterService;

    @Autowired
    private ProtectedResourceAgentMapper protectedResourceAgentMapper;

    @Autowired
    private DeployTypeService deployTypeService;

    @Autowired
    private DomainResourceSetService domainResourceSetService;

    @Autowired
    private UserDomainRelationService userDomainRelationService;

    public DefaultProtectAgentSelector(ResourceService resourceService, MemberClusterService memberClusterService) {
        this.resourceService = resourceService;
        this.memberClusterService = memberClusterService;
    }

    @Override
    public List<Endpoint> select(ProtectedResource protectedResource, Map<String, String> parameters) {
        String agents = parameters.get(AgentKeyConstant.AGENTS_KEY);
        String userStr = parameters.get(AgentKeyConstant.USER_INFO);
        UserInnerResponse userInfo = JSONObject.cast(userStr, UserInnerResponse.class, false);
        log.info("Job exec userId: {}.", Optional.ofNullable(userInfo).map(UserInnerResponse::getUserId).orElse(null));
        List<Endpoint> endpoints = VerifyUtil.isEmpty(agents)
                ? selectByResourceType(protectedResource, userInfo)
                : selectByAgentParameter(agents, userInfo);
        log.info("Job param agents: {}, finally find the agents: {}.", agents,
            endpoints.stream().map(Endpoint::getId).collect(Collectors.joining(",")));
        return endpoints;
    }

    /**
     * 根据下发参数选择可用agent
     *
     * @param agents 下发参数中的agent
     * @param userInnerResponse 任务用户信息
     * @return 可用的代理主机
     */
    public List<Endpoint> selectByAgentParameter(String agents, UserInnerResponse userInnerResponse) {
        return Arrays.stream(agents.split(";"))
            .map(id -> findAgentByUuid(id, userInnerResponse))
            .filter(Optional::isPresent)
            .map(Optional::get)
            .collect(Collectors.toList());
    }

    /**
     * 自动选择子类型和保护资源的子类型一致且主类型为插件的资源的父资源，父资源即为保护代理
     *
     * @param protectedResource 受保护资源{@link ProtectedResource}
     * @param userInnerResponse 用户信息{@link UserInnerResponse}
     * @return 保护代理列表
     */
    private List<Endpoint> selectByResourceType(ProtectedResource protectedResource,
        UserInnerResponse userInnerResponse) {
        Map<String, Object> filter = new HashMap<>();
        filter.put("type", PLUGIN);
        filter.put("subType", protectedResource.getSubType() + PLUGIN);
        if (HCS_SUB_TYPE_LIST.contains(protectedResource.getSubType())) {
            // 为了适配hcs升级，以前插件写的HCScontainer
            filter.put("subType", "HCScontainer" + PLUGIN);
        }
        List<String> parentUuids = queryResourcesByFilter(filter, SIZE).stream()
            .map(ResourceBase::getParentUuid)
            .filter(Objects::nonNull)
            .distinct()
            .collect(Collectors.toList());
        if (VerifyUtil.isEmpty(parentUuids)) {
            return Collections.emptyList();
        }

        Map<String, Object> uuidMap = new HashMap<>();
        uuidMap.put("uuid", parentUuids);
        List<ProtectedResource> protectedResources = queryResourcesByFilter(uuidMap, parentUuids.size());
        return protectedResources.stream()
            .map(e -> {
                if (HCS_SUB_TYPE_LIST.contains(protectedResource.getSubType()) || deployTypeService.isCyberEngine()) {
                    // HCS场景
                    if (OpServiceUtil.isHcsService() && ResourceSubTypeEnum.HCS_CONTAINER.getType().equals(
                        protectedResource.getEnvironment().getSubType())) {
                        // 如果root节点的资源类型是HCSContainer（即线下场景），并且是HCS OP服务，则只使用外置代理。注：线上场景是HcsEnvOp
                        return findExternalAgentByResource(e, userInnerResponse);
                    } else {
                        return findAllAgentByResource(e, userInnerResponse);
                    }
                }

                // 其他应用默认只下发外置代理
                return findExternalAgentByResource(e, userInnerResponse);
            })
            .filter(Optional::isPresent).map(Optional::get).collect(Collectors.toList());
    }

    private List<ProtectedResource> queryResourcesByFilter(Map<String, Object> filter, int size) {
        ResourceQueryParams queryParams = new ResourceQueryParams();
        queryParams.setPage(0);
        queryParams.setSize(size);
        queryParams.setConditions(filter);
        queryParams.setShouldDecrypt(false);
        queryParams.setShouldLoadEnvironment(false);
        return resourceService.query(queryParams).getRecords();
    }

    private Optional<Endpoint> findAllAgentByResource(ProtectedResource protectedResource,
        UserInnerResponse userInnerResponse) {
        List<String> sharedAgentIds = protectedResourceAgentMapper.querySharedAgentIds();
        return Optional.ofNullable(protectedResource)
                .filter(resource -> checkResourceOwnership(resource, userInnerResponse, sharedAgentIds))
                .filter(resource -> checkResourceInternalAgent(resource))
                .filter(resource -> resource instanceof ProtectedEnvironment)
                .map(resource -> (ProtectedEnvironment) resource)
                .filter(env -> LinkStatusEnum.ONLINE.getStatus()
                        .toString()
                        .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(env)))
                .map(env -> new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort(), env.getOsType()));
    }

    private Optional<Endpoint> findExternalAgentByResource(ProtectedResource protectedResource,
        UserInnerResponse userInnerResponse) {
        List<String> sharedAgentIds = protectedResourceAgentMapper.querySharedAgentIds();
        return Optional.ofNullable(protectedResource)
            .filter(resource -> checkResourceOwnership(resource, userInnerResponse, sharedAgentIds))
            .filter(resource -> AgentTypeEnum.EXTERNAL_AGENT.getValue()
                        .equals(resource.getExtendInfo().get(INTERNAL_AGENT_KEY)))
            .filter(resource -> resource instanceof ProtectedEnvironment)
            .map(resource -> (ProtectedEnvironment) resource)
            .filter(env -> LinkStatusEnum.ONLINE.getStatus()
                .toString()
                .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(env)))
            .map(env -> new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort(), env.getOsType()));
    }

    private Optional<Endpoint> findAgentByUuid(String resUuid, UserInnerResponse userInnerResponse) {
        List<String> sharedAgentIds = protectedResourceAgentMapper.querySharedAgentIds();
        return resourceService.getResourceById(false, resUuid)
            .filter(resource -> checkResourceOwnership(resource, userInnerResponse, sharedAgentIds))
            .filter(resource -> checkResourceInternalAgent(resource))
            .filter(resource -> resource instanceof ProtectedEnvironment)
            .map(resource -> (ProtectedEnvironment) resource)
            .filter(env -> LinkStatusEnum.ONLINE.getStatus()
                .toString()
                .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(env)))
            .map(env -> new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort(), env.getOsType()));
    }

    private boolean checkResourceInternalAgent(ProtectedResource resource) {
        boolean isInternalAgent = AgentTypeEnum.INTERNAL_AGENT.getValue()
            .equals(resource.getExtendInfo().get(INTERNAL_AGENT_KEY));
        if (!isInternalAgent || !memberClusterService.clusterEstablished()) {
            log.debug("Agent is internalAgent:{}, uuid:{}, or not cluster", isInternalAgent, resource.getUuid());
            return true;
        }
        return memberClusterService.getCurrentClusterEsn().equals(resource.getExtendInfo().get(INTERNAL_AGENT_ESN));
    }

    private boolean checkResourceOwnership(ProtectedResource resource, UserInnerResponse userInnerResponse,
        List<String> sharedAgentIds) {
        if (userInnerResponse == null || StringUtils.isEmpty(userInnerResponse.getUserId())) {
            return true;
        }
        boolean isAdmin = userInnerResponse.getRolesSet()
            .stream()
            .noneMatch(roleInfo -> Constants.Builtin.ROLE_DP_ADMIN.equals(roleInfo.getRoleName()));
        if (isAdmin) {
            log.debug("Current job owner is admin, resource is used with permission, resource ID: {}.",
                resource.getUuid());
            return true;
        }
        if (AgentTypeEnum.INTERNAL_AGENT.getValue().equals(resource.getExtendInfo().get(INTERNAL_AGENT_KEY))
            || sharedAgentIds.contains(resource.getUuid())) {
            log.info("Resource: {} is internal or shared agent.", resource.getUuid());
            return true;
        }
        log.info("Check if the user(ID: {}) has permission to use the resource(ID: {}).", userInnerResponse.getUserId(),
            resource.getUserId());
        Optional<UserDomainRelationBo> relation = userDomainRelationService.queryRelationByUserId(
            userInnerResponse.getUserId());
        if (!relation.isPresent()) {
            return false;
        }
        return !VerifyUtil.isEmpty(domainResourceSetService.getDomainResourcesRelation(relation.get().getDomainId(),
            resource.getUuid(), ResourceSetTypeEnum.AGENT.getType()));
    }

    @Override
    public boolean applicable(String object) {
        return false;
    }
}
