package openbackup.informix.protection.access.provider.resource;

import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.informix.protection.access.service.InformixService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;

/**
 * Informix的agent配置
 *
 * @author dwx1009286
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-07-25
 */
@Slf4j
@Component
@AllArgsConstructor
public class InformixAgentProvider extends DataBaseAgentSelector {
    private final InformixService informixService;

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        return informixService.getAgentsByInstanceResource(agentSelectParam.getResource());
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        String subType = agentSelectParam.getResource().getSubType();
        return ResourceSubTypeEnum.INFORMIX_SINGLE_INSTANCE.getType().equals(subType)
            || ResourceSubTypeEnum.INFORMIX_CLUSTER_INSTANCE.getType().equals(subType);
    }
}
