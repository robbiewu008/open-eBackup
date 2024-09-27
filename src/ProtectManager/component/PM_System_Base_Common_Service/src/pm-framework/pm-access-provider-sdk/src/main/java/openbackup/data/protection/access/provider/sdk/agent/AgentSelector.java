package openbackup.data.protection.access.provider.sdk.agent;

import openbackup.data.protection.access.provider.sdk.base.DataProtectionProvider;
import openbackup.data.protection.access.provider.sdk.base.Endpoint;

import java.util.List;

/**
 * 根据资源选择selector
 *
 * @author z30027603
 * @version [OceanProtect DataBackup 1.5.0]
 * @since 2023/7/22
 */
public interface AgentSelector extends DataProtectionProvider<AgentSelectParam> {
    /**
     * 执行备份、扫描任务时根据资源选择agent
     *
     * 父方法{@link DataProtectionProvider#applicable(Object)} 入参为AgentSelectParam
     *
     * @param agentSelectParam agentSelectParam
     * @return Endpoint
     */
    List<Endpoint> getSelectedAgents(AgentSelectParam agentSelectParam);
}
