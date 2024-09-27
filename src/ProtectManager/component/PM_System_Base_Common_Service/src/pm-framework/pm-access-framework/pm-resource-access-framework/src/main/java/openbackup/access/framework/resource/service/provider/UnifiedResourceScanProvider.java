package openbackup.access.framework.resource.service.provider;

import openbackup.access.framework.resource.service.ProtectedEnvironmentRetrievalsService;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.Application;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfig;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceScanProvider;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.UUIDGenerator;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.util.BeanTools;

import com.fasterxml.jackson.databind.JsonNode;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.Map;
import java.util.Optional;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * 功能描述: 通用资源扫描逻辑
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-06-08
 */
@Slf4j
@Component
public class UnifiedResourceScanProvider implements ResourceScanProvider {
    private static final int PAGE_SIZE = 100;

    private final PluginConfigManager pluginConfigManager;
    private final AgentUnifiedService agentService;
    private final ResourceService resourceService;
    private final ProtectedEnvironmentRetrievalsService envRetrievalsService;

    /**
     * 构造器注入
     *
     * @param pluginConfigManager pluginConfigManager
     * @param agentService agentService
     * @param resourceService resourceService
     * @param envRetrievalsService envRetrievalsService
     */
    public UnifiedResourceScanProvider(PluginConfigManager pluginConfigManager, AgentUnifiedService agentService,
            ResourceService resourceService, ProtectedEnvironmentRetrievalsService envRetrievalsService) {
        this.pluginConfigManager = pluginConfigManager;
        this.agentService = agentService;
        this.resourceService = resourceService;
        this.envRetrievalsService = envRetrievalsService;
    }

    @Override
    public boolean applicable(ProtectedEnvironment environment) {
        return false;
    }

    @Override
    public List<ProtectedResource> scan(ProtectedEnvironment environment) {
        List<ProtectedEnvironment> agents = getAllOnlineAgents(environment);
        long errorCode = CommonErrorCode.AGENT_NOT_EXIST;
        for (ProtectedResource agent : agents) {
            try {
                return scanResourceBySingleAgent(agent.getUuid(), environment);
            } catch (LegoCheckedException e) {
                // 单个agent扫描失败，记录失败日志，继续通过其他的agent扫描资源
                errorCode = e.getErrorCode();
            }
        }
        throw new LegoCheckedException(errorCode, "Resource scan failed.");
    }

    private List<ProtectedResource> scanResourceBySingleAgent(String agentId, ProtectedEnvironment env) {
        log.info("Scan resource start, env name: {}, agent id: {}", env.getName(), agentId);
        Endpoint endpoint = getAgentEndpoint(agentId);
        List<ProtectedResource> resources = doScanResources(env, endpoint, Collections.emptyList());
        log.info("Scan resource finish, env name: {}, resource num: {}, resources: {}.",
                env.getName(), resources.size(), buildResourceListPrintInfo(resources));
        return resources;
    }

    private List<ProtectedResource> doScanResources(ProtectedEnvironment env, Endpoint endpoint,
        List<Application> appList) {
        List<ProtectedResource> allChildResources = queryAllChildResources(env, endpoint, appList);
        List<ProtectedResource> validChildResources = filterResources(env, allChildResources);
        if (VerifyUtil.isEmpty(validChildResources)) {
            return Collections.emptyList();
        }
        fillResourceInfo(env, appList, validChildResources);
        List<ProtectedResource> result = new ArrayList<>(validChildResources);
        for (ProtectedResource resource : validChildResources) {
            List<Application> subAppList = new ArrayList<>(appList);
            subAppList.add(BeanTools.copy(resource, Application::new));
            List<ProtectedResource> resources = doScanResources(env, endpoint, subAppList);
            result.addAll(resources);
        }
        return result;
    }

    private List<ProtectedResource> queryAllChildResources(ProtectedEnvironment env, Endpoint endpoint,
        List<Application> appList) {
        int pageNo = 0;
        List<ProtectedResource> resources = new ArrayList<>();
        ListResourceV2Req request = new ListResourceV2Req();
        request.setPageSize(PAGE_SIZE);
        request.setAppEnv(BeanTools.copy(env, AppEnv::new));
        request.setApplications(appList);
        PageListResponse<ProtectedResource> response;
        do {
            pageNo++;
            request.setPageNo(pageNo);
            response = agentService.getDetailPageList(env.getSubType(), endpoint.getIp(), endpoint.getPort(), request);
            resources.addAll(response.getRecords());
        } while (pageNo * PAGE_SIZE < response.getTotalCount());
        log.info("Query all child resources finish, env name: {}, resource num: {}, resources:{}.",
                env.getName(), resources.size(), buildResourceListPrintInfo(resources));
        return resources;
    }

