package openbackup.obs.plugin.agent;

import openbackup.access.framework.resource.persistence.dao.ProtectedResourceAgentMapper;
import openbackup.access.framework.resource.service.AgentBusinessService;
import openbackup.data.access.framework.agent.ProtectAgentSelector;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.protection.common.constants.AgentKeyConstant;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.data.protection.access.provider.sdk.resource.model.AgentTypeEnum;
import com.huawei.oceanprotect.repository.tapelibrary.common.util.JsonUtil;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.utils.JSONObject;
import openbackup.system.base.sdk.auth.UserInnerResponse;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.apache.commons.lang3.StringUtils;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.stream.Collectors;

/**
 * 对象备份agent筛选器
 *
 * @author l00370588
 * @since 2024-1-13
 */
@Component
@Slf4j
public class ObjectStorageAgentSelector implements ProtectAgentSelector {
    // 内置agent的key
    private static final String INTERNAL_AGENT_KEY = "scenario";

    private final AgentBusinessService agentBusinessService;

    private final ResourceService resourceService;

    private final ProtectedResourceAgentMapper protectedResourceAgentMapper;

    public ObjectStorageAgentSelector(AgentBusinessService agentBusinessService, ResourceService resourceService,
        ProtectedResourceAgentMapper protectedResourceAgentMapper) {
        this.resourceService = resourceService;
        this.agentBusinessService = agentBusinessService;
        this.protectedResourceAgentMapper = protectedResourceAgentMapper;
    }

    /**
     * 获取agent列表
     *
     * @param protectedResource 资源信息{@link ProtectedResource}
     * @param parameters 任务参数
     * @return agent 列表
     */
    @Override
    public List<Endpoint> select(ProtectedResource protectedResource, Map<String, String> parameters) {
        parameters.put(AgentKeyConstant.USER_INFO, null);
        String originAgents = parameters.get(AgentKeyConstant.AGENTS_KEY);
        if (StringUtils.isEmpty(originAgents)) {
            if (protectedResource.getEnvironment() != null
                && protectedResource.getEnvironment().getExtendInfo() != null) {
                originAgents = protectedResource.getEnvironment().getExtendInfo().get(AgentKeyConstant.AGENTS_KEY);
            }
            // 手动索引框架无agents，需要手动查询
            if (StringUtils.isEmpty(originAgents)) {
                Optional<ProtectedResource> resourceById = resourceService.getResourceById(
                    parameters.get(AgentKeyConstant.ENVIRONMENT_UUID_KEY));
                if (resourceById.isPresent()) {
                    originAgents = resourceById.get().getExtendInfo().get(AgentKeyConstant.AGENTS_KEY);
                }
            }
            parameters.put(AgentKeyConstant.AGENTS_KEY, originAgents);
        }
        // 老版本默认下发内置agent
        getInternalAgents(parameters);
        String agents = parameters.get(AgentKeyConstant.AGENTS_KEY);
        String userStr = parameters.get(AgentKeyConstant.USER_INFO);
        UserInnerResponse userInnerResponse = null;
        if (StringUtils.isNotEmpty(userStr)) {
            userInnerResponse = JSONObject.cast(userStr, UserInnerResponse.class, false);
            log.info("Job exec owner user ID: {}.", userInnerResponse.getUserId());
        }
        List<Endpoint> endpoints = new ArrayList<>();
        if (StringUtils.isNotEmpty(agents)) {
            endpoints = selectByAgentParameter(agents, userInnerResponse);
        }
        log.info("Object Storage agent select endpoints {}", JsonUtil.toJsonString(endpoints));
        return endpoints;
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
        if (StringUtils.isEmpty(resource.getUserId())) {
            log.debug("resource is not authorized to any user, resource ID: {}.", resource.getUuid());
            // 如果是内置代理或共享代理，返回true
            return AgentTypeEnum.INTERNAL_AGENT.getValue().equals(resource.getExtendInfo().get(INTERNAL_AGENT_KEY))
                || sharedAgentIds.contains(resource.getUuid());
        }

        log.info("Check if the user(ID: {}) has permission to use the resource(ID: {}).", userInnerResponse.getUserId(),
            resource.getUserId());
        return userInnerResponse.getUserId().equals(resource.getUserId());
    }

    private List<Endpoint> selectByAgentParameter(String agents, UserInnerResponse userInnerResponse) {
        return Arrays.stream(agents.split(";"))
            .map(id -> findAgentByUuid(id, userInnerResponse))
            .filter(Optional::isPresent)
            .map(Optional::get)
            .collect(Collectors.toList());
    }

    private Optional<Endpoint> findAgentByUuid(String resUuid, UserInnerResponse userInnerResponse) {
        List<String> sharedAgentIds = protectedResourceAgentMapper.querySharedAgentIds();
        return resourceService.getResourceById(false, resUuid)
            .filter(resource -> checkResourceOwnership(resource, userInnerResponse, sharedAgentIds))
            .filter(resource -> resource instanceof ProtectedEnvironment)
            .map(resource -> (ProtectedEnvironment) resource)
            .filter(env -> LinkStatusEnum.ONLINE.getStatus()
                .toString()
                .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(env)))
            .map(env -> new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort(), env.getOsType()));
    }

    private void getInternalAgents(Map<String, String> parameters) {
        if (StringUtils.isEmpty(parameters.get(AgentKeyConstant.AGENTS_KEY))) {
            List<Endpoint> endpoints = agentBusinessService.queryInternalAgents();
            String agents = endpoints.stream().map(Endpoint::getId).collect(Collectors.joining(";"));
            parameters.put(AgentKeyConstant.AGENTS_KEY, agents);
        }
    }

    @Override
    public boolean applicable(String type) {
        return StringUtils.equals(ResourceTypeEnum.OBJECT_STORAGE.getType(), type) || StringUtils.equals(
            ResourceSubTypeEnum.OBJECT_SET.getType(), type);
    }
}