package openbackup.opengauss.resources.access.provider;

import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.opengauss.resources.access.service.OpenGaussAgentService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * OpenGauss的插件选择
 *
 * @author dwx1009286
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023/7/25
 */
@Slf4j
@Component
@AllArgsConstructor
public class OpenGaussAgentProvider extends DataBaseAgentSelector {
    private final OpenGaussAgentService openGaussAgentService;

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        return openGaussAgentService.getAgentEndpoint(agentSelectParam.getResource().getEnvironment().getUuid());
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        String subType = agentSelectParam.getResource().getSubType();
        return Arrays
            .asList(ResourceSubTypeEnum.OPENGAUSS_DATABASE.getType(), ResourceSubTypeEnum.OPENGAUSS_INSTANCE.getType())
            .contains(subType);
    }
}
