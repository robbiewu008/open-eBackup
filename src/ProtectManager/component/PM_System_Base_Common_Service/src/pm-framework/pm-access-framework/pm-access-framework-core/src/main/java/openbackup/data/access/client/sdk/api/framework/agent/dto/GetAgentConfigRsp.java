package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.Data;

/**
 * The LogLevelInfo
 *
 * @author w00504341
 * @since 2023-01-19
 */
@Data
public class GetAgentConfigRsp {
    /**
     * 备份代理日志信息
     */
    private AgentLogLevelInfo log;
}
