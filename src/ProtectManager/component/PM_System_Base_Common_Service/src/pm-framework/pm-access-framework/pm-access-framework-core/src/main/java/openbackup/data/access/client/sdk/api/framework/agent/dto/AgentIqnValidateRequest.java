package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.Data;

/**
 * 客户端IQN验证请求体
 *
 * @author z00613137
 * @since 2023-06-21
 */
@Data
public class AgentIqnValidateRequest {
    /**
     * GUI填写的IQN
     */
    private String[] configValues;
}