package openbackup.access.framework.resource.service.provider;

import openbackup.data.access.client.sdk.api.framework.agent.dto.AgentBaseDto;
import openbackup.data.access.framework.core.agent.AgentUnifiedService;
import openbackup.data.access.framework.core.common.util.EnvironmentLinkStatusHelper;
import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfig;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigConstants;
import openbackup.data.protection.access.provider.sdk.plugin.PluginConfigManager;
import openbackup.data.protection.access.provider.sdk.resource.ActionResult;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentCheckProvider;
import openbackup.data.protection.access.provider.sdk.resource.EnvironmentParamProvider;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.utils.VerifyUtil;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;

import com.fasterxml.jackson.databind.JsonNode;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.stereotype.Component;

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 功能描述: UnifiedEnvironmentCheckProvider
 *
 * @author l00570077
 * @version [OceanProtect X8000 1.2.1]
 * @since 2022-05-23
 */
@Slf4j
@Component
public class UnifiedEnvironmentCheckProvider implements EnvironmentCheckProvider {
    private static final int DEFAULT_ENV_SPEC = 8;

    private static final String AGENTS = "agents";
    private static final String SUCCESS = "0";

    private final ResourceService resourceService;
    private final AgentUnifiedService agentService;
    private final PluginConfigManager pluginConfigManager;
    private final ProviderManager providerManager;
    private final UnifiedEnvironmentParamProvider unifiedParamProvider;

    /**
     * 构造器注入
     *
     * @param resourceService resourceService
     * @param agentService agentService
     * @param pluginConfigManager pluginConfigManager
     * @param providerManager providerManager
     * @param unifiedParamProvider unifiedParamProvider
     */
    public UnifiedEnvironmentCheckProvider(ResourceService resourceService, AgentUnifiedService agentService,
        PluginConfigManager pluginConfigManager, ProviderManager providerManager,
        @Qualifier("unifiedEnvironmentParamProvider")
        UnifiedEnvironmentParamProvider unifiedParamProvider) {
        this.resourceService = resourceService;
        this.agentService = agentService;
        this.pluginConfigManager = pluginConfigManager;
        this.providerManager = providerManager;
        this.unifiedParamProvider = unifiedParamProvider;
    }

    @Override
    public boolean applicable(ProtectedEnvironment environment) {
        return false;
    }

    @Override
    public void check(ProtectedEnvironment environment) {
        log.info("Environment check start, uuid: {}, name: {}", environment.getUuid(), environment.getName());

        // 校验已注册的相同subtype的环境数量
        checkRegisteredEnvironmentCount(environment);

        // 校验同一受保护环境是否被重复注册
        checkEnvironmentRepeat(environment);

        // 校验参数格式，并填充需要从其他模块获取的参数
        checkAndPrepareParam(environment);

        // 校验连通性
        checkConnectivity(environment);

        // 某些特性需要从agent中查询信息并回填到environment中
        updateEnvironment(environment);

        // 更新环境状态
        updateStatus(environment);

        log.info("Environment check finish, uuid: {}, name: {}", environment.getUuid(), environment.getName());
    }

    private void checkAndPrepareParam(ProtectedEnvironment env) {
        EnvironmentParamProvider paramProvider = providerManager.findProviderOrDefault(EnvironmentParamProvider.class,
                env, unifiedParamProvider);
        paramProvider.checkAndPrepareParam(env);
    }

    private void checkEnvironmentRepeat(ProtectedEnvironment env) {
        EnvironmentParamProvider paramProvider = providerManager.findProviderOrDefault(EnvironmentParamProvider.class,
                env, unifiedParamProvider);
        paramProvider.checkEnvironmentRepeat(env);
    }

