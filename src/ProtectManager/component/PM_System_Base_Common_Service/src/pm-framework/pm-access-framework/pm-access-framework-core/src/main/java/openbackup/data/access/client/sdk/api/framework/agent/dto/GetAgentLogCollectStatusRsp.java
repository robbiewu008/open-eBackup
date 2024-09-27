package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.Data;

/**
 * The LogCollectStatusResponse
 *
 * @author w00504341
 * @since 2023-01-19
 */
@Data
public class GetAgentLogCollectStatusRsp {
    /**
     * 状态
     */
    private String status;
}
