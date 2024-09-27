package openbackup.data.access.client.sdk.api.framework.agent.dto;

import lombok.Getter;
import lombok.Setter;

/**
 * 更新代理资源类型请求
 *
 * @author c30035089
 * @since 2023-08-11
 */
@Getter
@Setter
public class UpdateAgentPluginTypeReq {
    /**
     * 下载链接
     */
    private String downloadLink;

    /**
     * agent id
     */
    private String agentId;

    /**
     * agent name
     */
    private String agentName;

    /**
     * 任务id
     */
    private String jobId;

    /**
     * 密钥,已加密
     */
    private String certSecretKey;

    /**
     * 安装包大小(kb)
     */
    private long newPackageSize;
}
