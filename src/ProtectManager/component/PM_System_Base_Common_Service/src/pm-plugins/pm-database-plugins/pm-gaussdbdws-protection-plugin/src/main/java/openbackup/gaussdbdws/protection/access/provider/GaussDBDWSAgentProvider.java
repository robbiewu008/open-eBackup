package openbackup.gaussdbdws.protection.access.provider;

import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.gaussdbdws.protection.access.service.GaussDBBaseService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;

/**
 * DWS的插件选择
 *
 * @author dwx1009286
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-07-25
 */
@Slf4j
@Component
@AllArgsConstructor
public class GaussDBDWSAgentProvider extends DataBaseAgentSelector {
    private final GaussDBBaseService gaussDBBaseService;

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        return gaussDBBaseService.supplyAgent(agentSelectParam);
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        String subType = agentSelectParam.getResource().getSubType();
        return ResourceSubTypeEnum.GAUSSDB_DWS_DATABASE.getType().equals(subType)
            || ResourceSubTypeEnum.GAUSSDB_DWS_SCHEMA.getType().equals(subType)
            || ResourceSubTypeEnum.GAUSSDB_DWS_TABLE.getType().equals(subType);
    }
}
