/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2022. All rights reserved.
 */

package openbackup.system.base.sdk.agent.model;

import lombok.Data;

import org.apache.commons.lang3.StringUtils;

/**
 * agent更新请求体
 *
 * @author w00426202
 * @since 2022-11-30
 */
@Data
public class AgentUpdateRequest {
    /**
     * host ip
     */
    private String ip;

    /**
     * host port
     */
    private String port;

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
     * 是否需要代理
     */
    private boolean canUseProxy;

    /**
     * 密钥,已加密
     */
    private String certSecretKey;

    /**
     * 任务id
     */
    private String jobId;

    /**
     * 安装包大小(kb)
     */
    private long newPackageSize;

    @Override
    public String toString() {
        final StringBuffer sb = new StringBuffer("AgentUpdateRequest{");
        sb.append("ip='").append(ip).append('\'');
        sb.append(", port='").append(port).append('\'');
        sb.append(", downloadLink'").append(downloadLink).append('\'');
        sb.append(", agentId='").append(agentId).append('\'');
        sb.append(", agentName='").append(agentName).append('\'');
        sb.append(", newPackageSize='").append(newPackageSize).append('\'');
        sb.append(", canUseProxy=").append(canUseProxy);
        sb.append(", certSecretKey exist ? =").append(StringUtils.isNotBlank(certSecretKey)).append('\'');
        sb.append('}');
        return sb.toString();
    }
}
