package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.Data;

/**
 * Agent返回的支持的应用数据模型
 *
 * @author w00616953
 * @since 2021-12-03
 */
@Data
public class SupportApplicationDto {
    /**
     * 应用类型
     */
    private String application;

    /**
     * 支持应用的最小版本，可能没有限制
     */
    private String minVersion;

    /**
     * 支持应用的最大版本，可能没有限制
     */
    private String maxVersion;
}
