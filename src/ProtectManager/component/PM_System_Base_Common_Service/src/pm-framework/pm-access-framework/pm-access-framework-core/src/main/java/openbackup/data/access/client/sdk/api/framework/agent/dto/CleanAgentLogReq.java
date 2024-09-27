package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.Builder;
import lombok.Data;

/**
 * 功能描述
 *
 * @author w00504341
 * @since 2023-02-28
 */
@Builder
@Data
public class CleanAgentLogReq {
    /**
     * agent记录ID
     */
    private String id;
}