    private List<ProtectedEnvironment> getAllOnlineAgents(ProtectedEnvironment env) {
        Map<ProtectedResource, List<ProtectedEnvironment>> map = envRetrievalsService.collectConnectableResources(env);
        List<ProtectedEnvironment> agentEnv = map.values().stream().flatMap(List::stream)
                .filter(agent -> LinkStatusEnum.ONLINE.getStatus().toString()
                .equals(EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(agent)))
                .collect(Collectors.toList());
        log.info("Query online agent success, online agent size: {}.", agentEnv.size());
        return agentEnv;
    }

    private Endpoint getAgentEndpoint(String agentId) {
        Optional<ProtectedResource> optResource = resourceService.getResourceById(agentId);
        return optResource.filter(resource -> resource instanceof ProtectedEnvironment)
                .map(resource -> (ProtectedEnvironment) resource)
                .map(env -> new Endpoint(env.getUuid(), env.getEndpoint(), env.getPort()))
                .orElseThrow(() -> new LegoCheckedException(CommonErrorCode.AGENT_NOT_EXIST, "Agent is empty"));
    }

    private List<ProtectedResource> filterResources(ProtectedEnvironment env, List<ProtectedResource> resources) {
        PluginConfig config = pluginConfigManager.getPluginConfig(env.getSubType()).get();
        JsonNode children = config.getConfigMap().get("children");
        Set<String> typeSet = new HashSet<>(children.findValuesAsText("type"));
        Set<String> subTypeSet = new HashSet<>(children.findValuesAsText("subType"));
        List<ProtectedResource> validChildResources = resources.stream()
                .filter(resource -> typeSet.contains(resource.getType()))
                .filter(resource -> subTypeSet.contains(resource.getSubType()))
                .collect(Collectors.toList());
        log.info("Filter valid resource finish, env name: {}, resource num: {}, resources: {}.",
                env.getName(), validChildResources.size(), buildResourceListPrintInfo(resources));
        return validChildResources;
    }

    // 如果agent插件已填充相关信息，以agent插件的为准；若插件未填充，则PM填充
    private void fillResourceInfo(ProtectedEnvironment env, List<Application> apps, List<ProtectedResource> resources) {
        String parentId = apps.isEmpty() ? env.getUuid() : apps.get(apps.size() - 1).getUuid();
        String parentName = apps.isEmpty() ? env.getName() : apps.get(apps.size() - 1).getName();
        StringBuilder parentPath = new StringBuilder(env.getPath()).append(File.separator);
        apps.forEach(app -> parentPath.append(app.getName()).append(File.separator));
        for (ProtectedResource resource : resources) {
            resource.setUuid(Optional.ofNullable(resource.getUuid()).orElse(UUIDGenerator.getUUID()));
            resource.setParentUuid(Optional.ofNullable(resource.getParentUuid()).orElse(parentId));
            resource.setParentName(Optional.ofNullable(resource.getParentName()).orElse(parentName));
            resource.setPath(Optional.ofNullable(resource.getPath()).orElse(parentPath + resource.getName()));
            if (!VerifyUtil.isEmpty(env.getUserId()) && VerifyUtil.isEmpty(resource.getUserId())) {
                resource.setUserId(env.getUserId());
                resource.setAuthorizedUser(env.getAuthorizedUser());
            }
        }
    }

    private String buildResourceListPrintInfo(List<ProtectedResource> resources) {
        return resources.stream().map(this::buildSingleResourcePrintInfo).collect(Collectors.toList()).toString();
    }

    private String buildSingleResourcePrintInfo(ProtectedResource resource) {
        return new StringBuilder()
                .append("uuid: ").append(resource.getUuid()).append(", ")
                .append("name: ").append(resource.getName()).append(", ")
                .append("type: ").append(resource.getType()).append(", ")
                .append("subType: ").append(resource.getSubType()).append(", ")
                .append("parentName: ").append(resource.getParentName()).append(", ")
                .append("parentId: ").append(resource.getParentUuid()).append(", ")
                .append("extendInfo: ").append(resource.getExtendInfo())
                .toString();
    }
}