    private void checkRegisteredEnvironmentCount(ProtectedEnvironment env) {
        if (!VerifyUtil.isEmpty(env.getUuid())) {
            log.info("Update env(uuid: {}, name: {}), no need check count.", env.getUuid(), env.getName());
            return;
        }
        Map<String, Object> filter = new HashMap<>();
        filter.put("type", env.getType());
        filter.put("subType", env.getSubType());
        int existedCount = resourceService.query(0, 1, filter).getTotalCount();
        int envSpec = parseEnvSpecFromConfig(env);
        log.info("Env subType: {}, spec: {}, existCount: {}.", env.getSubType(), envSpec, existedCount);
        if (existedCount >= envSpec) {
            throw new LegoCheckedException(CommonErrorCode.ENV_COUNT_OVER_LIMIT,
                    new String[]{String.valueOf(envSpec)},
                    "Environment count over limit {0}");
        }
    }

    private void checkConnectivity(ProtectedEnvironment env) {
        List<ProtectedResource> agents = env.getDependencies().get(AGENTS);
        log.info("Begin to check connectivity by agent, agent count: {}", agents.size());
        for (ProtectedResource agent : agents) {
            AgentBaseDto response = checkConnectivityThroughSingleAgent(agent.getUuid(), env);
            if (!VerifyUtil.isEmpty(response) && SUCCESS.equals(response.getErrorCode())) {
                continue;
            }
            log.error("Environment check failed, agent is not available agent, endpoint: {}.", agent.getEndpoint());
            if (VerifyUtil.isEmpty(response) || VerifyUtil.isEmpty(response.getErrorMessage())) {
                throw new LegoCheckedException(CommonErrorCode.AGENT_NETWORK_ERROR, "Agent is empty");
            }
            ActionResult result = JsonUtil.read(response.getErrorMessage(), ActionResult.class);
            if (!VerifyUtil.isEmpty(result.getDetailParams())) {
                throw new LegoCheckedException(Long.parseLong(result.getBodyErr()),
                    result.getDetailParams().toArray(new String[0]), result.getMessage());
            } else {
                throw new LegoCheckedException(Long.parseLong(result.getBodyErr()), result.getMessage());
            }
        }
    }

    private AgentBaseDto checkConnectivityThroughSingleAgent(String agentId, ProtectedEnvironment env) {
        Optional<ProtectedEnvironment> optAgentEnv = resourceService.getResourceById(agentId)
                .filter(resource -> resource instanceof ProtectedEnvironment)
                .map(resource -> (ProtectedEnvironment) resource);
        if (!optAgentEnv.isPresent()
                || !EnvironmentLinkStatusHelper.getLinkStatusAdaptMultiCluster(optAgentEnv.get())
                .equals(LinkStatusEnum.ONLINE.getStatus().toString())) {
            log.error("Agent does not exist or status is not online, agentId: {}", agentId);
            AgentBaseDto response = new AgentBaseDto();
            response.setErrorCode(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            ActionResult actionResult = new ActionResult();
            actionResult.setCode(CommonErrorCode.AGENT_NETWORK_ERROR);
            actionResult.setBodyErr(String.valueOf(CommonErrorCode.AGENT_NETWORK_ERROR));
            response.setErrorMessage(JsonUtil.json(actionResult));
            return response;
        }
        return agentService.checkApplication(env, optAgentEnv.get());
    }

    private void updateEnvironment(ProtectedEnvironment env) {
        EnvironmentParamProvider paramProvider = providerManager.findProviderOrDefault(EnvironmentParamProvider.class,
                env, unifiedParamProvider);
        paramProvider.updateEnvironment(env);
    }

    private void updateStatus(ProtectedEnvironment env) {
        env.setPath(Optional.ofNullable(env.getPath()).orElse(env.getEndpoint()));
        env.setLinkStatus(LinkStatusEnum.ONLINE.getStatus().toString());
    }

    private int parseEnvSpecFromConfig(ProtectedEnvironment environment) {
        Optional<PluginConfig> optConfig = pluginConfigManager.getPluginConfig(environment.getSubType());
        return optConfig.map(PluginConfig::getConfigMap)
                .map(configMap -> configMap.get(PluginConfigConstants.FUNCTION))
                .map(functionNode -> functionNode.findValue(PluginConfigConstants.ENVIRONMENTS))
                .map(envNode -> envNode.findValue(PluginConfigConstants.SPECIFICATION))
                .map(JsonNode::asInt)
                .orElse(DEFAULT_ENV_SPEC);
    }
}