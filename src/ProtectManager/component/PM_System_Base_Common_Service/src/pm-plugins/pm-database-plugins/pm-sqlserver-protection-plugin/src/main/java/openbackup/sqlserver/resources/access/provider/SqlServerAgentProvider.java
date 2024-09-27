package openbackup.sqlserver.resources.access.provider;

import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.sqlserver.protection.service.SqlServerBaseService;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.AllArgsConstructor;
import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.Arrays;
import java.util.List;

/**
 * SqlServer的agent配置
 *
 * @author dwx1009286
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023-07-25
 */
@Slf4j
@Component
@AllArgsConstructor
public class SqlServerAgentProvider extends DataBaseAgentSelector {
    private final SqlServerBaseService sqlServerBaseService;

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        return sqlServerBaseService.convertNodeListToAgents(agentSelectParam.getResource().getUuid());
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        String subType = agentSelectParam.getResource().getSubType();
        return Arrays.asList(ResourceSubTypeEnum.SQL_SERVER_INSTANCE.getType(),
            ResourceSubTypeEnum.SQL_SERVER_DATABASE.getType(), ResourceSubTypeEnum.SQL_SERVER_ALWAYS_ON.getType(),
            ResourceSubTypeEnum.SQL_SERVER_CLUSTER_INSTANCE.getType()).contains(subType);
    }
}
