package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.Builder;
import lombok.Data;

/**
 * The AgentLogLevelInfo
 *
 * @author w00504341
 * @since 2023-01-19
 */
@Builder
@Data
public class AgentLogLevelInfo {
    /**
     * 日志级别
     */
    private int level;
}
