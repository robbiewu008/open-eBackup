package openbackup.mongodb.protection.access.provider.resource;

import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * MongoDB的插件选择
 *
 * @author dwx1009286
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-07-25
 */
@Slf4j
@Component
public class MongoDBAgentProvider extends DataBaseAgentSelector {
    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        List<Endpoint> agents = super.getSelectedAgents(agentSelectParam);
        Set<String> uuid = new HashSet<>();
        return agents.stream().filter(endpoint -> uuid.add(endpoint.getId())).collect(Collectors.toList());
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        String subType = agentSelectParam.getResource().getSubType();
        return Arrays.asList(ResourceSubTypeEnum.MONGODB_CLUSTER.getType(),
            ResourceSubTypeEnum.MONGODB_SINGLE.getType()).contains(subType);
    }
}
