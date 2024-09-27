package openbackup.postgre.protection.access.provider.agent;

import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.postgre.protection.access.service.PostgreInstanceService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * Postgre资源查询agent主机的provider
 *
 * @author lWX776769
 * @version [DataBackup 1.5.0]
 * @since 2023/7/27
 */
@Component
public class PostgreAgentProvider extends DataBaseAgentSelector {
    private final PostgreInstanceService postgreInstanceService;

    public PostgreAgentProvider(PostgreInstanceService postgreInstanceService) {
        this.postgreInstanceService = postgreInstanceService;
    }

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        return postgreInstanceService.getAgentsByInstanceResource(agentSelectParam.getResource());
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        return Arrays.asList(ResourceSubTypeEnum.POSTGRE_INSTANCE.getType(),
            ResourceSubTypeEnum.POSTGRE_CLUSTER_INSTANCE.getType())
            .contains(agentSelectParam.getResource().getSubType());
    }
}
