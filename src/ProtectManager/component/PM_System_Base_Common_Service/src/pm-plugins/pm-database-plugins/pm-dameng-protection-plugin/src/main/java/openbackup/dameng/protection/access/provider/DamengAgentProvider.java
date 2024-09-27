package openbackup.dameng.protection.access.provider;

import openbackup.dameng.protection.access.service.DamengService;
import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;

/**
 * Dameng的插件选择
 *
 * @author dwx1009286
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023/7/25
 */
@Slf4j
@Component
@AllArgsConstructor
public class DamengAgentProvider extends DataBaseAgentSelector {
    private final DamengService damengService;

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        if (ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType().equals(agentSelectParam.getResource().getSubType())) {
            return super.getSelectedAgents(agentSelectParam);
        }
        return damengService.getEndpointList(agentSelectParam.getResource().getUuid());
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        String subType = agentSelectParam.getResource().getSubType();
        return ResourceSubTypeEnum.DAMENG_SINGLE_NODE.getType().equals(subType)
            || ResourceSubTypeEnum.DAMENG_CLUSTER.getType().equals(subType);
    }
}
