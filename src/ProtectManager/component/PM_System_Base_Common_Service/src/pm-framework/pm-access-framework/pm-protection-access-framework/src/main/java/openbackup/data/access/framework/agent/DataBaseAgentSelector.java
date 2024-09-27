package openbackup.data.access.framework.agent;

import openbackup.data.access.framework.core.manager.ProviderManager;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelector;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResourceChecker;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;

import lombok.extern.slf4j.Slf4j;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.stereotype.Component;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 根据资源选择selector
 *
 * @author z30027603
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023/7/22
 */
@Component
@Slf4j
public class DataBaseAgentSelector implements AgentSelector {
    @Autowired
    private ProviderManager providerManager;

    @Autowired
    private ResourceService resourceService;

    @Autowired
    @Qualifier("unifiedResourceConnectionChecker")
    private ProtectedResourceChecker protectedResourceChecker;

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        Optional<ProtectedResource> resOptional =
                resourceService.getResourceById(agentSelectParam.getResource().getUuid());
        if (!resOptional.isPresent()) {
            return Collections.emptyList();
        }
        ProtectedResourceChecker checker =
                providerManager.findProviderOrDefault(
                        ProtectedResourceChecker.class, resOptional.get(), this.protectedResourceChecker);
        Map<ProtectedResource, List<ProtectedEnvironment>> protectedResourceMap =
                checker.collectConnectableResources(resOptional.get());
        List<Endpoint> endpointList = new ArrayList<>();
        protectedResourceMap.forEach(
                ((protectedResource, protectedEnvironments) -> {
                    for (ProtectedEnvironment protectedEnvironment : protectedEnvironments) {
                        Endpoint endpoint = new Endpoint();
                        endpoint.setId(protectedEnvironment.getUuid());
                        endpoint.setIp(protectedEnvironment.getEndpoint());
                        endpoint.setPort(protectedEnvironment.getPort());
                        endpointList.add(endpoint);
                    }
                }));
        return endpointList;
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        return false;
    }
}
