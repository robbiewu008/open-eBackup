package openbackup.db2.protection.access.provider.agent;

import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.db2.protection.access.service.Db2Service;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * DB2资源查询agent主机的provider
 *
 * @author lWX776769
 * @version [DataBackup 1.5.0]
 * @since 2023/7/27
 */
@Component
public class Db2AgentProvider extends DataBaseAgentSelector {
    private final Db2Service db2Service;

    public Db2AgentProvider(Db2Service db2Service) {
        this.db2Service = db2Service;
    }

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        return db2Service.getAgentsByInstanceResource(agentSelectParam.getResource());
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        return Arrays.asList(ResourceSubTypeEnum.DB2_DATABASE.getType(), ResourceSubTypeEnum.DB2_TABLESPACE.getType())
            .contains(agentSelectParam.getResource().getSubType());
    }
}
