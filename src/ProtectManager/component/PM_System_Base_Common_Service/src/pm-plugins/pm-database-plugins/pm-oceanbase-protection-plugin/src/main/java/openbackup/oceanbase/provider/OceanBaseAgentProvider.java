package openbackup.oceanbase.provider;

import openbackup.data.access.framework.agent.DataBaseAgentSelector;
import openbackup.data.protection.access.provider.sdk.agent.AgentSelectParam;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.oceanbase.common.util.OceanBaseUtils;
import openbackup.oceanbase.service.OceanBaseService;
import openbackup.system.base.sdk.job.model.JobTypeEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;

import lombok.extern.slf4j.Slf4j;

import org.springframework.stereotype.Component;

import java.util.List;
import java.util.Objects;

/**
 * 功能描述
 *
 * @author c00826511
 * @since 2023-07-28
 */
@Slf4j
@Component
public class OceanBaseAgentProvider extends DataBaseAgentSelector {
    private final OceanBaseService oceanBaseService;

    public OceanBaseAgentProvider(OceanBaseService oceanBaseService) {
        this.oceanBaseService = oceanBaseService;
    }

    @Override
    public List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam) {
        if (Objects.equals(agentSelectParam.getJobType(), JobTypeEnum.BACKUP.getValue())) {
            ProtectedResource resource = agentSelectParam.getResource();
            ProtectedEnvironment clusterEnv = oceanBaseService.getEnvironmentById(resource.getRootUuid());
            return OceanBaseUtils.supplyAgent(clusterEnv, oceanBaseService);
        } else if (Objects.equals(agentSelectParam.getJobType(), JobTypeEnum.RESTORE.getValue())) {
            ProtectedResource resource = agentSelectParam.getResource();
            ProtectedEnvironment clusterEnv = oceanBaseService.getEnvironmentById(resource.getRootUuid());
            return OceanBaseUtils.supplyAgentWithSingleClient(clusterEnv, oceanBaseService);
        } else {
            return super.getSelectedAgents(agentSelectParam);
        }
    }

    @Override
    public boolean applicable(AgentSelectParam agentSelectParam) {
        return ResourceSubTypeEnum.OCEAN_BASE_CLUSTER.getType().equals(agentSelectParam.getResource().getSubType())
            || ResourceSubTypeEnum.OCEAN_BASE_TENANT.getType().equals(agentSelectParam.getResource().getSubType());
    }
}
